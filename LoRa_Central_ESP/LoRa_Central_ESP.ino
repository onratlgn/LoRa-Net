#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266HTTPClient.h>
#include <ArduinoJson.h>
#include <WebSocketClient.h>
#include <math.h>

// memory allocation
#define MaxT 3
#define MaxR 11
byte R[MaxR];
byte T[MaxT];
int L = 0;

// pin definitions
#define EN  D7
#define SET D8

// may be unnecesssary
const byte CentID[] = {0x00, 0x03};
const byte NodeID[] = {0x00, 0x04};
const byte CastID[] = {0xFF, 0xFF};

// WiFi config
const char* ssid = "IoX_Test";
const char* password = "ioxlab7766";
//const char* ssid = "USERSPOTS5";
//const char* password = "etkilab7766";

// http endpoint
const char* endpoint = "http://realsensor.herokuapp.com/poll";

// constant chars
const byte tempFlag     = 0x54;       //char T
const byte humFlag      = 0x48;       //char H
const byte lightFlag    = 0x4c;       //char L
const byte measureFlag  = 0x4D;       //char M


HTTPClient http;

/* Suspended until websocket
  // Channel initiation

  WiFiClient client;
  WebSocketClient wsclient;
  char* host = "192.168.2.8";
  int   port = 40510;
  char* path = "/";
  String data;
*/


// JSON initiation
String body;
StaticJsonBuffer<200> jsonBuffer;
JsonObject& root = jsonBuffer.createObject();

void setup() {

  // open serial port
  Serial.begin(9600);
  delay(20);

  // start wifi conection
  WiFi.disconnect();
  delay(20);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(100);
    //Serial.print('.');
  }
  //Serial.println(WiFi.localIP());

  /* Suspended until websocket
    // start websocket
    wsclient.host = host;
    wsclient.path = path;
    client.connect(host, port);
    wsclient.handshake(client);
  */

  // initiate LoRa modem
  pinMode(EN, OUTPUT);
  pinMode(SET, OUTPUT);
  digitalWrite(EN, LOW);
  digitalWrite(SET, HIGH);

}

void loop() {

  // when received data from a nodes
  L = 0;
  while (Serial.available() && L < MaxR) {
    R[L] = Serial.read();
    L++;
  }
  if (L > 0) {
    sendToServer();
  }

  /* Suspended until websocket
    // check WebSocket
    if (client.connected()) {
    wsclient.getData(data);
    if (data.length() > 0) {
      configureNode();
    }
    }
  */
}

/* Suspended until websocket
  void configureNode() {

  // alocate memory
  int len = data.length()+1;
  byte buf[len];

  // write data into memory
  data.getBytes(buf,len);

  // send data over UART
  Serial.write(buf,len);

  // reset data
  data = "";

  }
*/


void sendToServer() {

  // create request JSON body
  root["ID"]   = (int)((R[0] << 8) + R[1]);
  root["temp"] = (float)2.3*(175.72 * ((float)((R[3] << 8) + R[4]) / 65536) - 46.85);
  root["hum"]  = (float)(125.00 * ((float)((R[6] << 8) + R[7]) / 65536) - 6);
  root["light"] = 1.25e7*pow((double)(((R[9] << 8) + R[10])*26700)/1024,-1.4059);

  // write JSON into string
  root.printTo(body);

  //Serial.println(body);

  // make http post request
  http.begin(endpoint);
  http.addHeader("Content-Type", "application/json");
  http.POST(body);
  http.end();

  // reset body
  body = "";
}




