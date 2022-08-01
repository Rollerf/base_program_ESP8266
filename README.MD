# Base project for ESP8266 projects
## Description
This is with ESP8266.

## Configuration
You'll need your own variables for MQTT and WIFI connection. I use in this format:

WIFI connection file:
const char *ssid = "";
const char *password = "";

MQTT connection file:
const char *mqtt_broker = "";
const char *topicState = "";
const char *topicCommand = "";
const char *mqtt_username = "";
const char *mqtt_password = "";
const int mqtt_port = ;
const char *client_name = "";
