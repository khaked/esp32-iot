#include <WiFi.h>
#include <HTTPClient.h>
#include "HX711.h"

// === WiFi ===
const char* ssid = "globalnet";
const char* password = "souha2002";

// === Firestore ===
const char* projectId = "souha-2d6ff";
const char* apiKey = "AIzaSyBHB-K3Jpx34aBOKRwn0RTaXI_Q_J355Fo";  // API key Firebase

// === HX711 ===
#define LOADCELL_DOUT 21
#define LOADCELL_SCK 22
HX711 scale;

// Document cible
const String targetDocId = "legume"; // document fixe sur Firestore
float minQuantity = 1.0;             // tu peux laisser configurable si besoin

// ----------------- WiFi -----------------
void connectWiFi() {
  WiFi.begin(ssid, password);
  Serial.print("Connexion WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println(" ✅ connecté !");
}

// ----------------- Mise à jour quantité -----------------
void updateQuantityFirestore(String docId, float quantity) {
  if (WiFi.status() != WL_CONNECTED) return;

  HTTPClient http;
  String url = "https://firestore.googleapis.com/v1/projects/" + String(projectId) +
               "/databases/(default)/documents/products/" + docId +
               "?key=" + String(apiKey) +
               "&updateMask.fieldPaths=quantity"; // ne mettre à jour que quantity

  http.begin(url);
  http.addHeader("Content-Type", "application/json");

  String payload = "{\"fields\":{\"quantity\":{\"doubleValue\":" + String(quantity, 2) + "}}}";
  int httpCode = http.PATCH(payload);

  if (httpCode == 200) {
    Serial.println("✅ Quantité mise à jour: " + String(quantity));
  } else if (httpCode == 404) {
    Serial.println("⚠️ Document non trouvé !"); // aucun document créé automatiquement
  } else {
    Serial.print("❌ Erreur PATCH: "); Serial.println(httpCode);
  }

  http.end();
}

// ----------------- Setup -----------------
void setup() {
  Serial.begin(115200);
  connectWiFi();

  scale.begin(LOADCELL_DOUT, LOADCELL_SCK);
 scale.set_scale(197);
 // ajuster selon votre HX711
  scale.tare();
}

// ----------------- Loop -----------------
void loop() {
  float measuredQuantity = scale.get_units(5); // moyenne sur 5 mesures
  Serial.print("📦 Document: "); Serial.print(targetDocId);
  Serial.print(" - Nouvelle quantité: "); Serial.println(measuredQuantity);

  updateQuantityFirestore(targetDocId, measuredQuantity);
  delay(10000); // attendre 10 secondes avant prochaine mesure
}
