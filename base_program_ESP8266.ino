#include "WIFIconfig.h"
#include "MQTTconfig.h"

#include <ESP8266WiFi.h>
#include <ESP8266mDNS.h>
#include <WiFiUdp.h>
#include <ArduinoOTA.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>
#include <Timer.h>

WiFiClient espClient;
PubSubClient client(espClient);

//CONSTANTS:
const boolean START = true;
const boolean RESET = false;

//TIMERS:
TON* tPublishInfo;
TON* tCheckConnection;

void setup() {
  Serial.begin(115200);

  WIFIConnection();

  OTAConfig();

  MQTTConnection();

  Serial.println("Connected to the WiFi network");

  tPublishInfo = new TON(1000);
  tCheckConnection = new TON(52000);
}

void publishInfo() {
  if (tPublishInfo->IN(START)) {
    StaticJsonDocument<192> jsonDoc;
    JsonObject recorrido = jsonDoc.createNestedObject("recorrido");

    String payload = "";
    char* estado = "x";

    jsonDoc["estado"] = estado;
    jsonDoc["consumo"] = 14;
    jsonDoc["consumoLimite"] = 50;

    recorrido["recorridoGuardado"] = true;
    recorrido["t_PosicionFinal"] = 14500;

    serializeJson(jsonDoc, payload);
    client.publish(topicState, (char*)payload.c_str());

    tPublishInfo->IN(RESET);
  }
}

void checkMqttConnection() {
  if (tCheckConnection->IN(START)) {
    if (!client.connected()) {
      ESP.restart();
    }

    tCheckConnection->IN(RESET);
  }
}

void loop()
{
  ArduinoOTA.handle();
  client.loop();
  yield();

  publishInfo();
}

void WIFIConnection() {
  // connecting to a WiFi network
  WiFi.begin(ssid, password);

  while (WiFi.waitForConnectResult() != WL_CONNECTED) {
    Serial.println("Connecting to WiFi..");
    delay(5000);
    ESP.restart();
  }

  WiFi.setAutoReconnect(true);
  WiFi.persistent(true);
}

void OTAConfig() {
  ArduinoOTA.setHostname(client_name);
  ArduinoOTA.onStart([]() {
    String type;
    if (ArduinoOTA.getCommand() == U_FLASH) {
      type = "sketch";
    } else { // U_FS
      type = "filesystem";
    }

    // NOTE: if updating FS this would be the place to unmount FS using FS.end()
    Serial.println("Start updating " + type);
  });
  ArduinoOTA.onEnd([]() {
    Serial.println("\nEnd");
  });
  ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
    Serial.printf("Progress: %u%%\r", (progress / (total / 100)));
  });
  ArduinoOTA.onError([](ota_error_t error) {
    Serial.printf("Error[%u]: ", error);
    if (error == OTA_AUTH_ERROR) {
      Serial.println("Auth Failed");
    } else if (error == OTA_BEGIN_ERROR) {
      Serial.println("Begin Failed");
    } else if (error == OTA_CONNECT_ERROR) {
      Serial.println("Connect Failed");
    } else if (error == OTA_RECEIVE_ERROR) {
      Serial.println("Receive Failed");
    } else if (error == OTA_END_ERROR) {
      Serial.println("End Failed");
    }
  });
  ArduinoOTA.begin();
}

void MQTTConnection() {
  //connecting to a mqtt broker
  client.setServer(mqtt_broker, mqtt_port);
  client.setCallback(callback);

  while (!client.connected()) {
    String client_id = client_name;
    client_id += String(WiFi.macAddress());
    Serial.printf("The client %s connects to the public mqtt broker\n", client_id.c_str());
    if (client.connect(client_id.c_str(), mqtt_username, mqtt_password)) {
      Serial.println("Public emqx mqtt broker connected");
    } else {
      Serial.print("failed with state ");
      Serial.print(client.state());
      delay(2000);
    }
  }

  client.subscribe(topicCommand);
}

void callback(char *topicCommand, byte *payload, unsigned int length) {
    Serial.print("Message arrived in topic: ");
    Serial.println(topicCommand);
    Serial.print("Message:");
  String payload_n;

  for (int i = 0; i < length; i++) {
    payload_n += (char) payload[i];
  }

    Serial.println(payload_n);
    Serial.println("-----------------------");

  StaticJsonDocument<256> doc;
  DeserializationError error = deserializeJson(doc, payload_n);
  if (error) return;
  String orden = doc["orden"];
  int consumoLimite = doc["consumoLimite"];
  boolean recorridoGuardadoRecibido = doc["recorrido"]["recorridoGuardado"];
  unsigned long t_posicionFinal = doc["recorrido"]["t_PosicionFinal"];
}
