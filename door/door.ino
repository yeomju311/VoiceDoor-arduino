#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <SoftwareSerial.h> // 블루투스

char ssid[] = "339-1"; // network SSID
char pass[] = "339789789*"; // network password
const char * mqtt_server = "192.168.0.100"; // MQTT broker 기기가 연결된 IP주소
WiFiClient esp8266Client;
PubSubClient client(esp8266Client);

SoftwareSerial BTSerial(13, 12); // SoftwareSerial(RX, TX), 통신을 하기 위한 RX, TX 연결 핀번호

int relayPin = 14; // relay to connect pin D13

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  BTSerial.begin(9600); // 블루투스 모듈 초기화, 블루투스 연결
    
  setup_wifi();
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);
  delay(500);
  client.subscribe("fre"); // mqtt 토픽명
  pinMode(relayPin, OUTPUT);
  digitalWrite(relayPin, HIGH);
}

void setup_wifi() {
  delay(10);
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, pass);
  while(WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address : ");
  Serial.println(WiFi.localIP());
}

void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("]");
  String Message = "";
  int i = 0;
  while(i<length) {
    Message += (char)payload [i++];
  }
  Serial.println(Message);

  if(Message == "1") { // MQTT 토픽 fre에 1이 수신되면 릴레이를 켬
    digitalWrite(relayPin, LOW); // 켜짐
    client.publish("entry", "open");
    delay(5000);
    digitalWrite(relayPin, HIGH); // 5초 뒤 꺼짐
  }
  if(Message == "0") {
    digitalWrite(relayPin, HIGH);
    client.publish("entry", "close");
  }
}

void reconnect() {
  while(WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
    setup_wifi();
  }
  while(!client.connected()) {
    Serial.println("Attempting MQTT connection...");
    if(client.connect("arduinoClientRelay")) { // MQTT client name 고유한 이름으로 중복되지 않도록 주의
      Serial.println("connected");
    }
    else {
      Serial.print("MQTT connection failed, retry count: ");
      Serial.println(client.state());
      Serial.println("try again in 3 seconds");
      delay(3000);
    }
  }
}

void loop() {
  // put your main code here, to run repeatedly:
  delay(50);
  if(!client.connected()) {
    reconnect();
    client.subscribe("fre");
    delay(500);
  }
  client.loop();
  delay(50);

  if(BTSerial.available()) { // 블루투스로 데이터 수신, 블루투스에서 신호가 있으면
  /*
    byte data = BTSerial.read();
    Serial.write(data); // 수신된 데이터를 시리얼 모니터로 출력
    if(data=='1') {
      static char userid[6];
      static int count = 0;
      while(BTSerial.available()) {
        byte id = BTSerial.read();
        userid[count++] = id;
      }
      Serial.print("userid  ");
      Serial.println(userid);
      digitalWrite(relayPin, LOW); // 켜짐
      delay(5000);
      digitalWrite(relayPin, HIGH);
    }
    */
    blue();
  }
}

void blue() {
    static char userid[5];
    static int count;
    byte data = BTSerial.read();
    Serial.write(data); // 수신된 데이터를 시리얼 모니터로 출력
    if(data=='1') {
      count = 0;
      while(BTSerial.available()) {
        byte id = BTSerial.read();
        userid[count++] = id;
      }
      Serial.println(userid);
      client.publish("id", userid);
      /*
      digitalWrite(relayPin, LOW); // 켜짐
      delay(5000);
      digitalWrite(relayPin, HIGH);
      */
    }
}
