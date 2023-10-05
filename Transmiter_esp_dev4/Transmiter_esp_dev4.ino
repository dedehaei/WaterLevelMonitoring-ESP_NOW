/*
  Rui Santos
  Complete project details at https://RandomNerdTutorials.com/esp-now-esp32-arduino-ide/
  
  Permission is hereby granted, free of charge, to any person obtaining a copy
  of this software and associated documentation files.
  
  The above copyright notice and this permission notice shall be included in all
  copies or substantial portions of the Software.
*/

#include <esp_now.h>
#include <WiFi.h>
#include <NewPing.h>

char ssid[] = "Daffadandi";
char pass[] = "Daffabulet";

#define TRIGGER_PIN 19      // Pin untuk trigger sensor ultrasonik
#define ECHO_PIN 18          // Pin untuk echo sensor ultrasonik
#define MAX_DISTANCE 200     // Jarak maksimal yang akan diukur (dalam cm)

NewPing sonar(TRIGGER_PIN, ECHO_PIN, MAX_DISTANCE);

// float duration;
// float distance;

// REPLACE WITH YOUR RECEIVER MAC Address
uint8_t broadcastAddress[] = {0x08, 0xB6, 0x1F, 0x3B, 0x30, 0x08};

// Structure example to send data
// Must match the receiver structure
typedef struct struct_message {
  char a[32];
  int b;
  float c;
  bool d;
} struct_message;

// Create a struct_message called myData
struct_message myData;

esp_now_peer_info_t peerInfo;

// callback when data is sent
void OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status) {
  // Serial.print("\r\nLast Packet Send Status:\t");
  // Serial.println(status == ESP_NOW_SEND_SUCCESS ? "Delivery Success" : "Delivery Fail");
}

 
void setup() {
  // Init Serial Monitor
  Serial.begin(115200);

  // pinMode(ECHO_PIN, INPUT);
  // pinMode(TRIG_PIN, OUTPUT);
 
  // Set device as a Wi-Fi Station
  // WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, pass);
    WiFi.mode(WIFI_AP_STA);
  
  // Set device as a Wi-Fi Station
  // WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    // Serial.println("Setting as a Wi-Fi Station..");
  }
  Serial.print("Station IP Address: ");
  Serial.println(WiFi.localIP());
  Serial.print("Wi-Fi Channel: ");
  Serial.println(WiFi.channel());

  // Init ESP-NOW
  if (esp_now_init() != ESP_OK) {
    Serial.println("Error initializing ESP-NOW");
    return;
  }

  // Once ESPNow is successfully Init, we will register for Send CB to
  // get the status of Trasnmitted packet
  esp_now_register_send_cb(OnDataSent);
  
  // Register peer
  memcpy(peerInfo.peer_addr, broadcastAddress, 6);
  peerInfo.channel = 0;  
  peerInfo.encrypt = false;
  
  // Add peer        
  if (esp_now_add_peer(&peerInfo) != ESP_OK){
    // Serial.println("Failed to add peer");
    return;
  }
}

// void measureDistance(){
//   // Set the trigger pin LOW for 2uS
//   digitalWrite(TRIGPIN, LOW);
//   delayMicroseconds(2);
 
//   // Set the trigger pin HIGH for 20us to send pulse
//   digitalWrite(TRIGPIN, HIGH);
//   delayMicroseconds(20);
 
//   // Return the trigger pin to LOW
//   digitalWrite(TRIGPIN, LOW);
 
//   // Measure the width of the incoming pulse
//   duration = pulseIn(ECHOPIN, HIGH);
 
//   // Determine distance from duration
//   // Use 343 metres per second as speed of sound
//   // Divide by 1000 as we want millimeters
 
//   distance = ((duration / 2) * 0.343)/10;
//   myData.b = distance;
// }
 
void loop() {
  unsigned int distance = sonar.ping_cm(); // Baca jarak dalam cm
  Serial.print("jarak Air: ");
  Serial.print(distance);
  Serial.println("cm");
  // measureDistance();
  // Set values to send
  // strcpy(myData.a, "THIS IS A CHAR");
  myData.b = distance;

  // Serial.println(myData.b);
  // myData.c = 1.2;
  // myData.d = false;
  
  // Send message via ESP-NOW
  esp_err_t result = esp_now_send(broadcastAddress, (uint8_t *) &myData, sizeof(myData));
   
  if (result == ESP_OK) {
    Serial.println("Sent with success");
  }
  else {
    Serial.println("Error sending the data");
  }
  delay(500);
}