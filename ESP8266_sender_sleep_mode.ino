//--------------------------------------------------------------------------------------------
//  ESP-8266 master (Sender)
//--------------------------------------------------------------------------------------------
//  Include all essential libraries
//--------------------------------------------------------------------------------------------

#include <espnow.h>
#include <ESP8266WiFi.h>
#include <Adafruit_Sensor.h>
#include <DHT.h>

//--------------------------------------------------------------------------------------------
// Defining essential variables and constants globally
//--------------------------------------------------------------------------------------------

// Define the board ID e.g for ESP8266 Master 1, Board ID will be 1. Similarly, for ESP8266 Master 2, board ID will be 2.
// ID : Identification number

#define BOARD_ID 1

//Define Slave's (Receiver) MAC Address
uint8_t broadcastAddress[] = {0xF4,0xCF,0xA2,0xEF,0xBE,0xF6};

// Define the DHT sensor type
// Uncomment the type of sensor in use:
//#define DHTTYPE    DHT11     // DHT 11
#define DHTTYPE    DHT22     // DHT 22 (AM2302)
//#define DHTTYPE    DHT21     // DHT 21 (AM2301)
 

// Define the GPIO PIN connected to DHT sensor
#define DHTPin 2  

//Create an object for DHT sensor
DHT dht(DHTPin, DHTTYPE);

//Create a Structure of information to be sent
//Must match the receiver structure

typedef struct struct_message {
    int   id;
    float temp;
    float hum;
} struct_message;

//Create a variable of type struct_message to store varaible values
struct_message myData;

//--------------------------------------------------------------------------------------------
//  Wi-Fi Handling
//--------------------------------------------------------------------------------------------
// Insert your SSID
constexpr char WIFI_SSID[] = "Asim_Home_2.4G";

int32_t getWiFiChannel(const char *ssid) {
  if (int32_t n = WiFi.scanNetworks()) {
    for (uint8_t i=0; i<n; i++) {
      if (!strcmp(ssid, WiFi.SSID(i).c_str())) {
        return WiFi.channel(i);
      }
    }
  }
  return 0;
}

//--------------------------------------------------------------------------------------------
//  Function to return temperature's reading
//--------------------------------------------------------------------------------------------
float Read_DHT_Temperature() {
  // By default read temperature as Celsius 
  float t = dht.readTemperature();
  // Check if sensor failed to read temperature
  if (isnan(t)) {    
    Serial.println("Failed to read from DHT sensor!");
    return 0;
  }
  else {
    Serial.println(t);
    return t;
  }
}

//--------------------------------------------------------------------------------------------
//  Function to return Humidity's reading
//--------------------------------------------------------------------------------------------
float Read_DHT_Humidity() {
  float h = dht.readHumidity();
  // Check if sensor failed to read humidity reading
  if (isnan(h)) {
    Serial.println("Failed to read from DHT sensor!");
    return 0;
  }
  else {
    Serial.println(h);
    return h;
  }
}

//--------------------------------------------------------------------------------------------
//  Callback Function for data successful delievery
//--------------------------------------------------------------------------------------------
void OnDataSent(uint8_t *mac_addr, uint8_t sendStatus) {
  Serial.print("Last Packet Send Status: ");
  if (sendStatus == 0){
    Serial.println("Delivery success");
  }
  else{
    Serial.println("Delivery fail");
  }

  //-------------------------------------------------------------------------------------------- 
  //  Deep Sleep Mode
  //-------------------------------------------------------------------------------------------- 
  
  Serial.println("Sleep mode");
  Serial.println("----------------------------------------------------------");
  //Sleep mode is set to 60 seconds, you can change seconds as per your convenience
  ESP.deepSleep(60e6); 
}

//--------------------------------------------------------------------------------------------
// ESP-NOW and Wi-Fi gateway functions to be executed
//-------------------------------------------------------------------------------------------- 
void setup() {

  Serial.begin(115200); //Iniatialize Serial Monitor with 115200 baud rate
  dht.begin(); 

  WiFi.mode(WIFI_STA); //Set Master as a Wi-Fi Station

  //-------------------------------------------------------------------------------------------- 
  // (Wi-Fi Handling) Set Matched channel with Slave (Receiver)
  //-------------------------------------------------------------------------------------------- 
  int32_t channel = getWiFiChannel(WIFI_SSID);

  //WiFi.printDiag(Serial); // Uncomment to verify channel number before
  wifi_promiscuous_enable(1);
  wifi_set_channel(channel);
  wifi_promiscuous_enable(0);
  //WiFi.printDiag(Serial); // Uncomment to verify channel change after

  //-------------------------------------------------------------------------------------------- 
  // (ESP_NOW Handling) Initializing ESP_NOW
  //-------------------------------------------------------------------------------------------- 
  if (esp_now_init() != 0) {
    Serial.println("Error initializing ESP-NOW");
    return;
  }
  
  //Set board as Master
  esp_now_set_self_role(ESP_NOW_ROLE_CONTROLLER); 
  
  // Once ESPNow is successfully Init, we will register for Send CB to
  // get the status of Trasnmitted packet
  
  esp_now_register_send_cb(OnDataSent);  
  
  Serial.println("");
  Serial.println("----------------------------------------------------------");
  Serial.println("ESP8266 Master");
  Serial.println("Awake");
  
  //-------------------------------------------------------------------------------------------- 
  //  Add peer 
  //-------------------------------------------------------------------------------------------- 
  esp_now_add_peer(broadcastAddress, ESP_NOW_ROLE_SLAVE, 1, NULL, 0);

  //-------------------------------------------------------------------------------------------- 
  //  Set values to send to receiver
  //-------------------------------------------------------------------------------------------- 

  myData.id = BOARD_ID;
  myData.temp = Read_DHT_Temperature();
  myData.hum = Read_DHT_Humidity();
    
  //Send Data to receiver 
  esp_now_send(broadcastAddress, (uint8_t *) &myData, sizeof(myData)); 
  
}
 
void loop() {
}
