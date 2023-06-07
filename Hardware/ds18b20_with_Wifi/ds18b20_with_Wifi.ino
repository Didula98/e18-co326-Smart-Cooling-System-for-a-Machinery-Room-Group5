#include "FS.h"
#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <NTPClient.h>
#include <WiFiUdp.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include <Wire.h>
#include "MAX30100_PulseOximeter.h"
#include <TinyGPS++.h>
#include <SoftwareSerial.h>

// Data wire is plugged into digital pin 2 on the Arduino
#define ONE_WIRE_BUS 2  //D4

//Initializing the buzzer at D3
int buzzer = D3;

// Define NTP Client to get time
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP);

//SSID and the password of Wifi connection
char* ssid = "Dialog 4G 517";
char* password = "576E5Fc3";
const char* mqtt_server = "192.168.41.113";

//Username and the password of mqtt broker
char* userName="serverCO326";
char* passWord="group5";

// Variables to save date and time
String formattedDate;
String dayStamp;
String timeStamp;

//Initializing fan
uint8_t fan = 3;  //Rx pin (GPIO3)(green wire)

// Setup a oneWire instance to communicate with any OneWire device
OneWire oneWire(ONE_WIRE_BUS);  

// Pass oneWire reference to DallasTemperature library
DallasTemperature sensors(&oneWire);

WiFiClient espClient;
PubSubClient client(espClient);

int speed=0;    //Speed will be a integer between 0 and 10

void updateSpeed(void Speed){
  //This function updates the speed of the fan by writing to the pin connected to the fan
  analogWrite(fan,Speed*124);
}

void takeTime(){
  while(!timeClient.update()) {
    timeClient.forceUpdate();
  }
  // The formattedDate comes with the following format:
  // 2018-05-28T16:00:13Z
  // We need to extract date and time
  formattedDate = timeClient.getFormattedDate();
  Serial.println(formattedDate);

  int splitT = formattedDate.indexOf("T");
  dayStamp = formattedDate.substring(0, splitT);    // Extract date
  timeStamp = formattedDate.substring(splitT+1, formattedDate.length()-1);    // Extract time
}

void callback(char* topic, byte* payload, unsigned int length){
  //This function will receive incoming packets and call neccessary functions 
  Serial.print("Message arrived [");
  Serial.print(topic);
  String receivedMsg = "";
  for(int i=0;i<length;i++){
    receivedMsg = receivedMsg + (char)payload[i];    
  }
  
  updateSpeed(speed); //update the speed  
  Serial.println();
}

void sendTemperature(long Temp){
  //This function will send temperature to the broker (in json format)
  snprintf(msg,200,"{\"temperature\": %f, \"date\": \"%s\", \"time\": \"%s\"}",Temp,dayStamp,timeStamp);
  client.publish("UoP_CO_326_E18_05_Temp",msg);
}

void setup_wifi(){
  delay(10);
  // We start by connecting to a WiFi network
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  randomSeed(micros());

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}

void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Create a random client ID
    String clientId = "ESP8266Client-";
    clientId += String(random(0xffff), HEX);
    // Attempt to connect
    if (client.connect(clientId,userName,passWord)) {
      Serial.println("connected");
      client.subscribe("UoP_CO_326_E18_GrNo_Fan");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

void setup(void)
{
  sensors.begin();  // Start up the library
  pinMode(buzzer,OUTPUT);  //Set D3 as output for buzzer
  pinMode(fan,OUTPUT);
  analogWrite(fan,0);
  Serial.begin(115200);
  Serial.setDebugOutput(true);

  setup_wifi();
  delay(1000);
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);

  // Initialize a NTPClient to get time
  timeClient.begin();
  timeClient.setTimeOffset(3600);
}

float temp;
long lastMsg;

void loop(void)
{ 
  if(!client.connected()){
    reconnect();    
  }
  client.loop();
  long now = millis();
  if(now-lastMsg>2000){
    lastMsg = now;
    // Send the command to get temperatures
    sensors.requestTemperatures(); 
  
    //print the temperature in Celsius
    Serial.print("Temperature: ");
    temp = sensors.getTempCByIndex(0);
    Serial.print(temp);
    Serial.print((char)176);//shows degrees character
    Serial.print("C");

    sendTemperature(temp);

    if(temp>35){
      tone(buzzer,1000,200); 
    }
  }
}
