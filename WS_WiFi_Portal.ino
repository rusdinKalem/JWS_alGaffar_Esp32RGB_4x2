/*************************************************************************************
 * Web Portal & WiFi SoftAP Responsif untuk JWS P5 RGB (ESP32)
 * Menampilkan Web Dashboard modern untuk setting via browser Android / iPhone / Laptop
 * Akses: Hubungkan WiFi ke "JWS-RGB-P5", buka browser ke http://192.168.4.1
 *************************************************************************************/

#include "Config_Params.h"

#if ENABLE_WIFI
#include <WiFi.h>
#include <WebServer.h>

extern WebServer server;
extern void GetPrm();
extern void updateTime();
extern void update_All_data();
extern void setMatrixBrightness(uint8_t bright);
extern void dfPlayManual(uint8_t folder, uint8_t track);
extern void dfStop();
extern void dfSetVolume(uint8_t vol);

// Halaman HTML Utama
static const char INDEX_HTML[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html lang="id">
<head>
<meta charset="UTF-8">
<meta name="viewport" content="width=device-width, initial-scale=1.0">
<title>Kontrol JWS P5 RGB</title>
<style>
:root{--primary:#00897b;--primary-dark:#004d40;--accent:#ffd54f;--bg:#eceff1;--card:#ffffff;--text:#263238;}
*{box-sizing:border-box;margin:0;padding:0;font-family:-apple-system,BlinkMacSystemFont,"Segoe UI",Roboto,sans-serif;}
body{background:var(--bg);color:var(--text);padding:12px;}
.header{background:linear-gradient(135deg,var(--primary-dark),var(--primary));color:#fff;padding:16px;border-radius:12px;text-align:center;margin-bottom:12px;box-shadow:0 4px 6px rgba(0,0,0,0.1);}
.header h1{font-size:20px;margin-bottom:4px;}
.header p{font-size:12px;opacity:0.9;}
.card{background:var(--card);border-radius:10px;padding:14px;margin-bottom:12px;box-shadow:0 2px 4px rgba(0,0,0,0.06);}
.card h2{font-size:15px;color:var(--primary-dark);margin-bottom:10px;border-bottom:2px solid #e0f2f1;padding-bottom:6px;}
.form-group{margin-bottom:10px;}
label{display:block;font-size:12px;font-weight:600;margin-bottom:4px;color:#455a64;}
input,select{width:100%;padding:9px;border:1px solid #cfd8dc;border-radius:6px;font-size:13px;}
input:focus,select:focus{border-color:var(--primary);outline:none;}
.btn{background:var(--primary);color:#fff;border:none;padding:10px 14px;border-radius:6px;font-size:13px;font-weight:600;cursor:pointer;width:100%;margin-top:6px;transition:0.2s;}
.btn:hover{background:var(--primary-dark);}
.btn-sync{background:#ff9800;color:#fff;}
.btn-sync:hover{background:#e65100;}
.btn-stop{background:#e53935;}
.btn-stop:hover{background:#b71c1c;}
.grid-2{display:grid;grid-template-columns:1fr 1fr;gap:8px;}
.grid-3{display:grid;grid-template-columns:1fr 1fr 1fr;gap:6px;}
.range-wrap{display:flex;align-items:center;gap:10px;}
.range-wrap input{flex:1;}
.range-val{font-size:13px;font-weight:bold;width:30px;text-align:center;}
.status-box{background:#e0f2f1;border-radius:6px;padding:8px;text-align:center;font-weight:bold;color:var(--primary-dark);margin-bottom:8px;}
.nav-tab{display:flex;overflow-x:auto;gap:6px;margin-bottom:12px;}
.tab-btn{background:#cfd8dc;border:none;padding:8px 12px;border-radius:6px;font-size:12px;font-weight:600;white-space:nowrap;cursor:pointer;}
.tab-btn.active{background:var(--primary);color:#fff;}
.tab-content{display:none;}
.tab-content.active{display:block;}
</style>
</head>
<body>

<div class="header">
  <h1>JWS P5 RGB 256x64</h1>
  <p>Panel Kontrol Jam Waktu Sholat ESP32 ElektronMart V2</p>
</div>

<div class="card">
  <div class="status-box" id="timeStatus">Sinkronisasi status waktu...</div>
  <button class="btn btn-sync" onclick="syncHpTime()">🕒 SINKRONKAN JAM DENGAN HP SEKARANG</button>
</div>

<div class="nav-tab">
  <button class="tab-btn active" onclick="openTab('tabMasjid', this)">Masjid & Lokasi</button>
  <button class="tab-btn" onclick="openTab('tabJadwal', this)">Jadwal Sholat</button>
  <button class="tab-btn" onclick="openTab('tabTeks', this)">Running Text</button>
  <button class="tab-btn" onclick="openTab('tabDisplay', this)">Display & Suara</button>
  <button class="tab-btn" onclick="openTab('tabMp3', this)">Audio MP3</button>
  <button class="tab-btn" onclick="openTab('tabMode', this)">🔄 Mode Bluetooth</button>
</div>

<!-- TAB 1: MASJID & LOKASI -->
<div id="tabMasjid" class="tab-content active">
  <form id="formMasjid" onsubmit="saveForm(event, '/save-masjid')">
    <div class="card">
      <h2>Nama & Tipe Masjid</h2>
      <div class="form-group">
        <label>Tipe Tempat Ibadah</label>
        <select name="MT" id="fMT">
          <option value="1">Masjid</option>
          <option value="2">Musholla</option>
          <option value="3">Surau</option>
          <option value="4">Langgar</option>
        </select>
      </div>
      <div class="form-group">
        <label>Nama Masjid / Musholla</label>
        <input type="text" name="MN" id="fMN" maxlength="38" required>
      </div>
      <div class="form-group">
        <label>Alamat / Kota</label>
        <input type="text" name="MA" id="fMA" maxlength="48" required>
      </div>
    </div>

    <div class="card">
      <h2>Koordinat GPS & Lokasi</h2>
      <button type="button" class="btn" style="background:#0288d1;margin-bottom:8px;" onclick="getGpsHp()">📍 AMBIL LOKASI DARI GPS HP</button>
      <div class="grid-2">
        <div class="form-group">
          <label>Latitude (Lintang)</label>
          <input type="number" step="0.000001" name="LA" id="fLA" required>
        </div>
        <div class="form-group">
          <label>Longitude (Bujur)</label>
          <input type="number" step="0.000001" name="LO" id="fLO" required>
        </div>
      </div>
      <div class="grid-2">
        <div class="form-group">
          <label>Altitude (Ketinggian m)</label>
          <input type="number" step="0.1" name="AL" id="fAL" required>
        </div>
        <div class="form-group">
          <label>Timezone (WIB=7, WITA=8, WIT=9)</label>
          <input type="number" step="1" name="TZ" id="fTZ" required>
        </div>
      </div>
      <button type="submit" class="btn">💾 SIMPAN PENGATURAN LOKASI</button>
    </div>
  </form>
</div>

<!-- TAB 2: JADWAL SHOLAT -->
<div id="tabJadwal" class="tab-content">
  <form id="formJadwal" onsubmit="saveForm(event, '/save-jadwal')">
    <div class="card">
      <h2>Ihtiyati & Koreksi Waktu (Menit)</h2>
      <div class="form-group">
        <label>Ihtiyati Pengaman (+Menit)</label>
        <input type="number" name="IH" id="fIH" min="0" max="10">
      </div>
      <div class="grid-3">
        <div class="form-group"><label>Subuh</label><input type="number" name="IS" id="fIS"></div>
        <div class="form-group"><label>Dzuhur</label><input type="number" name="IL" id="fIL"></div>
        <div class="form-group"><label>Ashar</label><input type="number" name="IA" id="fIA"></div>
      </div>
      <div class="grid-3">
        <div class="form-group"><label>Maghrib</label><input type="number" name="IM" id="fIM"></div>
        <div class="form-group"><label>Isya</label><input type="number" name="II" id="fII"></div>
        <div class="form-group"><label>Koreksi Hijriah</label><input type="number" name="CH" id="fCH"></div>
      </div>
    </div>

    <div class="card">
      <h2>Durasi Adzan, Iqomah & Sholat</h2>
      <div class="grid-2">
        <div class="form-group"><label>Durasi Adzan (Mnt)</label><input type="number" name="AD" id="fAD"></div>
        <div class="form-group"><label>Lama Sholat (Mnt)</label><input type="number" name="SO" id="fSO"></div>
      </div>
      <div class="grid-3">
        <div class="form-group"><label>Iqomah Subuh</label><input type="number" name="I1" id="fI1"></div>
        <div class="form-group"><label>Iqomah Dzuhur</label><input type="number" name="I4" id="fI4"></div>
        <div class="form-group"><label>Iqomah Ashar</label><input type="number" name="I5" id="fI5"></div>
      </div>
      <div class="grid-2">
        <div class="form-group"><label>Iqomah Maghrib</label><input type="number" name="I6" id="fI6"></div>
        <div class="form-group"><label>Iqomah Isya</label><input type="number" name="I7" id="fI7"></div>
      </div>
      <button type="submit" class="btn">💾 SIMPAN PENGATURAN JADWAL</button>
    </div>
  </form>
</div>

<!-- TAB 3: RUNNING TEXT -->
<div id="tabTeks" class="tab-content">
  <form id="formTeks" onsubmit="saveForm(event, '/save-teks')">
    <div class="card">
      <h2>Pesan Berjalan (Running Text)</h2>
      <div class="form-group">
        <label>Info 1</label>
        <input type="text" name="N1" id="fN1" maxlength="148">
      </div>
      <div class="form-group">
        <label>Info 2</label>
        <input type="text" name="N2" id="fN2" maxlength="148">
      </div>
      <div class="form-group">
        <label>Info 3</label>
        <input type="text" name="N3" id="fN3" maxlength="148">
      </div>
      <div class="form-group">
        <label>Pesan Menjelang Sholat Biasa</label>
        <input type="text" name="SM" id="fSM" maxlength="148">
      </div>
      <div class="form-group">
        <label>Pesan Sholat Jum'at</label>
        <input type="text" name="JM" id="fJM" maxlength="148">
      </div>
      <button type="submit" class="btn">💾 SIMPAN PESAN TEKS</button>
    </div>
  </form>
</div>

<!-- TAB 4: DISPLAY & SUARA -->
<div id="tabDisplay" class="tab-content">
  <form id="formDisplay" onsubmit="saveForm(event, '/save-display')">
    <div class="card">
      <h2>Pengaturan Tampilan</h2>
      <div class="form-group">
        <label>Kecerahan LED (0 - 255)</label>
        <div class="range-wrap">
          <input type="range" name="BL" id="fBL" min="10" max="255" oninput="fBL_val.innerText=this.value">
          <span class="range-val" id="fBL_val">60</span>
        </div>
      </div>
      <div class="form-group">
        <label>Kecepatan Running Text (ms - Makin kecil makin cepat)</label>
        <input type="number" name="RT" id="fRT" min="10" max="100">
      </div>
      <div class="form-group">
        <label>Bunyi Buzzer</label>
        <select name="BZ" id="fBZ">
          <option value="1">Aktif</option>
          <option value="0">Nonaktif</option>
        </select>
      </div>
      <div class="grid-3">
        <div class="form-group">
          <label>Tampil Imsak</label>
          <select name="SI" id="fSI"><option value="1">Ya</option><option value="0">Tidak</option></select>
        </div>
        <div class="form-group">
          <label>Tampil Terbit</label>
          <select name="ST" id="fST"><option value="1">Ya</option><option value="0">Tidak</option></select>
        </div>
        <div class="form-group">
          <label>Tampil Dhuha</label>
          <select name="SU" id="fSU"><option value="1">Ya</option><option value="0">Tidak</option></select>
        </div>
      </div>
      <button type="submit" class="btn">💾 SIMPAN DISPLAY & SUARA</button>
    </div>
  </form>
</div>

<!-- TAB 5: AUDIO MP3 -->
<div id="tabMp3" class="tab-content">
  <form id="formMp3" onsubmit="saveForm(event, '/save-mp3')">
    <div class="card">
      <h2>Pengaturan DFPlayer Mini</h2>
      <div class="form-group">
        <label>Master Audio MP3</label>
        <select name="enable" id="fMP3_EN">
          <option value="1">Aktif (Tartil & Tarhim Otomatis)</option>
          <option value="0">Nonaktif</option>
        </select>
      </div>
      <div class="form-group">
        <label>Volume Suara (0 - 30)</label>
        <div class="range-wrap">
          <input type="range" name="volume" id="fMP3_VOL" min="0" max="30" oninput="fVOL_val.innerText=this.value">
          <span class="range-val" id="fVOL_val">25</span>
        </div>
      </div>
      <button type="submit" class="btn">💾 SIMPAN SETTING AUDIO</button>
    </div>
  </form>

  <div class="card">
    <h2>Uji Coba Putar Audio Manual</h2>
    <div class="grid-2">
      <button class="btn" onclick="fetch('/play-track?folder=1&track=1')">▶ Putar Tartil Track 1</button>
      <button class="btn" onclick="fetch('/play-track?folder=2&track=1')">▶ Putar Tarhim Track 1</button>
    </div>
    <button class="btn btn-stop" onclick="fetch('/stop-audio')">⏹ STOP AUDIO SEKARANG</button>
  </div>
</div>

<!-- TAB 6: MODE BLUETOOTH / DUAL-MODE SWITCH -->
<div id="tabMode" class="tab-content">
  <div class="card" style="border: 2px solid #00897b; background: #e0f2f1;">
    <h2 style="color: #004d40;">🔄 Saklar Sistem: Beralih ke Mode Bluetooth</h2>
    <p style="font-size:13px; line-height: 1.6; margin-bottom:12px; color:#263238;">
      Saat ini JWS beroperasi dalam <b>Mode WiFi Web Portal</b>.<br>
      Jika Anda ingin mengontrol atau menyetel JWS menggunakan aplikasi Android <b>alGaffar</b>, tekan tombol di bawah ini.
    </p>
    <button type="button" class="btn btn-sync" onclick="switchModeBT()">📱 BERALIH KE MODE BLUETOOTH (ALGAFFAR)</button>
    <div style="font-size:11px; margin-top:14px; color:#546e7a; border-top:1px dashed #80cbc4; padding-top:8px;">
      💡 <b>Tips Saklar Fisik:</b> Anda juga dapat menukar mode (WiFi &harr; Bluetooth) kapan saja secara fisik tanpa smartphone, cukup dengan <b>menekan tombol RESET pada modul ESP32 sebanyak 2 kali berturut-turut</b> (dalam 4 detik). Buzzer akan berbunyi beep 3 kali sebagai tanda mode berhasil ditukar!
    </div>
  </div>
</div>

<script>
function openTab(tabId, el){
  document.querySelectorAll('.tab-content').forEach(d => d.classList.remove('active'));
  document.querySelectorAll('.tab-btn').forEach(b => b.classList.remove('active'));
  document.getElementById(tabId).classList.add('active');
  el.classList.add('active');
}

function loadAllData(){
  fetch('/get-data')
    .then(r => r.json())
    .then(d => {
      document.getElementById('timeStatus').innerText = 'Waktu Sistem: ' + d.time + ' | ' + d.dateM;
      document.getElementById('fMN').value = d.MN || '';
      document.getElementById('fMA').value = d.MA || '';
      document.getElementById('fMT').value = d.MT;
      document.getElementById('fLA').value = d.LA;
      document.getElementById('fLO').value = d.LO;
      document.getElementById('fAL').value = d.AL;
      document.getElementById('fTZ').value = d.TZ;
      document.getElementById('fIH').value = d.IH;
      document.getElementById('fIS').value = d.IS;
      document.getElementById('fIL').value = d.IL;
      document.getElementById('fIA').value = d.IA;
      document.getElementById('fIM').value = d.IM;
      document.getElementById('fII').value = d.II;
      document.getElementById('fCH').value = d.CH;
      document.getElementById('fAD').value = d.AD;
      document.getElementById('fSO').value = d.SO;
      document.getElementById('fI1').value = d.I1;
      document.getElementById('fI4').value = d.I4;
      document.getElementById('fI5').value = d.I5;
      document.getElementById('fI6').value = d.I6;
      document.getElementById('fI7').value = d.I7;
      document.getElementById('fN1').value = d.N1 || '';
      document.getElementById('fN2').value = d.N2 || '';
      document.getElementById('fN3').value = d.N3 || '';
      document.getElementById('fSM').value = d.SM || '';
      document.getElementById('fJM').value = d.JM || '';
      document.getElementById('fBL').value = d.BL;
      document.getElementById('fBL_val').innerText = d.BL;
      document.getElementById('fRT').value = d.RT;
      document.getElementById('fBZ').value = d.BZ;
      document.getElementById('fSI').value = d.SI;
      document.getElementById('fST').value = d.ST;
      document.getElementById('fSU').value = d.SU;
      document.getElementById('fMP3_EN').value = d.mp3_en;
      document.getElementById('fMP3_VOL').value = d.mp3_vol;
      document.getElementById('fVOL_val').innerText = d.mp3_vol;
    });
}

function saveForm(e, url){
  e.preventDefault();
  const formData = new FormData(e.target);
  fetch(url, {method:'POST', body:formData})
    .then(r => r.text())
    .then(t => {
      alert('Berhasil disimpan!');
      loadAllData();
    });
}

function syncHpTime(){
  const d = new Date();
  const pad = n => (n<10?'0':'')+n;
  const tgl = pad(d.getDate());
  const bln = pad(d.getMonth()+1);
  const thn = pad(d.getFullYear()%100);
  const jam = pad(d.getHours());
  const mnt = pad(d.getMinutes());
  const dtk = pad(d.getSeconds());
  let dow = d.getDay(); // 0=Ahad, 1=Senin
  dow = (dow === 0) ? 7 : dow; // alGaffar format: 1=Senin..7=Ahad

  const cmd = `SDT${tgl}${bln}${thn}${jam}${mnt}${dtk}${dow}`;
  fetch('/cmd?val=' + cmd)
    .then(r => r.text())
    .then(() => {
      alert('Waktu HP berhasil disinkronkan ke Jam Masjid!');
      loadAllData();
    });
}

function getGpsHp(){
  if (navigator.geolocation) {
    navigator.geolocation.getCurrentPosition(pos => {
      document.getElementById('fLA').value = pos.coords.latitude.toFixed(6);
      document.getElementById('fLO').value = pos.coords.longitude.toFixed(6);
      alert('Lokasi GPS HP berhasil diambil!');
    }, err => {
      alert('Gagal mengambil GPS: ' + err.message);
    });
  } else {
    alert('Browser tidak mendukung Geolocation.');
  }
}

function switchModeBT(){
  if (confirm("Beralih ke Mode Bluetooth untuk Aplikasi Android alGaffar?\n\nESP32 akan merestart ke mode Bluetooth.")) {
    fetch('/switch-mode')
      .then(() => alert("Perintah terkirim! ESP32 sedang merestart ke Mode Bluetooth.\nSilakan hubungkan aplikasi alGaffar ke Bluetooth 'JWS-RGB-P5'."))
      .catch(() => alert("ESP32 sedang merestart ke Mode Bluetooth."));
  }
}

window.onload = loadAllData;
</script>
</body>
</html>
)rawliteral";

// -----------------------------------------------------------------------------------
// Endpoint Web Server Handlers
// -----------------------------------------------------------------------------------

void handleRoot() {
  server.send(200, "text/html", INDEX_HTML);
}

void handleGetData() {
  char buf[1024];
  char dateStr[20];
  snprintf(dateStr, sizeof(dateStr), "%02d:%02d:%02d", now.hour(), now.minute(), now.second());

  snprintf(buf, sizeof(buf),
    "{\"time\":\"%s\",\"dateM\":\"%s\","
    "\"MN\":\"%s\",\"MA\":\"%s\",\"MT\":%d,"
    "\"LA\":%.6f,\"LO\":%.6f,\"AL\":%.1f,\"TZ\":%.1f,"
    "\"IH\":%d,\"IS\":%d,\"IL\":%d,\"IA\":%d,\"IM\":%d,\"II\":%d,\"CH\":%d,"
    "\"AD\":%d,\"SO\":%d,\"I1\":%d,\"I4\":%d,\"I5\":%d,\"I6\":%d,\"I7\":%d,"
    "\"N1\":\"%s\",\"N2\":\"%s\",\"N3\":\"%s\",\"SM\":\"%s\",\"JM\":\"%s\","
    "\"BL\":%d,\"RT\":%d,\"BZ\":%d,\"SI\":%d,\"ST\":%d,\"SU\":%d,"
    "\"mp3_en\":%d,\"mp3_vol\":%d}",
    dateStr, drawDateM(),
    cachedMasjidName, cachedMasjidAddress, Prm.MT,
    Prm.L_LA, Prm.L_LO, Prm.L_AL, Prm.L_TZ,
    Prm.IH, Prm.IS, Prm.IL, Prm.IA, Prm.IM, Prm.II, Prm.CH,
    Prm.AD, Prm.SO, Prm.I1, Prm.I4, Prm.I5, Prm.I6, Prm.I7,
    drawInfo(130), drawInfo(280), drawInfo(430), drawInfo(580), drawInfo(730),
    Prm.BL, Prm.RT, Prm.BZ, Prm.SI, Prm.ST, Prm.SU,
    Mp3Prm.enable, Mp3Prm.volume
  );

  server.send(200, "application/json", buf);
}

void handleSaveMasjid() {
  if (server.hasArg("MT")) Prm.MT = server.arg("MT").toInt();
  if (server.hasArg("LA")) Prm.L_LA = server.arg("LA").toFloat();
  if (server.hasArg("LO")) Prm.L_LO = server.arg("LO").toFloat();
  if (server.hasArg("AL")) Prm.L_AL = server.arg("AL").toFloat();
  if (server.hasArg("TZ")) Prm.L_TZ = server.arg("TZ").toFloat();

  if (server.hasArg("MN")) {
    writeEepromText(40, 39, server.arg("MN").c_str());
  }
  if (server.hasArg("MA")) {
    writeEepromText(80, 49, server.arg("MA").c_str());
  }

  EEPROM.put(0, Prm);
  EEPROM.commit();
  loadDisplayCache();
  update_All_data();

  server.send(200, "text/plain", "OK");
}

void handleSaveJadwal() {
  if (server.hasArg("IH")) Prm.IH = server.arg("IH").toInt();
  if (server.hasArg("IS")) Prm.IS = server.arg("IS").toInt();
  if (server.hasArg("IL")) Prm.IL = server.arg("IL").toInt();
  if (server.hasArg("IA")) Prm.IA = server.arg("IA").toInt();
  if (server.hasArg("IM")) Prm.IM = server.arg("IM").toInt();
  if (server.hasArg("II")) Prm.II = server.arg("II").toInt();
  if (server.hasArg("CH")) Prm.CH = server.arg("CH").toInt();
  if (server.hasArg("AD")) Prm.AD = server.arg("AD").toInt();
  if (server.hasArg("SO")) Prm.SO = server.arg("SO").toInt();
  if (server.hasArg("I1")) { Prm.I1 = server.arg("I1").toInt(); Iqomah[1] = Prm.I1; }
  if (server.hasArg("I4")) { Prm.I4 = server.arg("I4").toInt(); Iqomah[4] = Prm.I4; }
  if (server.hasArg("I5")) { Prm.I5 = server.arg("I5").toInt(); Iqomah[5] = Prm.I5; }
  if (server.hasArg("I6")) { Prm.I6 = server.arg("I6").toInt(); Iqomah[6] = Prm.I6; }
  if (server.hasArg("I7")) { Prm.I7 = server.arg("I7").toInt(); Iqomah[7] = Prm.I7; }

  EEPROM.put(0, Prm);
  EEPROM.commit();
  update_All_data();

  server.send(200, "text/plain", "OK");
}

void handleSaveTeks() {
  if (server.hasArg("N1")) writeEepromText(130, 149, server.arg("N1").c_str());
  if (server.hasArg("N2")) writeEepromText(280, 149, server.arg("N2").c_str());
  if (server.hasArg("N3")) writeEepromText(430, 149, server.arg("N3").c_str());
  if (server.hasArg("SM")) writeEepromText(580, 149, server.arg("SM").c_str());
  if (server.hasArg("JM")) writeEepromText(730, 149, server.arg("JM").c_str());

  cachedRunningAddress = 0xFFFF; // Reset cache teks
  server.send(200, "text/plain", "OK");
}

void handleSaveDisplay() {
  if (server.hasArg("BL")) {
    Prm.BL = server.arg("BL").toInt();
    setMatrixBrightness(Prm.BL);
  }
  if (server.hasArg("RT")) Prm.RT = server.arg("RT").toInt();
  if (server.hasArg("BZ")) Prm.BZ = server.arg("BZ").toInt();
  if (server.hasArg("SI")) Prm.SI = server.arg("SI").toInt();
  if (server.hasArg("ST")) Prm.ST = server.arg("ST").toInt();
  if (server.hasArg("SU")) Prm.SU = server.arg("SU").toInt();

  EEPROM.put(0, Prm);
  EEPROM.commit();
  server.send(200, "text/plain", "OK");
}

void handleSaveMp3() {
  if (server.hasArg("enable")) Mp3Prm.enable = server.arg("enable").toInt();
  if (server.hasArg("volume")) {
    Mp3Prm.volume = server.arg("volume").toInt();
    dfSetVolume(Mp3Prm.volume);
  }
  saveMp3Prm();
  server.send(200, "text/plain", "OK");
}

void handlePlayTrack() {
  uint8_t f = server.hasArg("folder") ? server.arg("folder").toInt() : 1;
  uint8_t t = server.hasArg("track") ? server.arg("track").toInt() : 1;
  dfPlayManual(f, t);
  server.send(200, "text/plain", "PLAYING");
}

void handleStopAudio() {
  dfStop();
  server.send(200, "text/plain", "STOPPED");
}

extern void setCommMode(uint8_t newMode);

void handleSwitchMode() {
  server.send(200, "text/plain", "OK");
  delay(300);
  setCommMode(COMM_MODE_BT);
}

void handleRawCmd() {
  if (server.hasArg("val")) {
    String cmd = server.arg("val");
    strncpy(CH_Prm, cmd.c_str(), sizeof(CH_Prm) - 1);
    CH_Prm[sizeof(CH_Prm) - 1] = '\0';
    LoadPrm();
    server.send(200, "text/plain", "OK");
  } else {
    server.send(400, "text/plain", "NO_CMD");
  }
}

// -----------------------------------------------------------------------------------
// Inisialisasi WiFi Access Point & WebServer
// -----------------------------------------------------------------------------------
void initWiFiPortal() {
  WiFi.disconnect(true);
  delay(50);
  WiFi.mode(WIFI_AP);
  
  IPAddress local_ip(192, 168, 4, 1);
  IPAddress gateway(192, 168, 4, 1);
  IPAddress subnet(255, 255, 255, 0);
  WiFi.softAPConfig(local_ip, gateway, subnet);

  bool ok = WiFi.softAP("JWS-RGB-P5", "12345678");
  if (ok) {
    Serial.print("[WIFI] Access Point Aktif: JWS-RGB-P5 | IP: ");
    Serial.println(WiFi.softAPIP());
  } else {
    Serial.println("[WIFI] GAGAL memulai Access Point!");
  }

  server.on("/", HTTP_GET, handleRoot);
  server.on("/get-data", HTTP_GET, handleGetData);
  server.on("/save-masjid", HTTP_POST, handleSaveMasjid);
  server.on("/save-jadwal", HTTP_POST, handleSaveJadwal);
  server.on("/save-teks", HTTP_POST, handleSaveTeks);
  server.on("/save-display", HTTP_POST, handleSaveDisplay);
  server.on("/save-mp3", HTTP_POST, handleSaveMp3);
  server.on("/play-track", HTTP_GET, handlePlayTrack);
  server.on("/stop-audio", HTTP_GET, handleStopAudio);
  server.on("/cmd", HTTP_GET, handleRawCmd);
  server.on("/switch-mode", HTTP_GET, handleSwitchMode);

  server.begin();
  Serial.println("[WEB] Web Server Port 80 Siap");
}

void serviceWiFiPortal() {
  server.handleClient();
}

#else

void initWiFiPortal() {}
void serviceWiFiPortal() {}

#endif
