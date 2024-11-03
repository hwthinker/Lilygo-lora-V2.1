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

void onReceive(int packetSize);
void displayTask(void *pvParameters);

SSD1306Wire display(0x3c, OLED_SDA, OLED_SCL);

int RxDataRSSI = 0;
char Str1[15];
bool dataUpdated = false;

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

  LoRa.onReceive(onReceive);
  LoRa.receive();

  display.setFont(ArialMT_Plain_24);
  display.drawString(0, 0, "HwThinker");
  display.display();
  delay(1000);
  display.clear();

  // Membuat task terpisah untuk display
  xTaskCreatePinnedToCore(
    displayTask,       // Function yang akan dijalankan sebagai task
    "Display Task",    // Nama task
    4096,              // Ukuran stack task
    NULL,              // Parameter task
    1,                 // Prioritas task
    NULL,              // Handle task
    0                  // Menjalankan di Core 0
  );
}

void loop() {
  // Kosongkan loop utama
  vTaskDelay(1);  // Memberi waktu untuk task lainnya
}

void displayTask(void *pvParameters) {
  for (;;) {
    if (dataUpdated) {
      display.clear();
      display.setFont(ArialMT_Plain_16);
      display.drawString(0, 0, "Lora Receiver");

      display.setFont(ArialMT_Plain_10);
      display.drawString(0, 26, "rx Data:" + String(Str1));
      display.drawString(0, 45, "RSSI : " + String(RxDataRSSI));
      display.display();

      dataUpdated = false;
    }
    vTaskDelay(100 / portTICK_PERIOD_MS);  // Mengatur interval update display
  }
}

void onReceive(int packetSize) {
  Serial.print("Received packet '");
  memset(Str1, 0, sizeof(Str1));

  for (int i = 0; i < packetSize; i++) {
    Str1[i] = (char)LoRa.read();
  }
  Serial.print(Str1);

  RxDataRSSI = LoRa.packetRssi();
  Serial.print("' with RSSI ");
  Serial.println(RxDataRSSI);

  digitalWrite(LED_BUILTIN, !digitalRead(LED_BUILTIN));
  dataUpdated = true;
}
