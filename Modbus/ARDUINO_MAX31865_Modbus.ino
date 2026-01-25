//使用Arduino 連接MAX31865熱電偶 3線 PT100 RTD模組，並實現Modbus RTU Slave通訊，讓Artisan軟體可以透過USB讀取溫度數據
//安裝 Adafruit MAX31865 library
//安裝modbus-esp8266 library（by Alexander Emelianov） https://downloads.arduino.cc/libraries/github.com/emelianov/modbus_esp8266-4.1.0.zip
#include <SPI.h>
#include <Adafruit_MAX31865.h>
#include <ModbusRTU.h>

// ===== MAX31865 =====
#define MAX_CS 4
#define MAX_DI 5
#define MAX_DO 6
#define MAX_CLK 7

Adafruit_MAX31865 rtd = Adafruit_MAX31865(MAX_CS, MAX_DI, MAX_DO, MAX_CLK);

// PT100
#define RNOMINAL 100.0
#define RREF 430.0

// ===== Modbus =====
ModbusRTU mb;
uint16_t regET = 0;
uint16_t regBT = 0;

// ===== Filter =====
float filteredTemp = 25.0;
const float alpha = 0.2;          // EMA factor
const float maxStep = 2.0;        // °C per update

void setup() {
  Serial.begin(9600);

  // MAX31865 init
  rtd.begin(MAX31865_3WIRE);
  rtd.enable50Hz(true);   // 台灣必開

  // Modbus slave
  mb.begin(&Serial);
  mb.slave(1);

  mb.addHreg(1, regET);   // 40001
  mb.addHreg(2, regBT);   // 40002

  delay(500);
}

void loop() {
  mb.task();

  static uint32_t lastRead = 0;
  if (millis() - lastRead > 500) {
    lastRead = millis();

    float temp = rtd.temperature(RNOMINAL, RREF);

    if (!isnan(temp) && temp > 0 && temp < 500) {

      // ===== EMA filter =====
      float ema = alpha * temp + (1.0 - alpha) * filteredTemp;

      // ===== Anti-spike =====
      float diff = ema - filteredTemp;
      if (diff > maxStep) diff = maxStep;
      if (diff < -maxStep) diff = -maxStep;

      filteredTemp += diff;

      uint16_t value = (uint16_t)(filteredTemp * 10.0);

      regET = value;
      regBT = value;

      mb.Hreg(1, regET);
      mb.Hreg(2, regBT);
    }
  }
}
