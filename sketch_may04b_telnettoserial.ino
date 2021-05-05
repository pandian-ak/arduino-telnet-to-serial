#include <WiFi.h>
#include <WiFiMulti.h>

WiFiMulti wifiMulti;

//how many clients should be able to telnet to this ESP32
#define MAX_SRV_CLIENTS 1
const char* ssid = "SRINIVAS";
const char* password = "9500065577";

WiFiServer server(23);
WiFiClient serverClients[MAX_SRV_CLIENTS];

void setup() {
  Serial.begin(115200);
  Serial.println("\nConnecting");

   WiFi.begin(ssid, password);

  // -->Try to connect to particular host for 20 times, If still not connected then automatically resets.
  Serial.print("\nConnecting to "); Serial.println(ssid);
  Serial.println(WiFi.status());
  uint8_t i = 0;
  while (WiFi.status() != WL_CONNECTED && i++ < 20) delay(500);
  if(i == 21){
    Serial.print("Could not connect to"); Serial.println(ssid);
    while(1);
  }

  //start UART and the server
  //Serial.begin(115200);
  server.begin();
  server.setNoDelay(true);

  Serial.print("Ready! Use 'telnet ");
  Serial.print(WiFi.localIP());
  Serial.println(" 23' to connect");
}

void loop() {
  uint8_t i;
  //check if there are any new clients
  if (server.hasClient()){
    for(i = 0; i < MAX_SRV_CLIENTS; i++){
      //find free/disconnected spot
      if (!serverClients[i] || !serverClients[i].connected()){
        if(serverClients[i]) serverClients[i].stop();
        serverClients[i] = server.available();
        Serial.print("New client: "); Serial.print(i);
        continue;
      }
    }
    //no free/disconnected spot so reject
    WiFiClient serverClient = server.available();
    serverClient.stop();
  }
    //check clients for data
    for(i = 0; i < MAX_SRV_CLIENTS; i++){
      if (serverClients[i] && serverClients[i].connected()){
        if(serverClients[i].available()){
          //get data from the telnet client and push it to the UART
          while(serverClients[i].available()) Serial.write(serverClients[i].read());
        }
      }
      else {
        if (serverClients[i]) {
          serverClients[i].stop();
        }
      }
    }
    //check UART for data
    if(Serial.available()){
      size_t len = Serial.available();
      uint8_t sbuf[len];
      Serial.readBytes(sbuf, len);
      //push UART data to all connected telnet clients
      for(i = 0; i < MAX_SRV_CLIENTS; i++){
        if (serverClients[i] && serverClients[i].connected()){
          serverClients[i].write(sbuf, len);
          delay(1);
        }
      }
    }
}
