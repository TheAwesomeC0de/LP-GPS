#include <SoftwareSerial.h>
#include<TinyGPS.h>
//Create software serial object to communicate with SIM800L
SoftwareSerial mySerial(3,2); //SIM800L Tx & Rx is connected to Arduino #3 & #2

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
    Serial.println(sapbr("0,1"));
  }

class http
{
  public:
  void req(String request)
  {
    String temp="HTTP";

    Serial.println(t_com(temp+"INIT"));
    delay(10);
    Serial.println(t_com(temp+"PARA=\"CID\",1"));
    delay(10);
    Serial.println(t_com(temp+"PARA=\"URL\",\""+request+"\""));
    delay(10);
    Serial.println(t_com(temp+"ACTION=0"));
    mySerial.flush();
    while(!mySerial.available())
    {
      delay(50);
    }
    Serial.write(mySerial.read());
    delay(10);
    Serial.println(t_com(temp+"READ"));
    t_com(temp+"TERM");
  }
};

void setup()
{
  //Begin serial communication with Arduino and Arduino IDE (Serial Monitor)
  Serial.begin(9600);
  
  //Begin serial communication with Arduino and SIM800L
  mySerial.begin(9600);

  Serial.println("Initializing...");
  delay(1000);
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
  gprs_start("portalnmms"); // APN name, depends on the 2g service provider, (portalnmms for vodafone)
  http comm;
  comm.req("httpbin.org/get?data=hello+world");
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
