#include <SPI.h>
#include <LoRa.h>

// Pin definitions
#define SCK     5
#define MISO    19
#define MOSI    27
#define SS      18
#define RST     12
#define DI0     26
#define BAND    915E6

// Pin LED
#define LED_PIN 25

// LED state variable
bool ledState = false;

// prototipe function here
void onReceive(int packetSize);

void setup() {
  Serial.begin(9600);

  // Initialize LED pin
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, LOW);

  Serial.println("LoRa Receiver Callback");

  // Configure LoRa pins
  SPI.begin(SCK, MISO, MOSI, SS);
  LoRa.setPins(SS, RST, DI0);

  if (!LoRa.begin(BAND)) {
    Serial.println("Starting LoRa failed!");
    while (1);
  }

  // Uncomment the next line to disable the default AGC and set LNA gain, values between 1 - 6 are supported
  // LoRa.setGain(6);

  // register the receive callback
  LoRa.onReceive(onReceive);

  // put the radio into receive mode
  LoRa.receive();

  Serial.println("LoRa Initializing OK!");
}

void loop() {
// Memberikan "breather" pada CPU dan Mencegah watchdog timer timeout
  delay(10);
}

void onReceive(int packetSize) {
  // Toggle LED state
  ledState = !ledState;
  digitalWrite(LED_PIN, ledState);

  // received a packet
  Serial.print("Received packet '");
  String message = "";
  // read packet
  for (int i = 0; i < packetSize; i++) {
    message += (char)LoRa.read();
  }

  Serial.print(message);

  // print RSSI of packet
  Serial.print("' with RSSI ");
  Serial.println(LoRa.packetRssi());
}