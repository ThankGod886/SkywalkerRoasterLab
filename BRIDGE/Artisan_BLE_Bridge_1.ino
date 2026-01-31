#include <Arduino.h>
#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEScan.h>
#include <BLEAdvertisedDevice.h>
#include <Adafruit_NeoPixel.h>

/* ================= 使用者設定 ================= */
#define BLE_TARGET_NAME "ESP32_Skycommand_BLE"

#define SERVICE_UUID  "6e400001-b5a3-f393-e0a9-e50e24dcca9e"
#define CHAR_UUID_RX  "6e400002-b5a3-f393-e0a9-e50e24dcca9e"
#define CHAR_UUID_TX  "6e400003-b5a3-f393-e0a9-e50e24dcca9e"

#define LED_PIN   48
#define LED_COUNT 1

#define BLE_MTU_SIZE 185
/* ============================================ */

Adafruit_NeoPixel led(LED_COUNT, LED_PIN, NEO_GRB + NEO_KHZ800);

BLEClient* client = nullptr;
BLERemoteCharacteristic* txChar = nullptr;
BLERemoteCharacteristic* rxChar = nullptr;
BLEAdvertisedDevice* targetDevice = nullptr;

bool bleConnected = false;
bool bleReady = false;
bool scanning = true;

unsigned long lastBlink = 0;
bool blinkState = false;
unsigned long lastScanRestart = 0;

/* ================= LED ================= */

void setLED(uint8_t r, uint8_t g, uint8_t b) {
  led.setPixelColor(0, led.Color(r, g, b));
  led.show();
}

void ledScanning() {
  if (millis() - lastBlink > 400) {
    lastBlink = millis();
    blinkState = !blinkState;
    blinkState ? setLED(0, 80, 0) : setLED(0, 0, 80);
  }
}

/* ================= Notify Callback ================= */

static void notifyCallback(
  BLERemoteCharacteristic*,
  uint8_t* data,
  size_t length,
  bool
) {
  Serial.write(data, length); // 完整模擬 TC4
}

/* ================= Client Callback ================= */

class ClientCallbacks : public BLEClientCallbacks {
  void onDisconnect(BLEClient*) override {
    bleConnected = false;
    bleReady = false;
    scanning = true;

    delete client;
    client = nullptr;

    BLEDevice::getScan()->start(3, false);
  }
};

/* ================= 掃描 Callback ================= */

class ScanCallbacks : public BLEAdvertisedDeviceCallbacks {
  void onResult(BLEAdvertisedDevice advertisedDevice) override {
    if (advertisedDevice.haveName() &&
        advertisedDevice.getName() == BLE_TARGET_NAME) {

      BLEDevice::getScan()->stop();
      targetDevice = new BLEAdvertisedDevice(advertisedDevice);
    }
  }
};

/* ================= BLE 連線 ================= */

bool connectToTC4() {
  client = BLEDevice::createClient();
  client->setClientCallbacks(new ClientCallbacks());
  client->setMTU(BLE_MTU_SIZE);

  if (!client->connect(targetDevice)) return false;

  BLERemoteService* service =
    client->getService(BLEUUID(SERVICE_UUID));
  if (!service) return false;

  txChar = service->getCharacteristic(BLEUUID(CHAR_UUID_TX));
  rxChar = service->getCharacteristic(BLEUUID(CHAR_UUID_RX));
  if (!txChar || !rxChar) return false;

  if (txChar->canNotify())
    txChar->registerForNotify(notifyCallback);

  bleConnected = true;
  bleReady = true;
  scanning = false;

  setLED(0, 0, 150); // 藍色常亮

  // Artisan reset / 喚醒 TC4
  rxChar->writeValue((uint8_t*)"\n", 1, false);

  return true;
}

/* ================= 初始化 ================= */

void setup() {
  Serial.begin(115200);
  delay(1000);

  led.begin();
  setLED(0, 0, 0);

  BLEDevice::init("ESP32_TC4_Bridge");

  BLEScan* scan = BLEDevice::getScan();
  scan->setAdvertisedDeviceCallbacks(new ScanCallbacks());
  scan->setActiveScan(true);
  scan->start(3, false);   // 非阻塞掃描
}

/* ================= 主迴圈 ================= */

void loop() {

  /* ===== 尚未連線 ===== */
  if (!bleConnected) {

    if (scanning) {
      ledScanning();
    }

    if (targetDevice) {
      scanning = false;

      if (!connectToTC4()) {
        delete targetDevice;
        targetDevice = nullptr;
        scanning = true;
        BLEDevice::getScan()->start(3, false);
      }
    }

    if (!BLEDevice::getScan()->isScanning() &&
        scanning &&
        millis() - lastScanRestart > 500) {

      lastScanRestart = millis();
      BLEDevice::getScan()->start(3, false);
    }

    return;
  }

  /* ===== Artisan → BLE ===== */
  while (Serial.available()) {
    String cmd = Serial.readStringUntil('\n');
    cmd += "\n";

    if (bleReady && rxChar) {
      rxChar->writeValue(
        (uint8_t*)cmd.c_str(),
        cmd.length(),
        false
      );
    }
  }
}
