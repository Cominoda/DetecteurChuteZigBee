#include <Wire.h>
#include <MPU6050.h>
#include <SoftwareSerial.h>

SoftwareSerial XBee(10, 11); // TX sur 10, RX sur 11
MPU6050 mpu;

const float CONVERSION = 16384.0;     // Diviseur pour obtenir l'accélération en g
const float SEUIL_CHUTE_LIBRE = 0.3;  // Seuil en g pour la chute libre
const float SEUIL_IMPACT = 2;         // Seuil en g pour l’impact
bool chuteDetectee = false;

void setup() {
    Serial.begin(9600);
    Wire.begin();
    XBee.begin(9600);
    pinMode(13, OUTPUT);
    pinMode(4, INPUT);

    mpu.initialize();
    if (!mpu.testConnection()) {
        Serial.println("❌ Erreur de connexion au MPU6050 !");
        while (1);
    }
}

void loop() {
    int16_t AcX, AcY, AcZ;
    mpu.getAcceleration(&AcX, &AcY, &AcZ);
    if (digitalRead(4) == LOW) { // Si le bouton est pressé
      Serial.print("Ça fonctionne !");
    } 

    // Convertir en "g"
    float accelX = AcX / CONVERSION;
    float accelY = AcY / CONVERSION;
    float accelZ = AcZ / CONVERSION;

    // Calcul de l’accélération totale
    float totalAccel = sqrt(accelX * accelX + accelY * accelY + accelZ * accelZ);

    Serial.print("Accélération : "); Serial.print(totalAccel); Serial.println(" g");

    // Phase 1 : Détection de la chute libre
    if (totalAccel < SEUIL_CHUTE_LIBRE) {
        Serial.println(" Début de chute détecté...");
        XBee.println("{\"ALERTE\": \"Début de chute\", \"X\": " + String(AcX) + ", \"Y\": " + String(AcY) + ", \"Z\": " + String(AcZ) + "}");

        delay(100);  // Attendre pour éviter les faux positifs
    }

    // Phase 2 : Détection de l’impact
    if (totalAccel > SEUIL_IMPACT && !chuteDetectee) {
        Serial.println("🚨 ALERTE : CHUTE DÉTECTÉE !");
        
        // ⚡ Envoi du message à XBee
        XBee.println("{\"ALERTE\": \"CHUTE\", \"X\": " + String(AcX) + ", \"Y\": " + String(AcY) + ", \"Z\": " + String(AcZ) + "}");

        chuteDetectee = true;

        digitalWrite(13, HIGH); // On allume la LED
        delay(3000); // On attend 3 secondes
        digitalWrite(13, LOW); // On ré-éteint la LED
    }
      
    // Réinitialisation après 3 secondes pour détecter une nouvelle chute
    delay(300);
    chuteDetectee = false;
}
