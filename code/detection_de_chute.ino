#include <Wire.h>
#include <MPU6050.h>
#include <SoftwareSerial.h>

SoftwareSerial XBee(10, 11); // TX sur 10, RX sur 11
MPU6050 mpu;

const float CONVERSION = 16384.0;     // Diviseur pour obtenir l'accélération en g
const float SEUIL_CHUTE_LIBRE = 0.3;  // Seuil en g pour la chute libre
const float SEUIL_IMPACT = 2;         // Seuil en g pour l’impact
const int BUTTON_PIN = 4;
const int LED_PIN = 12;
const int TEMPS_LED = 10000;
int buttonMillis = 0;
int currentMillis = 0;
bool chuteDetectee = false;
bool ledAllume = false;

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
    if ((ledAllume == true) && digitalRead(BUTTON_PIN) == LOW) {
      digitalWrite(LED_PIN, LOW); // On éteint la LED
      ledAllume = false;
      XBee.println("Fausse alerte");
      chuteDetectee = false;
    }

    // Calcul de l’accélération totale
    totalAccel = sqrt(accelX * accelX + accelY * accelY + accelZ * accelZ);

    currentMillis = millis();
    if ((ledAllume == true) && (currentMillis - buttonMillis > TEMPS_LED)) {
        digitalWrite(LED_PIN, LOW); // On éteint la LED
        ledAllume = false;
    }

    Serial.print("Accélération : "); Serial.print(totalAccel); Serial.println(" g");

    // Phase 1 : Détection de la chute libre
    if (totalAccel < SEUIL_CHUTE_LIBRE) {
        char message[50];
        sprintf(message, "ALERTE_DEBUT_CHUTE;X:%d;Y:%d;Z:%d", AcX, AcY, AcZ);
        Serial.println("🚀 Envoi : " + String(message));  // Debug dans le Moniteur Série
        XBee.println(message);
        delay(100);  // Attendre pour éviter les faux positifs
    }

    // Phase 2 : Détection de l’impact
    if (totalAccel > SEUIL_IMPACT && !chuteDetectee) {
        char message[50];
        sprintf(message, "ALERTE_CHUTE;X:%d;Y:%d;Z:%d", AcX, AcY, AcZ);
        Serial.println("🚀 Envoi : " + String(message));  // Debug dans le Moniteur Série
        XBee.println(message);
        digitalWrite(LED_PIN, HIGH); // On allume la LED
        buttonMillis = currentMillis; // reset the clock
        ledAllume = true;
        chuteDetectee = false;
    }
      
    // Réinitialisation après 0.3 secondes pour détecter une nouvelle chute
    delay(300);
    chuteDetectee = false;
}
