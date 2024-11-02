#include <Arduino.h>
#include <SPI.h>
#include <LoRa.h>

// Pin LoRa
#define SCK     5
#define MISO    19
#define MOSI    27
#define SS      18
#define RST     12
#define DI0     26
#define BAND    915E6

// Pin LED
#define LED_PIN 25

int counter = 0;

void setup() {
  Serial.begin(9600);
  while (!Serial);

  // Konfigurasi pin LED
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, LOW);  // Mulai dengan LED mati

  // Konfigurasi pin LoRa
  LoRa.setPins(SS, RST, DI0);

  // Inisialisasi LoRa
  if (!LoRa.begin(BAND)) {
    Serial.println("Gagal menginisialisasi LoRa");
    while (1);
  }
  Serial.println("LoRa inisialisasi berhasil!");
}

void loop() {
  String message = "Pesan ke-" + String(counter);
  
  // Kirim pesan
  LoRa.beginPacket();
  LoRa.print(message);
  LoRa.endPacket();

  Serial.print("Mengirim: ");
  Serial.println(message);
  
  digitalWrite(LED_BUILTIN, !digitalRead(LED_BUILTIN));

  counter++;
  delay(2000); // Kirim pesan setiap 2 detik
}
