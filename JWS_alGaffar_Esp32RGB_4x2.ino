/*************************************************************************************
 * FIRMWARE JWS LED MATRIX P5 RGB HUB-75 (ESP32 EDITION)
 * Resolusi Layar: 256 x 64 Piksel (4 Kolom x 2 Baris Panel P5)
 * Hardware PCB: ElektronMart V2.1 (Menggunakan Buffer 74HC245)
 * Fitur:
 * - Driver I2S DMA High Performance via ESP32-HUB75-MatrixPanel-I2S-DMA
 * - Hisab Jadwal Sholat Astronomis Akurat & Kalender Hijriah
 * - RTC DS3231 via I2C (SDA: 32, SCL: 33)
 * - DFPlayer Mini via Hardware Serial2 (RX2: 13, TX2: 14) untuk Tartil/Tarhim Otomatis
 * - Buzzer Aktif pada GPIO 23
 * - Kontrol Ganda:
 *   1. Bluetooth Classic SPP ("JWS-RGB-P5") untuk aplikasi Android alGaffar
 *   2. WiFi Access Point ("JWS-RGB-P5") & Web Dashboard di http://192.168.4.1
 *************************************************************************************/

#include <Arduino.h>
#include <Wire.h>
#include <EEPROM.h>
#include <DS3231.h>
#include "Config_Pins.h"
#include "Config_Params.h"
#include "Display_HUB75.h"

// Objek Hardware Display HUB-75 DMA & Canvas Virtual
MatrixPanel_I2S_DMA *dma_display = nullptr;
VirtualMatrixPanel_T<VIRTUAL_CHAIN_TYPE> *matrix = nullptr;

// Objek RTC DS3231 & DFPlayer Mini (Hardware Serial2)
RTClib RTC;
DS3231 Clock;
HardwareSerial SerialMP3(2);

#if ENABLE_BLUETOOTH
#include <BluetoothSerial.h>
BluetoothSerial SerialBT;
#endif

#if ENABLE_WIFI
#include <WebServer.h>
WebServer server(80);
#endif

// -----------------------------------------------------------------------------------
// Variabel Global
// -----------------------------------------------------------------------------------
struct_param Prm;
hijir_date nowH;
struct_mp3_prm Mp3Prm;

DateTime now;
float floatnow = 0;
uint8_t daynow = 0;
int8_t SholatNow = -1;
boolean jumat = false;
boolean azzan = false;
uint8_t reset_x = 0;
boolean rtcTimeValid = false;
boolean displayReady = false;
uint32_t lastRtcReadMs = 0;

float sholatT[8] = { 0, 0, 0, 0, 0, 0, 0, 0 };
uint8_t Iqomah[8] = { 0, 0, 0, 0, 0, 0, 0, 0 };

char CH_Prm[155];
int RunSel = 1;
int RunFinish = 0;

// Forward declarations
void dfStop();
void dfSetVolume(uint8_t vol);
void dfPlayManual(uint8_t folder, uint8_t track);
void updateTime();
void update_All_data();
void check_azzan();
void check_mp3();
void serviceBluetooth();
void initWiFiPortal();
void serviceWiFiPortal();
void GetPrm();
void mp3_init();
void sholatCal();
hijir_date toHijri(uint16_t Y, uint8_t M, uint8_t D, uint8_t cor);
void setRunSel(int val);
void setJumat(bool val);
void Buzzer(uint8_t state);
void BuzzerBeep(uint16_t durationMs);
void dwMrq(const char* msg, int speed, int drawAdd);
void drawOnAzzan(int DrawAdd);
void drawAzzan(int DrawAdd);
void drawIqomah(int DrawAdd);
void blinkBlock(int DrawAdd);
char* drawWelcome();
char* drawDateH();
char* drawDateM();
char* drawInfo(uint16_t addr);

// -----------------------------------------------------------------------------------
// Inisialisasi Hardware Display HUB75 & Virtual Canvas
// -----------------------------------------------------------------------------------
void initDisplay() {
  HUB75_I2S_CFG::i2s_pins _pins = {
    EM_R1_PIN, EM_G1_PIN, EM_B1_PIN,
    EM_R2_PIN, EM_G2_PIN, EM_B2_PIN,
    EM_A_PIN,  EM_B_PIN,  EM_C_PIN,  EM_D_PIN, EM_E_PIN,
    EM_LAT_PIN, EM_OE_PIN, EM_CLK_PIN
  };

  HUB75_I2S_CFG mxconfig(
    PANEL_RES_X,     // Lebar modul (64)
    PANEL_RES_Y,     // Tinggi modul (32)
    PANEL_CHAIN_LEN, // Total panel (8)
    _pins
  );

  mxconfig.clkphase = false;
  mxconfig.i2sspeed = HUB75_I2S_CFG::HZ_10M;
  mxconfig.setPixelColorDepthBits(5); // 5-bit color depth menghemat ~80KB DMA RAM untuk kestabilan WiFi & BT

  dma_display = new MatrixPanel_I2S_DMA(mxconfig);
  dma_display->begin();

  // Inisialisasi Virtual Canvas untuk menyusun 4x2 panel menjadi 256x64 piksel
  matrix = new VirtualMatrixPanel_T<VIRTUAL_CHAIN_TYPE>(
    VDISP_NUM_ROWS,
    VDISP_NUM_COLS,
    PANEL_RES_X,
    PANEL_RES_Y
  );
  matrix->setDisplay(*dma_display);
  matrix->clearScreen();

  displayReady = true;
  setMatrixBrightness(Prm.BL);
}

void setMatrixBrightness(uint8_t bright) {
  if (!dma_display) return;
  // Prm.BL bernilai 0 - 255 dari aplikasi alGaffar/Web
  dma_display->setBrightness8(bright);
}

void clearMatrix() {
  if (matrix) matrix->clearScreen();
}

// -----------------------------------------------------------------------------------
// Helper EEPROM & State Machine
// -----------------------------------------------------------------------------------
void eeprom_update_byte(int address, uint8_t val) {
  if (EEPROM.read(address) != val) {
    EEPROM.write(address, val);
    EEPROM.commit();
  }
}

void setRunSel(int val) {
  if (RunSel == val) return;
  RunSel = val;
  if (val == 1 || (val >= 100 && val <= 104)) {
    eeprom_update_byte(ADDR_RUNSEL, (uint8_t)val);
  }
}

void setJumat(bool val) {
  if (jumat == val) return;
  jumat = val;
  eeprom_update_byte(ADDR_JUMAT, val ? 1 : 0);
}

// -----------------------------------------------------------------------------------
// Update Jam & Data Jadwal Sholat
// -----------------------------------------------------------------------------------
void updateTime() {
  const uint32_t currentMs = millis();
  if (rtcTimeValid && (uint32_t)(currentMs - lastRtcReadMs) < 1000UL) {
    return;
  }

  now = RTC.now();
  floatnow = (float)now.hour() + (float)now.minute() / 60.0f + (float)now.second() / 3600.0f;
  daynow = ((now.dayOfTheWeek() + 6) % 7) + 1; // 1=Senin..7=Ahad
  lastRtcReadMs = currentMs;
  rtcTimeValid = true;
}

void Timer_Minute(int repeat_time) {
  static uint32_t lsRn = 0;
  uint32_t Tmr = millis();
  if ((Tmr - lsRn) > ((uint32_t)repeat_time * 60000UL)) {
    lsRn = Tmr;
    update_All_data();
  }
}

void update_All_data() {
  uint8_t date_cor = 0;
  updateTime();
  sholatCal();
  if (floatnow > sholatT[6]) {
    date_cor = 1;
  }
  nowH = toHijri(now.year(), now.month(), now.day(), date_cor);

  if (displayReady) {
    // Mode hemat energi malam hari (21:00 s.d. 03:30)
    if ((floatnow > 21.0f) || (floatnow < 3.5f)) {
      setMatrixBrightness(10);
    } else {
      setMatrixBrightness(Prm.BL);
    }
  }
}

// -----------------------------------------------------------------------------------
// Deteksi Waktu Masuk Sholat & Adzan
// -----------------------------------------------------------------------------------
void check_azzan() {
  static uint8_t lastAzzanDay = 0;
  static int8_t  lastAzzanPrayer = -1;
  SholatNow = -1;

  uint16_t currentMinute = (uint16_t)now.hour() * 60U + (uint16_t)now.minute();

  for (uint8_t i = 0; i < 8; i++) {
    if (i == 0 || i == 2 || i == 3) continue; // Skip Imsak, Terbit, Dhuha

    uint16_t prayerMinute = (uint16_t)ceil((sholatT[i] * 60.0f) - 0.0001f);

    if (currentMinute >= prayerMinute) {
      SholatNow = i;
    }

    if (!azzan && (daynow != lastAzzanDay || i != lastAzzanPrayer) &&
        currentMinute >= prayerMinute && currentMinute < prayerMinute + 5U) {
      lastAzzanDay = daynow;
      lastAzzanPrayer = i;
      setJumat(daynow == 5 && i == 4 && Prm.MT == 1); // Sholat Jumat di Masjid
      SholatNow = i;
      azzan = true;
      dfStop();

      setRunSel(99);
      break;
    }
  }
}

uint8_t commMode = DEFAULT_COMM_MODE;
void setCommMode(uint8_t newMode);
RTC_DATA_ATTR static uint32_t rtc_reset_marker = 0;

void setCommMode(uint8_t newMode) {
  if (newMode > 1) return;
  EEPROM.write(ADDR_COMM_MODE, newMode);
  EEPROM.commit();
  Serial.printf("[SWITCH] Beralih ke mode: %s. Me-restart sistem...\n",
                (newMode == COMM_MODE_WIFI) ? "WIFI" : "BLUETOOTH");
  BuzzerBeep(250);
  delay(300);
  ESP.restart();
}

// ===================================================================================
// === SETUP =========================================================================
// ===================================================================================
void setup() {
  Serial.begin(115200);
  delay(100);
  Serial.println("\n[BOOT] JWS P5 RGB 256x64 (ElektronMart V2) Memulai...");

  // 1. Inisialisasi Pin Buzzer
  pinMode(BUZZ_PIN, OUTPUT);
  digitalWrite(BUZZ_PIN, LOW);

  // 4. Inisialisasi EEPROM Flash (1024 bytes)
  EEPROM.begin(1024);
  Serial.println("[EEPROM] Flash EEPROM Siap");

  // Baca Mode Komunikasi dari EEPROM (0=WiFi, 1=Bluetooth, default: Bluetooth)
  commMode = EEPROM.read(ADDR_COMM_MODE);
  if (commMode > 1) {
    commMode = DEFAULT_COMM_MODE;
    EEPROM.write(ADDR_COMM_MODE, DEFAULT_COMM_MODE);
    EEPROM.commit();
  }

  // Deteksi Saklar Fisik Double-Reset (< 4 detik setelah boot sebelumnya):
  if (rtc_reset_marker == 0xDEADBEEF) {
    rtc_reset_marker = 0;
    // Balikkan mode: WiFi <-> Bluetooth
    commMode = (commMode == COMM_MODE_WIFI) ? COMM_MODE_BT : COMM_MODE_WIFI;
    EEPROM.write(ADDR_COMM_MODE, commMode);
    EEPROM.commit();

    // Bunyikan buzzer 3x konfirmasi saklar aktif
    for (int b = 0; b < 3; b++) {
      digitalWrite(BUZZ_PIN, HIGH); delay(120);
      digitalWrite(BUZZ_PIN, LOW); delay(100);
    }
    Serial.printf("[SWITCH] SAKLAR RESET CEPAT TERDETEKSI! Mode dibalik menjadi: %s\n",
                  (commMode == COMM_MODE_WIFI) ? "WIFI" : "BLUETOOTH");
  } else {
    rtc_reset_marker = 0xDEADBEEF;
  }

  // 2. Inisialisasi I2C (RTC DS3231) pada Pin ElektronMart (SDA: 32, SCL: 33)
  Wire.begin(RTC_SDA, RTC_SCL);
  Wire.setTimeOut(100);
  Serial.println("[RTC] I2C RTC DS3231 Siap (SDA: 32, SCL: 33)");

  // 3. Inisialisasi Hardware Serial2 untuk DFPlayer Mini (RX2: 13, TX2: 14)
  SerialMP3.begin(9600, SERIAL_8N1, MP3_RX, MP3_TX);
  Serial.println("[MP3] Hardware Serial2 DFPlayer Siap (RX: 13, TX: 14)");

  // Inisialisasi parameter sebelum beep
  GetPrm();
  updateTime();
  mp3_init();

  // 5. Beep selamat datang
  if (rtc_reset_marker == 0xDEADBEEF) {
    BuzzerBeep(150);
  }

  uint8_t lastSel = EEPROM.read(ADDR_RUNSEL);
  RunSel = (lastSel >= 100 && lastSel <= 104) ? lastSel : 1;
  if (RunSel >= 100) {
    jumat = (EEPROM.read(ADDR_JUMAT) == 1);
  }

  // 6. Inisialisasi Hardware Display P5 RGB HUB-75
  Serial.printf("[SYSTEM] Free Heap sebelum Display: %d bytes\n", ESP.getFreeHeap());
  initDisplay();
  Serial.printf("[SYSTEM] Free Heap sesudah Display: %d bytes\n", ESP.getFreeHeap());
  update_All_data();

  // Tampilkan Splash Screen Mode Aktif pada Layar Matrix (1.5 detik)
  if (matrix) {
    matrix->clearScreen();
    matrix->setTextSize(1);
    matrix->setTextColor(RGB_GOLD);
    matrix->setCursor(4, 6);
    matrix->print("JWS P5 RGB (256x64)");

    if (commMode == COMM_MODE_WIFI) {
      matrix->setTextColor(RGB_CYAN);
      matrix->setCursor(4, 22);
      matrix->print("MODE: WIFI WEB DASHBOARD");
      matrix->setTextColor(RGB_LIME);
      matrix->setCursor(4, 36);
      matrix->print("SSID: JWS-RGB-P5 (192.168.4.1)");
    } else {
      matrix->setTextColor(RGB_CYAN);
      matrix->setCursor(4, 22);
      matrix->print("MODE: BLUETOOTH (alGaffar)");
      matrix->setTextColor(RGB_LIME);
      matrix->setCursor(4, 36);
      matrix->print("Nama BT: JWS-RGB-P5");
    }
    matrix->setTextColor(RGB_DARKGREY);
    matrix->setCursor(4, 50);
    matrix->print("Tekan Reset 2x ganti mode");
    delay(1500);
    matrix->clearScreen();
  }

  // 7. Aktifkan HANYA SALAH SATU komunikasi untuk menjamin RAM bebas melimpah (>100KB)
  if (commMode == COMM_MODE_WIFI) {
#if ENABLE_WIFI
    initWiFiPortal();
    Serial.println("[SYSTEM] Mode Komunikasi: WIFI WEB PORTAL (192.168.4.1)");
#endif
  } else {
#if ENABLE_BLUETOOTH
    SerialBT.enableSSP();
    // disableBLE = true agar memori BLE dialihkan ke heap Classic BT (menghemat >25KB RAM)
    bool btOk = SerialBT.begin("JWS-RGB-P5", false, true);
    if (btOk) {
      Serial.println("[BT] SerialBT.begin('JWS-RGB-P5') BERHASIL");
      esp_bt_gap_set_scan_mode(ESP_BT_CONNECTABLE, ESP_BT_GENERAL_DISCOVERABLE);
      Serial.println("[BT] Bluetooth Siap! Nama Perangkat: JWS-RGB-P5");
    } else {
      Serial.println("[BT] ERROR: SerialBT.begin() GAGAL!");
    }
    Serial.println("[SYSTEM] Mode Komunikasi: BLUETOOTH SPP (alGaffar)");
#endif
  }

  Serial.printf("[SYSTEM] Free Heap: %d bytes\n", ESP.getFreeHeap());
  Serial.println("[SYSTEM] JWS P5 RGB Siap Beroperasi!");
}

// ===================================================================================
// === MAIN LOOP =====================================================================
// ===================================================================================
void loop() {
  // Layani koneksi Bluetooth/Serial & Web WiFi
  serviceBluetooth();
#if ENABLE_WIFI
  if (commMode == COMM_MODE_WIFI) {
    serviceWiFiPortal();
  }
#endif

  // Bersihkan reset marker setelah 4 detik beroperasi normal
  static uint32_t bootStartMs = millis();
  if (rtc_reset_marker != 0 && (uint32_t)(millis() - bootStartMs) > 4000) {
    rtc_reset_marker = 0;
  }

  updateTime();
  check_mp3();
  check_azzan();
  Timer_Minute(1);

  // Daftar Komponen Tampilan (State Machine)
  if (RunSel == 1)
    dwMrq(drawWelcome(), int(Prm.RT), 1);
  if (RunSel == 2)
    dwMrq(drawDateH(), int(Prm.RT), 2);
  if (RunSel == 3)
    dwMrq(drawDateM(), int(Prm.RT), 3);
  if (RunSel == 4)
    dwMrq(drawInfo(130), int(Prm.RT), 4);
  if (RunSel == 5)
    dwMrq(drawInfo(280), int(Prm.RT), 5);
  if (RunSel == 6)
    dwMrq(drawInfo(430), int(Prm.RT), 6);

  // Mode Khusus Saat Waktu Sholat Tiba
  drawOnAzzan(99);
  drawAzzan(100);
  drawIqomah(101);
  if (RunSel == 102)
    dwMrq(drawInfo(580), Prm.RT, 102);  // Pesan Sholat biasa
  if (RunSel == 103)
    dwMrq(drawInfo(730), Prm.RT, 103);  // Pesan Sholat jumat
  blinkBlock(104);

  // Transisi Status Tampilan
  switch (RunFinish) {
    case 1:  setRunSel(2); break;
    case 2:  setRunSel(3); break;
    case 3:  setRunSel(4); break;
    case 4:  setRunSel(5); break;
    case 5:  setRunSel(6); break;
    case 6:  setRunSel(1); break;
    case 98: setRunSel(99); break;
    case 99: setRunSel(100); break;
    case 100:
      if (jumat) {
        setRunSel(103);
        reset_x = 1;
      } else {
        setRunSel(101);
      }
      break;
    case 101:
      setRunSel(102);
      reset_x = 1;
      break;
    case 102:
      setRunSel(104);
      break;
    case 103:
      setRunSel(104);
      break;
    case 104:
      setRunSel(1);
      reset_x = 1;
      break;
    default:
      break;
  }
  RunFinish = 0;

  // Berikan waktu proses singkat untuk background network stack WiFi
  delay(2);
}
