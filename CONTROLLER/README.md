這個Esp32 s3 devkitc-1的韌體是我使用下面網址https://github.com/MagnmCI/SkywalkerRoasterLab/tree/main/HiBean/SkiBeanQuickSV<br>的程式修改編譯而來，原本的程式主要是使用Esp32 s3 zero。

底下是修改增加的內容：<br>
1.可以使用usb連接Artisan<br>
2.修正緊急降溫時，火力不會關閉<br>
3.增加BT高於攝氏230°C時會執行緊急降溫<br>
4.增加devkitc-1的led燈號

This Esp32 s3 devkitc-1 firmware was modified and compiled from the program at https://github.com/MagnmCI/SkywalkerRoasterLab/tree/main/HiBean/SkiBeanQuickSV. The original program primarily used Esp32 s3 zero.
Below are the modifications and additions:
1. USB connection to Artisan is now possible.
2. Fixed issue where the power wouldn't shut off during emergency cooling.
3. Added emergency cooling functionality when BT exceeds 230°C.
4. Added LED indicator to the devkitc-1.
