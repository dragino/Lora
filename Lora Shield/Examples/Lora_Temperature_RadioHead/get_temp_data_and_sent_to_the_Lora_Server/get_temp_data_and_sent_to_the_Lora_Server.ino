/*
  get temp data and sent to the Lora Server

  This is an example to show how to get sensor data from a remote Arduino via Wireless Lora Protocol
  The exampels requries below hardwares:
  1) Client Side: Arduino + Lora Shield (868Mhz) + DS18B20 (Temperature Sensor). 
  2) Server Side: Arduino + Lora Shield (868Mhz) + Yun Shield + USB flash. make sure the USB flash has
  this file datalog.csv in the data directory of root. 

  requrie below software:
  Radiohead library from:  http://www.airspayce.com/mikem/arduino/RadioHead/  
  OneWire library for Arduino
  DallasTemperature library for Arduino

  Client side will get the temperature and keep sending out to the server via Lora wireless. 
  Server side will listin on the Lora wireless frequency, once it get the data from Client side, it will 
  turn on the LED and log the sensor data to a USB flash, 

*/
//Include required lib so Arduino can communicate with the temperature sensorDS18B20
#include <OneWire.h>
#include <DallasTemperature.h>
// Singleton instance of the radio driver
#include <SPI.h>
#include <RH_RF95.h>
RH_RF95 rf95;

// Data wire is plugged into port 3 on the Arduino
#define ONE_WIRE_BUS 3

// Setup a oneWire instance to communicate with any OneWire devices (not just Maxim/Dallas temperature ICs)
OneWire oneWire(ONE_WIRE_BUS);

// Pass our oneWire reference to Dallas Temperature. 
DallasTemperature sensors(&oneWire);
float data;
String datastring="";
char databuf[10];
uint8_t dataoutgoing[10];
void setup() 
{
  Serial.begin(9600);
  if (!rf95.init())
    Serial.println("init failed");
   // Defaults after init are 434.0MHz, 13dBm, Bw = 125 kHz, Cr = 4/5, Sf = 128chips/symbol, CRC on
   // Need to change to 868.0Mhz in RH_RF95.cpp 
   sensors.begin();
}

void loop()
{
  // Print Sending to rf95_server
  Serial.println("Sending to rf95_server");
  
  // Get the temperature and send the message to rf95_server
  sensors.requestTemperatures();
  data = sensors.getTempCByIndex(0);
  datastring +=dtostrf(data, 4, 2, databuf);
  strcpy((char *)dataoutgoing,databuf);
  Serial.println(databuf);
  rf95.send(dataoutgoing, sizeof(dataoutgoing));
  rf95.waitPacketSent();
  // Now wait for a reply
  uint8_t indatabuf[RH_RF95_MAX_MESSAGE_LEN];
  uint8_t len = sizeof(indatabuf);

  if (rf95.waitAvailableTimeout(3000))
  { 
    // Should be a reply message for us now   
    if (rf95.recv(indatabuf, &len))
   {
      // Serial print "got reply:" and the reply message from the server
      Serial.print("got reply: ");
      Serial.println((char*)indatabuf);
   }
     else
     {
      //
      Serial.println("recv failed");
     }
  }
  else
  {
    // Serial print "No reply, is rf95_server running?" if don't get the reply .
    Serial.println("No reply, is rf95_server running?");
  }
  delay(400);
}

