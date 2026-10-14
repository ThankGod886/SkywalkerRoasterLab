搭配原廠控制器的esp32 s3 n16r8(n8r2)+tft lcd st7789 顯示烘焙曲線圖完成了。
不需電腦、平板、手機
這個esp32 s3 程式只接收溫度資料，會計算ror，會自動偵測標示入豆、回溫點、轉黃點(攝氏150度)、一爆點(攝氏183度)及下豆。
使用usb type a公 串接接兩條type a母，用來接esp32 s3 來顯示曲線圖。
當使用原廠控制器已經預熱完成等待下豆時就可以按一下esp32 s3的boot按扭就會進入曲線記錄模式，烘焙結束再按一下boot按扭就會停止記錄，再按一下boot按扭就會清除曲線進入等待。
<img src="https://github.com/ThankGod886/SkywalkerRoasterLab/blob/main/TFT_LCD/USB%20Type-A%20%E4%B8%80%E5%88%86%E4%BA%8C%E6%8E%A5%E7%B7%9A%E5%9C%96.png" width="50%">
