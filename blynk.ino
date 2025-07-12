#define BLYNK_PRINT Serial
#define BLYNK_TEMPLATE_ID "xxxxxx"
#define BLYNK_TEMPLATE_NAME "xxxxx"

#include <WiFi.h>
#include <BlynkSimpleEsp32.h>
#include <ESP32Servo.h> 

// Définition des broches
#define TRIG_PIN 32
#define ECHO_PIN 33
#define SOIL_SENSOR_PIN 34
#define WATER_PUMP_PIN 25
#define IRRIGATION_PUMP_PIN 26
#define LDR1 35
#define LDR2 36
#define LDR3 39
#define LDR4 27  // Correction : évite le conflit avec le capteur de sol
#define BATTERY_PIN 34  // Utilisation correcte de l'ADC ESP32
#define SERVO_X_PIN 14

// Seuils et constantes
const int LEVEL_HIGH = 10;
const int LEVEL_LOW = 17;
const int SOIL_THRESHOLD = 500;

Servo servoX;
Servo servoY;

int posX = 90;
int posY = 90;

float waterLevel;
int soilMoisture;
float batteryVoltage;

// Connexion WiFi & Blynk
char auth[] = "xxxxxx";
char ssid[] = "xxxx";
char pass[] = "xxxxxxxx";

BlynkTimer timer;

// Fonction pour mesurer la distance avec le capteur à ultrasons
long measureDistance() {
  digitalWrite(TRIG_PIN, LOW);
  delayMicroseconds(2);
  digitalWrite(TRIG_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG_PIN, LOW);
  return pulseIn(ECHO_PIN, HIGH) * 0.034 / 2; // Conversion en cm
}

// Fonction pour lire la tension de la batterie
void readBatteryVoltage() {
  int value = analogRead(BATTERY_PIN);
  float vout = value * (3.3 / 4095.0); // Correction : ESP32 a une résolution de 12 bits (0-4095) et 3.3V max
  float R1 = 10000.0, R2 = 100000.0;
  batteryVoltage = vout / (R2 / (R1 + R2));
  Blynk.virtualWrite(V0, batteryVoltage);
  Serial.print("Batterie: "); Serial.println(batteryVoltage);
}

// Fonction pour envoyer les données des capteurs à Blynk
void sendSensorData() {
  waterLevel = measureDistance();
  soilMoisture = analogRead(SOIL_SENSOR_PIN);
  readBatteryVoltage();

  // Affichage des valeurs sur le moniteur série
  Serial.print("Niveau d'eau: "); Serial.print(waterLevel); Serial.println(" cm");
  Serial.print("Humidité du sol: "); Serial.println(soilMoisture);

  // Envoi vers Blynk
  Blynk.virtualWrite(V1, soilMoisture);
  Blynk.virtualWrite(V2, posX);
  Blynk.virtualWrite(V3, waterLevel);
}

// Commande de la pompe d'irrigation via Blynk
BLYNK_WRITE(V4) {
  int irrigationState = param.asInt();
  digitalWrite(IRRIGATION_PUMP_PIN, irrigationState);
  Serial.print("Pompe d'irrigation : "); Serial.println(irrigationState ? "ON" : "OFF");
}

// Commande du servo via Blynk
BLYNK_WRITE(V5) {
  posX = param.asInt();
  servoX.write(posX);
  Blynk.virtualWrite(V2, posX);
  Serial.print("Servo X position: "); Serial.println(posX);
}

void setup() {
  Serial.begin(115200);
  Blynk.begin(auth, ssid, pass);

  // Configuration des broches
  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);
  pinMode(WATER_PUMP_PIN, OUTPUT);
  pinMode(IRRIGATION_PUMP_PIN, OUTPUT);
  pinMode(BATTERY_PIN, INPUT);

  // Configuration du servo
  servoX.attach(SERVO_X_PIN);
  servoX.write(posX);
  servoY.attach(27);  // Attach servo to GPIO 27

  // Lancement du timer pour l'envoi des données toutes les 2 secondes
  timer.setInterval(2000L, sendSensorData);
}

void loop() {
  Blynk.run();
  timer.run();

  // Gestion automatique de la pompe à eau
  if (waterLevel >= LEVEL_LOW) {
    digitalWrite(WATER_PUMP_PIN, HIGH);
    Serial.println("Pompe à eau : ON");
  } else if (waterLevel <= LEVEL_HIGH) {
    digitalWrite(WATER_PUMP_PIN, LOW);
    Serial.println("Pompe à eau : OFF");
  }
}
