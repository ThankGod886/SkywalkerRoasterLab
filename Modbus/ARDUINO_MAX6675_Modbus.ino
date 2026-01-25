//使用Arduino 連接MAX6675熱電偶模組，並實現Modbus RTU Slave通訊，讓Artisan軟體可以透過USB讀取溫度數據
//安裝 Adafruit MAX6675 library
//安裝modbus-esp8266 library（by Alexander Emelianov） https://downloads.arduino.cc/libraries/github.com/emelianov/modbus_esp8266-4.1.0.zip
#include <ModbusRTU.h>
#include <SPI.h>
#include <max6675.h>

// ===== MAX6675 pin =====
const int thermoSO = 4;
const int thermoCS = 5;
const int thermoSCK = 6;

MAX6675 thermocouple(thermoSCK, thermoCS, thermoSO);

// ===== Modbus =====
ModbusRTU mb;

uint16_t regET = 0;  // 40001
uint16_t regBT = 0;  // 40002

void setup() {
  Serial.begin(9600);

  // Modbus Slave ID = 1
  mb.begin(&Serial);
  mb.slave(1);

  // Holding Registers
  mb.addHreg(1, regET); // 40001
  mb.addHreg(2, regBT); // 40002

  delay(500);
}

void loop() {
  mb.task();

  static uint32_t lastRead = 0;
  if (millis() - lastRead > 500) {   // 500ms 更新一次
    lastRead = millis();

    double tempC = thermocouple.readCelsius();

    if (!isnan(tempC) && tempC > 0 && tempC < 1000) {
      uint16_t value = (uint16_t)(tempC * 10.0);

      regET = value;
      regBT = value;

      mb.Hreg(1, regET);
      mb.Hreg(2, regBT);
    }
  }
}
