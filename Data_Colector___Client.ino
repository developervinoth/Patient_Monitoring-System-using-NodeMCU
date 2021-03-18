// initializes or defines the output pin of the LM35 temperature sensor










const char * host = "192.168.43.242";  //Your_Server_IP_Address" (Vibrator's IP Adress)

const int httpPort = 80;

const char* Commands;                       // The command variable that is sent to the server                     

                  // The variable to detect the button has been pressed
int con = 0;    





int outputpin= A0;
//this sets the ground pin to LOW and the input voltage pin to high
 
String textForSMS;


int tilt_flag=0;
int beat_flag=0;
int temp_flag=0;

#include <Wire.h>
#include "MAX30105.h"

#include "heartRate.h"

MAX30105 particleSensor;

const byte RATE_SIZE = 4; //Increase this for more averaging. 4 is good.
byte rates[RATE_SIZE]; //Array of heart rates
byte rateSpot = 0;
long lastBeat = 0; //Time at which the last beat occurred

float beatsPerMinute;
int beatAvg;

int tiltpin=13;




#include <ESP8266WiFi.h>
#include <WiFiClientSecure.h>
#include <UniversalTelegramBot.h>
#include <ArduinoJson.h>

const char* ssid = "Hello";
const char* password = "12341234";

#define BOTtoken "1642671481:AAHNR5GIAA2A-JAzOhnC7M67medo4dCOeHc"  // your Bot Token (Get from Botfather)
#define CHAT_ID "878903220"

WiFiClientSecure client;
UniversalTelegramBot bot(BOTtoken, client);




void setup() {
  Serial.begin(115200);
  
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(500);
  }

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
  Serial.println("Initializing...");
  
  Serial.println("");
  Serial.println("Client-------------------------------");
  Serial.print("Connecting to Network");

  // Initialize sensor
  if (!particleSensor.begin(Wire, I2C_SPEED_FAST)) //Use default I2C port, 400kHz speed
  {
    Serial.println("MAX30105 was not found. Please check wiring/power. ");
    while (1);
  }
  Serial.println("Place your index finger on the sensor with steady pressure.");

  particleSensor.setup(); //Configure sensor with default settings
  particleSensor.setPulseAmplitudeRed(0x0A); //Turn Red LED to low to indicate sensor is running
  particleSensor.setPulseAmplitudeGreen(0); //Turn off Green LED 


  
   client.setInsecure();



  Serial.print("Connecting Wifi: ");
  Serial.println(ssid);


  bot.sendMessage(CHAT_ID, "Bot started up", "");

  
}



void loop()       //main loop

{
  long irValue = particleSensor.getIR();

  if (checkForBeat(irValue) == true)
  {
    //We sensed a beat!
    long delta = millis() - lastBeat;
    lastBeat = millis();

    beatsPerMinute = 60 / (delta / 1000.0);

    if (beatsPerMinute < 255 && beatsPerMinute > 20)
    {
      rates[rateSpot++] = (byte)beatsPerMinute; //Store this reading in the array
      rateSpot %= RATE_SIZE; //Wrap variable

      //Take average of readings
      beatAvg = 0;
      for (byte x = 0 ; x < RATE_SIZE ; x++)
        beatAvg += rates[x];
      beatAvg /= RATE_SIZE;
    }
  }
  Serial.print("IR=");
  Serial.print(irValue);
  Serial.print(", BPM=");
  Serial.print(beatsPerMinute);
  Serial.print(", Avg BPM=");
  Serial.print(beatAvg);




  if (irValue < 50000)
    Serial.print(" No finger?");

  Serial.println();



  int tilt_value=digitalRead(tiltpin);
  Serial.println("Tilt Value: ");
Serial.print(tilt_value);
int analogValue = analogRead(outputpin);
float millivolts = (analogValue/1024.0) * 3300; //3300 is the voltage provided by NodeMCU
float celsius = millivolts/10;
Serial.print("in DegreeC=   ");
Serial.println(celsius);

//---------- Here is the calculation for Fahrenheit ----------//

float fahrenheit = ((celsius * 9)/5 + 32);
Serial.print(" in Farenheit=   ");
Serial.println(fahrenheit);




if(irValue<=6000){
  
    bot.sendMessage(CHAT_ID, "Abnormal Beat Detected !", "");
Commands="LED_On";
          Serial.println(Commands);
          send_commands();

    Serial.println("Abnormal HeartBeat");

  }
  
if(fahrenheit>=102.00){

Commands="LED_On";
    bot.sendMessage(CHAT_ID, "Abnormal Body Temperature Detected !", "");
          Serial.println(Commands);
          send_commands();

    Serial.println("Abnormal Body Temperature Detected !");

  }
  
if(tilt_value==1){
     bot.sendMessage(CHAT_ID, "Body Position Changed", "");
  Commands="LED_On";
          Serial.println(Commands);
          send_commands();

 
    Serial.println("Body Position Changed");


}
          Commands="LED_Off";
          Serial.println(Commands);
          send_commands();
          delay(1000);
}    


void send_commands(){
  Serial.println("Sending command...");
  Serial.println("Don't press the button for now...");
  Serial.println("");
  Serial.print("Connecting to ");
  Serial.println(host);

  // Use WiFiClient class to create TCP connections
  WiFiClient client;
  
  if (!client.connect(host, httpPort)) {
    Serial.println("Connection failed");
    return;
  }

  // We now create a URI for the request  
  Serial.print("Requesting URL : ");
  Serial.println(Commands);

  // This will send the request to the server
  client.print(String("GET ") + Commands + " HTTP/1.1\r\n" + "Host: " + host + "\r\n" + "Connection: Close\r\n\r\n");
  unsigned long timeout = millis();
  while (client.available() == 0) {
    if (millis() - timeout > 5000) {      
      Serial.println(">>> Client Timeout !");
      client.stop();
      return;
    }
  }
  
  Serial.print("Server Reply = "); 
  // Read all the lines of the reply from server and print them to Serial
  while(client.available()){
    String line = client.readStringUntil('\r');
    Serial.print(line);
  }
  Serial.println("Now you can press the button ...");
  Serial.println("-------------------------------------");
  Serial.println("");
}
