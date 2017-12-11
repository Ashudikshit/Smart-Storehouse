#include <SoftwareSerial.h>
SoftwareSerial espSerial =  SoftwareSerial(2,3);      // arduino RX pin=2  arduino TX pin=3    connect the arduino RX pin to esp8266 module TX pin   -  connect the arduino TX pin to esp8266 module RX pin
#include <LiquidCrystal_I2C.h>
#include <DHT.h>
#define dht_spin A2
int moistsensorPin = A0;  
int moistsensorValue = 0;  
int percent = 0;
int relaypin =9;
int smokepin=A1;
int sv=0;
dht DHT;
String apiKey = "N3GP85UIGYUN56D";     // replace with your channel's thingspeak WRITE API key

String ssid="Lifi";    // Wifi network SSID
String password ="72533185";  // Wifi network password

boolean DEBUG=true;

//======================================================================== showResponce
void showResponse(int waitTime){
    long t=millis();
    char c;
    while (t+waitTime>millis()){
      if (espSerial.available()){
        c=espSerial.read();
        if (DEBUG) Serial.print(c);
      }
    }
                   
}

//========================================================================
boolean thingSpeakWrite(float value1, float value2, int value3, int value4){
  String cmd = "AT+CIPSTART=\"TCP\",\"";                  // TCP connection
  cmd += "184.106.153.149";                               // api.thingspeak.com
  cmd += "\",80";
  espSerial.println(cmd);
  if (DEBUG) Serial.println(cmd);
  if(espSerial.find("Error")){
    if (DEBUG) Serial.println("AT+CIPSTART error");
    return false;
  }
  
  
  String getStr = "GET /update?api_key=";   // prepare GET string
  getStr += apiKey;
  
  getStr +="&field1=";
  getStr += String(value1);
  getStr +="&field2=";
  getStr += String(value2);
  getStr +="&field3=";
  getStr += String(value3);
  getStr +="&field4=";
  getStr += String(value4);
  
  getStr += "\r\n\r\n";

  // send data length
  cmd = "AT+CIPSEND=";
  cmd += String(getStr.length());
  espSerial.println(cmd);
  if (DEBUG)  Serial.println(cmd);
  
  delay(100);
  if(espSerial.find(">")){
    espSerial.print(getStr);
    if (DEBUG)  Serial.print(getStr);
  }
  else{
    espSerial.println("AT+CIPCLOSE");
    // alert user
    if (DEBUG)   Serial.println("AT+CIPCLOSE");
    return false;
  }
  return true;
}
//================================================================================ setup
void setup() {                
  DEBUG=true;           // enable debug serial
  Serial.begin(9600); 
  
  
    pinMode(relaypin, OUTPUT);
 digitalWrite(relaypin, HIGH);
  
  espSerial.begin(57600);  // enable software serial
                          // Your esp8266 module's speed is probably at 115200. 
                          // For this reason the first time set the speed to 115200 or to your esp8266 configured speed 
                          // and upload. Then change to 9600 and upload again
  
  //espSerial.println("AT+RST");         // Enable this line to reset the module;
  //showResponse(1000);

  //espSerial.println("AT+UART_CUR=9600,8,1,0,0");    // Enable this line to set esp8266 serial speed to 9600 bps
  //showResponse(1000);
  
  

  espSerial.println("AT+CWMODE=1");   // set esp8266 as client
  showResponse(1000);

  espSerial.println("AT+CWJAP=\""+ssid+"\",\""+password+"\"");  // set your home router SSID and password
  showResponse(5000);

   if (DEBUG)  Serial.println("Setup completed");
}


// ====================================================================== loop
void loop() {

moistsensorValue = analogRead(moistsensorPin);
  percent = convertToPercent(moistsensorValue);
  printValuesToSerial();
  delay(1000);
  sv=analogRead(smokepin);
Serial.println(sv);

if(sv>=400)
{
  digitalWrite(relaypin,LOW);
  }
else {
   digitalWrite(relaypin,HIGH);
}
delay(1000);
  // Read sensor values
  DHT.read11( dht_spin);
   float t = DHT.temperature;//dht.readTemperature();
   float h = DHT.humidity;//dht.readHumidity();
        if (isnan(t) || isnan(h)) {
        if (DEBUG) Serial.println("Failed to read from DHT");
      }
      else {
          if (DEBUG)  Serial.println("Temp="+String(t)+" *C");
          if (DEBUG) Serial.println("Humidity="+String(h)+" %");
           thingSpeakWrite(t,h,percent,sv);                                      // Write values to thingspeak
      }
  
    
  // thingspeak needs 15 sec delay between updates,     
  delay(10000);  
}

int convertToPercent(int value)
{
  int percentValue = 0;
  percentValue = map(value, 1023, 150, 0, 100);
  return percentValue;
}
void printValuesToSerial()
{
  Serial.print("\nmoisture Percent: ");
  
  Serial.print(percent);
  
  Serial.println("%");
}



