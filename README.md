# TTGO Lora 915MHz V2.1

![Lilygo Lora V2.1](./assets/Lilygo%20Lora%20V2.1.png)

## pinout 

![Main image-6](./assets/Main%20image-6.jpg)


```
LED : IO25
LORA pin : IO18=NSS/SEL
IO5=SCK
IO27=MOSI/SDI
IO19=MISO/SDO
IO26=DI0/IO0
IO23=IRQ/RESET

SD card : SD3-CS-IO13
CMD-MOS1-IO15
CLK-SCK-IO14
SD0-MISO-IO2
DATA2:IO12
DATA2:IO4
```

## Contoh Koding

### Simple Lora

-  [sender](src/01a-sender/main.cpp)  
-  [receiver](src/01b-receiver-loop/main.cpp)  

### Simple Lora dengan OLED

-  [Sender OLED](src/02a-sender-with-oled/main.cpp)  
-  [Receiver OLED](src/02b-receiver-with-oled/main.cpp)  

### Simple Lora dengan OLED RTOS-lebih stabil
-  [Sender OLED](src/02a-sender-with-oled/main.cpp)  
-  [Receiver OLED RTOS](src/02b-receiver-with-oled-RTOS/main.cpp)  

### Penjelasan Modifikasi:

1. **Task `displayTask`**:
   - Kode display diisolasi ke dalam task `displayTask`, yang berjalan di Core 0. Dengan ini, Core 1 (yang menangani operasi LoRa) tidak terbebani oleh proses display.
   - `vTaskDelay(100 / portTICK_PERIOD_MS)` memberikan jeda 100 ms di antara tiap pembaruan display.
2. **Penggunaan `vTaskDelay(1)` dalam `loop()`**:
   - Memastikan bahwa `loop()` utama memberikan waktu ke task lain, membantu menghindari masalah dengan watchdog timer.
3. **Variabel `dataUpdated`**:
   - Variabel ini digunakan sebagai flag untuk menunjukkan bahwa data baru telah diterima dan perlu di-update ke display.



## Referensi:

- https://github.com/Xinyuan-LilyGO/LilyGo-LoRa-Series/blob/master/examples/RadioLibExamples/SX1276/SX1276_Receive_Interrupt/boards.h?spm=a2g0o.detail.1000023.6.3bb3mQhhmQhhVf&file=boards.h