#include <Arduino.h>
#include <OneWire.h>
#include <DallasTemperature.h>

// Data wire Temperature is plugged into port 32 on the ESP32
#define ESP32_PIN_TEMP 32

// Setup a oneWire instance to communicate with any OneWire devices (not just Maxim/Dallas temperature ICs)
OneWire oneWire(ESP32_PIN_TEMP);

// Pass our oneWire reference to Dallas Temperature.
DallasTemperature sensors(&oneWire);

// put interger function declarations here:
int myTemperatureFuction();

void setup()
{
  // put your setup code here, to run once:
  Serial.begin(115200);

  // Start up the sensors library
  sensors.begin();
}

void loop()
{
  // Get and store current tempreture
  int currentTemp = myTemperatureFuction();
  // Print current tempreture
  Serial.print("Temperature is: " + String(currentTemp) + "\r\n");
}

int myTemperatureFuction()
{
  sensors.requestTemperatures();
  return sensors.getTempCByIndex(0);
}