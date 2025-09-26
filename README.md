# RC_PanTiltHat

<p align="center">
  <img src="/assets/experiment setup.jpeg" alt="Fig.1 Experiment Setup" width="600">
</p>
<p align="center"><strong>Fig.1 Experiment Setup</strong></p>

## Project goal

build a pipeline to control a pan tilt hat with IMU, which is the foundation of remote camera control on First-Person-View(FPV) drones. In the scope of the project at the university there's budget limitation so we couldn't make it more complicated. This could also be extended to the teleoperation for robots once inertial navigation is applied with IMU sensor datas.

## Components

GY-85 9-axis IMU sensor

- Components: gryoscope ITG3205, accelerometer ADXL345 and magnetometer HMC5883L.
- Power: 3-5V.
- Communication: I2C.
- Amazon link: https://www.amazon.de/GY-85-sensor-module-6DOF-SENSOR/dp/B0D48MPR19?source=ps-sl-shoppingads-lpcontext&psc=1&smid=A2SQH2225NS6H6
- Taobao link: https://e.tb.cn/h.Sd50L8fu2FfUhqk?tk=HCfv4FcyEOR

RF-Nano V3.0 x 2

- Arduino nano variant, one as sender, which connects an IMU; the other functions as a receiver. This model has integrated nRF24L01 wireless connection module.
- Amazon link: https://www.amazon.de/Diarypiece-Zuverl%C3%A4ssige-Rf-Nano-kompatible-Serial-Modul-Boards-Nrf24L01-Module/dp/B0C86K2XJ2/ref=sr_1_10?__mk_de_DE=%C3%85M%C3%85%C5%BD%C3%95%C3%91&crid=1SAMN9R8XCEQ&dib=eyJ2IjoiMSJ9.goaA1vfqB1eC_mw_N2c2OAP9EBYg4L9-RcnRC_rq5xaUjs44Ir0x0WwYSboH1OAnZ56PFJmOcfpCwdPcWG-34l1hWk32OjfxaFVWhrRbgI3W5wIyMgKkpkUL33txujTp25X3MPgcjQhgfPia0WiBwxlXSYFQYXKzkSAt6jITdLsBoIi193KY3SWps3k3H3SG4szIl109zKEPLS2Wp_TD-3FZVDfR_2TxOvZlwbU4m4s.uuvsObRRE1CW60LW1eEMmHXaiLcw4ogHkG1x5IfwK10&dib_tag=se&keywords=rf+nano+nrf24l01&qid=1758303927&sprefix=rf+nano+nrf24l01%2Caps%2C131&sr=8-10
- Taobao link: https://e.tb.cn/h.S2h3lEbp76J7IZo?tk=IXUa4FcCmxC

Pan tilt hat (by Pimoroni)

- 2 degrees of freedom(DoF), use SG90 servo motors
- Link: https://shop.pimoroni.com/products/pan-tilt-hat?variant=22408353287

ESP32 based WiFi network camera

- Taobao Link：https://item.taobao.com/item.htm?abbucket=10&detail_redpacket_pop=true&id=814538029198&ltk2=1746736425589qym9w3voqjlac4o8wvua&ns=1&priceTId=2100c89117467361605063657e0bf4&query=wifi%E6%91%84%E5%83%8F%E5%A4%B4&skuId=5511308229605&spm=a21n57.1.hoverItem.2&utparam=%7B%22aplus_abtest%22%3A%22d5afbfdf46d22d7b5c3b3b61590d27a5%22%7D&xxc=taobaoSearch

3D printed connectors

- You can find it in the document and print it yourself.

## Pin Connection

Note: For this specific rf-nano model, in order to enable wireless connection, set CE to D10 and CSN to D9. There're many variants on the market. Some with CE-->D7&CSN-->D8 combination, some in reverse(CE --> D8 & CSN --> D7), and some with CE --> D9 & CSN --> D10. Extra attention is needed before make the wireless connection work. You can find the schematic in the folder "Circuit Schematic".

### send

| IMU | RF-Nano |
| :---: | :---: |
| SCL | A5 |
| SDA | A4 |
| VCC | 5V |
| GND | GND |

### receive

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

## Usage

run `./imu_send/imu_send.ino` and `imu_receive/imu_receive.ino`.

You can see in the Serial Monitor in Arduino IDE that there'll be a countdown for initialization. Place the imu still(preferably on a flat surface) and wait for the calibration to complete. Then the roll, pitch and yaw angle will be calculated based on the initialization pose rather than world coordinates, because accelerometer and magnetometer uses absolute measurement values.

## PCB
As shown in the figure below, if you want to use an integrated circuit board instead of flying leads, you can print the circuit board using the given PCB design.
<p align="center">
  <img src="/assets/Transmitter.png" alt="Fig.2 Transmiiter PCB Setup" width="600">
</p>
<p align="center"><strong>Fig.2 Transmiiter PCB Setup</strong></p>
<p align="center">
  <img src="/assets/Receiver.png" alt="Fig.3 Receiverer PCB Setup" width="600">
</p>
<p align="center"><strong>Fig.3 Receiver PCB Setup</strong></p>


## TODO

1. Use cross platform communication as the receiver board, e.g. stm32f103c8t6. Servo motor control has already been done. Configuration for nRF24L01 to the stm32 board needs extra work.
2. Incorporate gyroscope measurements to improve accuracy of yaw angle.
3. Explore limitation of microcontrollers by applying more computationally expensive filters to further smooth out the data fluctuations.
