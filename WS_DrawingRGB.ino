/*************************************************************************************
 * Engine Grafis JWS LED Matrix P5 RGB 4x2 (256x64 Piksel)
 * Menggunakan Adafruit_GFX API dari ESP32-HUB75-VirtualMatrixPanel_T
 * Full Color RGB565, Jam Digital Besar, Running Text 256px, Animasi Adzan & Iqomah
 *************************************************************************************/

#include "Display_HUB75.h"

// Dimensi Virtual Display
#define DISP_W 256
#define DISP_H 64

// Buffer teks & pembantu
static char timeStrBuf[16];
static char colonBlink = true;

void formatDuaAngka(int nilai, char* hasil) {
  hasil[0] = (nilai / 10) + '0';
  hasil[1] = (nilai % 10) + '0';
  hasil[2] = '\0';
}

void Buzzer(uint8_t state) {
  if (state == 1 && Prm.BZ == 1) {
    digitalWrite(BUZZ_PIN, HIGH);
  } else {
    digitalWrite(BUZZ_PIN, LOW);
  }
}

void BuzzerBeep(uint16_t durationMs) {
  if (Prm.BZ == 1) {
    digitalWrite(BUZZ_PIN, HIGH);
    delay(durationMs);
    digitalWrite(BUZZ_PIN, LOW);
  }
}

boolean dwDo(int DrawAdd) {
  return (RunSel == DrawAdd);
}

void dwDone(int DrawAdd) {
  RunFinish = DrawAdd;
  RunSel = 0;
}

// -----------------------------------------------------------------------------------
// Helper Gambar Teks Tengah (Centered Text)
// -----------------------------------------------------------------------------------
void drawTextCentered(int x1, int x2, int y, const char* str, uint16_t color, uint8_t size = 1) {
  if (!matrix) return;
  int len = strlen(str);
  int charWidth = 6 * size;
  int totalWidth = len * charWidth;
  int boxWidth = x2 - x1;
  int startX = x1 + (boxWidth - totalWidth) / 2;
  if (startX < x1) startX = x1;

  matrix->setTextSize(size);
  matrix->setTextColor(color);
  matrix->setCursor(startX, y);
  matrix->print(str);
}

// -----------------------------------------------------------------------------------
// Zona Kiri (0..105): Jam Digital Besar, Tanggal Masehi & Hijriah
// -----------------------------------------------------------------------------------
void drawLeftDashboard() {
  if (!matrix) return;

  // 1. Jam Digital Besar (HH:MM:SS)
  char bufH[3], bufM[3], bufS[3];
  formatDuaAngka(now.hour(), bufH);
  formatDuaAngka(now.minute(), bufM);
  formatDuaAngka(now.second(), bufS);

  // Background area jam
  matrix->fillRect(0, 0, 106, 47, RGB_BLACK);

  // HH:MM (Text Size 3 = 18x24 px per digit)
  matrix->setTextSize(3);
  matrix->setTextColor(RGB_GOLD);
  matrix->setCursor(2, 2);
  matrix->print(bufH);

  // Titik dua ':' berkedip
  if (millis() % 1000 < 500) {
    matrix->setTextColor(RGB_WHITE);
  } else {
    matrix->setTextColor(RGB_BLACK);
  }
  matrix->setCursor(38, 2);
  matrix->print(":");

  matrix->setTextColor(RGB_GOLD);
  matrix->setCursor(54, 2);
  matrix->print(bufM);

  // Detik kecil (:SS) di samping (Text Size 1)
  matrix->setTextSize(1);
  matrix->setTextColor(RGB_CYAN);
  matrix->setCursor(92, 4);
  matrix->print(bufS);

  // Garis pemisah kecil di bawah jam
  matrix->drawLine(2, 26, 102, 26, RGB_NAVY);

  // 2. Tanggal Masehi (Text Size 1)
  matrix->setTextSize(1);
  matrix->setTextColor(RGB_CYAN);
  matrix->setCursor(2, 29);
  matrix->print(drawDateM());

  // 3. Tanggal Hijriah (Text Size 1)
  matrix->setTextSize(1);
  matrix->setTextColor(RGB_LIME);
  matrix->setCursor(2, 38);
  matrix->print(drawDateH());

  // Garis batas vertikal antara Dashboard Kiri dan Kanan
  matrix->drawLine(106, 2, 106, 45, RGB_DARKGREY);
}

// -----------------------------------------------------------------------------------
// Zona Kanan (108..255): Kartu Jadwal Sholat Dinamis & Countdown
// -----------------------------------------------------------------------------------
void drawRightPrayerCard() {
  if (!matrix) return;

  static uint32_t lastCardRotMs = 0;
  static uint8_t rotatingIdx = 1; // 0=Imsak, 1=Subuh, 2=Terbit, 3=Dhuha, 4=Dzuhur, 5=Ashar, 6=Maghrib, 7=Isya
  uint32_t currentMs = millis();

  // Rotasi jadwal sholat setiap 4 detik
  if (currentMs - lastCardRotMs > 4000UL) {
    lastCardRotMs = currentMs;
    do {
      rotatingIdx = (rotatingIdx + 1) % 8;
      if (rotatingIdx == 0 && Prm.SI == 0) continue; // Skip imsak jika dinonaktifkan
      if (rotatingIdx == 2 && Prm.ST == 0) continue; // Skip terbit
      if (rotatingIdx == 3 && Prm.SU == 0) continue; // Skip dhuha
      break;
    } while (true);
  }

  // Bersihkan area kanan
  matrix->fillRect(107, 0, 149, 47, RGB_BLACK);

  // Bingkai kartu sholat
  matrix->drawRoundRect(108, 1, 147, 45, 3, RGB_TEAL);

  // Ambil data sholat aktif rotasi
  char* namaSholat = sholatN(rotatingIdx);
  float stime = sholatT[rotatingIdx];
  uint8_t shour = floor(stime);
  uint8_t sminute = floor((stime - (float)shour) * 60.0f);
  char bufSholH[3], bufSholM[3];
  formatDuaAngka(shour, bufSholH);
  formatDuaAngka(sminute, bufSholM);

  // 1. Header Nama Sholat
  uint16_t headerCol = RGB_YELLOW;
  if (rotatingIdx == SholatNow) headerCol = RGB_RED; // Sholat yang sedang berlangsung
  drawTextCentered(108, 254, 4, namaSholat, headerCol, 1);

  // Garis pembatas header kartu
  matrix->drawLine(112, 14, 250, 14, RGB_DARKGREY);

  // 2. Waktu Sholat Besar (Text Size 2)
  char timeBoxBuf[10];
  snprintf(timeBoxBuf, sizeof(timeBoxBuf), "%s:%s", bufSholH, bufSholM);
  drawTextCentered(108, 254, 18, timeBoxBuf, RGB_WHITE, 2);

  // 3. Status Countdown ke Waktu Sholat Berikutnya
  // Cari waktu sholat terdekat berikutnya
  uint32_t currentSec = (uint32_t)now.hour() * 3600UL + (uint32_t)now.minute() * 60UL + (uint32_t)now.second();
  int8_t nextPrayer = -1;
  int32_t minDiff = 86400L;

  for (uint8_t i = 1; i < 8; i++) {
    if (i == 2 || i == 3) continue; // Skip terbit/dhuha
    uint16_t pMin = (uint16_t)ceil((sholatT[i] * 60.0f) - 0.0001f);
    int32_t pSec = (int32_t)pMin * 60L;
    int32_t diff = pSec - (int32_t)currentSec;
    if (diff > 0 && diff < minDiff) {
      minDiff = diff;
      nextPrayer = i;
    }
  }

  char ctdBuf[30];
  if (nextPrayer >= 0) {
    uint8_t cdH = minDiff / 3600;
    uint8_t cdM = (minDiff % 3600) / 60;
    uint8_t cdS = minDiff % 60;
    snprintf(ctdBuf, sizeof(ctdBuf), "%s -%02d:%02d:%02d", sholatN(nextPrayer), cdH, cdM, cdS);
    drawTextCentered(108, 254, 36, ctdBuf, RGB_ORANGE, 1);
  } else {
    snprintf(ctdBuf, sizeof(ctdBuf), "WAKTU SHOLAT HARI INI");
    drawTextCentered(108, 254, 36, ctdBuf, RGB_LIME, 1);
  }
}

// -----------------------------------------------------------------------------------
// Zona Bawah (48..63): Running Text 256 Piksel
// -----------------------------------------------------------------------------------
void dwMrq(const char* msg, int speed, int drawAdd) {
  if (!matrix) return;
  if (!dwDo(drawAdd)) return;

  static int16_t scrollX = DISP_W;
  static uint32_t lastScrollMs = 0;
  uint32_t currentMs = millis();

  if (reset_x != 0) {
    scrollX = DISP_W;
    reset_x = 0;
  }

  int textLenPx = strlen(msg) * 6; // 6 piksel per karakter font size 1
  int totalWidth = textLenPx + DISP_W;

  // Garis pemisah horizontal antara atas dan bawah
  matrix->drawLine(0, 48, DISP_W - 1, 48, RGB_CYAN);
  matrix->fillRect(0, 49, DISP_W, 15, RGB_BLACK);

  // Gambar Dashboard Utama di bagian atas secara paralel
  drawLeftDashboard();
  drawRightPrayerCard();

  // Geser teks running text
  if (currentMs - lastScrollMs > (uint32_t)speed) {
    lastScrollMs = currentMs;
    scrollX--;
    if (scrollX < -textLenPx) {
      scrollX = DISP_W;
      dwDone(drawAdd);
      return;
    }
  }

  // Tampilkan teks berjalan
  matrix->setTextSize(1);
  matrix->setTextColor(RGB_WHITE);
  matrix->setCursor(scrollX, 53);
  matrix->print(msg);
}

// -----------------------------------------------------------------------------------
// FASE ADZAN: Peringatan Masuk Waktu Sholat (RunSel 99 & 100)
// -----------------------------------------------------------------------------------
void drawOnAzzan(int DrawAdd) {
  if (!matrix) return;
  if (!dwDo(DrawAdd)) return;

  uint8_t ct_kedip = 20; // Kedipan awal saat masuk waktu
  static uint8_t ct = 0;
  static uint32_t lsRn = 0;
  uint32_t Tmr = millis();

  if (Tmr - lsRn > 500 && ct <= ct_kedip) {
    lsRn = Tmr;
    matrix->clearScreen();

    if ((ct % 2) == 0) {
      // Bingkai luar berkedip
      matrix->drawRect(0, 0, DISP_W, DISP_H, RGB_RED);
      matrix->drawRect(2, 2, DISP_W - 4, DISP_H - 4, RGB_YELLOW);

      drawTextCentered(0, DISP_W, 6, "--- WAKTU SHOLAT TELAH TIBA ---", RGB_WHITE, 1);

      const char* pName = jumat ? "JUM'AT" : sholatN(SholatNow);
      drawTextCentered(0, DISP_W, 20, pName, RGB_YELLOW, 3);

      drawTextCentered(0, DISP_W, 48, "SAATNYA MENGUMANDANGKAN ADZAN", RGB_CYAN, 1);
      Buzzer(1);
    } else {
      Buzzer(0);
    }
    ct++;
  }

  if (ct > ct_kedip) {
    dwDone(DrawAdd);
    ct = 0;
    Buzzer(0);
  }
}

void drawAzzan(int DrawAdd) {
  if (!matrix) return;
  if (!dwDo(DrawAdd)) return;

  uint16_t az = Prm.AD;
  uint16_t in = Prm.IN;
  static int ct = 0;
  static uint32_t lsRn = 0;
  uint32_t Tmr = millis();

  int ct_limit = jumat ? (in * 60) : (az * 60);

  if (Tmr - lsRn > 1000 && ct <= ct_limit) {
    lsRn = Tmr;
    matrix->clearScreen();

    int mnt = (ct_limit - ct) / 60;
    int scd = (ct_limit - ct) % 60;
    char buffMnt[3], buffScd[3];
    formatDuaAngka(mnt, buffMnt);
    formatDuaAngka(scd, buffScd);

    matrix->drawRoundRect(4, 4, DISP_W - 8, DISP_H - 8, 4, RGB_RED);

    if (jumat) {
      drawTextCentered(0, DISP_W, 8, "ACARA SHOLAT JUM'AT", RGB_YELLOW, 1);
    } else {
      char judulAdzan[32];
      snprintf(judulAdzan, sizeof(judulAdzan), "ADZAN %s", sholatN(SholatNow));
      drawTextCentered(0, DISP_W, 8, judulAdzan, RGB_YELLOW, 1);
    }

    // Countdown Adzan Besar
    char cdStr[16];
    snprintf(cdStr, sizeof(cdStr), "%s:%s", buffMnt, buffScd);
    drawTextCentered(0, DISP_W, 22, cdStr, RGB_WHITE, 3);

    drawTextCentered(0, DISP_W, 50, "MENDENGARKAN & MENJAWAB ADZAN", RGB_LIME, 1);

    // Beep 5 detik terakhir sebelum adzan selesai
    if (ct > (ct_limit - 5)) {
      Buzzer(1);
    } else {
      Buzzer(0);
    }
    ct++;
  }

  if (ct > ct_limit) {
    dwDone(DrawAdd);
    ct = 0;
    Buzzer(0);
  }
}

// -----------------------------------------------------------------------------------
// FASE IQOMAH: Hitung Mundur Iqomah Raksasa (RunSel 101)
// -----------------------------------------------------------------------------------
void drawIqomah(int DrawAdd) {
  if (!matrix) return;
  if (!dwDo(DrawAdd)) return;

  static uint32_t lsRn = 0;
  uint32_t Tmr = millis();
  static int ct = 0;

  int cn_l = Iqomah[SholatNow] * 60;

  if (Tmr - lsRn > 1000 && ct <= cn_l) {
    lsRn = Tmr;
    matrix->clearScreen();

    int mnt = (cn_l - ct) / 60;
    int scd = (cn_l - ct) % 60;
    char buffMnt[3], buffScd[3];
    formatDuaAngka(mnt, buffMnt);
    formatDuaAngka(scd, buffScd);

    // Bingkai Luar
    matrix->drawRoundRect(4, 2, DISP_W - 8, DISP_H - 4, 4, RGB_CYAN);

    // Header
    char iqomahTitle[32];
    snprintf(iqomahTitle, sizeof(iqomahTitle), "MENJELANG IQOMAH %s", sholatN(SholatNow));
    drawTextCentered(0, DISP_W, 6, iqomahTitle, RGB_CYAN, 1);

    // Countdown Angka Raksasa (Text Size 3)
    char countStr[16];
    snprintf(countStr, sizeof(countStr), "%s:%s", buffMnt, buffScd);
    drawTextCentered(0, DISP_W, 20, countStr, RGB_GOLD, 3);

    // Progress Bar Iqomah di bagian bawah
    int progressPx = (int)((float)ct / (float)cn_l * (float)(DISP_W - 20));
    matrix->drawRect(10, 48, DISP_W - 20, 8, RGB_DARKGREY);
    matrix->fillRect(10, 48, progressPx, 8, RGB_GREEN);

    // Beep 10 detik terakhir menuju Iqomah
    if (ct > (cn_l - 11)) {
      Buzzer(1);
    } else {
      Buzzer(0);
    }
    ct++;
  }

  if (ct > cn_l) {
    dwDone(DrawAdd);
    ct = 0;
    Buzzer(0);
  }
}

// -----------------------------------------------------------------------------------
// FASE SHOLAT: Blackout / Layar Padam Selama Sholat (RunSel 104)
// -----------------------------------------------------------------------------------
void blinkBlock(int DrawAdd) {
  if (!matrix) return;
  if (!dwDo(DrawAdd)) return;

  static uint32_t lsRn = 0;
  static uint16_t ct = 0;
  uint32_t Tmr = millis();

  uint16_t ct_l = jumat ? ((Prm.JM + Prm.SO) * 60) : (Prm.SO * 60);

  if (Tmr - lsRn > 1000) {
    lsRn = Tmr;
    matrix->clearScreen();

    // 15 detik pertama: tampilkan himbauan luruskan shaf
    if (ct < 15) {
      matrix->drawRoundRect(4, 4, DISP_W - 8, DISP_H - 8, 4, RGB_YELLOW);
      drawTextCentered(0, DISP_W, 12, "RAPATKAN & LURUSKAN SHAF", RGB_YELLOW, 2);
      drawTextCentered(0, DISP_W, 36, "NONAKTIFKAN NADA DERING HANDPHONE", RGB_WHITE, 1);
      drawTextCentered(0, DISP_W, 48, "SHOLAT BERJAMAAH DIMULAI", RGB_CYAN, 1);
    } else {
      // Selama sholat: Layar padam (Blackout) total agar tidak silau,
      // hanya titik indikator kecil berkedip lembut di pojok kanan bawah
      if ((ct % 2) == 0) {
        matrix->drawPixel(DISP_W - 2, DISP_H - 2, RGB_DARKGREY);
      }
    }
    ct++;
  }

  if (ct > ct_l) {
    dwDone(DrawAdd);
    azzan = false;
    jumat = false;
    ct = 0;
    matrix->clearScreen();
  }
}
