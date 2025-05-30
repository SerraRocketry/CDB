# Avionics - Onboard Computer

This project is the Onboard Computer (OBC) for the SR1500 rocket of the Serra Rocketry model rocketry team. The OBC uses sensors and modules to monitor altitude, GPS location, and control the parachute deployment to ensure a safe flight.

## Features

- Altitude reading using the BMP280 sensor.
- GPS location monitoring with the NEO-6m GPS module.
- LoRa communication for data transmission to the ground station.
- Servo motor control for parachute deployment.
- Data storage using LittleFS.
- Acceleration and gyroscope monitoring using the MPU6050 sensor.
- Data logging in CSV format for later analysis.
- Buzzer signaling to indicate operational status.

## Hardware Used

- ESP32
- BMP280 pressure sensor
- NEO-6m GPS module
- LoRa module
- Servo motor
- Buzzer
- LED
- Step-down converter
- 18650 batteries
- MPU6050 sensor

## Pin Configuration

- **BUZZER_PIN**: 0  
- **SERVO_PIN**: 13  
- **RX_GPS**: 20  
- **TX_GPS**: 21  
- **SS_LORA**: 7  
- **RST_LORA**: 1  
- **DIO0_LORA**: 2  

## Libraries Used

- **Wire**  
- **SPI**  
- **Adafruit_BMP280**  
- **ESP32Servo**  
- **TinyGPS++**  
- **FS**  
- **LittleFS**  
- **LoRa**  
- **Adafruit_MPU6050**  
- **Adafruit_Sensor**  

## Code Structure

- **setup()**: Configures sensors, modules, and initializes variables.  
- **loop()**: Periodically reads sensor data, checks altitude, and controls parachute deployment.  
- **setupLittleFS()**: Initializes the LittleFS file system.  
- **setupBMP()**: Configures the BMP280 sensor.  
- **setupLoRa()**: Initializes the LoRa module.  
- **setupMPU()**: Configures the MPU6050 sensor.  
- **setupServo()**: Configures the servo motor.  
- **buzzSignal()**: Controls the buzzer for signaling.  
- **logData()**: Logs and prints current data.  
- **handleParachute()**: Controls parachute deployment based on altitude.  
- **checkHighest()**: Checks the highest altitude reached.  
- **writeFile()**: Writes data to the LittleFS file.  
- **appendFile()**: Appends data to the LittleFS file.  
- **printBoth()**: Prints messages to Serial and sends via LoRa.  
- **sendLoRa()**: Sends messages via LoRa.  
- **GPSData()**: Retrieves GPS data (latitude, longitude, satellites, altitude, date, and time).  
- **BMPData()**: Retrieves data from the BMP280 sensor (altitude and pressure).  
- **MPUData()**: Retrieves data from the MPU6050 sensor (acceleration and gyroscope).  
- **getDataString()**: Concatenates GPS, BMP280, MPU6050 data, and parachute status into a string.  

## Supporting Codes

The codes in `/extras` are auxiliary to development. These codes are functional and tested.  
The codes in `/test` are auxiliary to development, serving as steps for building the final code.  