#ifndef CONFIG_PINS_H
#define CONFIG_PINS_H

#include <Arduino.h>

/*************************************************************************************
 * KONFIGURASI PIN HARDWARE ESP32
 * Khusus untuk PCB Controller LED RGB ElektronMart V2.1 (Menggunakan Buffer 74HC245)
 * Referensi resmi: busel7/Arduino (elektronmart.com)
 *************************************************************************************/

// -----------------------------------------------------------------------------------
// 1. PIN HUB-75 LED Matrix (Melalui IC Buffer 74HC245 Level Shifter 5V)
// -----------------------------------------------------------------------------------
#define EM_R1_PIN   2   // Data Merah Baris Atas
#define EM_G1_PIN  15   // Data Hijau Baris Atas
#define EM_B1_PIN   4   // Data Biru Baris Atas

#define EM_R2_PIN  16   // Data Merah Baris Bawah
#define EM_G2_PIN  27   // Data Hijau Baris Bawah
#define EM_B2_PIN  17   // Data Biru Baris Bawah

#define EM_A_PIN    5   // Line Select Address A
#define EM_B_PIN   18   // Line Select Address B
#define EM_C_PIN   19   // Line Select Address C
#define EM_D_PIN   21   // Line Select Address D
#define EM_E_PIN   12   // Line Select Address E (untuk panel 32px MOD16 / scan 1/16)

#define EM_LAT_PIN 26   // Latch Strobe (LAT)
#define EM_OE_PIN  25   // Output Enable (OE) - Active Low
#define EM_CLK_PIN 22   // Clock Sinyal (CLK)

// -----------------------------------------------------------------------------------
// 2. PIN I2C RTC DS3231 (Jalur khusus di PCB ElektronMart)
// -----------------------------------------------------------------------------------
#define RTC_SDA    32   // I2C Data (SDA) -> Terhubung ke soket RTC PCB
#define RTC_SCL    33   // I2C Clock (SCL) -> Terhubung ke soket RTC PCB

// -----------------------------------------------------------------------------------
// 3. PIN DFPlayer Mini (Hardware Serial2 UART)
// -----------------------------------------------------------------------------------
#define MP3_RX     13   // ESP32 RX2 <- DFPlayer TX
#define MP3_TX     14   // ESP32 TX2 -> DFPlayer RX

// -----------------------------------------------------------------------------------
// 4. PIN Buzzer Aktif 5V
// -----------------------------------------------------------------------------------
#define BUZZ_PIN   23   // Terhubung ke transistor driver buzzer di PCB

// -----------------------------------------------------------------------------------
// 5. Konfigurasi Dimensi & Chaining Panel P5 (4x2 Panel = 256x64 Piksel)
// -----------------------------------------------------------------------------------
#define PANEL_RES_X     64    // Lebar piksel tiap modul panel P5 (64)
#define PANEL_RES_Y     32    // Tinggi piksel tiap modul panel P5 (32)

#define VDISP_NUM_COLS  4     // 4 Kolom panel horizontal
#define VDISP_NUM_ROWS  2     // 2 Baris panel vertikal
#define PANEL_CHAIN_LEN (VDISP_NUM_COLS * VDISP_NUM_ROWS) // Total 8 panel

// Chaining pattern:
// Default: CHAIN_TOP_LEFT_DOWN
// (Panel 1 di pojok kiri atas -> ke kanan -> turun ke baris bawah balik ke kiri)
// Opsi lain jika perkabelan fisik Anda berbeda:
// - CHAIN_BOTTOM_LEFT_UP
// - CHAIN_TOP_LEFT_DOWN_ZZ (ZigZag)
// - CHAIN_BOTTOM_LEFT_UP_ZZ
#define VIRTUAL_CHAIN_TYPE CHAIN_TOP_LEFT_DOWN

#endif // CONFIG_PINS_H
