/*
This program identifies and read the temperature from a DS18B20 temperature sensor and stores the 
value in a microSD cars using the ethernet shield.

You can find a cheap and preinterfaced DS18B20 chips at 
http://www.dealextreme.com/p/ds18b20-digital-temperature-sensor-module-for-arduino-55-125-c-135047?item=8

The circuit:
** - pin on GND pin
** + pin to +5 V pin
** S pin to pin 5

We initialize the comunication with the sensor in the setup function so we will have a faster read 
in the loop function. This is due to the relatively slow onewire comunication protocol.

created 14 Lug 2012
by Saverio Girardi
www.opensourceideas.it

This example code is in the public domain under GNU public license.

ATTENTION!! This is a NON VERIFIED code, I'm working on it. Use at your own risk!

*/

#include <SD.h>
#include <OneWire.h>

//the CS pin on the ethernet shield is 4.
//we will connect the sensor on digital input 5
const int chipSelect = 4;
const int tempSens = 5;

const int GreenLed = 7;
const int RedLed = 6;

//we define the log period in milliseconds. In this case we log every minute.
const int logPeriod = 60000;

OneWire dsTemp(5); //we generate the one wire object and define the pin to comunicate with.
byte tempAd[8];
int foundTemp = 0;

void setup()
{
  Serial.begin(9600);
  Serial.println("Initializing SD card...");
  
  pinMode(10, OUTPUT); //for the SD
  pinMode(6, OUTPUT); //for the two leds
  pinMode(7, OUTPUT);
  
  //check if the SD i ready to work and initialize the library
  if(!SD.begin(chipSelect))
  {
    Serial.println("failed! Try to fix the card or the connection.");
    digitalWrite(RedLed, HIGH);
    digitalWrite(GreenLed, LOW);
    return;
  }
  Serial.println("done. Card is ready, writing the header for the file...");
  
  digitalWrite(RedLed, LOW);
  digitalWrite(GreenLed, HIGH);
  
  //try to open the file on the card. If this does not exist it will create it
  File DataFile = SD.open("bubbles.txt",FILE_WRITE);

  //If the file is ready the programm will write an header to it to distinguish the three columns
  if(DataFile)
  {
    String header = "Time  Temp  Bubbles";
    DataFile.println(header);
    DataFile.close();
    Serial.println("Header successfully written in format:");
    Serial.println(header);
    digitalWrite(RedLed, LOW);
    digitalWrite(GreenLed, HIGH);
  }
  //otherwise it serial print the error
  else
  {
     Serial.println("Couldn't open the file!");
     digitalWrite(RedLed, HIGH);
     digitalWrite(GreenLed, LOW);
     return;
  }
  
  //Once the sd is ready we have to check if Arduino recognise the sensor. We first ask him and store his adress:
  Serial.println("Searching the temperature sensor...");
  
  foundTemp = dsTemp.search(tempAd);
  delay(250); //allows the searc() function to end the comunication with the sensor before reading the adress
  if(foundTemp)
  {
    int i = 0;
    Serial.print("Found a sensor with adress ");
    for(i=0; i <= 7; i++) //print the adress
    {
      Serial.print(tempAd[i]);
    }
    Serial.println(" ");
    digitalWrite(RedLed, LOW);
    digitalWrite(GreenLed, HIGH);
  }
  if(!foundTemp)
  {
    Serial.println("Sensor not found!");
    digitalWrite(RedLed, HIGH);
    digitalWrite(GreenLed, LOW);
    return;  //if we don't find the sensor the program exits
  }
  
}

//after the setup is complete and if there are no errors the program begins to log

void loop()
{  
  //Every minute from the beginning of the program we mesure the temperature and write the data on the SD card.
  if((millis()%logPeriod) == 0 && (millis()/logPeriod) >= 1)
  {
    File DataFile = SD.open("bubbles.txt", FILE_WRITE);
    
    if(DataFile)
    {
      
      
      String data = String(millis()) + ",  " + "Temp" + ",  " + "bubbles";
      DataFile.println(data);
      DataFile.close();
      String success = "Data at time " + String(millis()/60000) + " written on SD card";
      Serial.println(success);
      Serial.println("Next log within" + String(logPeriod/1000) + "seconds...");
      digitalWrite(RedLed, HIGH);
      digitalWrite(GreenLed, HIGH);
    }
    else
    {
      Serial.println("Couldn't access file");
      digitalWrite(RedLed, HIGH);
      digitalWrite(GreenLed, LOW);
      return;
    }
  }
}

//If you want to log faster or slower you just have to modify the logPeriod constant.
