/*
  Wireless Servo Control, with ESP as Access Point

  Usage: 
    Connect phone or laptop to "ESP_XXXX" wireless network, where XXXX is the ID of the robot
    Go to 192.168.4.1. 
    A webpage with four buttons should appear. Click them to move the robot.

  Installation: 
    In Arduino, go to Tools > ESP8266 Sketch Data Upload to upload the files from ./data to the ESP
    Then, in Arduino, compile and upload sketch to the ESP

  Requirements:
    Arduino support for ESP8266 board
      In Arduino, add URL to Files > Preferences > Additional Board Managers URL.
      See https://learn.sparkfun.com/tutorials/esp8266-thing-hookup-guide/installing-the-esp8266-arduino-addon

    Websockets library
      To install, Sketch > Include Library > Manage Libraries... > Websockets > Install
      https://github.com/Links2004/arduinoWebSockets
    
    ESP8266FS tool
      To install, create "tools" folder in Arduino, download, and unzip. See 
      https://github.com/esp8266/Arduino/blob/master/doc/filesystem.md#uploading-files-to-file-system

  Hardware: 
  * NodeMCU Amica DevKit Board (ESP8266 chip)
  * Motorshield for NodeMCU 
  * 2 continuous rotation servos plugged into motorshield pins D1, D2
  * Ultra-thin power bank 
  * Paper chassis

*/

#include <Arduino.h>

#include <Hash.h>
#include <FS.h>
#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <WebSocketsServer.h>
#include <ESP8266mDNS.h>

#include <Servo.h>
#include <ArduinoJson.h>
#include <SPI.h>
#include <Wire.h>
#include <VL53L0X.h>
#define SDA_PORT 14
#define SCL_PORT 12
VL53L0X sensor;
VL53L0X sensor2;
#define    MPU9250_ADDRESS            0x68
#define    MAG_ADDRESS                0x0C

#define    GYRO_FULL_SCALE_250_DPS    0x00  
#define    GYRO_FULL_SCALE_500_DPS    0x08
#define    GYRO_FULL_SCALE_1000_DPS   0x10
#define    GYRO_FULL_SCALE_2000_DPS   0x18

#define    ACC_FULL_SCALE_2_G        0x00  
#define    ACC_FULL_SCALE_4_G        0x08
#define    ACC_FULL_SCALE_8_G        0x10
#define    ACC_FULL_SCALE_16_G       0x18

float dest1[3] = {0,0,0};
float dest2[3] = {1,1,1};

#include "debug.h"
#include "file.h"
#include "server.h"

unsigned long StartTime = millis();
const int SERVO_LEFT = D2;
const int SERVO_RIGHT = D1;
Servo servo_left;
Servo servo_right;
int servo_left_ctr = 90;
int servo_right_ctr = 90;


// WiFi AP parameters
char ap_ssid[13];
char* ap_password = "";

float d1=10;
float d2 =30;
float a=90;
// WiFi STA parameters
char* sta_ssid = 
  "...";
char* sta_password = 
  "...";

char* mDNS_name = "paperbot";

String html;
String css;
WiFiServer server(100);
typedef struct Input
{
  float t_L;
  float t_R;
};
typedef struct Measure
{
  float lx;
  float ly;
  float alpha;
};
void setup() {
    setupPins();
    sensorsSetup();
    // sprintf(ap_ssid, "ESP_%08X", ESP.getChipId());
    for(uint8_t t = 4; t > 0; t--) {
        Serial.printf("[SETUP] BOOT WAIT %d...\n", t);
        Serial.flush();
        LED_ON;
        delay(500);
        LED_OFF;
        delay(500);
    }
    LED_ON;
    //setupSTA(sta_ssid, sta_password);
    setupAP(ap_ssid, ap_password);
    LED_OFF;

    setupFile();
    html = loadFile("/controls.html");
    css = loadFile("/style.css");
    registerPage("/", "text/html", html);
    registerPage("/style.css", "text/css", css);

    setupHTTP();
    setupWS(webSocketEvent);
    //setupMDNS(mDNS_name);

    stop();
    //server.begin();
}

void loop() {
    wsLoop();
    httpLoop();
    receiveData();
}


//
// Movement Functions //
//

void drive(int left, int right) {
  servo_left.write(left);
  servo_right.write(right);
}

void stop() {
  DEBUG("stop");
  drive(servo_left_ctr, servo_right_ctr);
  LED_OFF;
}

void forward() {
  DEBUG("forward");
  drive(0, 180);
}

void backward() {
  DEBUG("backward");
  drive(180, 0);
}

void left() {
  DEBUG("left");
  drive(0, 0);
}

void right() {
  DEBUG("right");
  drive(180, 180);
}



//
// Setup //
//

void setupPins() {
    // setup Serial, LEDs and Motors
    Serial.begin(115200);
    DEBUG("Started serial.");

    pinMode(LED_PIN, OUTPUT);    //Pin D0 is LED
    LED_OFF;                     //Turn off LED
    DEBUG("Setup LED pin.");

    servo_left.attach(SERVO_LEFT);
    servo_right.attach(SERVO_RIGHT);
    DEBUG("Setup motor pins");
}
void sensorsSetup() {  
  pinMode(D3, OUTPUT);
  pinMode(D4, OUTPUT);
  digitalWrite(D7, LOW);
  digitalWrite(D8, LOW);
  delay(500);
  Wire.begin(SDA_PORT,SCL_PORT);
  digitalWrite(D3, HIGH);
  delay(150);
  Serial.println("00");  
  sensor.init(true);
  Serial.println("01");
  delay(100);
  sensor.setAddress((uint8_t)22);
  digitalWrite(D4, HIGH);
  delay(150);
  sensor2.init(true);
  Serial.println("03");
  delay(100);
  sensor2.setAddress((uint8_t)25);
  Serial.println("04");
  Serial.println("addresses set");  
  Serial.println ("I2C scanner. Scanning ...");
  byte count = 0;
  Wire.begin(SDA_PORT,SCL_PORT);  
   // Set by pass mode for the magnetometers
  I2CwriteByte(MPU9250_ADDRESS,0x37,0x02);  
  // Request first magnetometer single measurement
  I2CwriteByte(MAG_ADDRESS,0x0A,0x01);
  for (byte i = 1; i < 120; i++)
  {
    Wire.beginTransmission (i);
    if (Wire.endTransmission () == 0)
    {
      Serial.print ("Found address: ");
      Serial.print (i, DEC);
      Serial.print (" (0x");
      Serial.print (i, HEX);
      Serial.println (")");
      count++;
      delay (1);  // maybe unneeded?
    } // end of good response
  } // end of for loop
  Serial.println ("Done.");
  Serial.print ("Found ");
  Serial.print (count, DEC);
  Serial.println (" device(s).");
  delay(3000);
}

void sendData(struct Input u, struct Measure y){
   // Check if a client has connected
  WiFiClient client = server.available();
  if (!client) {
    return;
  }
  Serial.println("Making Json file");
   // Read the first line of the request
  String req = client.readStringUntil('\r');
  Serial.println(req);
  client.flush();
  client.flush();

  
  StaticJsonBuffer<500> jsonBuffer;

  JsonObject& root = jsonBuffer.createObject();
  root["tau_L"]=u.t_L;
  root["tau_R"]=u.t_R;
  root["lx"]= y.lx;
  root["ly"]= y.ly;
  root["alpha"]= y.alpha;
  
  Serial.print(F("Sending: "));
  root.printTo(Serial);
  Serial.println();
  
  // Write response headers
  client.println("HTTP/1.0 200 OK");
  client.println("Content-Type: application/json");
  client.println("Connection: close");
  client.println();

  // Write JSON document
  root.prettyPrintTo(client);

  // Disconnect
  client.stop();
}

void receiveData(){
   // Check if a client has connected
  WiFiClient client = server.available();
  if (!client) {
    return;
  }
  Serial.println("Making Json file");
   // Read the first line of the request
  String req = client.readStringUntil('\r');
  req = req.substring(5, req.length());
  Serial.println(req);
  int ind1 = req.indexOf(',');  //finds location of first ,
  int L_1 = req.substring(0, ind1).toInt();   //captures first data String
  int ind2 = req.indexOf(',', ind1+1 );   //finds location of second ,
  int R_1 = req.substring(ind1+1, ind2+1).toInt();   //captures second data String
  int ind3 = req.indexOf(',', ind2+1 );
  int L_2 = req.substring(ind2+1, ind3+1).toInt();
  int ind4 = req.indexOf(',', ind3+1 );
  int R_2 = req.substring(ind3+1).toInt(); //captures remain part of data after last ,
  int ind5 = req.indexOf(',', ind4+1 );
  int L_3 = req.substring(ind4+1, ind5+1).toInt();
  int ind6 = req.indexOf(',', ind5+1 );
  int R_3 = req.substring(ind5+1).toInt();
  Serial.println(L_1);
  Serial.println(R_1);
  Serial.println(L_2);
  Serial.println(R_2);
  Serial.println(L_3);
  Serial.println(R_3);
  client.flush();
  client.flush();
  StaticJsonBuffer<500> jsonBuffer;
  JsonObject& root = jsonBuffer.createObject();
  root["garbage"] = "garbage";
  
  Serial.print(F("Sending: "));
  root.printTo(Serial);
  Serial.println();
  
  // Write response headers
  client.println("HTTP/1.0 200 OK");
  client.println("Content-Type: application/json");
  client.println("Connection: close");
  client.println();

  // Write JSON document
  root.prettyPrintTo(client);

  // Disconnect
  client.stop();
  // Now do the control inputs
  // Turning signal
  if (L_1 > R_1) {
    // turn right
    right();
    delay(L_1);
    stop();
  } else {
    // turn left
    left();
    delay(R_1);
    stop();
  }
  delay(500);
  // Straight Line Signal
  if (L_2 >= 0) {
    // Go forward
    forward();
    delay(L_2);
    stop();
  } else {
    backward();
    delay(-L_2);
    stop();
  }
  delay(500);
  // Turning signal
  if (L_3 > R_3) {
    // turn right
    right();
    delay(L_3);
    stop();
  } else {
    // turn left
    left();
    delay(R_3);
    stop();
  }
  // Send Measurement Data back after executing controls
   struct Input u;
   u.t_L = 0;
   u.t_R = 0;
   struct Measure y = getMeasurement();
   sendData(u,y);
}

void I2Cread(uint8_t Address, uint8_t Register, uint8_t Nbytes, uint8_t* Data)
{
  // Set register address
  Wire.beginTransmission(Address);
  Wire.write(Register);
  Wire.endTransmission();
  
  // Read Nbytes
  Wire.requestFrom(Address, Nbytes); 
  uint8_t index=0;
  while (Wire.available())
    Data[index++]=Wire.read();
}


// Write a byte (Data) in device (Address) at register (Register)
void I2CwriteByte(uint8_t Address, uint8_t Register, uint8_t Data)
{
  // Set register address
  Wire.beginTransmission(Address);
  Wire.write(Register);
  Wire.write(Data);
  Wire.endTransmission();
}
struct Measure getMeasurement() {
    // _____________________
  // :::  Magnetometer ::: 

  // Request first magnetometer single measurement
  I2CwriteByte(MAG_ADDRESS,0x0A,0x01);
  
  // Read register Status 1 and wait for the DRDY: Data Ready
  
  uint8_t ST1;
  do
  {
    I2Cread(MAG_ADDRESS,0x02,1,&ST1);
  }
  while (!(ST1&0x01));

  // Read magnetometer data  
  uint8_t Mag[7];  
  I2Cread(MAG_ADDRESS,0x03,7,Mag);

  // Create 16 bits values from 8 bits data
  
  // Magnetometer
  int16_t mx=(Mag[1]<<8 | Mag[0]);
  int16_t my=(Mag[3]<<8 | Mag[2]);
  int16_t mz=(Mag[5]<<8 | Mag[4]);
  mx = dest2[0] * (mx - dest1[0]);
  my = dest2[1] * (my - dest1[1]);
  mz = dest2[2] * (mz - dest1[2]);
  float heading = atan2(mx, my);

  // Once you have your heading, you must then add your 'Declination Angle',
  // which is the 'Error' of the magnetic field in your location. Mine is 0.0404 
  // Find yours here: http://www.magnetic-declination.com/
  
  // If you cannot find your Declination, comment out these two lines, your compass will be slightly off.
  float declinationAngle = 0.0467;
  heading += declinationAngle;

  // Correct for when signs are reversed.
  if(heading < 0)
    heading += 2*PI;

  // Check for wrap due to addition of declination.
  if(heading > 2*PI)
    heading -= 2*PI;

  // Convert radians to degrees for readability.
  float headingDegrees = heading * 180/PI; 
  struct Measure y;
  y.lx = sensor2.readRangeSingleMillimeters();
  y.ly = sensor.readRangeSingleMillimeters();
  y.alpha = heading;
  Serial.print("Theta:  ");
  Serial.print(headingDegrees);
  Serial.print("\t");
  Serial.print("lx: ");
  Serial.print(sensor2.readRangeSingleMillimeters());
  Serial.print("\t");
  Serial.print("ly: ");
  Serial.println(sensor.readRangeSingleMillimeters());
  return y;
}
struct Input actionToInput(char action, float dur){
  struct Input u;
  if (action == 'F') {
    u.t_L = dur;
    u.t_R = dur;
  } else if (action == 'B') {
    u.t_L = -dur;
    u.t_R = -dur;
  } else if (action == 'L') {
    u.t_L = -dur;
    u.t_R = dur;
  } else if (action == 'R') {
    u.t_L = dur;
    u.t_R = -dur;
  }
  Serial.print("\t");
  Serial.print("u = \t");
  Serial.print(u.t_L);
  Serial.print('\t');
  Serial.println(u.t_R);
  return u;
}
char SavedAction = 'Z';
void webSocketEvent(uint8_t id, WStype_t type, uint8_t * payload, size_t length) {

    switch(type) {
        case WStype_DISCONNECTED:
            DEBUG("Web socket disconnected, id = ", id);
            break;
        case WStype_CONNECTED: 
        {
            // IPAddress ip = webSocket.remoteIP(id);
            // Serial.printf("[%u] Connected from %d.%d.%d.%d url: %s\n", id, ip[0], ip[1], ip[2], ip[3], payload);
            DEBUG("Web socket connected, id = ", id);

            // send message to client
            wsSend(id, "Connected to ");
            wsSend(id, ap_ssid);
            break;
        }
        case WStype_BIN:
            DEBUG("On connection #", id)
            DEBUG("  got binary of length ", length);
            for (int i = 0; i < length; i++)
              DEBUG("    char : ", payload[i]);

            if (payload[0] == '~') 
              drive(180-payload[1], payload[2]);

        case WStype_TEXT:
            DEBUG("On connection #", id)
            DEBUG("  got text: ", (char *)payload);

            if (payload[0] == '#') {
                if(payload[1] == 'C') {
                  LED_ON;
                  wsSend(id, "Hello world!");
                }
                else if(payload[1] == 'F') {
                  SavedAction = 'F';
                  StartTime = millis();
                  forward();
                }
                else if(payload[1] == 'B') {
                  SavedAction = 'B';
                  StartTime = millis();
                  backward();
                }
                else if(payload[1] == 'L') {
                  SavedAction = 'L';
                  StartTime = millis();
                  left();
                }
                else if(payload[1] == 'R') {
                  SavedAction = 'R';
                  StartTime = millis();
                  right();
                }
                else if(payload[1] == 'U') {
                  if(payload[2] == 'L') 
                    servo_left_ctr -= 1;
                  else if(payload[2] == 'R') 
                    servo_right_ctr += 1;
                  char tx[20] = "Zero @ (xxx, xxx)";
                  sprintf(tx, "Zero @ (%3d, %3d)", servo_left_ctr, servo_right_ctr);
                  wsSend(id, tx);
                }
                else if(payload[1] == 'D') {
                  if(payload[2] == 'L') 
                    servo_left_ctr += 1;
                  else if(payload[2] == 'R') 
                    servo_right_ctr -= 1;
                  char tx[20] = "Zero @ (xxx, xxx)";
                  sprintf(tx, "Zero @ (%3d, %3d)", servo_left_ctr, servo_right_ctr);
                  wsSend(id, tx);
                }
                else {
                  stop();
                  unsigned long CurrentTime = millis();
                  unsigned long ElapsedTime = CurrentTime - StartTime;
                  Serial.println("");
                  Serial.print(SavedAction);
                  Serial.print(ElapsedTime);
                  struct Input u = actionToInput(SavedAction, ElapsedTime);    
                  struct Measure y = getMeasurement();
                  sendData(u,y);
                  Serial.print("Sent Data");
                }
            }

            break;
    }
}
