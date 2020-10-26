#include<WiFi.h>
#include<PubSubClient.h>

union{
  int data;
  unsigned char p[4];  
}myint;
union{
  float data;
  unsigned char p[4];  
}myfloat;
unsigned char rx_buf[50];
unsigned char cnt = 0;
char txt_buf[50];
unsigned char flag;   // 是否正常
int xd;               // 心电
int bs;               // 步数
float xl;             // 心率
float jl;             // 距离
float tw;             // 体温


// wifi
const char* ssid = "ssid";      // wifi名
const char* passwd = "passwd";  // wifi密码

// mqtt
const char* mqttServer = "localhost";     // 服务器地址
const char* mqttUser = "aaa";             // mqtt账号
const char* mqttPassword = "bbbbbb";      // mqtt密码
const char* Topic_Send = "CJLU_A2";       // 发布主题名
const char* Topic_Receive = "ESP32_Rec2"; // 订阅主题名

WiFiClient espClient;
PubSubClient client(espClient); 

// 连wifi
void connect_wifi() {
  while (WiFi.status()!= WL_CONNECTED) {
    WiFi.begin(ssid, passwd);
    Serial.println("Connecting to WiFi...");
    delay(1000);
  }
  Serial.println("Connected the WiFi network");
}

// 连MQTT
void connect_mqtt() {
  int cnt = 0;
  while (!client.connected()){
    Serial.println("Connecting to MQTT...");
    if (client.connect("ESP32_2", mqttUser, mqttPassword )) {
      Serial.println("Connected the MQTT server");
      // 订阅主题
      client.subscribe(Topic_Receive);
    }else {
      Serial.print("failedwith state ");
      Serial.print(client.state());
      delay(1000);
      if(cnt++ == 3)connect_wifi();
    }
  }
}

void callback(char*topic, byte* payload, unsigned int length) {
  Serial.println("-------------");
  Serial.print("Messagearrived in topic: ");
  Serial.println(topic);
  Serial.print("Message:");
  String msg = "";
  for (int i = 0; i< length; i++) {
    msg += (char)payload[i];
  }
  Serial.println(length);
  Serial.println(msg);
  Serial.println("-----------------------");
}

void setup() {
  Serial.begin(115200);
  Serial2.begin(115200);
  
  connect_wifi();
  
  client.setServer(mqttServer, 1883);
  client.setCallback(callback);
}

void loop() {
  if(Serial2.available()){
    rx_buf[cnt] = Serial2.read();
    if(rx_buf[0] == 0xa5)cnt++;
    if(cnt == 24){
      cnt = 0;
      if(rx_buf[23] == 0x5a){
        flag = rx_buf[1];
        for(unsigned char i=2; i<6; i++)
          myint.p[i-2] = rx_buf[i];
        xd = myint.data;
        for(unsigned char i=6; i<10; i++)
          myint.p[i-6] = rx_buf[i];
        bs = myint.data;
        for(unsigned char i=10; i<14; i++)
          myfloat.p[i-10] = rx_buf[i];
        xl = myfloat.data;
        for(unsigned char i=14; i<18; i++)
          myfloat.p[i-14] = rx_buf[i];
        jl = myfloat.data;
        for(unsigned char i=18; i<22; i++)
          myfloat.p[i-18] = rx_buf[i];
        tw = myfloat.data;
        sprintf(txt_buf, "%d %d %d %f %f %f", flag, xd, bs, xl, jl, tw);
        Serial.println(txt_buf);
        if (client.publish(Topic_Send, txt_buf) == true) {
          Serial.println("Success sending message");
        } else {
          Serial.println("Error sending message");
        }
      }
    }
  }

  if (!client.connected()) {
    connect_mqtt();
  }
  client.loop();
}
