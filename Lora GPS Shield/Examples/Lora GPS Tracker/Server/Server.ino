/*
In this project,we'll show how to get GPS data from a remote Arduino via Wireless Lora Protocol 
and show the track on the GoogleEarth.The construction of this project is similar to my last one:

1) Client Side: Arduino + Lora/GPS Shield (868Mhz).
2) Server Side: Arduino + Lora Shield (868Mhz) + Yun Shield + USB flash.

Client side will get GPS data and keep sending out to the server via Lora wireless. Server side 
will listin on the Lora wireless frequency, once it get the data from Client side, it will
turn on the LED and log the sensor data to a USB flash. 

  More about this example, please see:
*/

//Include required lib so Arduino can talk with the Lora Shield
#include <SPI.h>
#include <RH_RF95.h>

//Include required lib so Arduino can communicate with Yun Shield
#include <FileIO.h>
#include <Console.h>

// Singleton instance of the radio driver
RH_RF95 rf95;
int led = 4;
int reset_lora = 9;
String dataString = "";

void setup() 
{
  pinMode(led, OUTPUT); 
  pinMode(reset_lora, OUTPUT);     
  Bridge.begin();
  Console.begin();
  FileSystem.begin();

  // reset lora module first. to make sure it will works properly
  digitalWrite(reset_lora, LOW);   
  delay(1000);
  digitalWrite(reset_lora, HIGH); 
  
  //while(!Console);  // wait for Console port to connect.
  //Console.println("Log remote sensor data to USB flash\n");

  if (!rf95.init())
    Console.println("init failed");  
  // Defaults after init are 434.0MHz, 13dBm, Bw = 125 kHz, Cr = 4/5, Sf = 128chips/symbol, CRC on
  // Need to change to 868.0Mhz in RH_RF95.cpp 
}

void loop()
{
  dataString="";
  if (rf95.available())
  {
    Console.println("Get new message");
    // Should be a message for us now   
    uint8_t buf[RH_RF95_MAX_MESSAGE_LEN];
    uint8_t len = sizeof(buf);
    if (rf95.recv(buf, &len))
    {
      digitalWrite(led, HIGH);
      //RH_RF95::printBuffer("request: ", buf, len);
      Console.print("got message: ");
      Console.println((char*)buf);
      Console.print("RSSI: ");
      Console.println(rf95.lastRssi(), DEC);

      //make a string that start with a timestamp for assembling the data to log:
      dataString += String((char*)buf);
      dataString += ",";
      dataString += getTimeStamp();

      // Send a reply to client as ACK
      uint8_t data[] = "200 OK";
      rf95.send(data, sizeof(data));
      rf95.waitPacketSent();
      Console.println("Sent a reply");
      // open the file. note that only one file can be open at a time,
      // so you have to close this one before opening another.
      // The FileSystem card is mounted at the following "/mnt/FileSystema1"
      // Make sure you have this file in your system
      File dataFile = FileSystem.open("/mnt/sd1/data/datalog.csv", FILE_APPEND);

      // if the file is available, write to it:
      if (dataFile) {
        dataFile.println(dataString);
        dataFile.close();
        // print to the serial port too:
        Console.println(dataString);
        Console.println("");
      }  
       // if the file isn't open, pop up an error:
      else 
      {
        Console.println("error opening datalog.csv");
      } 
      digitalWrite(led, LOW);      
    }
    else
    {
      Console.println("recv failed");
    }
  }
}

// This function return a string with the time stamp
// Yun Shield will call the Linux "data" command and get the time stamp
String getTimeStamp() {
  String result;
  Process time;
  // date is a command line utility to get the date and the time 
  // in different formats depending on the additional parameter 
  time.begin("date");
  time.addParameter("+%D-%T");  // parameters: D for the complete date mm/dd/yy
                                //             T for the time hh:mm:ss    
  time.run();  // run the command

  // read the output of the command
  while(time.available()>0) {
    char c = time.read();
    if(c != '\n')
      result += c;
  }
  return result;
}


