/*
 SD bubble logger
 
 This example shows how to log datas from an IR gate sensor and write them 
 to an SD card using the SD library.
 	
 The circuit:
 ** Red LED in series with a 500 Ohm from Digital pin 6 to GND
 ** Green LED in series with a 500 Ohm from Digital pin 7 to GND
 
 The leds are use to monitor the proper function of the program when not connected to a serial monitor.
 
 The sensor is an IR photogate. You can buy a suitable and cheap one on dealextreme at URL:
 http://www.dealextreme.com/p/infrared-light-beam-photoelectric-sensor-module-140554?item=14
 
 The two 5V and GND pins have to be connected to the correspondent outs on the board. The OUT pin have to be connected 
 on digital pin 8.
 
 * SD card attached to SPI bus as follows:
 ** MOSI - pin 11
 ** MISO - pin 12
 ** CLK - pin 13
 ** CS - pin 4
 * using the ethernet shield
 
 created  10 Lug 2010
 by Saverio Girardi
 www.opensourceideas.it
 
 This example code is in the public domain under GNU public license.
 	 
 */

#include <SD.h>

//the CS pin on the ethernet shield is 4.
//we will connect the sensor on digital input 0
const int chipSelect = 4;
const int IRgate = 0;

const int GreenLed = 7;
const int RedLed = 6;

//we define the log period in milliseconds. In this case we log every minute.
const int logPeriod = 60000;

int bubbles = 0;

void setup()
{
  Serial.begin(9600);
  Serial.println("Initializing SD card...");
  
  pinMode(10, OUTPUT);
  pinMode(6, OUTPUT);
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
  
}

//after the setup is complete and if there are no errors the program begins to log

void loop()
{  
  //Every minute from the beginning of the program we write the data to the SD card and set the bubble variable to zero
  if((millis()%logPeriod) == 0 && (millis()/logPeriod) >= 1)
  {
    File DataFile = SD.open("bubbles.txt", FILE_WRITE);
    
    if(DataFile)
    {
      String data = String(millis()/1000) + ",  " + "Temp" + ",  " + String(bubbles);
      DataFile.println(data);
      DataFile.close();
      String success = "Data number " + String(millis()/60000) + " written on SD card";
      Serial.println(success);
      Serial.println("Next log within a minute...");
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
    bubbles = 0;
  }
  //when not writing to the SD we wait for the bubbles and each time we see one we sum her to the bubble variable. 
  //In this way we have a cumulative count until the variable is emptied every minute.
  else
  {
    int bubbleEvent = digitalRead(IRgate);
    
    digitalWrite(RedLed, LOW);
    digitalWrite(GreenLed, HIGH);
    
    if(bubbleEvent)
    {
      bubbles = bubbles + 1;
    }
  }  

}

//If you want to log faster or slower you just have to modify the logPeriod constant.
