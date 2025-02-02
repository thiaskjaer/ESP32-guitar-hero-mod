#define BOUNCE_WITH_PROMPT_DETECTION // Make button state changes available immediately

#include <Arduino.h>
#include <Bounce2.h>    // https://github.com/thomasfredericks/Bounce2
#include <BleGamepad.h> // https://github.com/lemmingDev/ESP32-BLE-Gamepad

#define TOTAL_BUTTONS 9

// Map pins to corresponding button
#define GREEN 16
#define RED 17
#define YELLOW 18
#define BLUE 19
#define ORANGE 33
#define START 26
#define DOWN 27
#define UP 32
#define SELECT 25

const byte buttonPins[TOTAL_BUTTONS] = {GREEN, RED, YELLOW, BLUE, ORANGE, START, DOWN, UP, SELECT};
const byte physicalButtons[TOTAL_BUTTONS] = {1, 2, 3, 4, 5, 6, 7, 8, 9};

const int whammyPin = 34;
int previousWhammyVal = 0;

BleGamepad bleGamepad("Thias' Guitar", "Ildis6 Inc©", 100);
Bounce debouncers[TOTAL_BUTTONS];

void setup()
{
    for (byte currentPinIndex = 0; currentPinIndex < TOTAL_BUTTONS; currentPinIndex++)
    {
        pinMode(buttonPins[currentPinIndex], INPUT_PULLUP);

        debouncers[currentPinIndex] = Bounce();
        debouncers[currentPinIndex].attach(buttonPins[currentPinIndex]);
        debouncers[currentPinIndex].interval(10); 
    }

    BleGamepadConfiguration bleGamepadConfig;  
    bleGamepadConfig.setButtonCount(TOTAL_BUTTONS);
    bleGamepadConfig.setAutoReport(false);
    bleGamepad.begin(&bleGamepadConfig);

    Serial.begin(115200);
}

void loop()
{
    if (bleGamepad.isConnected())
    {
        bool sendReport = false;

        for (byte currentIndex = 0; currentIndex < TOTAL_BUTTONS; currentIndex++)
        {
            debouncers[currentIndex].update();

            if (debouncers[currentIndex].fell())
            {
                bleGamepad.press(physicalButtons[currentIndex]);
                sendReport = true;
                Serial.printf("Button %d pushed.\n", physicalButtons[currentIndex]);
            }
            else if (debouncers[currentIndex].rose())
            {
                bleGamepad.release(physicalButtons[currentIndex]);
                sendReport = true;
                Serial.printf("Button %d released.\n", physicalButtons[currentIndex]);
            }
        }

        int currentWhammyVal = analogRead(whammyPin);
        if(previousWhammyVal != currentWhammyVal){
            Serial.printf("Whammy Val: %d\n", currentWhammyVal);
            bleGamepad.setRY(map(currentWhammyVal, 4095, 0, 0, 32767)); // resting at 4095
            sendReport = true;
            previousWhammyVal = currentWhammyVal;
        }
        if (sendReport)
        {
            bleGamepad.sendReport();
        }

        // delay(20);	// (Un)comment to remove/add delay between loops
    }
}
