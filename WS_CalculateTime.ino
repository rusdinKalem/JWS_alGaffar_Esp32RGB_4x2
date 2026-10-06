/*************************************************************************************
 * Perhitungan Waktu Sholat Astronomis & Kalender Hijriah
 * Algoritma hisab akurat berbasis koordinat lintang, bujur, ketinggian, dan timezone
 *************************************************************************************/

#define d2r(x) ((x)*(float)M_PI/180.0f)
#define r2d(x) ((x)*180.0f/(float)M_PI)

const float lunarY = 354.367068f;

float fix_hour(float a) {
  a = a - (float)24.0 * floor(a / 24.0);
  a = a < 0.0 ? a + 24.0 : a;
  return a;
}

float fix_angle(float a) {
  a = a - (float)360.0 * floor(a / 360.0);
  a = a < 0.0 ? a + 360.0 : a;
  return a;
}

// Julian Date at GMT mid day
float E_Julian_date(int Year, int Month, int Days, float Longitude) {
  if (Month <= 2) {
    Year -= 1;
    Month += 12;
  }
  float A = floor(((float)Year / 100.0));
  float B = 2 - A + floor(A / 4.0);
  float CLong = Longitude / (float)(15 * 24);
  float JD = floor(365.25 * (float)(Year + 4716))
             - 2451545
             + floor(30.6001 * (float)(Month + 1))
             + (float)Days + B
             - 1524.5
             - CLong;
  return JD;
}

// Sun Declination
float EqT(const float EJD) {
  float g = fix_angle(357.529f + 0.98560028f * EJD);
  float q = fix_angle(280.459f + 0.98564736f * EJD);
  float L = fix_angle(q + 1.915f * sin(d2r(g)) + 0.020f * sin(d2r(2 * g)));
  float e = (23.439f - 0.00000036f * EJD);

  float RA = r2d(atan2(cos(d2r(e)) * sin(d2r(L)), cos(d2r(L)))) / 15.0f;
  float Eq = (q / 15.0f - fix_hour(RA));
  return Eq;
}

float Dql(float EJD) {
  float g = fix_angle(357.529f + 0.98560028f * EJD);
  float q = fix_angle(280.459f + 0.98564736f * EJD);
  float L = fix_angle(q + 1.915f * sin(d2r(g)) + 0.020f * sin(d2r(2 * g)));
  float e = (23.439f - 0.00000036f * EJD);

  float dd = r2d(asin(sin(d2r(e)) * sin(d2r(L))));
  return dd;
}

float HourAngle(float Alfa, float Declination, float Latitude) {
  float cosH = (-sin(d2r(Alfa)) - sin(d2r(Latitude)) * sin(d2r(Declination)))
               / (cos(d2r(Latitude)) * cos(d2r(Declination)));
  cosH = constrain(cosH, -1.0f, 1.0f);
  return r2d(acos(cosH)) / 15.0f;
}

void Pray_Time(float TimeZone, float Latitude, float Longitude, float Altitude, float Declination, float EquationOfTime) {
  // 4. Dzuhur
  float BaseTime = fix_hour((float)12 + TimeZone - (Longitude / 15.0f) - EquationOfTime);
  sholatT[4] = BaseTime + (float)(Prm.IH + Prm.IL) / 60.0f;

  // 5. Ashar
  float alfa = r2d(-atan(1.0f / (1.0f + tan(d2r(fabs(Latitude - Declination))))));
  float HA = HourAngle(alfa, Declination, Latitude);
  sholatT[5] = BaseTime + HA + (float)(Prm.IH + Prm.IA) / 60.0f;

  // 6. Maghrib
  alfa = 0.8333f + 0.0347f * sqrt(Altitude > 0.0f ? Altitude : 0.0f);
  HA = HourAngle(alfa, Declination, Latitude);
  sholatT[6] = BaseTime + HA + (float)(Prm.IH + Prm.IM) / 60.0f;

  // 2. Terbit
  sholatT[2] = BaseTime - HA;

  // 7. Isya
  HA = HourAngle((float)18, Declination, Latitude);
  sholatT[7] = BaseTime + HA + (float)(Prm.IH + Prm.II) / 60.0f;

  // 1. Shubuh
  HA = HourAngle((float)20, Declination, Latitude);
  sholatT[1] = BaseTime - HA + (float)(Prm.IH + Prm.IS) / 60.0f;

  // 0. Imsak
  sholatT[0] = sholatT[1] - (float)10 / 60.0f;

  // 3. Dhuha
  HA = HourAngle((float)-4.5, Declination, Latitude);
  sholatT[3] = BaseTime - HA;
}

void sholatCal() {
  float EJD = E_Julian_date(now.year(), now.month(), now.day(), Prm.L_LO);
  float Decl = Dql(EJD);
  float EqOfTime = EqT(EJD);
  Pray_Time(Prm.L_TZ, Prm.L_LA, Prm.L_LO, Prm.L_AL, Decl, EqOfTime);
}

// -----------------------------------------------------------------------------------
// Konversi Tanggal Masehi ke Hijriah
// -----------------------------------------------------------------------------------
long Days(uint16_t Y, uint8_t M, uint8_t D) {
  if (M < 3) {
    Y -= 1;
    M += 12;
  }
  Y = Y - 2000;
  long ndays = floor(365.25 * Y) + floor(30.6001 * (M + 1)) + floor(Y / 100.0) + floor(Y / 400.0) + D + 196;
  return ndays;
}

long DaysHijri(uint16_t Y, uint8_t M, uint8_t D) {
  Y = Y - 1420;
  long hari = floor(29.5 * M - 28.999) + floor(lunarY * Y) + D;
  return hari;
}

hijir_date toHijri(uint16_t Y, uint8_t M, uint8_t D, uint8_t cor) {
  hijir_date BuffDate;
  long nday = Days(Y, M, D) + Prm.CH + cor;

  long tahun = floor(nday / lunarY) + 1420;
  long bulan = 1;
  long harike = 1;
  while (DaysHijri(tahun, bulan, 1) <= nday) { tahun++; }
  tahun--;
  while (DaysHijri(tahun, bulan, 1) <= nday) { bulan++; }
  bulan--;
  harike = 1 + nday - DaysHijri(tahun, bulan, 1);
  if (bulan == 13) {
    bulan = 12;
    harike += 29;
  }
  BuffDate.hD = (uint8_t)harike;
  BuffDate.hM = (uint8_t)bulan;
  BuffDate.hY = (uint16_t)tahun;

  return BuffDate;
}
