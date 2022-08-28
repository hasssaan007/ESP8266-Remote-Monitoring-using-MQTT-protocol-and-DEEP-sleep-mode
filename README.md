# ESP8266-Remote-Monitoring-using-MQTT-protocol-and-DEEP-sleep-mode
**Overview:**

This system contains two ESP8266 boards that will make connection using ESP-NOW and Master will send sensors (DHT22) data to Slave. 

Moreover, that slave will further send this data to MQTT server using Wi-Fi and MQTT protocol. 

In this Project, We have used the Many-to-One Master/Slave configuration of ESP-NOW. 

**ESP-NOW** 

It is a connectionless communication protocol developed by Espressif that features short packet transmission. 

This protocol enables multiple devices to talk to each other without using Wi-Fi.

In Many-to-One Configuration, there is several boards sending sensor readings via ESP-NOW to one receiver that sends data to MQTT broker using Wi-Fi.

**Components and Boards:**

1.  ESP8266 (Master)

2.  ESP8266 (Slave) and Gateway to Internet

3.  Sensors (DHT22)

**Explanation:**

Few important points if you want to use Wi-Fi and ESP-Now connection simultaneously:

1.  Master (Sender) should have same Wi-Fi channel as Wi-Fi channel of Slave (Receiver)

2.  The Wi-Fi mode of the receiver board must be access point and station (WIFI_AP_STA).
