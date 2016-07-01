/*
To sense the temperature by the LM35 and if this temperature is above the required temperature then send a message to the necessary person. Also all the data collected is sent to the cloud in the ThingSpeak account.
*/
#include <SPI.h>
#include <Ethernet.h>
#include<SoftwareSerial.h>
SoftwareSerial mySerial(9,10);

// Local Network Settings
byte mac[] = { 0xD4, 0x28, 0xB2, 0xFF, 0xA0, 0xA1 }; // Must be unique on local network

// ThingSpeak Settings
char thingSpeakAddress[] = "api.thingspeak.com";
String writeAPIKey ="xxxxxxxxxxxxxxxxxx";  //Enter the WriteAPI code
const int updateThingSpeakInterval = 16 * 1000;      // Time interval in milliseconds to update ThingSpeak (number of seconds * 1000 = interval)

// Variable Setup
long lastConnectionTime = 0; 
boolean lastConnected = false;
int failedCounter = 0;
float val;
int tempPin = A0;
#define TEMPMAX=50;
// Initialize Arduino Ethernet Client
EthernetClient client;

/////////////////////
//SETUP
void setup()
{
  // Start Serial for debugging on the Serial Monitor
  Serial.begin(9600);
mySerial.begin(9600);
analogReference(1); 
  // Start Ethernet on Arduino
  startEthernet();
//calls the GSM function to send message
  analogGSM();
}

void loop()
{
 
   String analogValue0 = String(analogRead(A0), DEC);
  
  // Print Update Response to Serial Monitor
  if (client.available())
  {
    char c = client.read();
    Serial.print(c);
  }

  // Disconnect from ThingSpeak
  if (!client.connected() && lastConnected)
  {
    Serial.println("...disconnected");
    Serial.println();
    
    client.stop();
  }
  
  // Update ThingSpeak
  if(!client.connected() && (millis() - lastConnectionTime > updateThingSpeakInterval))
  {
    updateThingSpeak("field1="+analogValue0);
  }
  
  // Check if Arduino Ethernet needs to be restarted
  if (failedCounter > 3 ) {startEthernet();}
  
  lastConnected = client.connected();
  
}
void analogGSM()
{
  val = analogRead(tempPin);//read temp
float mv = ( val/1024.0)*5000; //convert
float cel = mv/10;
 Serial.println(cel);
delay(5000);
if(cel>TEMPMAX)
{
  if(Serial.available()>0)
  {
    Serial.println("ready..."); //Ready to send message
    mySerial.println("AT+CMGF=1");
  delay(1000);
  Serial.print("0");
  mySerial.println("AT+CMGS=\"+91xxxxxxxxxx\"\r"); //Send the message to this number
  delay(1000);
  String stringOne =  String(cel);
  Serial.print("1");
   mySerial.println("The temperature of the appliance is greater than usual. It is  "); 
  mySerial.print(stringOne);
  Serial.print("2");
  delay(100);
  Serial.print("3");
  mySerial.println((char)26);
  delay(5000);
  }
 }
}

 

void updateThingSpeak(String tsData)
{
  if (client.connect(thingSpeakAddress, 80))
  {         
    client.print("POST /update HTTP/1.1\n");
    client.print("Host: api.thingspeak.com\n");
    client.print("Connection: close\n");
    client.print("X-THINGSPEAKAPIKEY: "+writeAPIKey+"\n");
    client.print("Content-Type: application/x-www-form-urlencoded\n");
    client.print("Content-Length: ");
    client.print(tsData.length());
    client.print("\n\n");

    client.print(tsData);
    
    lastConnectionTime = millis();
    
    if (client.connected()) //Connection to ThingSpeak possible
    {
      Serial.println("Connecting to ThingSpeak...");
      Serial.println();
      
      failedCounter = 0;
    }
    else
    {
      failedCounter++;
  
      Serial.println("Connection to ThingSpeak failed ("+String(failedCounter, DEC)+")");   
      Serial.println();
    }
    
  }
  else
  {
    failedCounter++;
    
    Serial.println("Connection to ThingSpeak Failed ("+String(failedCounter, DEC)+")");   
    Serial.println();
    
    lastConnectionTime = millis(); 
  }
}

void startEthernet()
{
  
  client.stop();

  Serial.println("Connecting Arduino to network...");
  Serial.println();  

  delay(1000);
  
  if (Ethernet.begin(mac) == 0)
  {
    Serial.println("DHCP Failed, reset Arduino to try again");  //unable to connect to network using 
//DHCP
    Serial.println();
  }
  else
  {
    Serial.println("Arduino connected to network using DHCP");
    Serial.println();
  }
  
  delay(1000);
}

