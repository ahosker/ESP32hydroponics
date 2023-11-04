// Include the Arduino library, which provides core Arduino functionality
#include <Arduino.h>

// Define constants for the TDS sensor pin, the reference voltage for the ADC, and the number of sample points
#define ESP32_PIN_TDS 34
#define VREF 5.0  // analog reference voltage(Volt) of the ADC
#define SCOUNT 30 // sum of sample point

// put interger function declarations here:
int myTdsFuction();

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

// Function to perform median filtering on the analog values
int getMedianNum(int bArray[], int iFilterLen)
{
    // Create a temporary array to store the values for sorting
    int bTab[iFilterLen];
    for (byte i = 0; i < iFilterLen; i++)
        bTab[i] = bArray[i];

    // Sort the array using bubble sort
    int i, j, bTemp;
    for (j = 0; j < iFilterLen - 1; j++)
    {
        for (i = 0; i < iFilterLen - j - 1; i++)
        {
            if (bTab[i] > bTab[i + 1])
            {
                // Swap the elements
                bTemp = bTab[i];
                bTab[i] = bTab[i + 1];
                bTab[i + 1] = bTemp;
            }
        }
    }

    // Calculate the median of the sorted array
    if ((iFilterLen & 1) > 0)
    {
        bTemp = bTab[(iFilterLen - 1) / 2];
    }
    else
    {
        bTemp = (bTab[iFilterLen / 2] + bTab[iFilterLen / 2 - 1]) / 2;
    }
    return bTemp;
}

// Setup function, runs once when the Arduino is powered on or reset
void setup()
{
    // Begin serial communication at 115200 baud
    Serial.begin(115200);

    // Set the TDS sensor pin as an input
    pinMode(ESP32_PIN_TDS, INPUT);
}

// Loop function, runs continuously after setup() completes
void loop()
{
    int currentTds = myTdsFuction();
    // Print current tempreture
    Serial.print("TDS is: " + String(currentTds) + "\r\n");
}

int myTdsFuction()
{
    // Declare a static variable to store the last time the analog value was sampled
    static unsigned long analogSampleTimepoint = millis();

    // If it's been more than 40 milliseconds since the last sample, take a new sample
    if (millis() - analogSampleTimepoint > 40U)
    {
        analogSampleTimepoint = millis();
        tds_buffer_tds[tds_buffer_index] = analogRead(ESP32_PIN_TDS); // read the analog value and store into the buffer
        tds_buffer_index++;
        if (tds_buffer_index == SCOUNT)
        {
            tds_buffer_index = 0;
        }
    }

    // Declare a static variable to store the last time the TDS value was printed
    static unsigned long printTimepoint = millis();

    // If it's been more than 800 milliseconds since the last print, calculate and print a new TDS value
    if (millis() - printTimepoint > 800U)
    {
        printTimepoint = millis();
        for (tds_copy_index = 0; tds_copy_index < SCOUNT; tds_copy_index++)
        {
            tds_buffer_temp[tds_copy_index] = tds_buffer_tds[tds_copy_index];

            // Calculate the average voltage by applying the median filter to the analog values and converting to voltage
            tds_average_voltage = getMedianNum(tds_buffer_temp, SCOUNT) * (float)VREF / 1024.0;

            // Calculate the compensation coefficient based on the current temperature
            float compensationCoefficient = 1.0 + 0.02 * (tds_temperature - 25.0);

            // Apply the temperature compensation to the average voltage
            float compensationVoltage = tds_average_voltage / compensationCoefficient;

            // Calculate the TDS value based on the compensated voltage
            tds_value = (133.42 * compensationVoltage * compensationVoltage * compensationVoltage - 255.86 * compensationVoltage * compensationVoltage + 857.39 * compensationVoltage) * 0.5;

            // Print the TDS value to the serial monitor
            // Serial.print("TDS Value:");
            // Serial.print(tds_value, 0);
            // Serial.println("ppm");
            return tds_value;
        }
    }
}