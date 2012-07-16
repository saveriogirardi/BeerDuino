/*
This program identifies and read the temperature from a DS18B20 temperature sensor and stores the 
value in a microSD cars using the ethernet shield. Between two temperature measure he counts the bubbles 
exiting from the airlock and sum the value for a logPeriod time interval

You can find a cheap and preinterfaced DS18B20 chips at 
http://www.dealextreme.com/p/ds18b20-digital-temperature-sensor-module-for-arduino-55-125-c-135047?item=8

The leds are use to monitor the proper function of the program when not connected to a serial monitor.
 
The bubble sensor is an IR photogate. You can buy a suitable and cheap one on dealextreme at URL:
http://www.dealextreme.com/p/infrared-light-beam-photoelectric-sensor-module-140554?item=14

The circuit:

--------Temperature sensor
** - pin on GND pin
** + pin to +5 V pin
** S pin to pin 5

--------Ir photogate
The two 5V and GND pins have to be connected to the correspondent outs on the board. The OUT pin have to be connected 
 on digital pin 8.

--------Leds
** Red LED in series with a 500 Ohm from Digital pin 6 to GND
** Green LED in series with a 500 Ohm from Digital pin 7 to GND

--------SD card attached to SPI bus as follows:
** MOSI - pin 11
** MISO - pin 12
** CLK - pin 13
** CS - pin 4
---> using the ethernet shield

We initialize the comunication with the sensor in the setup function so we will have a faster read 
in the loop function. This is due to the relatively slow onewire comunication protocol.

created 16 Lug 2012
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

const int IRgate = 0;

const int GreenLed = 7;
const int RedLed = 6;

//we define the log period in milliseconds. In this case we log every minute.
const int logPeriod = 60000;

OneWire dsTemp(5); //we generate the one wire object and define the pin to comunicate with.
byte tempAd[8];
byte temperature[8];
int foundTemp = 0;

int bubbles = 0;

void setup()
{
  Serial.begin(9600);
  Serial.println("Initializing SD card...");
  
  pinMode(10, OUTPUT); //for the SD
  pinMode(6, OUTPUT); //for the two leds
  pinMode(7, OUTPUT);
  pinMode(0, INPUT);
  
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
  //We check if the sensor belongs to the DS18S20 family because otherwise the library will not work
  if (tempAd[0] != 0x10) 
  {
    Serial.println("Temperature sensor is not a DS18S20 family device.");
    digitalWrite(RedLed, HIGH);
    digitalWrite(GreenLed, LOW);
    return;
  }
  //we finally perform a CRC check to verify the integrity of the data. note that CRC stands for 
  //Cyclic Redundancy Check.
  if(OneWire::crc8( tempAd, 7) != tempAd[7]) 
  {
    Serial.println("CRC is not valid!");
    digitalWrite(RedLed, HIGH);
    digitalWrite(GreenLed, LOW);
    return;
  }
  
  //if everithing is working our Sd card is ready as the sensor and we have the adress stored in tempAd variable.
  //we are ready to roll!
}

//The program begins to log

void loop()
{  
  //we declare the variables needed for the conversion of the temperature
  int HighByte, LowByte, TReading, SignBit, Tc_100, Whole, Fract;

  byte present = 0;
  int i = 0;
  
  //Every minute from the beginning of the program we mesure the temperature and write the data on the SD card.
  if((millis()%logPeriod) == 0 && (millis()/logPeriod) >= 1)
  {
    File DataFile = SD.open("bubbles.txt", FILE_WRITE);
    
    //we tell the sensor to begin the conversion of the actual temperature:
    dsTemp.reset();
    dsTemp.select(tempAd);
    dsTemp.write(0x44); //start the conversion and power it externally so we do not need to use the parassitic power option
      
    delay(1000); //wait a second to let the comunication to take place
      
    present = dsTemp.reset();
    dsTemp.select(tempAd);    
    dsTemp.write(0xBE); //now we ask the sensor to send us the temperature datas...
    
    for ( i = 0; i < 9; i++) {           //we reed the answer of the sensor and store the data in the temperature vector
      temperature[i] = dsTemp.read();
      Serial.print(temperature[i], HEX);
      Serial.print(" ");
    }
     
    //Now we have the temperature but in a format difficult to be processed. To convert it in a more 
    //friendly format we use the small algorithm that you can find at the official Arduino page of the 
    //library at http://www.arduino.cc/playground/Learning/OneWire . I've slightly modified it  to adapt 
    // at the purpose of this code. 
    
    LowByte = temperature[0];
    HighByte = temperature[1];
    TReading = (HighByte << 8) + LowByte;
    SignBit = TReading & 0x8000;  // test most sig bit
    if (SignBit) // negative
    {
      TReading = (TReading ^ 0xffff) + 1; // 2's comp
    }
    Tc_100 = (6 * TReading) + TReading / 4;    // multiply by (100 * 0.0625) or 6.25

    Whole = Tc_100 / 100;  // separate off the whole and fractional portions
    Fract = Tc_100 % 100;
    
    String sign;
    int decimal;
    
    if (SignBit){ sign = "-"; }
    else { sign = "+"; }
    if (Fract < 10){ decimal = 0; }
    else { decimal = Fract; }
    
    String tempString = String(sign) + String(Whole) + "." + String(decimal); //build the string with the temperature to be written on file
    
    //we can now write the data on the Sd
    if(DataFile)
    {      
      String data = String(millis()) + ",  " + tempString + ",  " + bubbles; //print the ACTUAL time elapsed that is much later than when we begin the conversion.
      DataFile.println(data);
      DataFile.close();
      String success = "Data at time " + String(millis()) + " written on SD card";
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
    bubbles = 0; //after writing the data to the SD we set the bubble variable to 0 to begin a new time count
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
