//使用Arduino 連接MAX6675熱電偶模組，並實現Modbus RTU Slave通訊，讓Artisan軟體可以透過USB讀取溫度數據
//安裝 Adafruit MAX6675 library及ModbusRTUSlave library提供者：C. M. Bulliner


#include <SPI.h>
#include <ModbusRTUSlave.h>
#include <max6675.h>

// MAX6675引腳定義
const int thermoSO = 4;
const int thermoCS = 5;
const int thermoSCK = 6;

// RS485控制引腳
const int rs485ControlPin = 2;

// Modbus參數
const byte slaveId = 1;  // 從站ID
const long baudRate = 19200;
const unsigned int modbusDataSize = 20;  // 保持寄存器數量

// 建立物件
MAX6675 thermocouple(thermoSCK, thermoCS, thermoSO);
ModbusRTUSlave modbus(Serial, rs485ControlPin);

// 保持寄存器陣列
uint16_t holdingRegisters[modbusDataSize];

void setup() {
  // 初始化MAX6675
  SPI.begin();
  
  // 初始化Modbus
  modbus.configureHoldingRegisters(holdingRegisters, modbusDataSize);
  modbus.begin(slaveId, baudRate);
  
  // 初始化保持寄存器
  for (int i = 0; i < modbusDataSize; i++) {
    holdingRegisters[i] = 0;
  }
  
  // RS485控制引腳設定為輸出
  pinMode(rs485ControlPin, OUTPUT);
  
  Serial.begin(19200);
  Serial.println("MAX6675 Modbus RTU Slave Ready");
  Serial.print("Slave ID: ");
  Serial.println(slaveId);
}

void loop() {
  static unsigned long lastUpdate = 0;
  const unsigned long updateInterval = 1000;  // 每1秒更新一次
  
  // 更新溫度讀取
  if (millis() - lastUpdate >= updateInterval) {
    readTemperature();
    lastUpdate = millis();
  }
  
  // 處理Modbus請求
  modbus.poll();
}

void readTemperature() {
  // 讀取攝氏溫度
  float celsius = thermocouple.readCelsius();
  
  // 檢查錯誤
  if (isnan(celsius) || celsius == NAN) {
    holdingRegisters[0] = 0xFFFF;  // 錯誤標誌
    holdingRegisters[1] = 0xFFFF;
    Serial.println("Error reading thermocouple");
    return;
  }
  
  // 讀取華氏溫度（可選）
  float fahrenheit = thermocouple.readFahrenheit();
  
  // 將浮點數轉換為兩個16位元寄存器
  // 寄存器0-1: 攝氏溫度（IEEE 754格式）
  // 寄存器2-3: 華氏溫度（IEEE 754格式）
  // 寄存器4: 狀態（0=正常, 1=錯誤）
  
  // 轉換攝氏溫度為IEEE 754格式的兩個16位元寄存器
  uint32_t celsiusBinary;
  memcpy(&celsiusBinary, &celsius, 4);
  holdingRegisters[0] = (celsiusBinary >> 16) & 0xFFFF;  // 高16位
  holdingRegisters[1] = celsiusBinary & 0xFFFF;          // 低16位
  
  // 轉換華氏溫度
  uint32_t fahrenheitBinary;
  memcpy(&fahrenheitBinary, &fahrenheit, 4);
  holdingRegisters[2] = (fahrenheitBinary >> 16) & 0xFFFF;
  holdingRegisters[3] = fahrenheitBinary & 0xFFFF;
  
  // 設定狀態寄存器
  holdingRegisters[4] = 0;  // 正常
  
  // 也將攝氏溫度轉換為整數（乘以10以保留一位小數）
  holdingRegisters[5] = (uint16_t)(celsius * 10);

  Serial.print("Temperature: ");
  Serial.print(celsius);
  Serial.print("°C (");
  Serial.print(holdingRegisters[5]);
  Serial.println(" x0.1°C)");
}