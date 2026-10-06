# JWS LED Matrix P5 RGB HUB-75 (4x2 Panel, 256x64 Piksel) - ESP32

Firmware Jam Waktu Sholat (JWS) otomatis berbasis **ESP32** untuk mengendalikan **LED Matrix P5 RGB HUB-75** berukuran **4x2 panel (256x64 piksel)**. Firmware ini dirancang khusus untuk bekerja optimal dengan **PCB Controller LED RGB ElektronMart V2.1 (Buffer 74HC245)**.

---

## 🌟 Fitur Utama

- **Layar Luas 256 x 64 Piksel Full Color**:
  - Menggunakan library resmi berkecepatan tinggi `ESP32-HUB75-MatrixPanel-I2S-DMA` & `ESP32-HUB75-VirtualMatrixPanel_T`.
  - Jam digital besar ukuran font tebal warna Emas/Gold (`HH:MM`) dengan titik dua berkedip dan detik kecil (`:SS`).
  - Tanggal Masehi dan tanggal Hijriah otomatis.
  - Kartu jadwal sholat RGB dinamis berputar tiap 4 detik dilengkapi informasi hitung mundur ke waktu sholat berikutnya (misal: `MENUJU ASYAR -00:15:32`).
  - Running text selebar 256 piksel yang bergulir mulus (Nama & Alamat Masjid, Info 1, Info 2, Info 3).
- **Hisab Waktu Sholat Astronomis Akurat**:
  - Perhitungan waktu sholat berdasarkan koordinat lintang, bujur, ketinggian, dan zona waktu.
  - Pengaman Ihtiyati dan koreksi waktu per jadwal sholat.
  - Penanggalan Hijriah otomatis dengan koreksi hisab.
- **Fase Masuk Waktu Sholat**:
  - **Fase Adzan**: Tampilan berkedip besar, nama sholat, hitung mundur adzan, dan bunyi buzzer.
  - **Fase Iqomah**: Hitung mundur raksasa menit & detik di tengah layar beserta progress bar visual dan bunyi beep penanda 10 detik terakhir.
  - **Fase Sholat Berjamaah**: Menampilkan pesan *"RAPATKAN & LURUSKAN SHAF"* diikuti layar padam (Blackout mode) selama sholat berlangsung agar tidak menyilaukan jamaah.
- **Audio DFPlayer Mini (Tartil & Tarhim Otomatis)**:
  - Otomatis memutar Tartil (Folder 01) sebelum waktu sholat sesuai menit yang ditentukan.
  - Otomatis memutar Tarhim (Folder 02) menjelang adzan.
  - Otomatis mematikan suara saat adzan atau sholat berlangsung.
- **Kontrol Ganda: Bluetooth & WiFi**:
  - **Bluetooth Classic SPP**: Nama `JWS-RGB-P5`, 100% kompatibel dengan aplikasi Android *alGaffar*.
  - **WiFi Hotspot & Web Dashboard**: Hubungkan HP ke WiFi `JWS-RGB-P5` (Password: `12345678`), buka browser ke `http://192.168.4.1` untuk Web Dashboard lengkap dengan tombol **1-Klik Sinkronkan Jam HP** dan **1-Klik Ambil GPS HP**.

---

## 🔌 Pemetaan Pin Hardware (PCB ElektronMart V2.1)

| Perangkat / Jalur | Pin ESP32 | Keterangan |
| :--- | :--- | :--- |
| **HUB-75 R1, G1, B1** | `GPIO 2`, `GPIO 15`, `GPIO 4` | Melalui IC Buffer 74HC245 (5V TTL) |
| **HUB-75 R2, G2, B2** | `GPIO 16`, `GPIO 27`, `GPIO 17` | Melalui IC Buffer 74HC245 (5V TTL) |
| **HUB-75 A, B, C, D, E** | `GPIO 5`, `GPIO 18`, `GPIO 19`, `GPIO 21`, `GPIO 12` | Address line panel 32px MOD16 / scan 1/16 |
| **HUB-75 LAT, OE, CLK** | `GPIO 26`, `GPIO 25`, `GPIO 22` | Kontrol Latch, Output Enable, Clock |
| **RTC DS3231 (I2C)** | `SDA: GPIO 32`, `SCL: GPIO 33` | Soket I2C khusus di PCB ElektronMart |
| **DFPlayer Mini (UART2)** | `RX2: GPIO 13`, `TX2: GPIO 14` | Soket modul MP3 di PCB ElektronMart |
| **Buzzer Aktif 5V** | `GPIO 23` | Driver transistor buzzer di PCB |

---

## 🛠️ Persyaratan Library & Konfigurasi Arduino IDE

### Library yang Dibutuhkan
Pastikan library berikut telah terpasang di Arduino IDE:
1. `ESP32 HUB75 LED MATRIX PANEL DMA Display` (oleh mrfaptastic / Louis Beaudoin)
2. `Adafruit GFX Library`
3. `DS3231` (oleh Andrew Wickert / NorthernWidget)

### Konfigurasi Board pada Arduino IDE
- **Board**: `ESP32 Dev Module`
- **Partition Scheme**: `Huge APP (3MB No OTA/1MB SPIFFS)` *(Wajib dipilih)*
- **Upload Speed**: `921600` (atau `115200`)
- **CPU Frequency**: `240MHz (WiFi/BT)`
- **Flash Frequency**: `80MHz`
- **Core Debug Level**: `None`

---

## 📱 Cara Penggunaan

### 1. Kontrol via Aplikasi Android (alGaffar)
1. Aktifkan Bluetooth di HP Android Anda.
2. Sandingkan (pair) dengan Bluetooth bernama **`JWS-RGB-P5`**.
3. Buka aplikasi alGaffar dan hubungkan ke perangkat. Anda dapat mengatur teks, jadwal sholat, waktu iqomah, koordinat, dan waktu secara langsung.

### 2. Kontrol via Web Dashboard Browser HP
1. Hubungkan koneksi WiFi smartphone ke hotspot **`JWS-RGB-P5`** (Password: `12345678`).
2. Buka browser (Chrome / Safari) dan akses alamat:
   ```
   http://192.168.4.1
   ```
3. Tekan tombol **"🕒 SINKRONKAN JAM DENGAN HP SEKARANG"** untuk mencocokkan waktu RTC secara instan.
4. Anda dapat mengubah seluruh parameter masjid, koordinat GPS, jadwal sholat, pesan running text, serta menguji audio DFPlayer Mini langsung dari browser tanpa perlu menginstal aplikasi tambahan.

---

## 👥 Kredit & Referensi
- Desain PCB & Skema: [ElektronMart.com](https://elektronmart.com) (Bonny Useful / busel7)
- Library DMA Display: [mrfaptastic/ESP32-HUB75-MatrixPanel-I2S-DMA](https://github.com/mrcodetastic/ESP32-HUB75-MatrixPanel-DMA)
- Referensi Sketch: JWS Baabul Gaffar ESP32 Edition
