/*
  ESP32 + Firebase Realtime Database röle kontrolü
  Kütüphane: ArduinoJson v7 (Library Manager'dan kur)
  Gizli bilgiler secrets.h dosyasında (secrets.example.h'yi kopyala).
*/
#include <WiFi.h>
#include <WiFiMulti.h>
#include <WiFiClientSecure.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include "secrets.h"

// Rölelerin web sayfasındaki id'leri ve ESP32 pinleri
const char* IDS[]  = {"r1", "r2", "r3", "r4"};
const int   PINS[] = {16, 17, 18, 19};
const int   N = 4;
const int   RELAY_ON  = LOW;   // Röle ters çalışırsa bu ikisini değiştir
const int   RELAY_OFF = HIGH;

WiFiMulti wifiMulti;
bool lastState[4] = {false, false, false, false};
unsigned long lastBeat = 0;

void connectWifi() {
  while (wifiMulti.run() != WL_CONNECTED) { delay(500); Serial.print("."); }
  Serial.println("\nWiFi: " + WiFi.SSID() + "  IP: " + WiFi.localIP().toString());
}

void setup() {
  Serial.begin(115200);
  for (int i = 0; i < N; i++) { pinMode(PINS[i], OUTPUT); digitalWrite(PINS[i], RELAY_OFF); }
  for (const Net& n : NETWORKS) wifiMulti.addAP(n.ssid, n.pass);
  connectWifi();
}

void loop() {
  if (wifiMulti.run() != WL_CONNECTED) connectWifi();

  WiFiClientSecure client;
  client.setInsecure();   // Basit kurulum. Üretimde kök sertifika kullan.
  HTTPClient http;

  // 1) Komutları oku
  http.begin(client, String(DB_URL) + "/relays.json?auth=" + DB_SECRET);
  int code = http.GET();
  bool changed = false;

  if (code == 200) {
    JsonDocument doc;
    if (!deserializeJson(doc, http.getString()) && doc.is<JsonObject>()) {
      for (int i = 0; i < N; i++) {
        bool cmd = doc[IDS[i]]["cmd"] | false;
        digitalWrite(PINS[i], cmd ? RELAY_ON : RELAY_OFF);
        if (cmd != lastState[i]) { lastState[i] = cmd; changed = true; }
      }
    }
  } else {
    Serial.printf("GET hata: %d\n", code);
  }
  http.end();

  // 2) Durumu ve "görüldü" zamanını yaz (değişince veya 5 sn'de bir)
  if (changed || millis() - lastBeat > 5000) {
    lastBeat = millis();
    JsonDocument up;
    for (int i = 0; i < N; i++) up[String("relays/") + IDS[i] + "/state"] = lastState[i];
    up["device/seen"][".sv"] = "timestamp";
    String body; serializeJson(up, body);

    http.begin(client, String(DB_URL) + "/.json?auth=" + DB_SECRET);
    http.addHeader("Content-Type", "application/json");
    http.PATCH(body);
    http.end();
  }

  delay(1000);
}
