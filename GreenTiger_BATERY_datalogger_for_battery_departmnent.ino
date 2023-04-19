TaskHandle_t Task1;
TaskHandle_t Task2;

//for Time
//#include <WiFi.h>
//#include "time.h"

#include <WiFi.h>
#include <NTPClient.h>
#include <WiFiUdp.h>

WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP);

String formattedDate;
String dayStamp;
String timeStamp;

const char* ssid       = "Z";
const char* password   = "123123456";

const char* ntpServer = "pool.ntp.org";
const long  gmtOffset_sec = 19800;
const int   daylightOffset_sec = 0;

#include <CAN.h>

// Libraries for SD card
#include "FS.h"
#include <SD.h>
#include <SPI.h>
// Define CS pin for the SD card module
#define SD_MISO     19
#define SD_MOSI     23
#define SD_SCLK     18
#define SD_CS       15
SPIClass sdSPI(VSPI);

byte Counter = 0;               
byte Stored;                    
float Voltage; 
float Current;                     
byte c0,c1,c2,c3,c4,c5;              //for voltage, current, remaining Ah
byte t0,t1,t2,t3,t4,t5;             //for temperature
byte s1,s2;                        //for SOC
byte a0,a1,a2,a3,a4,a5,a6,a7,a8,a9,a10,a11,a12,a13,a14,a15,a16,a17,a18,a19,a20,a21,a22,a23,a24,a25,a26,a27,a28,a29,a30,a31,a32,a33,a34,a35,a36,a37,a38,a39,a40;       //for 16 cell voltages
int Seconds;  
int soc;
int remainingAh;     

int CellVoltage1;
int CellVoltage2;
int CellVoltage3;
int CellVoltage4;
int CellVoltage5;
int CellVoltage6;
int CellVoltage7;
int CellVoltage8;
int CellVoltage9;
int CellVoltage10;
int CellVoltage11;
int CellVoltage12;
int CellVoltage13;
int CellVoltage14;
int CellVoltage15;
int CellVoltage16;
int CellVoltage17;
int CellVoltage18;
int CellVoltage19;
int CellVoltage20;


void setup() {
 
  Serial.begin(9600);
  if (!CAN.begin(250E3))
  {
    Serial.println("Starting CAN failed!");
    while (1);
  }

/*******************************************connect to WiFi****************/
Serial.print("Connecting to ");
Serial.println(ssid);
WiFi.begin(ssid, password);
while (WiFi.status() != WL_CONNECTED) {
  delay(500);
  Serial.print(".");
}

/**************************Init NTP client**********************************/
timeClient.begin();
timeClient.setTimeOffset(19800);

/***************************************Initialize SD card******************/
  //SD.begin(SD_CS);  
  sdSPI.begin(SD_SCLK, SD_MISO, SD_MOSI, SD_CS);
  if(!SD.begin(SD_CS, sdSPI)) {
    Serial.println("Card Mount Failed");
    return;
  }
  Serial.println("1");
  uint8_t cardType = SD.cardType();
  if(cardType == CARD_NONE) {
//    Serial.println("No SD card attached");
    return;
  }
//  Serial.println("Initializing SD card...");
  if (!SD.begin(SD_CS)) {
//    Serial.println("ERROR - SD card initialization failed!");
    return;    // init failed
  }
  SDCARD();

  xTaskCreatePinnedToCore(
                    Task1code,   /* Task function. */
                    "Task1",     /* name of task. */
                    10000,       /* Stack size of task */
                    NULL,        /* parameter of the task */
                    1,           /* priority of the task */
                    &Task1,      /* Task handle to keep track of created task */
                    0);          /* pin task to core 0 */                  
  delay(500); 

  xTaskCreatePinnedToCore(
                    Task2code,   /* Task function. */
                    "Task2",     /* name of task. */
                    10000,       /* Stack size of task */
                    NULL,        /* parameter of the task */
                    1,           /* priority of the task */
                    &Task2,      /* Task handle to keep track of created task */
                    1);          /* pin task to core 1 */
    delay(500);
    
}

// Write to the SD card (DON'T MODIFY THIS FUNCTION)
void writeFile(fs::FS &fs, const char * path, const char * message) {
  Serial.printf("Writing file: %s\n", path);

  File file = fs.open(path, FILE_WRITE);
  if(!file) {
//    Serial.println("Failed to open file for writing");
    return;
  }
  file.close();
}

// Append data to the SD card (DON'T MODIFY THIS FUNCTION)
void appendFile(fs::FS &fs, const char * path, const char * message) {
//  Serial.printf("Appending to file: %s\n", path);

  File file = fs.open(path, FILE_APPEND);
  if(!file) {
//    Serial.println("SD card OUT");
    digitalWrite(LED_BUILTIN, LOW); 
    return;
  }
  if(file.print(message)) {
//    Serial.println("SD card IN");
    digitalWrite(LED_BUILTIN, HIGH); 
//    return;
  } 
  file.close();
}

// Function to get date and time from NTPClient
void getTimeStamp() {
  while(!timeClient.update()) {
    timeClient.forceUpdate();
  }
  // The formattedDate comes with the following format:
  // 2018-05-28T16:00:13Z
  // We need to extract date and time
  formattedDate = timeClient.getFormattedDate();
//  Serial.println(formattedDate);

  // Extract date
  int splitT = formattedDate.indexOf("T");
  dayStamp = formattedDate.substring(0, splitT);
//  Serial.println(dayStamp);
  // Extract time
  timeStamp = formattedDate.substring(splitT+1, formattedDate.length()-1);
//  Serial.println(timeStamp);
}

void Task1code( void * pvParameters )
{
//  Serial.print("Task1000 running on core ");
//  Serial.println(xPortGetCoreID());

  for(;;)
  {
    CANSender();  
    getTimeStamp(); 
    SDCARD();

//    vTaskDelay(100);  // one tick delay (15ms) in between reads for stability
  }
}

void Task2code( void * pvParameters ){
//  Serial.print("Task2 running on core ");
//  Serial.println(xPortGetCoreID());

  for(;;)
  {
    CANReceiver();
//    vTaskDelay(100);  // one tick delay (15ms) in between reads for stability

  }
}


void loop() 
{
  
}

void CANSender()
{
  CAN.beginExtendedPacket(0x0E64090D);                // SOC of Total Voltage
  CAN.write(00);
  CAN.write(00);
  CAN.write(00);
  CAN.write(00);
  CAN.write(00);
  CAN.write(00);
  CAN.write(00);
  CAN.write(00);
  CAN.endPacket();
//  Serial.println("done");
  delay(1000);
}

void CANReceiver()
{
  
  Counter = 0;
  int packetSize = CAN.parsePacket();
  
  if (packetSize)
  {
    if (CAN.packetExtended()) 
    {
      if(CAN.packetId() == 174918921)                   // Green Tiger Battery Voltage & Current
      {
        while (CAN.available()) 
        {
          Stored =  CAN.read();
          Counter++;
          if(Counter ==1)
          {
           c0 = Stored;                                     
          }
  
          if(Counter ==2)
          {
           c1 = Stored;
          
          Voltage = (c0<<8) + c1;
//          Serial.print("Voltage = ");
//          Serial.print(Voltage * 0.1);                          
//          Serial.print("%");
//          Serial.println();
          }

          if(Counter ==3)
          {
           c2 = Stored;                                    
          }

          if(Counter ==4)
          {
           c3 = Stored;
           Current = (c2<<8) + c3;
//           if ( Current >= 65535 )
//           {
//           Serial.print("Current = ");
//           Serial.print((65535 - Current)*0.1);                          
//           Serial.print("%");
//           Serial.println();
//           }
//           else 
//           {
//           Serial.print("Current = ");
//           Serial.print( Current * 0.1 );                        
//           Serial.print("%");
//           Serial.println();

//           Serial.print("Power = ");
//           Serial.print((Current * 0.1 ) * ( Voltage * 0.1 ));  
//           Serial.println();
           }
           if(Counter ==5)
          {
           c4 = Stored;
          }
          if(Counter ==6)
          {
           c5 = Stored;
           remainingAh = (c4<<8) + c5;
        }
        }
      }

/*************************************************BATTERY SOC*****************************************************/
 if(CAN.packetId() == 174984457)                                         // Green Tiger Battery SOC
      {
        while (CAN.available()) 
        {
          Stored =  CAN.read();
          Counter++;
          if(Counter ==1)
          {
           s1 = Stored;    
//           Serial.print("s1 = ");
//          Serial.print(s1);                                 
          }
  
          if(Counter ==2)
          {
           s2 = Stored;
//          Serial.print("s2 = ");
//          Serial.print(s2); 
          soc = (s1<<8) + s2;
          Serial.print("SOC = ");
          Serial.print(soc);                          
          }
        }
      }

/*************************************************CELL VOLTAGE 1 to 4*********************************************/
      if(CAN.packetId() == 241437961)                                         // Green Tiger Battery 4 Cell voltages
      {
        while (CAN.available()) 
        {
          Stored =  CAN.read();
          Counter++;
          if(Counter ==1)
          {
           a0 = Stored;    
//           Serial.print("a0 = ");
//          Serial.print(a0);                                 
          }
  
          if(Counter ==2)
          {
           a1 = Stored;
//          Serial.print("a1 = ");
//          Serial.print(a1); 
          CellVoltage1 = (a0<<8) + a1;
//          Serial.print("Cell Voltage 1 = ");
//          Serial.print(CellVoltage1*0.001);                          
          }

          if(Counter ==3)
          {
           a2 = Stored;                                    
          }

          if(Counter ==4)
          {
           a3 = Stored;
           CellVoltage2 = (a2<<8) + a3;
//           Serial.print("Cell Voltage 2 = ");
//           Serial.print(CellVoltage2*0.001); 
           }
           if(Counter ==5)
          {
           a4 = Stored;
          }
          if(Counter ==6)
          {
           a5 = Stored;
           CellVoltage3 = (a4<<8) + a5;
//           Serial.print("Cell Voltage 3 = ");
//          Serial.print(CellVoltage3*0.001); 
        }
        if(Counter ==7)
          {
           a6 = Stored;
          }
          if(Counter ==8)
          {
           a7 = Stored;
           CellVoltage4 = (a6<<8) + a7;
//           Serial.print("Cell Voltage 4 = ");
//          Serial.print(CellVoltage4*0.001); 
        }
        }
      }

/*************************************************CELL VOLTAGE 5 to 8*********************************************/      

            if(CAN.packetId() == 241503497)                   // Green Tiger Battery 4 Cell voltages
      {
        while (CAN.available()) 
        {
          Stored =  CAN.read();
          Counter++;
          if(Counter ==1)
          {
           a8 = Stored;    
//           Serial.print("a8 = ");
//          Serial.print(a8);                                 
          }
  
          if(Counter ==2)
          {
           a9 = Stored;
//          Serial.print("a9 = ");
//          Serial.print(a9); 
          CellVoltage5 = (a8<<8) + a9;
//          Serial.print("Cell Voltage 1 = ");
//          Serial.print(CellVoltage1*0.001);                          
          }

          if(Counter ==3)
          {
           a10 = Stored;                                    
          }

          if(Counter ==4)
          {
           a11 = Stored;
           CellVoltage6 = (a10<<8) + a11;
//           Serial.print("Cell Voltage 2 = ");
//           Serial.print(CellVoltage2*0.001); 
           }
           if(Counter ==5)
          {
           a12 = Stored;
          }
          if(Counter ==6)
          {
           a13 = Stored;
           CellVoltage7 = (a12<<8) + a13;
//           Serial.print("Cell Voltage 3 = ");
//          Serial.print(CellVoltage3*0.001); 
        }
        if(Counter ==7)
          {
           a14 = Stored;
          }
          if(Counter ==8)
          {
           a15 = Stored;
           CellVoltage8 = (a14<<8) + a15;
//           Serial.print("Cell Voltage 4 = ");
//          Serial.print(CellVoltage4*0.001); 
        }
        }
      }

/*************************************************CELL VOLTAGE 9 to 12*********************************************/      

            if(CAN.packetId() == 241569033)                   // Green Tiger Battery 4 Cell voltages
      {
        while (CAN.available()) 
        {
          Stored =  CAN.read();
          Counter++;
          if(Counter ==1)
          {
           a16 = Stored;    
//           Serial.print("a0 = ");
//          Serial.print(a0);                                 
          }
  
          if(Counter ==2)
          {
           a17 = Stored;
//          Serial.print("a1 = ");
//          Serial.print(a1); 
          CellVoltage9 = (a16<<8) + a17;
//          Serial.print("Cell Voltage 1 = ");
//          Serial.print(CellVoltage1*0.001);                          
          }

          if(Counter ==3)
          {
           a18 = Stored;                                    
          }

          if(Counter ==4)
          {
           a19 = Stored;
           CellVoltage10 = (a18<<8) + a19;
//           Serial.print("Cell Voltage 2 = ");
//           Serial.print(CellVoltage2*0.001); 
           }
           if(Counter ==5)
          {
           a20 = Stored;
          }
          if(Counter ==6)
          {
           a21 = Stored;
           CellVoltage11 = (a20<<8) + a21;
//           Serial.print("Cell Voltage 3 = ");
//          Serial.print(CellVoltage3*0.001); 
        }
        if(Counter ==7)
          {
           a22 = Stored;
          }
          if(Counter ==8)
          {
           a23 = Stored;
           CellVoltage12 = (a22<<8) + a23;
//           Serial.print("Cell Voltage 4 = ");
//          Serial.print(CellVoltage4*0.001); 
        }
        }
      }

/*************************************************CELL VOLTAGE 13 to 16*********************************************/

            if(CAN.packetId() == 241634569)                   // Green Tiger Battery 4 Cell voltages
      {
        while (CAN.available()) 
        {
          Stored =  CAN.read();
          Counter++;
          if(Counter ==1)
          {
           a24 = Stored;    
//           Serial.print("a0 = ");
//          Serial.print(a0);                                 
          }
  
          if(Counter ==2)
          {
           a25 = Stored;
//          Serial.print("a1 = ");
//          Serial.print(a1); 
          CellVoltage13 = (a24<<8) + a25;
//          Serial.print("Cell Voltage 1 = ");
//          Serial.print(CellVoltage1*0.001);                          
          }

          if(Counter ==3)
          {
           a27 = Stored;                                    
          }

          if(Counter ==4)
          {
           a28 = Stored;
           CellVoltage14 = (a27<<8) + a28;
//           Serial.print("Cell Voltage 2 = ");
//           Serial.print(CellVoltage2*0.001); 
           }
           if(Counter ==5)
          {
           a29 = Stored;
          }
          if(Counter ==6)
          {
           a30 = Stored;
           CellVoltage15 = (a29<<8) + a30;
//           Serial.print("Cell Voltage 3 = ");
//          Serial.print(CellVoltage3*0.001); 
        }
        if(Counter ==7)
          {
           a31 = Stored;
          }
          if(Counter ==8)
          {
           a32 = Stored;
           CellVoltage16 = (a31<<8) + a32;
//           Serial.print("Cell Voltage 4 = ");
//          Serial.print(CellVoltage4*0.001); 
        }
        }
      }

/*************************************************CELL VOLTAGE 17 to 20*********************************************/

            if(CAN.packetId() == 241700105)                   // Green Tiger Battery Cell voltages
      {
        while (CAN.available()) 
        {
          Stored =  CAN.read();
          Counter++;
          if(Counter ==1)
          {
           a33 = Stored;    
//           Serial.print("a0 = ");
//          Serial.print(a0);                                 
          }
  
          if(Counter ==2)
          {
           a34 = Stored;
//          Serial.print("a1 = ");
//          Serial.print(a1); 
          CellVoltage17 = (a33<<8) + a34;
//          Serial.print("Cell Voltage 1 = ");
//          Serial.print(CellVoltage1*0.001);                          
          }

          if(Counter ==3)
          {
           a35 = Stored;                                    
          }

          if(Counter ==4)
          {
           a36 = Stored;
           CellVoltage18 = (a35<<8) + a36;
//           Serial.print("Cell Voltage 2 = ");
//           Serial.print(CellVoltage2*0.001); 
           }
           if(Counter ==5)
          {
           a37 = Stored;
          }
          if(Counter ==6)
          {
           a38 = Stored;
           CellVoltage19 = (a37<<8) + a38;
//           Serial.print("Cell Voltage 3 = ");
//          Serial.print(CellVoltage3*0.001); 
        }
        if(Counter ==7)
          {
           a39 = Stored;
          }
          if(Counter ==8)
          {
           a40 = Stored;
           CellVoltage20 = (a39<<8) + a40;
//           Serial.print("Cell Voltage 4 = ");
//          Serial.print(CellVoltage4*0.001); 
        }
        }
      }
/**********************************************BATTERY TEMPERATURE*******************************************/      
      
      if(CAN.packetId() == 241962249)                       // Green Tiger Battery Temp1,2,3,4,5
    {
      while (CAN.available()) 
      {
        Stored =  CAN.read();
        Counter++;
        
           if(Counter ==1)
             {
              t1 = Stored;
//              Serial.print("Temp 1 = ");
//              Serial.print(t1);
//              Serial.println();
             }
    
           if(Counter ==2)
             {
              t2 = Stored;
//              Serial.print("Temp 2 = ");
//              Serial.print(t2);
//              Serial.println();
             }
      
           if(Counter ==3)
             {
              t3 = Stored;
//              Serial.print("Temp 3 = ");
//              Serial.print(t3);
//              Serial.println();
             }
    
           if(Counter ==4)
             {
              t4 = Stored;
//              Serial.print("Temp 4 = ");
//              Serial.print(t4);
//              Serial.println();
             }
    
           if(Counter ==5)
             {
              t5 = Stored;
//              Serial.print("Temp 5 = ");
//              Serial.print(t5);
//              Serial.println();
             }
           }
         }
       }
     }
  }

void Time()
{
    Seconds = millis()/1000;  
}

void SDCARD() {
    String DATA = "";

        DATA += String(dayStamp);
        DATA += ","; 
        DATA += String(timeStamp);
        DATA += ","; 
        DATA += String(Voltage*0.1);
        DATA += ",";  
//        if(Current=6553.50)
//        {
////        DATA += String("0");
////        DATA += ","; 
////        }
////        else
////        {
//        DATA += String(((65535-Current)*0.1));
        DATA += String(Current*0.1);
        DATA += ",";
//        }
//           DATA += String(((65535-Current)*0.1) * ( Voltage * 0.1 ));          
//          DATA += ",";
          DATA += String(soc*0.1);
          DATA += ",";     
          //          DATA += String(remainingAh*0.1);
          //          DATA += ","; 
          DATA += String(CellVoltage1);
          DATA += ","; 
          DATA += String(CellVoltage2);
          DATA += ","; 
          DATA += String(CellVoltage3);
          DATA += ","; 
          DATA += String(CellVoltage4);
          DATA += ","; 
          DATA += String(CellVoltage5);
          DATA += ","; 
          DATA += String(CellVoltage6);
          DATA += ","; 
          DATA += String(CellVoltage7);
          DATA += ","; 
          DATA += String(CellVoltage8);
          DATA += ","; 
          DATA += String(CellVoltage9);
          DATA += ","; 
          DATA += String(CellVoltage10);
          DATA += ","; 
          DATA += String(CellVoltage11);
          DATA += ","; 
          DATA += String(CellVoltage12);
          DATA += ","; 
          DATA += String(CellVoltage13);
          DATA += ","; 
          DATA += String(CellVoltage14);
          DATA += ","; 
          DATA += String(CellVoltage15);
          DATA += ","; 
          DATA += String(CellVoltage16);
          DATA += ","; 
          DATA += String(CellVoltage17);
          DATA += ","; 
          DATA += String(CellVoltage18);
          DATA += ","; 
          DATA += String(CellVoltage19);
          DATA += ","; 
          DATA += String(CellVoltage20);
          DATA += ","; 
          DATA += String(t1);
          DATA += ",";          
          DATA += String(t2);
          DATA += ",";
          DATA += String(t3);
          DATA += ",";
          DATA += String(t4);
          DATA += ",";
          DATA += String(t5);
          DATA += "\r\n";

  Serial.println(DATA);  
  appendFile(SD, "/GreenTiger_Battery_DataLog.txt", DATA.c_str());

}
