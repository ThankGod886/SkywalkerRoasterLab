使用Arduino 連接MAX6675熱電偶模組，並實現Modbus RTU Slave通訊，讓Artisan軟體可以透過USB讀取溫度數據<br>
安裝 Adafruit MAX6675 library<br>
安裝modbus-esp8266 library（by Alexander Emelianov） https://downloads.arduino.cc/libraries/github.com/emelianov/modbus_esp8266-4.1.0.zip<br>
硬體連接<br>
MAX6675 ↔ Arduino Uno or Nano<br>
SO      ↔ D4<br>
CS      ↔ D5<br>
SCK     ↔ D6<br>
VCC     ↔ 5V<br>
GND     ↔ GND<br>
Connect an Arduino to a MAX6675 thermocouple module and implement Modbus RTU Slave communication, allowing Artisan software to read temperature data via USB.<br>
Install the Adafruit MAX6675 library<br>
Install modbus-esp8266 library（by Alexander Emelianov） https://downloads.arduino.cc/libraries/github.com/emelianov/modbus_esp8266-4.1.0.zip<br><br>
Hardware Connection<br>
MAX6675 ↔ Arduino Uno or Nano<br>
SO      ↔ D4<br>
CS      ↔ D5<br>
SCK     ↔ D6<br>
VCC     ↔ 5V<br>
GND     ↔ GND<br>

Artisan軟體設定<br>
第一步<br>
[設定][設備]<br>
[出風溫/豆溫]<br>
保持原本設定TC4不要更改。<br>
Artisan Software Settings<br>
Step 1<br>
[Config] [Device]<br>
[ET/BT]<br>
Keep the original setting TC4 and do not change it.<br>

[額外設備]<br>
按[加入]<br>
設備 選MODBUS<br>
顏色1 溫度曲線顏色自己選定<br>
標籤1 Extra 1，這名稱自己可以修改<br>
只用到一個所以勾選LCD1及曲線1，其餘設定不要動。<br>
按[確定]<br>
[Extra Devices]<br>
Click [Add]<br>
Select MODBUS as the device<br>
Color 1: Choose your own temperature profile color<br>
Label 1: Extra 1 (You can change this name)<br>
Since only one is used, check LCD1 and Curve1. Leave the other settings unchanged.<br>
Click [OK]<br>

第二步通訊埠設定<br>
{Modbus通信協定}<br>
[通訊埠] 選擇Arduino連接的COM埠<br>
[傳輸速率]9600<br>
[資料位元] 8<br>
[同位檢查] N<br>
[停止位] 1<br>
[逾時] 2.0<br>

{輸入1}<br>
[從動裝置] 1<br>
[暫存器] 1<br>
[功能碼] 3<br>
[除頻器] 1/10<br>
[模式] C<br>
[解碼] uInt16<br>

其他保留不變，按[確定]<br>

Step 2: Ports Configuration<br>
{ET/BT} Keep the original setting<br>

{Modbus}<br>
[Comm Port] Select the COM port connected to the Arduino<br>
[Baud Rate] 9600<br>
[Byte Size] 8<br>
[Parity] N<br>
[Stopbits] 1<br>
[Timeout] 2.0<br>

{Input 1}<br>
[Slave] 1<br>
[Register] 1<br>
[Function] 3<br>
[Divider] 1/10<br>
[Mode] C<br>
[Decode] uInt16<br>

Leave other settings unchanged and press [OK]<br>


