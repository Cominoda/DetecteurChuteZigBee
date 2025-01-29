#include <Wire.h>
#include <MPU6050.h>
#include <SoftwareSerial.h>

SoftwareSerial XBee(10, 11); // TX sur 10, RX sur 11
MPU6050 mpu;

const float SEUIL_CHUTE_LIBRE = 0.3;  // Seuil en g pour la chute libre
const float SEUIL_IMPACT = 2;       // Seuil en g pour l’impact
bool chuteDetectee = false;

void setup() {
    Serial.begin(9600);
    Wire.begin();
    XBee.begin(9600);

    mpu.initialize();
    if (!mpu.testConnection()) {
        Serial.println("❌ Erreur de connexion au MPU6050 !");
        while (1);
    }
}

void loop() {
    int16_t AcX, AcY, AcZ;
    mpu.getAcceleration(&AcX, &AcY, &AcZ);

    // Convertir en "g"
    float accelX = AcX / 16384.0;
    float accelY = AcY / 16384.0;
    float accelZ = AcZ / 16384.0;

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
    }
      
    // Réinitialisation après 3 secondes pour détecter une nouvelle chute
    delay(300);
    chuteDetectee = false;
}
