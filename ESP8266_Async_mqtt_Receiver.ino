//--------------------------------------------------------------------------------------------
//  ESP-32 Slave (Receiver) and Gateway to Internet using MQTT protocol
//--------------------------------------------------------------------------------------------
//  Include all essential libraries
//--------------------------------------------------------------------------------------------

#include <ESP8266WiFi.h>
#include <espnow.h>
#include <Ticker.h>
#include <AsyncMqttClient.h>
#include <Arduino_JSON.h>
#include <PubSubClient.h>

//--------------------------------------------------------------------------------------------
// Defining essential variables and constants globally
//--------------------------------------------------------------------------------------------

//MQTT Setup Start
#define mqtt_server "broker.hivemq.com"

//  Create an AsyncMqttClient object called mqttClient to handle the MQTT client 
//  and timers to reconnect to your MQTT broker and router when it disconnects.
AsyncMqttClient mqttClient;
Ticker mqttReconnectTimer;
WiFiEventHandler wifiConnectHandler;
WiFiEventHandler wifiDisconnectHandler;
Ticker wifiReconnectTimer;

//Topics to Publish
#define mqttDATA_1 "HM_IOT/DATA_1"
#define mqttDATA_2 "HM_IOT/DATA_2"
//MQTT Setup End

// Status check whether receiver is connected to router or not
#define Status 5

// Insert your network credentials (STATION)
const char* ssid = "Asim_Home_2.4G";
const char* password = "Asim@0332";

//Create JsonVarient Variable
JSONVar board;
String jsonString_1;
String jsonString_2;

//Variables for packet information
uint16_t packetIdPub1;
uint16_t packetIdPub2;

// Timers auxiliar variables
long now = millis();
long lastMeasure = 0;

//Create a Structure of information to be sent
//Must match the receiver structure
typedef struct struct_message {
  int id;
  float temp;
  float hum;
} struct_message;

//Create a variable of type struct_message to store varaible values
struct_message incomingReadings;

//--------------------------------------------------------------------------------------------
//  Callback Function Indicator for data successful reception
//--------------------------------------------------------------------------------------------
void OnDataRecv(uint8_t * mac_addr, uint8_t *incomingData, uint8_t len) { 
  
  // Copies the sender mac address to a string
  char macStr[18];
  Serial.println("----------------------------------------------------");
  Serial.println("ESP8266 SLAVE & Gateway to Internet");
  Serial.print("Packet received Successfully from: ");
  snprintf(macStr, sizeof(macStr), "%02x:%02x:%02x:%02x:%02x:%02x",
           mac_addr[0], mac_addr[1], mac_addr[2], mac_addr[3], mac_addr[4], mac_addr[5]);
  Serial.println(macStr);
  
  //Copy the data of "incomingReadings" to "incomingData" 
  memcpy(&incomingReadings, incomingData, sizeof(incomingReadings));
  
  //create a JSON String variable with the received information
  board["id"] = incomingReadings.id;

  //Convert temperature and humidity string to char array
  char tempString[8];
  board["temperature"] = dtostrf(incomingReadings.temp, 1, 2, tempString);
  char humString[8];
  board["humidity"] = dtostrf(incomingReadings.hum, 1, 2, humString);
  
  if (incomingReadings.id==1){
  jsonString_1 = JSON.stringify(board);}
  else if(incomingReadings.id==2){
  jsonString_2 = JSON.stringify(board);}

  //Print all data to serial monitor
  Serial.printf("Board ID %u: %u bytes\n", incomingReadings.id, len);
  Serial.printf("temperature value: %4.2f \n", incomingReadings.temp);
  Serial.printf("humidity value: %4.2f \n", incomingReadings.hum);
  Serial.println("----------------------------------------------------");
  Serial.println();
}

//-------------------------------------------------------------------------------------------- 
// (Wi-Fi Handling) Set Slave (Receiver) as Wi-Fi Access point and Station both
//-------------------------------------------------------------------------------------------- 
void connectToWifi() {
 
 delay(10);
 // We start by connecting to a WiFi network
 WiFi.mode(WIFI_AP_STA);
 Serial.println();
 Serial.print("Connecting to ");
 Serial.println(ssid);
 
 WiFi.begin(ssid, password);
 
 while (WiFi.status() != WL_CONNECTED) {
 delay(500); 
 digitalWrite (Status, HIGH);
 delay(50);
 digitalWrite (Status, LOW);
 delay(50);

 Serial.print(".");
 }
 
 Serial.println("");
 Serial.println("WiFi connected");
 Serial.println("IP address: ");
 digitalWrite (Status, HIGH);
 Serial.println(WiFi.localIP());
}

//-------------------------------------------------------------------------------------------- 
// "onWifiConnect" calls "connectToMqtt()" when connected to Wi-Fi
//-------------------------------------------------------------------------------------------- 

void onWifiConnect(const WiFiEventStationModeGotIP& event) {
  //Serial.println("Connected to Wi-Fi.");
  connectToMqtt();
}

//-------------------------------------------------------------------------------------------- 
// onWifiDisconnect() reconnects to Wi-Fi and refrain connecting to MQTT meanwhile
//-------------------------------------------------------------------------------------------- 

void onWifiDisconnect(const WiFiEventStationModeDisconnected& event) {
  Serial.println("Disconnected from Wi-Fi.");
  mqttReconnectTimer.detach(); // ensure we don't reconnect to MQTT while reconnecting to Wi-Fi
  wifiReconnectTimer.once(2, connectToWifi);
}

//-------------------------------------------------------------------------------------------- 
// "connectToMqtt" function connects ESP8266 to MQTT broker
//-------------------------------------------------------------------------------------------- 

void connectToMqtt() {
  Serial.println("Connecting to MQTT...");
  mqttClient.connect();
}

//-------------------------------------------------------------------------------------------- 
// onMqttConnect() function runs after starting a session with the broker.
//-------------------------------------------------------------------------------------------- 

void onMqttConnect(bool sessionPresent) {
  Serial.println("Connected to MQTT.");
  Serial.print("Session present: ");
  Serial.println(sessionPresent);
}

//-------------------------------------------------------------------------------------------- 
// onMqttDisconnect() prints that message if ESP8266 loses connection with the MQTT broker
//-------------------------------------------------------------------------------------------- 

void onMqttDisconnect(AsyncMqttClientDisconnectReason reason) {
  Serial.println("Disconnected from MQTT.");

  if (WiFi.isConnected()) {
    mqttReconnectTimer.once(2, connectToMqtt);
  }
}

//-------------------------------------------------------------------------------------------- 
// onMqttPublish() prints the packet id when message is being published to MQTT topic
//-------------------------------------------------------------------------------------------- 

void onMqttPublish(uint16_t packetId) {
  Serial.print("Publish acknowledged.");
  Serial.print("  packetId: ");
  Serial.println(packetId);
}

//--------------------------------------------------------------------------------------------
// ESP-NOW and Wi-Fi gateway functions to be executed once
//-------------------------------------------------------------------------------------------- 

void setup() {
  // Initialize Serial Monitor
  Serial.begin(115200);

  //Check status of router
  pinMode (Status,OUTPUT);
  
  // these two lines will allow both the MQTT broker and Wi-Fi connection to reconnect, in case the connection is lost.
  wifiConnectHandler = WiFi.onStationModeGotIP(onWifiConnect);
  wifiDisconnectHandler = WiFi.onStationModeDisconnected(onWifiDisconnect);

  //  Finally, assign all the callback functions. This means that these functions will be executed automatically when needed. 
  //  For example, when the ESP8266 connects to the broker, it automatically calls the onMqttConnect() function
  mqttClient.onConnect(onMqttConnect);
  mqttClient.onDisconnect(onMqttDisconnect);
  mqttClient.onPublish(onMqttPublish);

  
//--------------------------------------------------------------------------------------------
// Setup MQTT server
//-------------------------------------------------------------------------------------------- 

  mqttClient.setServer(mqtt_server, 1883);
  Serial.println("Connected ");
  Serial.print("MQTT Server ");
  Serial.print(mqtt_server);
  Serial.print(":");
  Serial.println(String(1883)); 
  Serial.print("ESP8266 IP ");
  Serial.println(WiFi.localIP());


  connectToWifi();
    
//-------------------------------------------------------------------------------------------- 
// (ESP_NOW Handling) Initializing ESP_NOW
//-------------------------------------------------------------------------------------------- 
  if (esp_now_init() != 0) {
    Serial.println("Error initializing ESP-NOW");
    return;
  }

  //Set board as Slave
  esp_now_set_self_role(ESP_NOW_ROLE_SLAVE);
  
  // Once ESPNow is successfully Init, we will register for recv CB to
  // get recv packer info
  
  esp_now_register_recv_cb(OnDataRecv);


}


void loop() {

 
 now = millis();
  // Publishes new temperature and humidity every 30 seconds
 if (now - lastMeasure > 30000) { 
 
 lastMeasure = now;

  //For Sensor1 readings to be published
   
        packetIdPub1 = mqttClient.publish(mqttDATA_1,1,true,jsonString_1.c_str());
        Serial.printf("Publishing on topic %s at QoS 1, packetId: %i ", mqttDATA_1, packetIdPub1);
        Serial.println("");

  //For Sensor2 readings to be published
  
        packetIdPub2 = mqttClient.publish(mqttDATA_2,1,true,jsonString_2.c_str());
        Serial.printf("Publishing on topic %s at QoS 1, packetId: %i ", mqttDATA_2, packetIdPub2);
        Serial.println("");
 }
}
