// Include the necessary libraries
#include <Arduino.h> // Arduino core library, provides core functions like delay(), digitalRead(), digitalWrite(), etc.
#include <Wire.h>    // Library for I2C communication, used for communicating with I2C devices

#define ESP32_PIN_PH 25 // Define the pin number where the pH sensor is connected

// put interger function declarations here:
int myPhFuction();

// Calibration value for the pH sensor
float ph_calibration_value = 21.34; // This value is used to calibrate the pH sensor, it may vary for different sensors

// Variable to store the pH value
int ph_value = 0; // This variable will hold the pH value read from the sensor

// Variable to store the average value
unsigned long int ph_avg_val; // This variable will hold the average of the pH values read from the sensor

// Array to store the buffer values
int ph_buffer_arr[10], temp; // This array will hold the pH values read in a loop, temp is used for swapping during sorting

// Setup function runs once when you press reset or power the board
void setup()
{
    // Begin serial communication at 115200 baud rate
    Serial.begin(115200); // This starts the serial communication with the baud rate of 115200
}

// Loop function runs over and over again forever
void loop()
{
    int currentPh = myPhFuction();
    // Print current tempreture
    Serial.print("PH is: " + String(currentPh) + "\r\n");
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