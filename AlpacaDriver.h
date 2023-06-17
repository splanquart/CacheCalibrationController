#ifndef ALPACADRIVER_H
#define ALPACADRIVER_H

#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <ESP8266HTTPUpdateServer.h>
#include <WiFiUdp.h>
#include <ArduinoJson.h>
#include <vector>
#include "RelayController.h"
#include "AlpacaDevice.h"
#include "HttpHandler.h"

#define DISCOVERY_PORT 32227 // port ASCom discovery
#define ALPACA_PORT 11111 // port ASCom HTTP port


class AlpacaDriver {
public:
  AlpacaDriver(HttpHandler &server);

  void begin();
  void addDevice(AlpacaDevice* device);
  void handleClient();
  void handlerNotFound();
  void handleDiscovery();

  /*
   * Implement the 3 routes about Alpaca management API
   * https://ascom-standards.org/api/?urls.primaryName=ASCOM%20Alpaca%20Management%20API#/
   */
  void handleAPIVersions();
  void handleDescription();
  void handleConfiguredDevices();


  void handleGetDevices();



  /*
   * Implement route about Alapaca setup
   * https://ascom-standards.org/api/?urls.primaryName=ASCOM%20Alpaca%20Management%20API#/HTML%20Browser%20User%20Interface
   */
  void handleSetup();
  
private:
  RelayController* _controller;
  std::vector<AlpacaDevice*> _devices;
  int _deviceId;
  WiFiUDP _udp;
  //WiFiServer _server;
  HttpHandler& _server;
  char _packetBuffer[255]; //buffer to hold incoming packet
  uint32_t _serverTransactionID = 0;
  
  
  // Alpaca properties
  //const char* _content_type = "application/json;charset=utf-8";

  const char* _manufacturer = "My Company";
  const char* _alpacaUniqueId = "mycompany.cachecalibrationcontroller";
  const char* _description = "Driver for Cache Calibration";
  const char* _driverVersion = "v1.0.0";
  //const char* _uniqueId = "ASW-c47773faa9a94638bb22abaf6e3c4aa6";  // generated by `python uniqueid.py`
  //const char* _uniqueId = "4431281c-8560-4ad7-982f-5a6e507dda20";  // random
  const char* _uniqueId = "4431281c85604ad7982f5a6e507dda20";  // random

};

#endif // ALPACADRIVER_H
