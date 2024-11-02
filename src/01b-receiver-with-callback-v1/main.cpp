#include <Arduino.h>
#include <SPI.h>
#include <LoRa.h>

#define SCK     5
#define MISO    19
#define MOSI    27
#define SS      18
#define RST     12
#define DI0     26
#define BAND    915E6

// Pin LED
#define LED_PIN 25

volatile bool messageReceived = false;
String receivedMessage = "";

void onReceive(int packetSize) {
  if (packetSize == 0) return;

  receivedMessage = "";
  
  while (LoRa.available()) {
    receivedMessage += (char)LoRa.read();
  }
  
  messageReceived = true; // Set flag to process in loop
}

void setup() {
  // Konfigurasi pin LED
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, LOW); // Mulai dengan LED mati

  Serial.begin(9600);
  while (!Serial);

  LoRa.setPins(SS, RST, DI0);

  if (!LoRa.begin(BAND)) {
    Serial.println("Gagal menginisialisasi LoRa");
    while (1);
  }
  Serial.println("LoRa inisialisasi berhasil!");

  LoRa.onReceive(onReceive);
  LoRa.receive();
}

void loop() {
  if (messageReceived) {
    Serial.print("Pesan diterima: ");
    Serial.println(receivedMessage);

    messageReceived = false; // Reset flag
    digitalWrite(LED_PIN,!digitalRead(LED_PIN));
  }

  delay(10); // Tambahkan delay untuk mencegah WDT
}
