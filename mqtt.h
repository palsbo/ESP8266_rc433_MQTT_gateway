#ifndef MQTT_H
#define MQTT_H

#include <PubSubClient.h>
WiFiClient espClient;
PubSubClient client(espClient);

struct {
  typedef void (*mqtt_wcallback0)();
  typedef void (*mqtt_wcallback1)(int status);
  typedef void (*mqtt_wcallback2)(char* topic, char* payload);
  typedef void (*mqtt_wcallback3)(char* topic, byte* payload, unsigned int length);
  const char* mqtt_client;
  const char* mqtt_server;
  long mqtt_port;
  const char* mqtt_user;
  const char* mqtt_pass;
  mqtt_wcallback0 f_onConnected = []() {};
  mqtt_wcallback0 f_onConnecting = []() {};
  mqtt_wcallback1 f_onConnectFailed = [](int state) {};
  void onConnecting(mqtt_wcallback0 cb0) { f_onConnecting = cb0; };
  void onConnected(mqtt_wcallback0 cb0) {f_onConnected = cb0; };
  void onConnectFailed(mqtt_wcallback1 cb1) { f_onConnectFailed = cb1; };
  void begin(const char * _mqtt_client, const char * _mqtt_server, long _mqtt_port, const char* _mqtt_user, const char* _mqtt_pass) {
    String id = (String)_mqtt_client + "_" + String(ESP.getChipId());
    mqtt_client = &id[0];
    mqtt_server = _mqtt_server;
    mqtt_port = _mqtt_port;
    mqtt_user = _mqtt_user;
    mqtt_pass = _mqtt_pass;
    client.setServer(_mqtt_server, _mqtt_port);
  }
  void loop() {
    if (!client.connected()) {
      while (!client.connected()) {
        f_onConnecting();
        if (client.connect(mqtt_client, mqtt_user, mqtt_pass)) f_onConnected();
        else f_onConnectFailed(client.state());
      }
    }
    client.loop();
  }
  void onCallback(mqtt_wcallback3 cb3) {
    client.setCallback(cb3);
  }
  boolean publish(const char* topic, const char* payload) {
    return client.publish(topic, payload);
  };
  boolean publish(const char* topic, const char* payload, boolean retained) {
    return client.publish(topic, payload, retained);
  };
  boolean publish(const char* topic, const uint8_t * payload, unsigned int plength) {
    return client.publish(topic, payload, plength);
  };
  boolean publish(const char* topic, const uint8_t * payload, unsigned int plength, boolean retained) {
    return client.publish(topic, payload, plength, retained);
  };
  boolean publish_P(const char* topic, const uint8_t * payload, unsigned int plength, boolean retained) {
    return client.publish_P(topic, payload, plength, retained);
  };
  boolean subscribe(const char* topic) {
    client.subscribe(topic);
  };
  boolean subscribe(const char* topic, uint8_t qos) {
    client.subscribe(topic, qos);
  };
  boolean unsubscribe(const char* topic) {
    client.unsubscribe(topic);
  };
  void disconnect() { client.disconnect(); };
  bool connected() { return client.connected(); };
} mqtt;

#endif

