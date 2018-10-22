#include <WiFiEsp.h>
#include <WiFiEspClient.h>
#include "SoftwareSerial.h"
#include <PubSubClient.h>
#include <Keypad.h>
#include <LiquidCrystal_I2C.h> // LCD 함수 라이브러리
#include <Wire.h> // LCD I2C 방식 통신용 라이브러리

const byte ROWS = 4;
const byte COLS = 4;

char keys[ROWS][COLS] = { // 키패드 버튼 정의
  {'1','2','3','A'},
  {'4','5','6','B'},
  {'7','8','9','C'},
  {'*','0','#','D'}
};

// 0x27 I2C 주소를 가지고 있는 16X2 LCD 객체를 생성
LiquidCrystal_I2C lcd(0x27, 16, 2);

byte rowPins[ROWS] = {9,8,7,6}; // 키패드의 행이 연결된 아두이노 핀 번호
byte colPins[COLS] = {5,4,3,2}; // 키패드의 열이 연결된 아두이노 핀 번호

// 키패드 객체 생성
Keypad keypad = Keypad(makeKeymap(keys), rowPins, colPins, ROWS, COLS);

IPAddress server(192,168,0,100);     // your mosquitto broker server
char ssid[] = "339-1";       // your network SSID (name)
char pass[] = "339789789*";           // your network password
int status = WL_IDLE_STATUS;          // the Wifi radio's status
WiFiEspClient esp8266Client;
PubSubClient client(esp8266Client);
SoftwareSerial esp8266(10,11);      // pin10 RX, pin11 TX to ESP8266 TX,RX

void setup() {
  Serial.begin(9600);
  esp8266.begin(9600);  // initialize serial for ESP module

  keypad.addEventListener(keypadEvent); 

  lcd.init();
  lcd.backlight();
  displayMain();

  WiFi.init(&esp8266); // initialize ESP module 
  // check for the presence of the shield
  if (WiFi.status() == WL_NO_SHIELD) {
    Serial.println("WiFi shield not present");
    while (true);     // don't continue
  }
  // attempt to connect to WiFi network
  while ( status != WL_CONNECTED) {
    Serial.print("Attempting to connect to WPA SSID: ");
    Serial.println(ssid);
    status = WiFi.begin(ssid, pass); // Connect to WPA/WPA2 network
    }
  Serial.println("You're connected to the network"); // you're connected now, so print out the data
  
  //connect to MQTT server
  client.setServer(server, 1883);
  client.setCallback(callback);
}

void loop() {

  char key = keypad.getKey();

  if (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    client.connect("arduinoClient3");
    Serial.println("connected");   
    delay (100);
    client.subscribe("fre");   
    delay (100);
    /*client.publish("Switch1","0");
    delay (100);
    client.publish("Switch1_status","TURN OFF");
    delay (100);
    digitalWrite(8,LOW);
    delay (100);
    */
   }
  client.loop();
  delay (100);
}

//print any message received for subscribed topic
void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  //for (int i=0;i<length;i++) Serial.print((char)payload[i]);
  String Message = "";
  int i=0;
  while (i<length) Message +=(char)payload [i++];
  Serial.println (Message);
  
  if (Message=="1" ){  //MQTT 토픽 Switch1에 1이 수신되면 릴레이를 켬
     dispalyGranted();
     delay(5000);
     displayMain();
  }
  if (Message=="0" ){  //MQTT 토픽 Switch1에 0이 수신되면 릴레이를 끔
     dispalyDenied();
     delay(5000);
     displayMain();
  }
}

static char userinput[1];
static int count = 0;

void keypadEvent(KeypadEvent key) {

  switch(keypad.getState()) {
    case PRESSED:
      switch(key) {
        case '#':
        {
          Serial.println("pressed #");
          client.publish("pw",userinput);
          count=0;
          break;
        }
        case '*': 
        {
          displayPW();
          count=0;
          //lcd.setCursor(0,1);
          //lcd.print("                ");
          break;
        }
        default :
        {
          Serial.println(key);
          if(count<4) {
            lcd.setCursor(6+count, 1);
            lcd.print(key);
            userinput[count++] = key; 
          }
          break;
        }
      }
    break;
  }
}

void displayMain() {
  lcd.clear();
  lcd.setCursor(4,0);
  lcd.print("WELCOME!");
  lcd.setCursor(1,1);
  lcd.print("frequency Door");
}

void displayPW() {
  lcd.clear();
  lcd.setCursor(1,0);
  lcd.print("Enter your OTP");
  lcd.setCursor(0,1);
  lcd.print("                ");
}

void dispalyGranted(){
  lcd.clear();
  lcd.setCursor(1,0);
  lcd.print("ACCESS GRANTED");
  lcd.setCursor(4,1);
  lcd.print("WELCOME!!");
}

void dispalyDenied(){
  lcd.clear();
  lcd.setCursor(2,0);
  lcd.print("ACCESS DENIED");  
  lcd.setCursor(5,1);
  lcd.print("SORRY");
}
