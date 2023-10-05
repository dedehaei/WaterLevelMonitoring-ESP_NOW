#define BLYNK_TEMPLATE_ID "TMPL6ixOpklzj"
#define BLYNK_TEMPLATE_NAME "esp now"
#define BLYNK_AUTH_TOKEN "vOX6_R31pCCZXMvAJt4lUMMpS8Jufn9o"

char ssid[] = "Daffadandi";
char pass[] = "Daffabulet";

bool fetch_blynk_state = true;  //true or false

#define TANK_HEIGHT 120      // Ketinggian tandon air (dalam cm)
#define MIN_WATER_PERCENT 25 // Persentase minimum air dalam tandon sebelum menghidupkan pompa
#define MAX_WATER_PERCENT 80 // Persentase maksimum air dalam tandon sebelum mematikan pompa

bool isPumpRunning = false;
bool isManualControl = false;

//#define BLYNK_PRINT Serial
#include <WiFi.h>
#include <WiFiClient.h>
#include <BlynkSimpleEsp32.h>
#include <esp_now.h>
#include <LiquidCrystal_I2C.h>


int lcdColumns = 16;
int lcdRows = 2;

String scrollingText = "";
int scrollPosition = 0;

LiquidCrystal_I2C lcd(0x27, lcdColumns, lcdRows);  

typedef struct struct_message {
    char a[32];
    int b;
    float c;
    bool d;
} struct_message;

struct_message myData;


// define the GPIO connected with Relays and switches
#define RelayPin1   19

#define SwitchPin1 15

#define wifiLed   2   //D2

//Change the virtual pins according the rooms
#define VPIN_BUTTON_0    V0

#define VPIN_BUTTON_1    V1
#define VPIN_BUTTON_2    V2 
#define VPIN_BUTTON_3    V3 
#define VPIN_BUTTON_4    V4 

// Relay State
bool toggleState_0 = LOW;
bool toggleState_1 = LOW; 

// Switch State
bool SwitchState_1 = LOW;

int wifiFlag = 0;

char auth[] = BLYNK_AUTH_TOKEN;

BlynkTimer timer;
// When App button is pushed - switch the state

BLYNK_WRITE(VPIN_BUTTON_1) {
  toggleState_1 = param.asInt();
  digitalWrite(RelayPin1, toggleState_1);
  if(toggleState_1){
    isPumpRunning= HIGH;
  }
  else{
    isPumpRunning=LOW;
  }
}

BLYNK_WRITE(VPIN_BUTTON_0) {
  toggleState_0 = param.asInt();
    if(toggleState_0){
      isManualControl = true;
      lcd.setCursor(0, 0);
      lcd.print("System ON !!!");
    }
    else{
      isManualControl = false;
      lcd.setCursor(0, 0);
      lcd.print("System OFF !!!");
      digitalWrite(RelayPin1, LOW);
      isPumpRunning = false;
      toggleState_1 = LOW;
      Blynk.virtualWrite(VPIN_BUTTON_1, toggleState_1);

    }
  }

void OnDataRecv(const uint8_t * mac, const uint8_t *incomingData, int len) {
  memcpy(&myData, incomingData, sizeof(myData));
  // Serial.print("Bytes received: ");
  // Serial.println(len);
  // Serial.print("Char: ");
  // Serial.println(myData.a);
  // Serial.print("Int: ");
  // Serial.println(myData.b);
  // Serial.print("Float: ");
  // Serial.println(myData.c);
  // Serial.print("Bool: ");
  // Serial.println(myData.d);
  // Serial.println();
}
void sendSensor(){
  bool isconnected = Blynk.connected();
  if (isconnected == false) {
    wifiFlag = 1;
    Serial.println("Blynk Not Connected");
    digitalWrite(wifiLed, LOW);
  }
  else{
    Serial.println("Blynk Connected");
    digitalWrite(wifiLed, HIGH);
  }

  int distance = myData.b;
  Blynk.virtualWrite(VPIN_BUTTON_3, (String(distance) + " cm"));
  float tankHeight = TANK_HEIGHT - distance;
  float waterPercent = (tankHeight / TANK_HEIGHT) * 100.0;
  Serial.println(waterPercent);
  Serial.println(distance);
  Blynk.virtualWrite(VPIN_BUTTON_2, waterPercent);

  String pumpStatus = isPumpRunning ? "ON" : "OFF";

  // Tampilkan informasi pada LCD
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("System: ");
  lcd.print(isManualControl ? "ON " : "OFF");
  
  lcd.setCursor(0, 1);
  lcd.print("Air:");
  lcd.print(String(waterPercent));

  lcd.setCursor(9, 1);
  lcd.print("Pump ");
  lcd.print(pumpStatus);


if (isManualControl){
  if (waterPercent < MIN_WATER_PERCENT) {
    if (!isPumpRunning) {
      Serial.println("Menghidupkan Pompa");
      digitalWrite(RelayPin1, HIGH); // Hidupkan pompa
      isPumpRunning = true;
      toggleState_1 = HIGH;
      Blynk.virtualWrite(VPIN_BUTTON_1, toggleState_1);
    }
  } else if (waterPercent >= MAX_WATER_PERCENT) {
    if (isPumpRunning) {
      Serial.println("Mematikan Pompa");
      digitalWrite(RelayPin1, LOW);  // Matikan pompa
      isPumpRunning = false;
      toggleState_1 = LOW;
      Blynk.virtualWrite(VPIN_BUTTON_1, toggleState_1); 
    }
    
  }

  if (waterPercent >= 85) {
    Serial.println("Tandon Penuh");
    if (isPumpRunning) {
      Serial.println("Mematikan Pompa");
      digitalWrite(RelayPin1, LOW);  // Matikan pompa
      isPumpRunning = false;
      toggleState_1 = LOW;
      // SwitchState_1 = LOW;
      Blynk.virtualWrite(VPIN_BUTTON_1, toggleState_1); 
    }
  }
  delay(500);
  }
}

void checkBlynkStatus() { // called every 2 seconds by SimpleTimer

  // bool isconnected = Blynk.connected();
  // if (isconnected == false) {
  //   wifiFlag = 1;
  //   Serial.println("Blynk Not Connected");
  //   digitalWrite(wifiLed, LOW);
  // }
  // else{
  //   Serial.println("Blynk Connected");
  //   digitalWrite(wifiLed, HIGH);
  // }
}

BLYNK_CONNECTED() {
  // Request the latest state from the server
  if (fetch_blynk_state){
    Blynk.syncVirtual(VPIN_BUTTON_1);
  }
}

void control(){

  if (isManualControl){
  if (digitalRead(SwitchPin1) == LOW && SwitchState_1 == LOW) {
    Serial.println("Menghidupkan Pompa");
    digitalWrite(RelayPin1, HIGH);
    toggleState_1 = HIGH;
    SwitchState_1 = HIGH;
    isPumpRunning = true;
    Blynk.virtualWrite(VPIN_BUTTON_1, toggleState_1);
    Serial.println("Switch-1 on");
  }
  if (digitalRead(SwitchPin1) == HIGH && SwitchState_1 == HIGH ) {
    digitalWrite(RelayPin1, LOW);
    toggleState_1 = LOW;
    SwitchState_1 = LOW;
    isPumpRunning = false;
    Blynk.virtualWrite(VPIN_BUTTON_1, toggleState_1);
    Serial.println("Switch-1 off");
  }
  }
  else{
    digitalWrite(RelayPin1, LOW);
      isPumpRunning = false;
      toggleState_1 = LOW;
      Blynk.virtualWrite(VPIN_BUTTON_1, toggleState_1);
  }    
}


void setup() {
  // put your setup code here, to run once:

  Serial.begin(115200);

  lcd.init();
  lcd.backlight();

  lcd.setCursor(0, 0);
  lcd.print("Water Pump");
  lcd.setCursor(0, 1);
  lcd.print("Control System");
  delay(2000);
  lcd.clear();

  pinMode(RelayPin1, OUTPUT);
  pinMode(wifiLed, OUTPUT);

  pinMode(SwitchPin1, INPUT_PULLUP);

  digitalWrite(RelayPin1, toggleState_1);
  digitalWrite(wifiLed, LOW);

  WiFi.mode(WIFI_AP_STA);
  WiFi.begin(ssid, pass);

  // timer.setInterval(1000L, checkBlynkStatus); // check if Blynk server is connected every 2 seconds
  timer.setInterval(1000L, sendSensor); // Sending Sensor Data to Blynk Cloud every 1 second
  Blynk.config(auth);
  delay(1000);
  
  if (!fetch_blynk_state){
    Blynk.virtualWrite(VPIN_BUTTON_1, !toggleState_1);
  }

   if (esp_now_init() != ESP_OK) {
    Serial.println("Error initializing ESP-NOW");
    return;
  }
  esp_now_register_recv_cb(OnDataRecv);
}


void loop() {
  // put your main code here, to run repeatedly:
   control();
  // button0.check();
  // ir_remote(); //IR remote Control
  Blynk.run();
  timer.run();

}
