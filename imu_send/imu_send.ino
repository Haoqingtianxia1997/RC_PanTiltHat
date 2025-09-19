#include <Wire.h>
#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>

RF24 radio(10, 9);
const byte address[6] = "00001";

#define ADXL345 0x53
#define HMC5883L 0x1E

// Calibration variables
float pitch_offset = 0.0;
float roll_offset = 0.0;
float yaw_offset = 0.0;
bool calibrated = false;

// Low-pass filter variables
float filtered_roll = 0, filtered_pitch = 0, filtered_yaw = 0;
bool filter_initialized = false;
float alpha = 0.1; // Filter coefficient, can be adjusted as needed

void setup() {
  Serial.begin(115200);
  Wire.begin();

  // Initialize ADXL345
  Wire.beginTransmission(ADXL345);
  Wire.write(0x2D);  
  Wire.write(0x08);  
  Wire.endTransmission();

  // Set ADXL345 range to ±4g
  Wire.beginTransmission(ADXL345);
  Wire.write(0x31);  
  Wire.write(0x01);  
  Wire.endTransmission();

  // Initialize HMC5883L
  Wire.beginTransmission(HMC5883L);
  Wire.write(0x00); // Configuration A
  Wire.write(0x70); // 8 averages, 15Hz, normal measurement
  Wire.endTransmission();

  Wire.beginTransmission(HMC5883L);
  Wire.write(0x01); // Configuration B
  Wire.write(0xA0); // Gain
  Wire.endTransmission();

  Wire.beginTransmission(HMC5883L);
  Wire.write(0x02); // Mode register
  Wire.write(0x00); // Continuous measurement mode
  Wire.endTransmission();

  // Initialize nRF24L01
  radio.begin();
  radio.openWritingPipe(address);
  radio.setPALevel(RF24_PA_LOW);
  radio.setChannel(76);
  radio.setDataRate(RF24_1MBPS);
  radio.stopListening();

  Serial.println("System initialized."); 

  // Delay 5 seconds then calibrate
  delay(5000);
  performCalibration();
}

// Angle low-pass filter function (handles angle jumps)
float angleFilter(float new_angle, float old_filtered, float alpha) {
  float diff = new_angle - old_filtered;
  
  // Handle angle jumps (-180° to 180° wrap-around)
  if (diff > 180) diff -= 360;
  if (diff < -180) diff += 360;
  
  float result = old_filtered + alpha * diff;
  
  // Keep angle within [-180, 180] range
  if (result > 180) result -= 360;
  if (result <= -180) result += 360;
  
  return result;
}

void performCalibration() {
  Serial.println("Starting calibration... Keep device steady for 3 seconds!");
  delay(1000);
  
  float pitch_sum = 0.0;
  float roll_sum = 0.0;
  float yaw_sum = 0.0;
  int sample_count = 0;
  
  unsigned long start_time = millis();
  
  while (millis() - start_time < 3000) { // Collect data for 3 seconds
    // Read sensor data and calculate RPY (reuse main loop code)
    float pitch, roll, yaw;
    if (readSensorsAndCalculateRPY(pitch, roll, yaw)) {
      pitch_sum += pitch;
      roll_sum += roll;
      yaw_sum += yaw;
      sample_count++;
      
      Serial.print("Calibrating... Sample "); 
      Serial.print(sample_count);
      Serial.print(" - P:"); Serial.print(pitch);
      Serial.print(" R:"); Serial.print(roll);
      Serial.print(" Y:"); Serial.println(yaw);
    }
    delay(50); // 20Hz sampling
  }
  
  if (sample_count > 0) {
    pitch_offset = pitch_sum / sample_count;
    roll_offset = roll_sum / sample_count;
    yaw_offset = yaw_sum / sample_count;
    calibrated = true;
    
    Serial.println("Calibration completed!");
    Serial.print("Offsets - Pitch: "); Serial.print(pitch_offset);
    Serial.print(" Roll: "); Serial.print(roll_offset);
    Serial.print(" Yaw: "); Serial.println(yaw_offset);
  } else {
    Serial.println("Calibration failed - no valid samples!");
  }
}

bool readSensorsAndCalculateRPY(float &pitch, float &roll, float &yaw) {
  // Read ADXL345
  Wire.beginTransmission(ADXL345);
  Wire.write(0x32);  
  Wire.endTransmission(false);
  uint8_t bytesRead = Wire.requestFrom(ADXL345, 6, true);
  if (bytesRead < 6) {
    Serial.println("I2C read error: less than 6 bytes (ADXL345)");
    return false;
  }
  uint8_t xl = Wire.read();
  uint8_t xh = Wire.read();
  uint8_t yl = Wire.read();
  uint8_t yh = Wire.read();
  uint8_t zl = Wire.read();
  uint8_t zh = Wire.read();
  int16_t x = (int16_t)((xh << 8) | xl);
  int16_t y = (int16_t)((yh << 8) | yl);
  int16_t z = (int16_t)((zh << 8) | zl);

  // Calculate Pitch and Roll
  pitch = - atan2((float)x, sqrt((float)y * y + (float)z * z)) * 180.0 / PI;
  roll  = atan2((float)y, sqrt((float)x * x + (float)z * z)) * 180.0 / PI;
  float pitch_rad = pitch * PI / 180.0;
  float roll_rad  = roll * PI / 180.0;

  // Read HMC5883L
  Wire.beginTransmission(HMC5883L);
  Wire.write(0x03); // Data register start address
  Wire.endTransmission();
  uint8_t magRead = Wire.requestFrom(HMC5883L, 6);
  if (magRead < 6) {
    Serial.println("I2C read error: less than 6 bytes (HMC5883L)");
    return false;
  }
  int16_t mx = (Wire.read() << 8) | Wire.read(); // X
  int16_t mz = (Wire.read() << 8) | Wire.read(); // Z
  int16_t my = (Wire.read() << 8) | Wire.read(); // Y

  // Tilt compensation
  float mx_f = (float)mx;
  float my_f = (float)my;
  float mz_f = (float)mz;
  float mx2 = mx_f * cos(pitch_rad) + mz_f * sin(pitch_rad);
  float my2 = mx_f * sin(roll_rad) * sin(pitch_rad) + my_f * cos(roll_rad) - mz_f * sin(roll_rad) * cos(pitch_rad);

  yaw = atan2(-my2, mx2) * 180.0 / PI;

  return true;
}

// Angle difference calculation (handles -180 to 180 degree wrap-around)
float angleDifference(float angle, float offset) {
  float diff = angle - offset;
  // Keep angle difference within -180 to 180 degrees
  while (diff > 180.0) diff -= 360.0;
  while (diff < -180.0) diff += 360.0;
  return diff;
}

void loop() {
  float pitch, roll, yaw;
  if (!readSensorsAndCalculateRPY(pitch, roll, yaw)) {
    delay(100);
    return;
  }

  // Print raw data
  Serial.print(" Raw -> Roll: "); Serial.print(roll);
  Serial.print(" Pitch: "); Serial.print(pitch);
  Serial.print(" Yaw: "); Serial.print(yaw);

  // Apply calibration offsets
  float calibrated_pitch, calibrated_roll, calibrated_yaw;
  if (calibrated) {
    calibrated_pitch = - pitch - pitch_offset;
    calibrated_roll = roll - roll_offset;
    calibrated_yaw = angleDifference(yaw, yaw_offset);
    
    Serial.print("| Calibrated -> Roll: "); Serial.print(calibrated_roll);
    Serial.print(" Pitch: "); Serial.print(calibrated_pitch);
    Serial.print(" Yaw: "); Serial.println(calibrated_yaw);

    // Low-pass filtering
    if (!filter_initialized) {
      // First run, initialize with current values
      filtered_roll = calibrated_roll;
      filtered_pitch = calibrated_pitch;
      filtered_yaw = calibrated_yaw;
      filter_initialized = true;
    } else {
      // Use standard low-pass filter for roll and pitch
      filtered_roll = alpha * calibrated_roll + (1 - alpha) * filtered_roll;
      filtered_pitch = alpha * calibrated_pitch + (1 - alpha) * filtered_pitch;
      // Use angle filter for yaw (handles angle jumps)
      filtered_yaw = angleFilter(calibrated_yaw, filtered_yaw, alpha);
    }

    Serial.print("| Filtered -> Roll: "); Serial.print(filtered_roll);
    Serial.print(" Pitch: "); Serial.print(filtered_pitch);
    Serial.print(" Yaw: "); Serial.println(filtered_yaw);
    
    // Pack data (send calibrated data)
    float data[3] = { calibrated_roll, calibrated_pitch, calibrated_yaw };

    // Send
    bool ok = radio.write(&data, sizeof(data));
    Serial.println(ok ? "Send OK" : "Send Fail");

    delay(100);
  } 
}
