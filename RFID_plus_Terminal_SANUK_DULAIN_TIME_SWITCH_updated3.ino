//noob code

#define BLYNK_TEMPLATE_ID "TMPL6PbBAqUSA"
#define BLYNK_TEMPLATE_NAME "SecureLink App"
#define BLYNK_AUTH_TOKEN "UZlxhZO5vGQKziaO8rcxGa1vKL2gmcae"

#define BLYNK_PRINT Serial

#include <MFRC522.h>
#include <SPI.h>
#include <WiFi.h>
#include <BlynkSimpleEsp32.h>
#include <Wire.h>
#include <RTClib.h>

char auth[] = BLYNK_AUTH_TOKEN;
const char* ssid = "DNG";
const char* password = "87654321";

// RFID and relay pin setup
#define RST_PIN1 22
#define SDA_PIN1 21
#define RST_PIN2 15
#define SDA_PIN2 2

#define RELAY_PIN1 14
#define RELAY_PIN2 27

#define RED_PIN1 13
#define RED_PIN2 12

#define SDA_PIN 25
#define SCL_PIN 33

#define BUTTON_PIN 4

int timeInput1 = -1; // Variable to store time from Blynk V0 (in seconds)
int timeInput2 = -1; // Variable to store time from Blynk V1 (in seconds)

RTC_DS3231 rtc; // Create RTC instance

MFRC522 rfid1(SDA_PIN1, RST_PIN1);
MFRC522 rfid2(SDA_PIN2, RST_PIN2);

void doorUnlock(int relayPin, int RED_PIN){
  digitalWrite(RED_PIN, LOW);
  digitalWrite(relayPin, HIGH); // Unlock the solenoid
  delay(3500); //Keep the solanoid unlocked for 5 seconds
  digitalWrite(relayPin, LOW);
  digitalWrite(RED_PIN, HIGH);
}

void checkAndUnlock(MFRC522 &rfid, int relayPin, int blynkVirtualPin, int RED_PIN, const String& dateTimeStr) {
  
  if (rfid.PICC_IsNewCardPresent() && rfid.PICC_ReadCardSerial()) {
    
    String UID = "";
    for (byte i = 0; i < rfid.uid.size; i++) {
      UID += String(rfid.uid.uidByte[i], HEX);
    }
        
    UID.toUpperCase(); // Ensure it's in uppercase

    if (UID == "D27C8C1B" || UID == "443AAFDE") {
      Serial.println("Unlocking Relay Pin " + String(relayPin));

      String message = "";

      if (UID == "D27C8C1B" && relayPin == RELAY_PIN1) {
        message = "User_1 has unlocked Door 1 on ";
        message += dateTimeStr;
        Blynk.virtualWrite(blynkVirtualPin, message ); // Write to Blynk terminal
      }

      if (UID == "D27C8C1B" && relayPin == RELAY_PIN2) {
        message = "User_1 has unlocked Door 2 on ";
        message += dateTimeStr;
        Blynk.virtualWrite(blynkVirtualPin, message);
      }

      if (UID == "443AAFDE" && relayPin == RELAY_PIN1) {
        message = "User_2 has unlocked Door 1 on ";
        message += dateTimeStr;
        Blynk.virtualWrite(blynkVirtualPin, message);
      }

      if (UID == "443AAFDE" && relayPin == RELAY_PIN2) {
        message = "User_2 has unlocked Door 2 on ";
        message += dateTimeStr;
        Blynk.virtualWrite(blynkVirtualPin, message);
      }

      doorUnlock(relayPin, RED_PIN);
    } 
    else { // If the UID is not authorized
      if(relayPin == RELAY_PIN1){
        Serial.print("Unauthorized access attempt: Relay Pin ");
        Serial.println(RELAY_PIN1);
        String message = "Unauthorized Access Attempt at Door 1 on ";
        message += dateTimeStr;
        Blynk.virtualWrite(blynkVirtualPin, message); // Write to Blynk terminal
      }

      if(relayPin == RELAY_PIN2){
        Serial.print("Unauthorized access attempt: Relay Pin ");
        Serial.println(RELAY_PIN2);
        String message = "Unauthorized Access Attempt at Door 2 on ";
        message += dateTimeStr;
        Blynk.virtualWrite(blynkVirtualPin, message); // Write to Blynk terminal
      }
    }

    rfid.PICC_HaltA(); // Halt the card
  }
}

void setup() {

  Serial.begin(115200);

  Wire.begin(SDA_PIN, SCL_PIN);// Initialize I2C with custom SDA and SCL pins
  Wire.begin();
  if (!rtc.begin()) {
    Serial.println("Couldn't find RTC");
    while (1); // 
    // Stop if RTC is not found
  }

  //Uncomment only to reset the timer
  //rtc.adjust(DateTime(__DATE__,__TIME__));
  //rtc.adjust(DateTime(2024, 4, 27, 18, 30, 0)); 

  // Connect to WiFi
  Serial.print("Connecting to WiFi");
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println(" Connected to WiFi");

  // Connect to Blynk
  Blynk.begin(auth, ssid, password);

  // Initialize RFID and SPI
  SPI.begin();
  rfid1.PCD_Init();
  rfid2.PCD_Init();

  // Set relay pins as output
  pinMode(RELAY_PIN1, OUTPUT);
  pinMode(RELAY_PIN2, OUTPUT);

  pinMode(RED_PIN1, OUTPUT);
  pinMode(RED_PIN2, OUTPUT);

  pinMode(BUTTON_PIN, INPUT_PULLUP);

  // Initially lock both relays
  digitalWrite(RELAY_PIN1, LOW);
  digitalWrite(RELAY_PIN2, LOW);

  digitalWrite(RED_PIN1, HIGH);
  digitalWrite(RED_PIN2, HIGH);    
}

// Function to capture Time Input widget data from Virtual Pin 0 (V0)
BLYNK_WRITE(V0) {
  // Get the entire string data from the Time Input widget
  String timer1 = param.asStr();
  timeInput1 = atoi(timer1.c_str()); //converting to integer

  int hrs = timeInput1/3600;
  int remainingsecs = timeInput1%3600;
  int mins = remainingsecs/60;
  String Timer = "";
  Timer += hrs;
  Timer += ":";
  Timer += mins;
  Timer += " AM";
  String message = "Door 1 is set to open at ";
  message += Timer;
  Blynk.virtualWrite(V2, message);

  // Display the full content of the Time Input widget
  Serial.print("Timer 1: ");
  Serial.println(timer1);
}

// Function to capture Time Input widget data from Virtual Pin 0 (V0)
BLYNK_WRITE(V1) {
  // Get the entire string data from the Time Input widget
  String timer2 = param.asStr();
  timeInput2 = atoi(timer2.c_str());

  int hrs = timeInput2/3600;
  int remainingsecs = timeInput2%3600;
  int mins = remainingsecs/60;
  String Timer = "";
  Timer += hrs;
  Timer += ":";
  Timer += mins;
  Timer += " AM";
  String message = "Door 2 is set to open at ";
  message += Timer;
  Blynk.virtualWrite(V2, message);

  // Display the full content of the Time Input widget
  Serial.print("Timer 2: ");
  Serial.println(timer2);
}

void loop() {

  DateTime now = rtc.now(); // Get the current time from the RTC
  int seconds = now.hour()*3600 + now.minute()*60 + now.second();

  String dateTimeStr = "";
  dateTimeStr += now.year();
  dateTimeStr += "/";
  dateTimeStr += now.month();
  dateTimeStr += "/";
  dateTimeStr += now.day();
  dateTimeStr += " ";
  dateTimeStr += now.hour();
  dateTimeStr += ":";
  dateTimeStr += now.minute();
  dateTimeStr += " AM";

  String timerMessage = "";

  if (digitalRead(BUTTON_PIN) == LOW) { // Button is pressed
    Serial.println("Button is pressed.");
    timerMessage = "Door 1 is unlocked from inside on ";
    timerMessage += dateTimeStr;
    Blynk.virtualWrite(V2, timerMessage);
    doorUnlock(RELAY_PIN1, RED_PIN1); // Unlock the first door
  }

  Blynk.run(); // Run Blynk

  // Check both RFID scanners and unlock the appropriate relay
  checkAndUnlock(rfid1, RELAY_PIN1, V2, RED_PIN1, dateTimeStr); // Pass Blynk virtual pin V2
  checkAndUnlock(rfid2, RELAY_PIN2, V2, RED_PIN2, dateTimeStr); // Pass Blynk virtual pin V2

  if (timeInput1 >= 0 && seconds == timeInput1) {
    Serial.println("Timer 1 is triggered: Unlocking Door 1");
    timerMessage = "Auto unlocked Door 1 on ";
    timerMessage += dateTimeStr;
    Blynk.virtualWrite(V2, timerMessage);
    doorUnlock(RELAY_PIN1, RED_PIN1);
  }

  if (timeInput2 >= 0 && seconds == timeInput2) {
    Serial.println("Timer 2 is triggered: Unlocking Door 2");
    timerMessage = "Auto unlocked Door 2 on ";
    timerMessage += dateTimeStr;
    Blynk.virtualWrite(V2, timerMessage);
    doorUnlock(RELAY_PIN2, RED_PIN2);
  }

  delay(250); // Delay for 1 second before checking again
}
