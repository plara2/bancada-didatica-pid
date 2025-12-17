#include <SPI.h>
#include <max6675.h>

const int thermoDO = 12;              
const int thermoCS = 10;              
const int thermoCLK = 13;             
const int SSR_PIN = 9;                
const unsigned long SAMPLE_MS = 1000; // agora imprime a cada 1s

double SETPOINT = 50.0;
double HYST = 0.5;

MAX6675 thermocouple(thermoCLK, thermoCS, thermoDO);

unsigned long lastSample = 0;
unsigned long elapsedSeconds = 0; // contador de tempo iniciado em 0
bool actuatorOn = false;

void setup() {
  Serial.begin(115200);
  pinMode(SSR_PIN, OUTPUT);
  digitalWrite(SSR_PIN, LOW);
  delay(500);

  thermocouple.readCelsius(); // descarta primeira leitura

  Serial.println("time_s,temperature_C,setpoint,actuator,event");
}

double readTempFiltered() {
  return thermocouple.readCelsius();
}

void setActuator(bool on) {
  actuatorOn = on;
  digitalWrite(SSR_PIN, on ? HIGH : LOW);
}

void loop() {
  unsigned long now = millis();
  if (now - lastSample >= SAMPLE_MS) {
    lastSample = now;

    elapsedSeconds++; // primeira impressão será 1s

    double T = readTempFiltered();

    // erro de leitura
    if (isnan(T)) {
      setActuator(false);
      Serial.print(elapsedSeconds);
      Serial.print(",nan,");
      Serial.print(SETPOINT);
      Serial.print(",0,ERROR_nan\n");
      return;
    }

    // controle ON/OFF com histerese
    if (!actuatorOn && T <= (SETPOINT - HYST)) {
      setActuator(true);
      Serial.print(elapsedSeconds);
      Serial.print(",");
      Serial.print(T, 3);
      Serial.print(",");
      Serial.print(SETPOINT);
      Serial.print(",");
      Serial.print(actuatorOn);
      Serial.println(",ON_from_below");
    }
    else if (actuatorOn && T >= (SETPOINT + HYST)) {
      setActuator(false);
      Serial.print(elapsedSeconds);
      Serial.print(",");
      Serial.print(T, 3);
      Serial.print(",");
      Serial.print(SETPOINT);
      Serial.print(",");
      Serial.print(actuatorOn);
      Serial.println(",OFF_from_above");
    }
    else {
      Serial.print(elapsedSeconds);
      Serial.print(",");
      Serial.print(T, 3);
      Serial.print(",");
      Serial.print(SETPOINT);
      Serial.print(",");
      Serial.print(actuatorOn);
      Serial.println(",");
    }
  }
}
