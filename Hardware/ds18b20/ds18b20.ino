#include <OneWire.h>
#include <DallasTemperature.h>
#include <Wire.h>
#include "MAX30100_PulseOximeter.h"

// Data wire is plugged into digital pin 2 on the Arduino
#define ONE_WIRE_BUS 2  //D4

//Initializing the buzzer at D3
int buzzer = D3;

//Initializing fan
uint8_t fan = 3;  //Rx pin (GPIO3)(green wire)

// Setup a oneWire instance to communicate with any OneWire device
OneWire oneWire(ONE_WIRE_BUS);  

// Pass oneWire reference to DallasTemperature library
DallasTemperature sensors(&oneWire);

void setup(void)
{
  sensors.begin();  // Start up the library
  pinMode(buzzer,OUTPUT);  //Set D3 as output for buzzer
  pinMode(fan,OUTPUT);
  analogWrite(fan,0);
  Serial.begin(9600);
}

float temp;
void loop(void)
{ 
  // Send the command to get temperatures
  sensors.requestTemperatures(); 

  //print the temperature in Celsius
  Serial.print("Temperature: ");
  temp = sensors.getTempCByIndex(0);
  Serial.print(temp);
  Serial.print((char)176);//shows degrees character
  Serial.print("C  |  ");
  analogWrite(fan,int((temp-33.0)*256.0));
  Serial.print(int((temp-33.0)*256.0));
  
  //print the temperature in Fahrenheit
  Serial.print((sensors.getTempCByIndex(0) * 9.0) / 5.0 + 32.0);
  Serial.print((char)176);//shows degrees character
  Serial.println("F");
  
  if(temp>35){
    tone(buzzer,1000,200); 
  }
  
  delay(2000);

}
