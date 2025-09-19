# RC_PanTiltHat

<p align="center">
  <img src="/assets/experiment setup.jpeg" alt="Fig.1 Experiment Setup" width="600">
</p>
<p align="center"><strong>Fig.1 Experiment Setup</strong></p>

## Project goal:

build a pipeline to control a pan tilt hat with IMU, which is the foundation of remote camera control on First-Person-View(FPV) drones. In the scope of the project at the university there's budget limitation so we couldn't make it more complicated. This could also be extended to the teleoperation for robots once inertial navigation is applied with IMU sensor datas. 

## Components: 

GY-85 9-axis IMU sensor

- Components: gryoscope ITG3205, accelerometer ADXL345 and magnetometer HMC5883L.
- Power: 3-5V. 
- Communication: I2C. 
- Link: 

RF-Nano V3.0 x 2

- Arduino nano variant, one as sender, which connects an IMU; the other functions as a receiver. This model has integrated nRF24L01 wireless connection module.\
- Link:  

Pan tilt hat (by Pimoroni)

- 2 degrees of freedom(DoF), use SG90 servo motors
- Link: https://shop.pimoroni.com/products/pan-tilt-hat?variant=22408353287

## Pin Connection:

Note: For this specific rf-nano model, in order to enable wireless connection, set CE to D10 and CSN to D9. There're many variants on the market. Some with CE-->D7&CSN-->D8 combination, some in reverse(CE --> D8 & CSN --> D7), and some with CE --> D9 & CSN --> D10. Extra attention is needed before make the wireless connection work. 

### send:

| IMU | RF-Nano | 
| :---: | :---: |
| SCL | A5 |
| SDA | A4 |
| VCC | 5V |
| GND | GND |

### receive:

| lower servo motor | RF-Nano |
| :---: | :---: |
| PWM | A0 |
| VCC | 4.8V(~5V) |
| GND | GND |

| higher servo motor | RF-Nano |
| :---: | :---: |
| PWM | A1 |
| VCC | 4.8V(~5V) |
| GND | GND |

External power supply is used for the motors in my experiments. Send part and receive part use separate RF-Nano boards.

## Usage:

run ./imu_send/imu_send.ino and imu_receive/imu_receive.ino. 

You can see in the Serial Monitor in Arduino IDE that there'll be a countdown for initialization. Place the imu still(preferably on a flat surface) and wait for the calibration to complete. Then the roll, pitch and yaw angle will be calculated based on the initialization pose rather than world coordinates, because accelerometer and magnetometer uses absolute measurement values. 

## TODO:

1. Use cross platform communication as the receiver board, e.g. stm32f103c8t6. Servo motor control has already been done. Configuration for nRF24L01 to the stm32 board needs extra work. 
2. Incorporate gyroscope measurements to improve accuracy of yaw angle. 
3. Explore limitation of microcontrollers by applying more computationally expensive filters to further smooth out the data fluctuations.
4. Add schematics and PCB design.
