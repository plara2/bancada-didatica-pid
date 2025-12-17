#include <SPI.h>
#include <max6675.h>
#include <PID_v1.h>

// MAX6675
int thermoSO  = 12;
int thermoCS  = 10;
int thermoSCK = 13;
MAX6675 thermocouple(thermoSCK, thermoCS, thermoSO);

// SSR
#define RELAY_PIN 9

// PID
double Setpoint, Input, Output;

// PARA ACHAR KU E TU (Ziegler–Nichols)
double Kp = 54.0;   // ajuste esse valor até oscilar
double Ki = 0.64;     // Z–N etapa 1
double Kd = 1134.0;

// PWM lento
unsigned long windowSize = 8000; 
unsigned long windowStartTime;
unsigned long lastPidComputeTime = 0;
unsigned long pidSampleTime = 250; 

PID myPID(&Input, &Output, &Setpoint, Kp, Ki, Kd, DIRECT);

void setup() {
  Serial.begin(115200);
  delay(500);

  Setpoint = 50.0;   // temperatura alvo para o teste

  // Verifica termopar
  Input = thermocouple.readCelsius();
  while (isnan(Input)) {
    delay(2000);
    Input = thermocouple.readCelsius();
  }

  pinMode(RELAY_PIN, OUTPUT);
  digitalWrite(RELAY_PIN, LOW);

  myPID.SetOutputLimits(0, windowSize);
  myPID.SetSampleTime(pidSampleTime);
  myPID.SetMode(AUTOMATIC);

  windowStartTime = millis();
}

void loop() {
  unsigned long now = millis();

  // PID (a cada pidSampleTime = 250 ms)
  if (now - lastPidComputeTime >= pidSampleTime) {
    lastPidComputeTime = now;

    double tempC = thermocouple.readCelsius();

    if (!isnan(tempC))
      Input = tempC;
    else {
      // segurança: desliga SSR
      digitalWrite(RELAY_PIN, LOW);
    }

    myPID.Compute();
  }

  // IMPRESSÃO EM CSV PARA PuTTY (a cada 1 segundo)
  static unsigned long lastPrint = 0;
  if (now - lastPrint >= 1000) {    // <-- igual ao código 2
    lastPrint = now;

    // Formato: tempo(s),temperatura,setpoint
    Serial.print(now / 1000.0, 2);
    Serial.print(",");
    Serial.print(Input);
    Serial.print(",");
    Serial.println(Setpoint);
  }

  // PWM lento do SSR
  if (now - windowStartTime > windowSize)
    windowStartTime += windowSize;

  if ((now - windowStartTime) < Output)
    digitalWrite(RELAY_PIN, HIGH);
  else
    digitalWrite(RELAY_PIN, LOW);
}
