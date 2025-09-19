#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>
#include <Servo.h>

// Create two servo objects
Servo servo_ch2;  
Servo servo_ch3; 
// Define servo connection pins
const int pin_ch2 = A0;  // Servo 1 on A0
const int pin_ch3 = A1;  // Servo 2 on A1

void SetServoAngle(float angle_ch2, float angle_ch3, uint16_t delay_ms)
{
  // Calculate pulse width (STM32 version uses 50~250 units, Arduino uses microseconds)
  int pwm_ch2 = (int)(1425 + (angle_ch2 * 1000.0) / 90.0);  // Center 1500, ±90° corresponds to 500~2500
  int pwm_ch3 = (int)(1200 - (angle_ch3 * 1000.0) / 90.0);  // Pitch direction reversed

  // Limit range to prevent exceeding servo capability
  if (pwm_ch2 < 500) pwm_ch2 = 500;
  if (pwm_ch2 > 2500) pwm_ch2 = 2500;
  if (pwm_ch3 < 500) pwm_ch3 = 500;
  if (pwm_ch3 > 2500) pwm_ch3 = 2500;

  // Output to servos
  servo_ch2.writeMicroseconds(pwm_ch2);
  servo_ch3.writeMicroseconds(pwm_ch3);

  delay(delay_ms);
}

// RF-Nano onboard nRF24L01 CE, CSN pins
RF24 radio(10, 9);
const byte address[6] = "00001";

// Array for receiving data
float data[3]; // [0]: Roll, [1]: Pitch, [2]: Yaw

void setup() {
  // Initialize servos
  servo_ch2.attach(pin_ch2);
  servo_ch3.attach(pin_ch3);

  // Move to center position
  SetServoAngle(0, 0, 1000);

  Serial.begin(115200);

  // Initialize nRF24L01
  radio.begin();
  radio.openReadingPipe(1, address);
  radio.setPALevel(RF24_PA_LOW);
  radio.setChannel(76);
  radio.setDataRate(RF24_1MBPS);
  radio.startListening();         // Receive mode

  Serial.println("nRF24L01 Receiver Ready.");
}

void loop() {
  if (radio.available()) {
    radio.read(&data, sizeof(data));

    // Check data validity
    if (isnan(data[0]) || isnan(data[1]) || isnan(data[2]) ||
        isinf(data[0]) || isinf(data[1]) || isinf(data[2])) {
      Serial.println("Recv -> Invalid data (NaN or Inf)");
      return;
    }

    // Check data range (assuming pitch/roll in -180~180, yaw in 0~360)
    if (abs(data[0]) > 180 || abs(data[1]) > 180 || abs(data[2]) > 180) {
      Serial.println("Recv -> Data overflow detected");
      return;
    }

    SetServoAngle(data[2], data[1], 0);

    // Print attitude angles
    Serial.print("Recv ->  Roll: ");
    Serial.print(data[0], 2);
    Serial.print("  Pitch: ");
    Serial.print(data[1], 2);
    Serial.print("  Yaw: ");
    Serial.println(data[2], 2);
  }
}
