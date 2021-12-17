#include <SoftwareSerial.h>
#include<ArduinoJson.h>
#include <TinyGPSPlus.h> 
TinyGPSPlus tinyGPS; 

#define GPS_BAUD 9600 
#include <SoftwareSerial.h>
#define ARDUINO_GPS_RX 7 // GPS TX, Arduino RX pin
#define ARDUINO_GPS_TX 6 // GPS RX, Arduino TX pin
SoftwareSerial ssGPS(ARDUINO_GPS_TX, ARDUINO_GPS_RX); 
#define gpsPort ssGPS
SoftwareSerial mySerial(10,11); //SIM800L Tx & Rx is connected to Arduino #10 & #11

StaticJsonBuffer<200> jsonBuffer; 

//2G functions  

String t_com(String str)
{
  str="AT+"+str;
  mySerial.println(str);
  String A;
  while(!mySerial.available())
  {
    delay(50);
  }
  A = String(mySerial.readString());
  return A;
}

String sapbr(String str)
{
  str="SAPBR="+str;
  return t_com(str);
}

void gprs_start(String apn)
  {
    Serial.println(sapbr("3,1,\"Contype\", \"GPRS\""));
    delay(50);
    Serial.println(sapbr("3,1,\"APN\",\""+apn+"\""));
    delay(50);
    Serial.println(sapbr("1,1")); // This is the command to connect the gprs, can take some time, look for led queue
    delay(50);
    Serial.println(sapbr("2,1")); // Display the gprs status
    delay(50);
  }
void gprs_close()
  {
    //Serial.println(sapbr("0,1"));
  }

class http
{
  public:
  void req(String request ,String sendtoserver)
  {
    String temp="HTTP";

    Serial.println(t_com(temp+"INIT"));
    delay(10);
    Serial.println(t_com(temp+"PARA=\"CID\",1"));
    delay(10);
    Serial.println(t_com(temp+"PARA=\"URL\",\""+request+"\""));
    delay(10);
    Serial.println(t_com("HTTPPARA=\"CONTENT\",\"application/json\""));
    delay(10);
    Serial.println(t_com("HTTPDATA=" + String(sendtoserver.length()) + ",100000"));
    mySerial.println(sendtoserver);
    while(!mySerial.available())
    {
      delay(50);
    }
    String A = String(mySerial.readString());
    Serial.println(A);
    Serial.println(t_com(temp+"ACTION=1"));
    mySerial.flush();
    while(!mySerial.available())
    {
      delay(50);
    }
    Serial.write(mySerial.read());
    delay(10);
    Serial.println(t_com(temp+"READ"));
    Serial.println(t_com(temp+"TERM"));
  }
};

void setup()
{
  //Begin serial communication with Arduino and Arduino IDE (Serial Monitor)
  Serial.begin(9600);
  
  //Begin serial communication with Arduino and SIM800L

  Serial.println("Initializing...");
  gpsPort.begin(GPS_BAUD);
  delay(10);
  double lat,lng;
  smartDelay(1000);
  lat=tinyGPS.location.lat();
  lng=tinyGPS.location.lng();
  Serial.println(lat);
  Serial.println(lng);
  gpsPort.end();
  JsonObject& object = jsonBuffer.createObject();
  object.set("lat",lat);
  object.set("long",lng);
  Serial.println("lat"+String(lat));
  Serial.println("lng"+String(lng));
  String body="";
  object.prettyPrintTo(body);

  mySerial.begin(9600);
  mySerial.println("AT");
  delay(50);
  mySerial.flush();
  Serial.println(t_com("CSQ")); // Signal quality
  delay(50);
  Serial.println(t_com("CCID"));
  mySerial.flush();
  String A=t_com("CREG?"); //Check whether it has registered in the network
  Serial.println(A);
  delay(50);
  gprs_start("airtelgprs.com"); // APN name, depends on the 2g service provider, (portalnmms for vodafone)
  http comm;
  //Begin serial communication with gps module
  comm.req("tracker.orgfree.com/device.php",body);
  Serial.println(body);
  delay(5000);
  gprs_close();
}

void loop()
{
  updateSerial(); 
}

void updateSerial()
{
  delay(500);
  while (Serial.available()) 
  {
    mySerial.write(Serial.read());//Forward what Serial received to Software Serial Port
  }
  while(mySerial.available()) 
  {
    Serial.write(mySerial.read());//Forward what Software Serial received to Serial Port
  }
}

//GPS functions

static void smartDelay(unsigned long ms)
{
  unsigned long start = millis();
  do
  {
    // If data has come in from the GPS module
    while (gpsPort.available())
      tinyGPS.encode(gpsPort.read()); // Send it to the encode function
    // tinyGPS.encode(char) continues to "load" the tinGPS object with new
    // data coming in from the GPS module. As full NMEA strings begin to come in
    // the tinyGPS library will be able to start parsing them for pertinent info
  } while (millis() - start < ms);
}

