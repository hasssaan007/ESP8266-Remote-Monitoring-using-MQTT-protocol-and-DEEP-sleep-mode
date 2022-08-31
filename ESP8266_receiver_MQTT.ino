//--------------------------------------------------------------------------------------------
//  ESP-32 Slave (Receiver) and Gateway to Internet using MQTT protocol
//--------------------------------------------------------------------------------------------
//  Include all essential libraries
//--------------------------------------------------------------------------------------------

#include <espnow.h>
#include <ESP8266WiFi.h>
#include <Arduino_JSON.h>
#include <PubSubClient.h>

//--------------------------------------------------------------------------------------------
// Defining essential variables and constants globally
//--------------------------------------------------------------------------------------------


//MQTT Setup Start
#define mqtt_server "broker.hivemq.com"
WiFiClient espClient;
PubSubClient client(espClient);
#define mqttDATA_1 "HM_IOT/DATA_1"
#define mqttDATA_2 "HM_IOT/DATA_2"
//MQTT Setup End


// Insert your network credentials (STATION)
const char* ssid = "Asim_Home_2.4G";
const char* password = "Asim@0332";


//Create JsonVarient Variable
JSONVar board;
String jsonString;

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
  jsonString = JSON.stringify(board);

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
void setup_wifi() {
 
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
// Reconnet ESP Slave
//-------------------------------------------------------------------------------------------- 

void reconnect() {
 // Loop until we're reconnected
 while (!client.connected()) {
 Serial.print("Attempting MQTT connection...");
 // Attempt to connect
 if (client.connect("ESP8266Client")) {
 
 Serial.println("connected");
 Serial.println("");
 client.subscribe("event");
 } else {
 Serial.print("failed, rc=");
 Serial.print(client.state());
 Serial.println(" try again in 5 seconds");
 // Wait 5 seconds before retrying
 //delay(5000);
 }
 }
}

//--------------------------------------------------------------------------------------------
// ESP-NOW and Wi-Fi gateway functions to be executed once
//-------------------------------------------------------------------------------------------- 

void setup() {
  // Initialize Serial Monitor
  Serial.begin(115200);

  setup_wifi();
  
//--------------------------------------------------------------------------------------------
// Setup MQTT server
//-------------------------------------------------------------------------------------------- 

  client.setServer(mqtt_server, 1883);
  Serial.println("Connected ");
  Serial.print("MQTT Server ");
  Serial.print(mqtt_server);
  Serial.print(":");
  Serial.println(String(1883)); 
  Serial.print("ESP8266 IP ");
  Serial.println(WiFi.localIP()); 
  Serial.println("Modbus RTU Master Online");


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

 //Publish Data to mqtt Server
 if (incomingReadings.id == 1)
 {
  client.publish(mqttDATA_1,jsonString.c_str(),true);
 }
 else if (incomingReadings.id == 2)
 {
  client.publish(mqttDATA_2,jsonString.c_str(),true);
 }
 else
 {
  Serial.println("No board");
 }

 if (!client.connected()) {
 reconnect();
 }
 
 client.loop();
 //Upload data to broker after every 5 mins
 delay(5*60*1000);
 }
