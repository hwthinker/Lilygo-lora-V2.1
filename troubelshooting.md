saya menggunakan ESP32+SX1276 untuk menerima data dari transmiter. program receiver di bawah ini di Arduino IDE sudah oke sayangnya ketika menggunakan platformio harus memberikan delay minimal 100ms pada main loop yang itu memboroskan waktu dan tetap tidak stabil. berikut konfigurasi di platformio.ini

```
[env:02b-receiver-with-oled-RTOS]
platform = espressif32
board = ttgo-lora32-v21
upload_speed = 921600
framework = arduino
build_src_filter = +<02b-receiver-with-oled-RTOS/*> 
upload_port =/dev/ttyACM1
monitor_speed =9600
lib_deps = 
  ThingPulse/ESP8266 and ESP32 OLED driver for SSD1306 displays @ ^4.2.0
  sandeepmistry/LoRa @ ^0.8.0
```

  saya coba menggunakan beberapa  pendekata tapi tetap hang dengan setelah beberapa lama waktunya. error seperti ini

```
Core  1 register dump:
PC      : 0x4008b274  PS      : 0x00060b35  A0      : 0x8008a186  A1      : 0x3ffbf20c  
A2      : 0x3ffb8fec  A3      : 0x3ffb81a4  A4      : 0x00000004  A5      : 0x00060b23  
A6      : 0x00060b23  A7      : 0x00000001  A8      : 0x3ffb81a4  A9      : 0x00000018  
A10     : 0x3ffb81a4  A11     : 0x00000018  A12     : 0x3ffc25ac  A13     : 0x00060b23  
A14     : 0x007bf3f8  A15     : 0x003fffff  SAR     : 0x0000000e  EXCCAUSE: 0x00000006  
EXCVADDR: 0x00000000  LBEG    : 0x40086971  LEND    : 0x40086981  LCOUNT  : 0xfffffffb  
Core  1 was running in ISR context:
EPC1    : 0x400df9a7  EPC2    : 0x00000000  EPC3    : 0x00000000  EPC4    : 0x00000000


Backtrace: 0x4008b271:0x3ffbf20c |<-CORRUPTED.
```

### Berikut kode program awal, Hang setelah 52 menit

```c++
#include <SPI.h>
#include <LoRa.h>
#include <Wire.h>  // Only needed for Arduino 1.6.5 and earlier
#include "SSD1306Wire.h" // legacy include: `#include "SSD1306.h"`

#ifdef ARDUINO_SAMD_MKRWAN1300
#error "This example is not compatible with the Arduino MKR WAN 1300 board!"
#endif

#define SCK     5    // GPIO5  -- lora SCK
#define MISO    19   // GPIO19 -- lora MISO
#define MOSI    27   // GPIO27 -- lora MOSI
#define SS      18   // GPIO18 -- lora CS
#define RST     12   // GPIO14 -- RESET (If Lora does not work, replace it with GPIO14)
#define DI0     26   // GPIO26 -- IRQ(Interrupt Request)
#define BAND    915E6

//replace default pin  OLED_SDA=4, OLED_SCL=15 with  OLED_SDA=21, OLED_SCL=22
#define OLED_SDA 21
#define OLED_SCL 22
#define OLED_RST 23   //try this, if problem change to 14 or 16


#define LED_BUILTIN 25

//prototype function here
void onReceive(int packetSize);
// Initialize the OLED display using Wire library
SSD1306Wire  display(0x3c, OLED_SDA, OLED_SCL); // OLED_SDA=4, OLED_SCL=15

int RxDataRSSI = 0;
char Str1[15];

void setup() {
  // START aktivas Oled
  pinMode(LED_BUILTIN, OUTPUT);
  pinMode(OLED_RST, OUTPUT);
  digitalWrite(OLED_RST, LOW);    // set GPIO16 low to reset OLED
  delay(50);
  digitalWrite(OLED_RST, HIGH); // while OLED is running, must set GPIO16 in high、

  // Initialising the UI will init the display too.
  display.init();
  display.flipScreenVertically();
  display.setFont(ArialMT_Plain_10);
  // clear the display
  display.clear();
  // aktivasi Oled END

  Serial.begin(9600);
  while (!Serial);
  SPI.begin(SCK, MISO, MOSI, SS);
  LoRa.setPins(SS, RST, DI0);

  Serial.println("LoRa Receiver Callback");

  if (!LoRa.begin(BAND)) {
    Serial.println("Starting LoRa failed!");
    while (1);
  }

  // register the receive callback
  LoRa.onReceive(onReceive);

  // put the radio into receive mode
  LoRa.receive();
  display.setFont(ArialMT_Plain_24);
  display.drawString(0, 0, "HwThinker");
  display.display();
  delay(100);
  display.clear();
}

void loop() {
  // do nothing
  display.setFont(ArialMT_Plain_16);
  display.drawString(0, 0, "Lora Receiver");

  display.setFont(ArialMT_Plain_10);
  display.drawString(0, 26, "rx Data:" + String(Str1));

  display.setTextAlignment(TEXT_ALIGN_LEFT);
  display.setFont(ArialMT_Plain_10);
  display.drawString(0, 45, "RSSI : " + String(LoRa.packetRssi()));
  display.display();
    // Memberikan "breather" pada CPU dan Mencegah watchdog timer timeout
  delay(100); 
}

void onReceive(int packetSize) {
  // received a packet
  Serial.print("Received packet '");
  memset(Str1, 0, sizeof(Str1));
  // read packet
  for (int i = 0; i < packetSize; i++) {
    Str1[i] = (char)LoRa.read();
  }
  Serial.print(Str1);

  // print RSSI of packet
  Serial.print("' with RSSI ");
  RxDataRSSI = LoRa.packetRssi();
  Serial.println(RxDataRSSI);
  //  digitalWrite(LED_BUILTIN, (state) ? HIGH : LOW);
  //  state = !state;
  digitalWrite(LED_BUILTIN, !digitalRead(LED_BUILTIN));
  display.clear();
}
```

### Coba dengan millist, hang setelah 5 detik

```c++
#include <SPI.h>
#include <LoRa.h>
#include <Wire.h>
#include "SSD1306Wire.h"

#ifdef ARDUINO_SAMD_MKRWAN1300
#error "This example is not compatible with the Arduino MKR WAN 1300 board!"
#endif

#define SCK     5    
#define MISO    19   
#define MOSI    27   
#define SS      18   
#define RST     12   
#define DI0     26   
#define BAND    915E6

#define OLED_SDA 21
#define OLED_SCL 22
#define OLED_RST 23

#define LED_BUILTIN 25

// Initialize the OLED display using Wire library
SSD1306Wire display(0x3c, OLED_SDA, OLED_SCL);

int RxDataRSSI = 0;
char Str1[15];

// Add variables for display update timing
unsigned long lastDisplayUpdate = 0;
const unsigned long DISPLAY_UPDATE_INTERVAL = 100; // Update display every 100ms

void setup() {
  pinMode(LED_BUILTIN, OUTPUT);
  pinMode(OLED_RST, OUTPUT);
  digitalWrite(OLED_RST, LOW);
  delay(50);
  digitalWrite(OLED_RST, HIGH);

  display.init();
  display.flipScreenVertically();
  display.setFont(ArialMT_Plain_10);
  display.clear();

  Serial.begin(9600);
  while (!Serial);

  SPI.begin(SCK, MISO, MOSI, SS);
  LoRa.setPins(SS, RST, DI0);

  Serial.println("LoRa Receiver Callback");

  if (!LoRa.begin(BAND)) {
    Serial.println("Starting LoRa failed!");
    while (1);
  }

  // register the receive callback
  LoRa.onReceive(onReceive);
  LoRa.receive();

  // Initial display
  display.setFont(ArialMT_Plain_24);
  display.drawString(0, 0, "HwThinker");
  display.display();
  delay(100);
  display.clear();
}

void loop() {
  unsigned long currentMillis = millis();

  // Only update display if enough time has passed
  if (currentMillis - lastDisplayUpdate >= DISPLAY_UPDATE_INTERVAL) {
    lastDisplayUpdate = currentMillis;
    

    display.clear();
    display.setFont(ArialMT_Plain_16);
    display.drawString(0, 0, "Lora Receiver");
    
    display.setFont(ArialMT_Plain_10);
    display.drawString(0, 26, "rx Data:" + String(Str1));
    
    display.setTextAlignment(TEXT_ALIGN_LEFT);
    display.setFont(ArialMT_Plain_10);
    display.drawString(0, 45, "RSSI : " + String(RxDataRSSI));
    
    display.display();

  }

  // Give other tasks a chance to run
  yield();
}

void onReceive(int packetSize) {
  Serial.print("Received packet '");
  memset(Str1, 0, sizeof(Str1));

  for (int i = 0; i < packetSize; i++) {
    Str1[i] = (char)LoRa.read();
  }

  Serial.print(Str1);
  Serial.print("' with RSSI ");
  RxDataRSSI = LoRa.packetRssi();
  Serial.println(RxDataRSSI);

  digitalWrite(LED_BUILTIN, !digitalRead(LED_BUILTIN));
}
```

###  Coba  konfigurasi watchdog, hang di menit ke 5

```c++
#include <SPI.h>
#include <LoRa.h>
#include <Wire.h>
#include "SSD1306Wire.h"
#include "esp_task_wdt.h"

#ifdef ARDUINO_SAMD_MKRWAN1300
#error "This example is not compatible with the Arduino MKR WAN 1300 board!"
#endif

#define SCK     5    
#define MISO    19   
#define MOSI    27   
#define SS      18   
#define RST     12   
#define DI0     26   
#define BAND    915E6

#define OLED_SDA 21
#define OLED_SCL 22
#define OLED_RST 23

#define LED_BUILTIN 25

// Initialize the OLED display using Wire library
SSD1306Wire display(0x3c, OLED_SDA, OLED_SCL);

int RxDataRSSI = 0;
char Str1[15];
volatile bool needDisplayUpdate = false;

// Semaphore untuk sinkronisasi antara ISR dan loop utama
portMUX_TYPE mux = portMUX_INITIALIZER_UNLOCKED;

// Task handle untuk core assignment
TaskHandle_t loraTask;

void displayTask(void *parameter) {
    for(;;) {
        if(needDisplayUpdate) {
            display.clear();
            display.setFont(ArialMT_Plain_16);
            display.drawString(0, 0, "Lora Receiver");
            
            // Masuk critical section untuk membaca data
            portENTER_CRITICAL(&mux);
            display.setFont(ArialMT_Plain_10);
            display.drawString(0, 26, "rx Data:" + String(Str1));
            display.setTextAlignment(TEXT_ALIGN_LEFT);
            display.drawString(0, 45, "RSSI : " + String(RxDataRSSI));
            portEXIT_CRITICAL(&mux);
            
            display.display();
            needDisplayUpdate = false;
        }
        // Delay yang aman untuk task RTOS
        vTaskDelay(pdMS_TO_TICKS(50));
    }
}

void setup() {
    // Konfigurasi watchdog timer
    esp_task_wdt_init(8, true); // timeout 8 detik
    esp_task_wdt_add(NULL);

    pinMode(LED_BUILTIN, OUTPUT);
    pinMode(OLED_RST, OUTPUT);
    digitalWrite(OLED_RST, LOW);
    delay(50);
    digitalWrite(OLED_RST, HIGH);

    display.init();
    display.flipScreenVertically();
    display.setFont(ArialMT_Plain_10);
    display.clear();

    Serial.begin(9600);
    while (!Serial);
    
    SPI.begin(SCK, MISO, MOSI, SS);
    LoRa.setPins(SS, RST, DI0);

    Serial.println("LoRa Receiver Callback");

    if (!LoRa.begin(BAND)) {
        Serial.println("Starting LoRa failed!");
        while (1);
    }

    // Membuat task untuk display pada core 0
    xTaskCreatePinnedToCore(
        displayTask,   // Function to implement the task
        "displayTask", // Name of the task
        4096,         // Stack size in words
        NULL,         // Task input parameter
        1,            // Priority of the task
        NULL,         // Task handle
        0             // Core where the task should run
    );

    // register the receive callback
    LoRa.onReceive(onReceive);
    LoRa.receive();

    // Initial display
    display.setFont(ArialMT_Plain_24);
    display.drawString(0, 0, "HwThinker");
    display.display();
    delay(100);
    display.clear();
}

void loop() {
    // Reset watchdog timer
    esp_task_wdt_reset();
    
    // Berikan waktu untuk task lain
    vTaskDelay(pdMS_TO_TICKS(1));
}

void onReceive(int packetSize) {
    // Masuk critical section untuk menulis data
    portENTER_CRITICAL(&mux);
    
    memset(Str1, 0, sizeof(Str1));
    for (int i = 0; i < packetSize && i < sizeof(Str1) - 1; i++) {
        Str1[i] = (char)LoRa.read();
    }
    
    RxDataRSSI = LoRa.packetRssi();
    needDisplayUpdate = true;
    
    portEXIT_CRITICAL(&mux);

    // Serial print di luar critical section
    Serial.print("Received packet '");
    Serial.print(Str1);
    Serial.print("' with RSSI ");
    Serial.println(RxDataRSSI);
    
    digitalWrite(LED_BUILTIN, !digitalRead(LED_BUILTIN));
}
```

### Menggunakan Que RTOS, hang setelah 11 menit

```
#include <SPI.h>
#include <LoRa.h>
#include <Wire.h>
#include "SSD1306Wire.h"
#include "esp_task_wdt.h"

#ifdef ARDUINO_SAMD_MKRWAN1300
#error "This example is not compatible with the Arduino MKR WAN 1300 board!"
#endif

#define SCK     5    
#define MISO    19   
#define MOSI    27   
#define SS      18   
#define RST     12   
#define DI0     26   
#define BAND    915E6

#define OLED_SDA 21
#define OLED_SCL 22
#define OLED_RST 23

#define LED_BUILTIN 25

// Initialize the OLED display using Wire library
SSD1306Wire display(0x3c, OLED_SDA, OLED_SCL);

// Struktur data untuk queue
struct LoRaData {
    char message[15];
    int rssi;
};

// Queue handle
QueueHandle_t loraQueue;

// Task handles
TaskHandle_t displayTaskHandle;
TaskHandle_t loraTaskHandle;

void displayTask(void *parameter) {
    LoRaData data;
    display.clear();
    display.display();

    while (true) {
        if (xQueueReceive(loraQueue, &data, pdMS_TO_TICKS(100)) == pdTRUE) {
            // Print to Serial
            Serial.print("Received packet '");
            Serial.print(data.message);
            Serial.print("' with RSSI ");
            Serial.println(data.rssi);
            
            // Toggle LED
            digitalWrite(LED_BUILTIN, !digitalRead(LED_BUILTIN));
            
            // Update display
            display.clear();
            display.setFont(ArialMT_Plain_16);
            display.drawString(0, 0, "Lora Receiver");
            
            display.setFont(ArialMT_Plain_10);
            display.drawString(0, 26, "rx Data:" + String(data.message));
            
            display.setTextAlignment(TEXT_ALIGN_LEFT);
            display.drawString(0, 45, "RSSI : " + String(data.rssi));
            
            display.display();
        }
        vTaskDelay(pdMS_TO_TICKS(10));
    }

}

void loraTask(void *parameter) {
    while (true) {
        LoRa.receive();
        vTaskDelay(pdMS_TO_TICKS(10));
    }
}

void setup() {
    // Initialize LED
    pinMode(LED_BUILTIN, OUTPUT);
    

    // Initialize serial
    Serial.begin(9600);
    
    // Configure watchdog with longer timeout
    esp_task_wdt_init(10, false);
    
    // Initialize queue
    loraQueue = xQueueCreate(5, sizeof(LoRaData));
    
    // Initialize display
    pinMode(OLED_RST, OUTPUT);
    digitalWrite(OLED_RST, LOW);
    delay(50);
    digitalWrite(OLED_RST, HIGH);
    
    display.init();
    display.flipScreenVertically();
    display.setFont(ArialMT_Plain_10);
    display.clear();
    display.display();
    
    // Initialize LoRa
    SPI.begin(SCK, MISO, MOSI, SS);
    LoRa.setPins(SS, RST, DI0);
    
    Serial.println("LoRa Receiver Callback");
    
    if (!LoRa.begin(BAND)) {
        Serial.println("Starting LoRa failed!");
        while (1);
    }
    
    // Set LoRa receive callback
    LoRa.onReceive([](int packetSize) {
        if (packetSize) {
            LoRaData data;
            memset(data.message, 0, sizeof(data.message));
            
            // Read packet
            int i = 0;
            while (LoRa.available() && i < sizeof(data.message) - 1) {
                data.message[i++] = (char)LoRa.read();
            }
            
            data.rssi = LoRa.packetRssi();
            
            // Send to queue from ISR
            BaseType_t xHigherPriorityTaskWoken = pdFALSE;
            xQueueSendFromISR(loraQueue, &data, &xHigherPriorityTaskWoken);
            if (xHigherPriorityTaskWoken) {
                portYIELD_FROM_ISR();
            }
        }
    });
    
    // Create tasks
    xTaskCreatePinnedToCore(
        displayTask,
        "DisplayTask",
        4096,
        NULL,
        1,
        &displayTaskHandle,
        0  // Run on Core 0
    );
    
    xTaskCreatePinnedToCore(
        loraTask,
        "LoRaTask",
        4096,
        NULL,
        2,
        &loraTaskHandle,
        1  // Run on Core 1
    );
    
    // Show initial display
    display.setFont(ArialMT_Plain_24);
    display.drawString(0, 0, "HwThinker");
    display.display();
    delay(100);
    display.clear();
    display.display();

}

void loop() {
    // Main loop is now empty as all work is done in tasks
    vTaskDelay(portMAX_DELAY);
}
```

### Menggunakan Simplifed method,  hang setelah detik ke 10

```c++
#include <SPI.h>
#include <LoRa.h>
#include <Wire.h>
#include "SSD1306Wire.h"

#ifdef ARDUINO_SAMD_MKRWAN1300
#error "This example is not compatible with the Arduino MKR WAN 1300 board!"
#endif

#define SCK     5    
#define MISO    19   
#define MOSI    27   
#define SS      18   
#define RST     12   
#define DI0     26   
#define BAND    915E6

#define OLED_SDA 21
#define OLED_SCL 22
#define OLED_RST 23

#define LED_BUILTIN 25

// Initialize the OLED display using Wire library
SSD1306Wire display(0x3c, OLED_SDA, OLED_SCL);

char Str1[15];
int RxDataRSSI = 0;
volatile bool dataReceived = false;
unsigned long lastDisplayUpdate = 0;
const unsigned long DISPLAY_INTERVAL = 100;  // 100ms interval for display updates

void setup() {
    // Initialize LED
    pinMode(LED_BUILTIN, OUTPUT);
    

    // Initialize display
    pinMode(OLED_RST, OUTPUT);
    digitalWrite(OLED_RST, LOW);
    delay(50);
    digitalWrite(OLED_RST, HIGH);
    
    display.init();
    display.flipScreenVertically();
    display.setFont(ArialMT_Plain_10);
    display.clear();
    display.display();
    
    // Initialize Serial
    Serial.begin(9600);
    while (!Serial);
    
    // Initialize SPI and LoRa
    SPI.begin(SCK, MISO, MOSI, SS);
    LoRa.setPins(SS, RST, DI0);
    
    Serial.println("LoRa Receiver Starting...");
    
    if (!LoRa.begin(BAND)) {
        Serial.println("Starting LoRa failed!");
        while (1);
    }
    
    // Register the receive callback
    LoRa.onReceive(onReceive);
    LoRa.receive();
    
    // Show initial display
    display.setFont(ArialMT_Plain_24);
    display.drawString(0, 0, "HwThinker");
    display.display();
    delay(100);
    display.clear();
    display.display();

}

void onReceive(int packetSize) {
    if (packetSize == 0) return;
    

    // Clear the buffer
    memset(Str1, 0, sizeof(Str1));
    
    // Read packet data
    int i = 0;
    while (LoRa.available() && i < sizeof(Str1) - 1) {
        Str1[i++] = (char)LoRa.read();
    }
    
    // Get RSSI
    RxDataRSSI = LoRa.packetRssi();
    
    // Print to Serial
    Serial.print("Received packet '");
    Serial.print(Str1);
    Serial.print("' with RSSI ");
    Serial.println(RxDataRSSI);
    
    // Toggle LED
    digitalWrite(LED_BUILTIN, !digitalRead(LED_BUILTIN));
    
    // Set flag for display update
    dataReceived = true;

}

void updateDisplay() {
    display.clear();
    display.setFont(ArialMT_Plain_16);
    display.drawString(0, 0, "Lora Receiver");
    

    display.setFont(ArialMT_Plain_10);
    display.drawString(0, 26, "rx Data:" + String(Str1));
    
    display.setTextAlignment(TEXT_ALIGN_LEFT);
    display.drawString(0, 45, "RSSI : " + String(RxDataRSSI));
    
    display.display();

}

void loop() {
    unsigned long currentMillis = millis();
    

    // Update display at fixed interval if new data received
    if (dataReceived && (currentMillis - lastDisplayUpdate >= DISPLAY_INTERVAL)) {
        updateDisplay();
        lastDisplayUpdate = currentMillis;
        dataReceived = false;
    }
    
    // Keep LoRa in receive mode
    LoRa.receive();
    
    // Small delay to prevent watchdog issues
    delay(1);

}
```

