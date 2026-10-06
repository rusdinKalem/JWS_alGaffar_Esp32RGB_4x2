#ifndef CONFIG_PARAMS_H
#define CONFIG_PARAMS_H

#include <Arduino.h>

/*************************************************************************************
 * STRUKTUR PARAMETER & ALAMAT EEPROM
 * Sepenuhnya kompatibel dengan protokol aplikasi Android alGaffar & sketch Esp32-01
 *************************************************************************************/

#define PARAM_VERSION 100
#define MP3_PARAM_VERSION 101

#define ADDR_MP3_PRM 880
#define ADDR_JUMAT   1022
#define ADDR_RUNSEL  1023

// -----------------------------------------------------------------------------------
// Pilihan Fitur Komunikasi (Default: Keduanya Aktif = 1)
// Jika kedua fitur aktif (1), WAJIB pilih Partition Scheme: "Huge APP (3MB No OTA)"
// Jika hanya aktifkan salah satu (0 dan 1), sketch muat di skema partisi Default.
// -----------------------------------------------------------------------------------
#define ENABLE_BLUETOOTH 1  // 1 = Bluetooth Classic SPP Aktif (Aplikasi Android alGaffar)
#define ENABLE_WIFI      1  // 1 = WiFi SoftAP & Web Dashboard Aktif (http://192.168.4.1)

#pragma pack(push, 1)
typedef struct {
  uint8_t state;  // 1 byte  add 0 (PARAM_VERSION)
  float L_LA;     // 4 byte  add 1 (Latitude)
  float L_LO;     // 4 byte  add 5 (Longitude)
  float L_AL;     // 4 byte  add 9 (Altitude)
  float L_TZ;     // 4 byte  add 13 (Timezone)
  uint8_t MT;     // 1 byte  add 17 (1=Masjid, 2=Mushollah, 3=Surau, 4=Langgar)
  uint8_t BL;     // 1 byte  add 18 (Kecerahan LED 0-255)
  uint8_t RT;     // 1 byte  add 19 (Kecepatan Running Text ms)
  uint8_t IH;     // 1 byte  add 20 (Ihtiyati menit)
  uint8_t AD;     // 1 byte  add 21 (Durasi Adzan menit)
  uint8_t SO;     // 1 byte  add 22 (Lama Sholat menit)
  uint8_t JM;     // 1 byte  add 23 (Lama Sholat Jumat menit)
  uint8_t I1;     // 1 byte  add 24 (Iqomah Subuh menit)
  uint8_t I4;     // 1 byte  add 25 (Iqomah Dzuhur menit)
  uint8_t I5;     // 1 byte  add 26 (Iqomah Ashar menit)
  uint8_t I6;     // 1 byte  add 27 (Iqomah Maghrib menit)
  uint8_t I7;     // 1 byte  add 28 (Iqomah Isya menit)
  uint8_t BZ;     // 1 byte  add 29 (Buzzer 1=Aktif, 0=Nonaktif)
  uint8_t SI;     // 1 byte  add 30 (Tampilkan Imsak 1=Ya, 0=Tidak)
  uint8_t ST;     // 1 byte  add 31 (Tampilkan Terbit 1=Ya, 0=Tidak)
  uint8_t SU;     // 1 byte  add 32 (Tampilkan Dhuha 1=Ya, 0=Tidak)
  int8_t  IS;     // 1 byte  add 33 (Koreksi Subuh menit)
  int8_t  IL;     // 1 byte  add 34 (Koreksi Dzuhur menit)
  int8_t  IA;     // 1 byte  add 35 (Koreksi Ashar menit)
  int8_t  IM;     // 1 byte  add 36 (Koreksi Maghrib menit)
  int8_t  II;     // 1 byte  add 37 (Koreksi Isya menit)
  int8_t  CH;     // 1 byte  add 38 (Koreksi Hijriah hari)
  uint8_t IN;     // 1 byte  add 39 (Durasi Acara Jumat menit)
} struct_param;

typedef struct {
  uint8_t hD;     // Tanggal Hijriah
  uint8_t hM;     // Bulan Hijriah
  uint16_t hY;    // Tahun Hijriah
} hijir_date;

typedef struct {
  uint8_t  version;        // 1 byte  add 880 (MP3_PARAM_VERSION)
  uint8_t  enable;         // 1 byte  add 881 (1=On, 0=Off)
  uint8_t  volume;         // 1 byte  add 882 (Volume 0-30)
  uint8_t  tartilMin[6];   // 6 byte  add 883-888 (Subuh, Dzuhur, Ashar, Maghrib, Isya, Jumat)
  uint8_t  tartilTrack[6]; // 6 byte  add 889-894
  uint8_t  tarhimTrack[6]; // 6 byte  add 895-900
  uint16_t tarhimSec[6];   // 12 byte add 901-912 (Durasi detik)
} struct_mp3_prm;
#pragma pack(pop)

// -----------------------------------------------------------------------------------
// Definisi Warna RGB565 Standar
// -----------------------------------------------------------------------------------
#define RGB_BLACK     0x0000
#define RGB_WHITE     0xFFFF
#define RGB_RED       0xF800
#define RGB_GREEN     0x07E0
#define RGB_BLUE      0x001F
#define RGB_YELLOW    0xFFE0
#define RGB_CYAN      0x07FF
#define RGB_MAGENTA   0xF81F
#define RGB_ORANGE    0xFD20
#define RGB_GOLD      0xFEA0
#define RGB_LIME      0x87E0
#define RGB_TEAL      0x0410
#define RGB_PINK      0xFC18
#define RGB_DARKGREY  0x39E7
#define RGB_NAVY      0x000F

#endif // CONFIG_PARAMS_H
