

/*
  SD card datalogger
 
 This example shows how to log data from three analog sensors 
 to an SD card using the SD library.
 	
 The circuit:
 ** Red LED in series with a 500 Ohm from Digital pin 6 to GND
 ** Green LED in series with a 500 Ohm from Digital pin 7 to GND
 
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
 
 This example code is in the public domain.
 	 
 */

#include <SD.h>

const int chipSelect = 4;
const int IRgate = 0;

int bubbles = 0;

void setup()
{
  Serial.begin(9600);
  Serial.println("Initializing SD card...");
  
  pinMode(10, OUTPUT);
  
  if(!SD.begin(chipSelect))
  {
    Serial.println("failed! Try to fix the card or the connection.");
    return;
  }
  Serial.println("done. Card is ready, writing the header for the file...");
  
  File DataFile = SD.open("bubbles.txt",FILE_WRITE);
  if(DataFile)
  {
    String header = "Time  Temp  Bubbles";
    DataFile.println(header);
    DataFile.close();
    Serial.println("Header successfully written in format:");
    Serial.println(header);
  }
  else
  {
     Serial.println("Couldn't open the file!");
  }
  
}

void loop()
{  
  if((millis()%60000) == 0 && (millis()/60000) >= 1)
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
    }
    else
    {
      Serial.println("Couldn't access file");
    }
    bubbles = 0;
  }
  else
  {
    int bubbleEvent = digitalRead(IRgate);
    
    if(bubbleEvent)
    {
      bubbles = bubbles + 1;
    }
  }  

}






