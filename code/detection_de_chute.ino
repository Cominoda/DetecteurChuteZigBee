#include <Wire.h>
#include <MPU6050.h>
#include <SoftwareSerial.h>

SoftwareSerial XBee(10, 11); // TX sur 10, RX sur 11
MPU6050 mpu;

const float CONVERSION = 16384.0;     // Diviseur pour obtenir l'accélération en g
const float SEUIL_CHUTE_LIBRE = 1.5;  // Seuil en g pour la chute libre
const float SEUIL_IMPACT = 2;         // Seuil en g pour l’impact
const int BUTTON_PIN = 4;
const int LED_PIN = 12;
const int TEMPS_ATTENTE_ALERTE = 10000; // 10 secondes
const int TEMPS_LED = 10000;
int buttonMillis = 0;
int currentMillis = 0;
bool chuteDetectee = false;
bool ledAllume = false;
bool alerteAnnulee = false;
char message[50];

void setup() {
    Serial.begin(9600);
    Wire.begin();
    XBee.begin(9600);
    pinMode(LED_PIN, OUTPUT);
    pinMode(BUTTON_PIN, INPUT);

    mpu.initialize();
    if (!mpu.testConnection()) {
        Serial.println("❌ Erreur de connexion au MPU6050 !");
        while (1);
    }
}

void loop() {
    int16_t AcX, AcY, AcZ; 
    float accelX, accelY, accelZ, totalAccel;
    mpu.getAcceleration(&AcX, &AcY, &AcZ);

    // Convertir en "g"
    accelX = AcX / CONVERSION;
    accelY = AcY / CONVERSION;
    accelZ = AcZ / CONVERSION;

    // Détection du bouton pour éteindre la LED + envoyer la fausse alerte
    if (digitalRead(BUTTON_PIN) == LOW) {
        alerteAnnulee = true;
        digitalWrite(LED_PIN, LOW); // On éteint la LED
        Serial.print("ok1");
        ledAllume = false;
        Serial.print("ok");
        XBee.println("FAUSSE_ALERTE");
        Serial.print("ok2");
        chuteDetectee = true;
    }

    // Calcul de l’accélération totale
    totalAccel = sqrt(accelX * accelX + accelY * accelY + accelZ * accelZ);

    currentMillis = millis();
    if (ledAllume && (currentMillis - buttonMillis > TEMPS_LED)) {
        digitalWrite(LED_PIN, LOW); // On éteint la LED
        ledAllume = false;
        XBee.println(message);
    }

    Serial.print("Accélération : "); Serial.print(totalAccel); Serial.println(" g");

    // Phase 1 : Détection de la chute libre
    if (totalAccel > SEUIL_CHUTE_LIBRE && totalAccel < SEUIL_IMPACT) {
      alerteAnnulee = false;
      sprintf(message, "ALERTE_DEBUT_CHUTE;X:%d;Y:%d;Z:%d", AcX, AcY, AcZ);
      Serial.println("🚀 Envoi : " + String(message));  // Debug dans le Moniteur Série  
    }

    // Phase 2 : Détection de l’impact
    if (totalAccel > SEUIL_IMPACT) {
        digitalWrite(LED_PIN, HIGH); // On allume la LED
        Serial.println("🚀 chute 10s : " );  // Debug dans le Moniteur Série
        delay(TEMPS_ATTENTE_ALERTE); // Attendre 10 secondes avant d'envoyer l'alerte

        if (!alerteAnnulee) {
          sprintf(message, "ALERTE_CHUTE;X:%d;Y:%d;Z:%d", AcX, AcY, AcZ);
          Serial.println("🚀 Envoi : " + String(message));  // Debug dans le Moniteur Série
          buttonMillis = currentMillis; // reset the clock
          ledAllume = true;
        }
    }
      
    // Réinitialisation après 0.3 secondes pour détecter une nouvelle chute
    delay(300);
    chuteDetectee = false;
}
