#include "AlpacaDriver.h"


AlpacaDriver::AlpacaDriver(HttpHandler &server)
    : _server(server), _setup(server) {}

void AlpacaDriver::begin() {
  _udp.begin(DISCOVERY_PORT);
 
  // Print the IP address
  Serial.println("");
  _rootUrl = String("http://") + WiFi.localIP().toString() + ":" + _server.getPort() + "/";

  Serial.print("Use this URL to connect: ");
  Serial.println(_rootUrl);

  _server.bind("/management/apiversions", HTTP_GET, std::bind(&AlpacaDriver::handleAPIVersions, this));
  _server.bind("/management/v1/description", HTTP_GET, std::bind(&AlpacaDriver::handleDescription, this));
  _server.bind("/management/v1/configureddevices", HTTP_GET, std::bind(&AlpacaDriver::handleConfiguredDevices, this));

  _server.bind("/setup", HTTP_GET, std::bind(&AlpacaDriver::handleSetup, this));
  
  _server.begin();

  _setup.addStyle(R"(
    .device-link a, .device-link a {
        text-decoration: none;
        color: #cfcaca;
    }
    .device-link {
        background-color: #2b2b2b;
        background: linear-gradient(0deg, rgb(0 0 0) 0%, rgb(102 102 102) 100%);
        border: none;
        color: white;
        padding: 1em 1.5em;
        text-align: center;
        text-decoration: none;
        display: inline-block;
        font-size: 16px;
        margin: 0.5em 0.2em;
        cursor: pointer;
        border-radius: 1em;
    }
    @media (prefers-color-scheme: dark) {
      .device-link a, .device-link {
          color: #4b4848;
      }
      .device-link {
          background-color: #ffffff;
          background: linear-gradient(0deg, rgb(104 104 104) 0%, rgb(217 217 217) 100%);
      }
    }
  )");
  Serial.println("Server started");
}
String AlpacaDriver::getSetupUrl() {
   return _rootUrl + "setup";
}
void AlpacaDriver::addDevice(AlpacaDevice* device) {
    _devices.push_back(device);
  }
void AlpacaDriver::handleClient() {
  // handle GET and PUT requests from clients
  _server.handleClient();
}
void AlpacaDriver::handleDiscovery() {
  // if there's data available, read a packet
  int packetSize = _udp.parsePacket();
  if (packetSize) {
    Serial.print("Received packet of size: ");
    Serial.println(packetSize);
    Serial.print("From ");
    IPAddress remoteIp = _udp.remoteIP();
    Serial.print(remoteIp);
    Serial.print(", on port ");
    Serial.println(_udp.remotePort());

    // read the packet into packetBufffer
    int len = _udp.read(_packetBuffer, 255);
    if (len > 0) {
      //Ensure that it is null terminated
      _packetBuffer[len] = 0;
    }
    Serial.print("Contents: ");
    Serial.println(_packetBuffer);

    // No undersized packets allowed
    if (len < 16)
    {
      return;
    }

    if (strncmp("alpacadiscovery1", _packetBuffer, 16) != 0)
    {
      return;
    }
    // prepare response
    char response[36] = {0};
    sprintf(response, "{\"AlpacaPort\": %d}", ALPACA_PORT);
    // send a reply, to the IP address and port that sent us the packet we received
    _udp.beginPacket(_udp.remoteIP(), _udp.remotePort());
    _udp.write(response);
    _udp.endPacket();
    Serial.print("Response: ");
    Serial.println(response);
  }
}

void AlpacaDriver::handleAPIVersions() {
  _server.logRequest(__func__);
  const size_t CAPACITY = JSON_ARRAY_SIZE(5);
  StaticJsonDocument<CAPACITY> devices;
  JsonArray array = devices.to<JsonArray>();
  array.add(1);
  _server.returnResponse(array);

}
void AlpacaDriver::handleDescription() {
  _server.logRequest(__func__);
  DynamicJsonDocument val(1024);

  val["ServerName"] = _description;
  val["Manufacturer"] = _manufacturer;
  val["ManufacturerVersion"] = _driverVersion; //"1.0";
  val["Location"] = "FR";

  const size_t CAPACITY = JSON_ARRAY_SIZE(5);
  StaticJsonDocument<CAPACITY> devices;
  JsonArray array = devices.to<JsonArray>();
  array.add(val);
  _server.returnResponse(val);

}

void AlpacaDriver::handleConfiguredDevices() {
    _server.logRequest(__func__);

    DynamicJsonDocument devices(1024);
    JsonArray array = devices.to<JsonArray>();

    for (AlpacaDevice* device : _devices) {
        array.add(device->getDeviceInfo());
    }

    _server.returnResponse(array);
}

void AlpacaDriver::handleSetup() {
  _server.logRequest(__func__);
    _setup.render([this](HttpHandler& server) {
      server.sendContent("<h1>Setup main page</h1>");
      for (int i = 0; i < _devices.size(); ++i) {
        server.sendContent("<a class=\"device-link\" href=\"" + _devices[i]->getDeviceSetupUrl() + "\">Configuration " + _devices[i]->getDeviceName() + "</a><br/>");
      }
    });
}
