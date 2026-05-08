#include <Wire.h>
#include <I2Cdev.h>
#include <MPU6050.h>
#include <LedControl.h>

#define GREEN_MATRIX_PIN 7 // Transistor control for Green Matrix

// MAX7219 Setup
int DIN  = 12;
int CS   = 11;
int CLK  = 10;
int numDevices = 4;  // Number of MAX7219 modules

MPU6050 mpu;
LedControl lc = LedControl(DIN, CLK, CS, numDevices - 1);

int16_t ax, ay, az;
int16_t gx, gy, gz;

struct MyData {
  byte X;
  byte Y;
  byte Z;
};

MyData data;

void setup() {
    Serial.begin(9600);
    Wire.begin();
    mpu.initialize();
    pinMode(GREEN_MATRIX_PIN, OUTPUT); // Set Green Matrix Pin as OUTPUT

    for (int i = 0; i < numDevices; i++) {
        lc.shutdown(i, false);  // Wake up each MAX7219
        lc.setIntensity(i, 1);  // Set brightness level (0-15)
        lc.clearDisplay(i);     // Clear display
    }
}

void loop() {
    // Read MPU6050 Accelerometer & Gyroscope Data
    mpu.getMotion6(&ax, &ay, &az, &gx, &gy, &gz);

    // Map values for better usability
    data.X = map(ax, -17000, 17000, 0, 255);
    data.Y = map(ay, -17000, 17000, 0, 255);
    data.Z = map(az, -17000, 17000, 0, 255);

    // Print values every 500ms for debugging
    static unsigned long lastPrint = 0;
    if (millis() - lastPrint > 500) {
        Serial.print("X = ");
        Serial.print(data.X);
        Serial.print(" | Y = ");
        Serial.print(data.Y);
        Serial.print(" | Z = ");
        Serial.println(data.Z);
        lastPrint = millis();
    }

    // **GESTURE 1: STOP (RED MATRIX ON)**
    if (data.Y > 145 && data.Z > 140) {  
        Serial.println("STOP GESTURE - Turning ON RED MATRIX");
        for (int dev = 0; dev < numDevices; dev++) { 
            for (int row = 0; row < 8; row++) {
                lc.setRow(dev, row, 0xFF);  // Turn on Red Matrices
            }
        }
        digitalWrite(GREEN_MATRIX_PIN, LOW); // Ensure Green Matrix is OFF
    }

    // **GESTURE 2: CALLING / GO (GREEN MATRIX ON)**
    else if (data.X < 50 && data.Y < 135 && data.Z > 120) {  
        Serial.println("GO GESTURE - Turning ON GREEN MATRIX");
        digitalWrite(GREEN_MATRIX_PIN, HIGH);
        for (int i = 0; i < numDevices; i++) {
            lc.clearDisplay(i);  // Ensure Red Matrices are OFF
        }
    }

    // **GESTURE 3: RESET TO NEUTRAL (TURN OFF BOTH)**
    else if (data.Y < 80) {  
        Serial.println("Neutral Position - Turning OFF BOTH");
        digitalWrite(GREEN_MATRIX_PIN, LOW);
        for (int i = 0; i < numDevices; i++) {
            lc.clearDisplay(i);  
        }
    }
}
