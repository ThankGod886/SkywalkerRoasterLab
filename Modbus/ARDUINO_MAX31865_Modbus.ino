//使用Arduino 連接MAX31865熱電偶RTD模組，並實現Modbus RTU Slave通訊，讓Artisan軟體可以透過USB讀取溫度數據
//安裝 Adafruit MAX31865 library及ModbusRTUSlave library提供者：C. M. Bulliner

#include <SPI.h>
#include <ModbusRTUSlave.h>
#include <Adafruit_MAX31865.h>

// MAX31865引腳定義
const int thermoCS = 4;  // SPI片選引腳
const int thermoDI = 5;  // SPI MOSI (不使用時可忽略)
const int thermoDO = 6;  // SPI MISO
const int thermoCLK = 7; // SPI SCK

// RTD參數
const float RREF = 430.0;     // 參考電阻值 (430Ω for PT100)
const float RNOMINAL = 100.0; // RTD標稱電阻值 (PT100 = 100Ω)

// RS485控制引腳
const int rs485ControlPin = 2;

// Modbus參數
const byte slaveId = 1;  // 從站ID
const long baudRate = 19200;
const unsigned int modbusDataSize = 20;  // 保持寄存器數量

// 建立MAX31865物件
Adafruit_MAX31865 max31865 = Adafruit_MAX31865(thermoCS, thermoDI, thermoDO, thermoCLK);
ModbusRTUSlave modbus(Serial, rs485ControlPin);

// 保持寄存器陣列
uint16_t holdingRegisters[modbusDataSize];

// RTD類型選擇 (根據你的RTD傳感器選擇)
#define RTD_TYPE 1  // 1=PT100, 2=PT1000

void setup() {
  Serial.begin(19200);
  Serial.println("MAX31865 Modbus RTU Slave - RTD溫度讀取");
  
  // 初始化MAX31865
  max31865.begin(MAX31865_3WIRE);  // 根據你的接線方式選擇: 2WIRE, 3WIRE, 4WIRE
  
  // 初始化Modbus
  modbus.configureHoldingRegisters(holdingRegisters, modbusDataSize);
  modbus.begin(slaveId, baudRate);
  
  // 初始化保持寄存器
  for (int i = 0; i < modbusDataSize; i++) {
    holdingRegisters[i] = 0;
  }
  
  // RS485控制引腳設定為輸出
  pinMode(rs485ControlPin, OUTPUT);
  
  // 設置故障檢測閾值
  max31865.setWires(MAX31865_3WIRE);
   
  Serial.println("MAX31865 Modbus RTU Slave Ready");
  Serial.print("Slave ID: ");
  Serial.println(slaveId);
  Serial.print("RTD Type: ");
  Serial.println(RTD_TYPE == 1 ? "PT100" : "PT1000");
  Serial.print("Reference Resistor: ");
  Serial.print(RREF);
  Serial.println("Ω");
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
  // 檢查故障
  uint8_t fault = max31865.readFault();
  
  if (fault) {
    // 有故障發生
    Serial.print("Fault 0x");
    Serial.println(fault, HEX);
    
    // 清除故障
    if (fault & MAX31865_FAULT_HIGHTHRESH) {
      Serial.println("RTD High Threshold");
    }
    if (fault & MAX31865_FAULT_LOWTHRESH) {
      Serial.println("RTD Low Threshold");
    }
    if (fault & MAX31865_FAULT_REFINLOW) {
      Serial.println("REFIN- > 0.85 x Bias");
    }
    if (fault & MAX31865_FAULT_REFINHIGH) {
      Serial.println("REFIN- < 0.85 x Bias - FORCE- open");
    }
    if (fault & MAX31865_FAULT_RTDINLOW) {
      Serial.println("RTDIN- < 0.85 x Bias - FORCE- open");
    }
    if (fault & MAX31865_FAULT_OVUV) {
      Serial.println("Under/Over voltage");
    }
    
    max31865.clearFault();
    
    // 設置錯誤標誌
    holdingRegisters[0] = 0xFFFF;  // 錯誤標誌
    holdingRegisters[1] = fault;   // 故障代碼
    holdingRegisters[4] = 1;       // 狀態寄存器設為錯誤
    
    return;
  }
  
  // 讀取RTD電阻值
  float resistance = max31865.readRTD();
  Serial.print("RTD Resistance: ");
  Serial.print(resistance, 2);
  Serial.println("Ω");
  
  // 計算溫度
  float celsius = max31865.temperature(RNOMINAL, RREF);
  
  // 讀取華氏溫度（可選）
  float fahrenheit = celsius * 9.0 / 5.0 + 32.0;
  
  // 將浮點數轉換為兩個16位元寄存器
  // 寄存器0-1: 攝氏溫度（IEEE 754格式）
  // 寄存器2-3: 華氏溫度（IEEE 754格式）
  // 寄存器4: 狀態（0=正常, 1=錯誤）
  // 寄存器5: RTD電阻值整數部分（乘以100）
  // 寄存器6: RTD電阻值小數部分（乘以100）
  
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
  
  // 存攝氏溫度整數格式（乘以10保留1位小數）
  holdingRegisters[5] = (uint16_t)(celsius * 10);
  
  // 存儲RTD電阻值（乘以100保留2位小數）
  holdingRegisters[6] = (uint16_t)(resistance * 100);
  
   
  // 可選：存儲原始RTD ADC值
  uint16_t rtdADC = max31865.readRTD();
  holdingRegisters[7] = rtdADC;
  
  Serial.print("Temperature: ");
  Serial.print(celsius, 2);
  Serial.print("°C (");
  Serial.print(holdingRegisters[6]);
  Serial.println(" x0.1°C)");
  Serial.print("Fahrenheit: ");
  Serial.print(fahrenheit, 2);
  Serial.println("°F");
}
