// Include the necessary libraries
#include <Arduino.h> // Arduino core library, provides core functions like delay(), digitalRead(), digitalWrite(), etc.
#include <OneWire.h>
#include <DallasTemperature.h>
#include <algorithm>

// Define PINs
#define ESP32_PIN_TEMP 32 // Define the pin number where the temperature sensor is connected
#define ESP32_PIN_PH 25   // Define the pin number where the pH sensor is connected
#define ESP32_PIN_TDS 34  // Define the pin number where the TDS sensor is connected

//-------------------- Temperature --------------------

// Temperature - Setup a oneWire instance to communicate with any OneWire devices (not just Maxim/Dallas temperature ICs)
OneWire oneWire(ESP32_PIN_TEMP);

// Temperature - Pass our oneWire reference to Dallas Temperature.
DallasTemperature sensors(&oneWire);

//-------------------- PH --------------------

// PH - Calibration value for the pH sensor
float ph_calibration_value = 21.34; // This value is used to calibrate the pH sensor, it may vary for different sensors

// PH - Variable to store the pH value
int ph_value = 0; // This variable will hold the pH value read from the sensor

// PH - Variable to store the average value
unsigned long int ph_avg_val; // This variable will hold the average of the pH values read from the sensor

// PH - Array to store the buffer values
int ph_buffer_arr[10], temp; // This array will hold the pH values read in a loop, temp is used for swapping during sorting

//-------------------- TDS --------------------
#define VREF 5.0  // analog reference voltage(Volt) of the ADC
#define SCOUNT 30 // sum of sample point

// Declare arrays to store the analog values read from the ADC
int tds_buffer_tds[SCOUNT];
int tds_buffer_temp[SCOUNT];

// Declare indices for the analog buffer
int tds_buffer_index = 0;
int tds_copy_index = 0;

// Declare variables to store the average voltage, TDS value, and temperature
float tds_average_voltage = 0;
float tds_value = 0;
float tds_temperature = 19.9; // current temperature for compensation

// put interger function declarations here:
int myTemperatureFuction();
int myPhFuction();
int myTdsFuction();

void setup()
{
    // Begin serial communication at 115200 baud
    Serial.begin(115200);

    // Set the TDS sensor pin as an input
    pinMode(ESP32_PIN_TDS, INPUT);

    // Start up the sensors library for Temperature
    sensors.begin();
}

void loop()
{
    // Get Values
    int currentTds = myTdsFuction();
    int currentPh = myPhFuction();
    int currentTemp = myTemperatureFuction();
    // Print Values
    Serial.print("TDS is: " + String(currentTds) + "\r\n");
    Serial.print("PH is: " + String(currentPh) + "\r\n");
    Serial.print("Temperature is: " + String(currentTemp) + "\r\n");
    // Line Break with dashes
    Serial.println("----------------------------------------");
}

int myTemperatureFuction()
{
    sensors.requestTemperatures();
    return sensors.getTempCByIndex(0);
}

int myPhFuction()
{
    // Read the analog value from pin ESP32_PIN_PH 10 times and store it in the buffer array
    for (int i = 0; i < 10; i++) // Loop 10 times
    {
        ph_buffer_arr[i] = analogRead(ESP32_PIN_PH); // Read the analog value from the pH sensor and store it in the buffer array
        delay(30);                                   // Wait for 30 milliseconds before the next reading
    }

    // Sort the buffer array in ascending order
    for (int i = 0; i < 9; i++) // Loop through the array elements
    {
        for (int j = i + 1; j < 10; j++) // Loop through the next elements
        {
            if (ph_buffer_arr[i] > ph_buffer_arr[j]) // If the current element is greater than the next element
            {
                // Swap the elements
                temp = ph_buffer_arr[i];             // Store the current element in temp
                ph_buffer_arr[i] = ph_buffer_arr[j]; // Replace the current element with the next element
                ph_buffer_arr[j] = temp;             // Replace the next element with the value from temp (the original current element)
            }
        }
    }

    ph_avg_val = 0;                                        // Reset the average value
    for (int i = 2; i < 8; i++)                            // Loop through the middle 6 elements of the sorted array (ignoring the 2 smallest and 2 largest values)
        ph_avg_val += ph_buffer_arr[i];                    // Add the current element to the average value
    float ph_volt = (float)ph_avg_val * 5.0 / 1024 / 6;    // Convert the average value to voltage (assuming a 5V reference and 10-bit ADC)
    float ph_act = -5.70 * ph_volt + ph_calibration_value; // Calculate the actual pH value using the calibration value
    return ph_act;
}

int myTdsFuction()
{
    // 1,807 = Read Value Raw
    // 1,620 = Read Value, temp adusted
    // 1,760 = Reading from meter
    // TDS Target is 750 to 1500

    double tds_temperature_coefficient = 0.02;    // temperature coefficient. 0.02°C^-1 is a commonly used coefficient,
    double tds_refference_temperature = 25;       // reference temperature in °C
    double tds_value = analogRead(ESP32_PIN_TDS); // read the analog value from the sensor

    double tds_value_normalised = tds_value * (1 + tds_temperature_coefficient * (tds_temperature - tds_refference_temperature));

    return tds_value_normalised;
}