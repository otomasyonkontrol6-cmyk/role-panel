// Bu dosyayı "secrets.h" adıyla kopyala ve kendi bilgilerini yaz.
// secrets.h .gitignore'da olduğu için GitHub'a yüklenmez.
#pragma once

const char* DB_URL    = "https://PROJE_ID-default-rtdb.firebaseio.com"; // sonunda / yok
const char* DB_SECRET = "DATABASE_SECRET";

struct Net { const char* ssid; const char* pass; };
const Net NETWORKS[] = {
  {"EV_WIFI",         "ev_sifre"},
  {"IS_WIFI",         "is_sifre"},
  {"TELEFON_HOTSPOT", "hotspot_sifre"},
};
