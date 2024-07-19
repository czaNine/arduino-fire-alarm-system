#include <SoftwareSerial.h>
#include <String.h>
#include <Wire.h>

SoftwareSerial gprsSerial(3, 1);

#define IR_PIN 34
#define GAS_PIN 35
#define BUZZER_PIN 32

int fire_reading = 0;
int gas_reading = 0;

int average_measurement_fire = 115;
int average_measurement_gas = 115;

int alert_level_fire = 90;
int alert_level_gas = 500;

const unsigned long eventInterval = 600000; // 10 minutes
unsigned long previousTime = 0;

void setup()
{
  Serial.begin(9600);

  gprsSerial.begin(9600);
  pinMode(IR_PIN, INPUT);
  pinMode(GAS_PIN, INPUT);
  pinMode(BUZZER_PIN, OUTPUT);
  delay(1000);
}

void loop()
{
  fire_reading = analogRead(IR_PIN) - 800;
  gas_reading = analogRead(GAS_PIN);
  Serial.print("Fire = ");
  Serial.print(fire_reading);
  Serial.print("....Gas = ");
  Serial.println(gas_reading);

  average_measurement_fire = (average_measurement_fire * 99 + fire_reading) / 100;
  average_measurement_gas = (average_measurement_gas * 99 + gas_reading) / 100;

  Serial.print("Ave Fire = ");
  Serial.print(average_measurement_fire);
  Serial.print("....Ave Gas = ");
  Serial.println(average_measurement_gas);

  if (millis() - previousTime >= eventInterval)
  {
    Serial.println("Writing into the ThinkSpeak");
    sendReading(average_measurement_fire, average_measurement_gas);
    previousTime = millis();
  }

  if (average_measurement_fire < alert_level_fire)
  {
    tone(BUZZER_PIN, 2400, 3000);
    sendSMS(fire_reading, "fire");
    triggerCall();
    average_measurement_fire = 115;
  }
  else
  {
    noTone(BUZZER_PIN);
  }

  if (average_measurement_gas > alert_level_gas)
  {
    tone(BUZZER_PIN, 2400, 3000);
    sendSMS(gas_reading, "gas");
    average_measurement_gas = 115;
  }
  else
  {
    noTone(BUZZER_PIN);
  }

  delay(2000);
}

void ShowSerialData() {
  while (gprsSerial.available() != 0)
    Serial.write(gprsSerial.read());
  delay(1000);
}

void triggerCall()
{
  gprsSerial.println("AT");
  delay(1000);
  while (gprsSerial.available())
  {
    Serial.write(gprsSerial.read());
  }
  gprsSerial.println("ATD+639100782108;");
  delay(20000);
  gprsSerial.println("ATH");
  delay(1000);
}

void sendSMS(int reading, String type)
{
  gprsSerial.println("AT+CSCS=\"GSM\"");
  delay(1000);
  while (gprsSerial.available())
  {
    Serial.write(gprsSerial.read());
  }
  delay(2000);
  gprsSerial.println("AT+CMGS=\"+639100782108\"");
  delay(2000);
  gprsSerial.print("****************");
  gprsSerial.println("SMART HOME SECURITY SYSTEM");
  if (type == "gas")
  {
    gprsSerial.println("Gas Leak has been detected!");
  }
  else
  {
    gprsSerial.println("Fire has been detected!");
  }
  gprsSerial.println("Intensity: ");
  gprsSerial.println(reading);
  gprsSerial.println("****************");
  delay(2000);
  gprsSerial.println((char)26);
  delay(5000);
  gprsSerial.println();
  delay(5000);
}

void sendReading(int fire_reading, int gas_reading)
{
  if (gprsSerial.available())
  {
    Serial.write(gprsSerial.read());
  }

  gprsSerial.println("AT");
  delay(1000);
  gprsSerial.println("AT+CPIN?");
  delay(1000);
  gprsSerial.println("AT+CREG?");
  delay(1000);
  gprsSerial.println("AT+CGATT?");
  delay(1000);
  gprsSerial.println("AT+CIPSHUT");
  delay(1000);
  gprsSerial.println("AT+CIPSTATUS");
  delay(1000);
  gprsSerial.println("AT+CIPMUX=0");
  delay(1000);
  while (gprsSerial.available())
  {
    Serial.write(gprsSerial.read());
  }
  gprsSerial.println("AT+CSTT=\"internet\"");
  delay(2000);
  while (gprsSerial.available())
  {
    Serial.write(gprsSerial.read());
  }
  gprsSerial.println("AT+CIICR");
  delay(1000);
  while (gprsSerial.available())
  {
    Serial.write(gprsSerial.read());
  }
  gprsSerial.println("AT+CIFSR");
  delay(2000);
  while (gprsSerial.available())
  {
    Serial.write(gprsSerial.read());
  }
  gprsSerial.println("AT+CIPSPRT=0");
  delay(1000);
  while (gprsSerial.available())
  {
    Serial.write(gprsSerial.read());
  }
  gprsSerial.println("AT+CIPSTART=\"TCP\",\"api.thingspeak.com\",\"80\"");
  delay(3000);
  while (gprsSerial.available())
  {
    Serial.write(gprsSerial.read());
  }
  gprsSerial.println("AT+CIPSEND");
  delay(2000);
  while (gprsSerial.available())
  {
    Serial.write(gprsSerial.read());
  }

  String str = "GET /update?api_key=O3F9FN4450FHX9LO&field1=" + String(fire_reading) + "&field2=" + String(gas_reading);
  gprsSerial.print("AT+CIPSEND=");
  gprsSerial.println(str.length());
  delay(1000);
  while (gprsSerial.available())
  {
    Serial.write(gprsSerial.read());
  }
  gprsSerial.println(str);
  delay(2000);
  while (gprsSerial.available())
  {
    Serial.write(gprsSerial.read());
  }
  gprsSerial.println((char)26);
  delay(3000);
  gprsSerial.println();
  while (gprsSerial.available())
  {
    Serial.write(gprsSerial.read());
  }
  gprsSerial.println("AT+CIPSHUT");
  delay(100);
  while (gprsSerial.available())
  {
    Serial.write(gprsSerial.read());
  }
}