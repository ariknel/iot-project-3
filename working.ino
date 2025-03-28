#include <ESP32Servo.h>
#include <LiquidCrystal_I2C.h>
#define BLYNK_TEMPLATE_ID   "template";
#define BLYNK_TEMPLATE_NAME "server";
#define BLYNK_PRINT Serial
#include <WiFi.h>
#include <WiFiClient.h>
#include <BlynkSimpleEsp32.h>
char auth[] = "key";
char ssid[] = "ssid";
char pass[] = "pass";
Servo myservo;
int servoPin = 18;     
volatile int val = 90; 
const int trigPin = 5;
const int echoPin = 19;
const int ldrPin = 34;
int lcdColumns = 16;
int lcdRows = 2;
int servoPot = 4;
int volume = 0;
bool isDark;
LiquidCrystal_I2C lcd(0x27, lcdColumns, lcdRows);
int lastVal = -1;
unsigned long previousMillis = 0;
const long interval = 500;
void setup() {
    ESP32PWM::allocateTimer(0);
    myservo.setPeriodHertz(50);
    myservo.attach(servoPin, 500, 2400);
    pinMode(trigPin, OUTPUT);
    pinMode(echoPin, INPUT);
    pinMode(ldrPin, INPUT);
    pinMode(servoPot, INPUT);
    pinMode(33, OUTPUT);
    pinMode(26, OUTPUT);
    lcd.init();
    lcd.backlight();

    ///// blynk
   Serial.begin(9600); //begin serial
    Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.begin(ssid, pass); //connect to wifi and print connection status later
  int wifi_ctr = 0;
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("WiFi connected");
  Blynk.begin(auth, ssid, pass, "server.wyns.it", 8081);
}

void loop() {
  Blynk.run();
    // Read LDR value
    int ldrValue = analogRead(ldrPin);
    isDark = (ldrValue < 1000); // Adjust threshold as needed
    Blynk.virtualWrite(V4, ldrValue); //lees ldr value -> in blynk widget led aan door reroute
LCD(); //runs LCD function
    ////display on Blynk
    Blynk.virtualWrite(V19,volume);
    digitalWrite(26, (volume == 0) ? HIGH : LOW);  // Led on if volume is 0%
    digitalWrite(33, isDark ? HIGH : LOW);            // Led on if dark
    
}

void LCD() {
  unsigned long currentMillis = millis();
  if (currentMillis - previousMillis >= interval) {
    previousMillis = currentMillis;  
    float distance = getDistance(); //get volume calc
    volume = mapVolume(distance);
      lcd.clear(); //print on LCD
    lcd.setCursor(0, 0);
    lcd.print("Volume: ");
    lcd.print(volume);
    lcd.print("%");
    lcd.setCursor(0, 1);
    lcd.print("Light: ");
    lcd.print(isDark ? "Dark" : "Light");
  }
}

BLYNK_WRITE(V3) {
 int pinValue = param.asInt(); 
 if (pinValue == 1) {
    myservo.write(180);
  } else {
   myservo.write(0);
 }
}
// Function to get distance from SR04
float getDistance() {
    digitalWrite(trigPin, LOW);
    delayMicroseconds(2);
    digitalWrite(trigPin, HIGH);
    delayMicroseconds(10);
    digitalWrite(trigPin, LOW);
    long duration = pulseIn(echoPin, HIGH);
    return duration * 0.034 / 2; // calculate to cm
}
//map distances to 2 as lowest treshold and 15+ as full)
int mapVolume(float distance) {
    if (distance <= 2) return 100;   
    if (distance >= 15) return 0;   
    return map(distance, 2, 15, 100, 0); 
}
