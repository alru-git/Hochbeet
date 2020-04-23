/**
 *******************************
 *
 * Funktionen:
 * 
 *- Temperaturmessung im Hochbeet
   * Example sketch showing how to send in DS1820B OneWire temperature readings back to the controller
   * http://www.mysensors.org/build/temp
 * 
 *
 * Ver. 0.9: Erste Versione
 * Ver. 1.0: Erster produktiver Einsatz mit neuem Netzteil (9V, 300mA)
 * 
 * 
 */


// Enable debug prints to serial monitor
#define MY_DEBUG

// Enable and select radio type attached
#define MY_RADIO_RF24
// MSG1 (Frequenz 2496 MHz)
#define MY_RF24_CHANNEL 96
// MSG2 (Frequenz 2506 MHz)
// #define MY_RF24_CHANNEL 106

// RF Power
#define MY_RF24_PA_LEVEL RF24_PA_MAX

// Optional: Define Node ID
#define MY_NODE_ID 124
// Node 0: MSG1 oder MSG2
#define MY_PARENT_NODE_ID 0
#define MY_PARENT_NODE_IS_STATIC

#include <MySensors.h>

unsigned long SLEEP_TIME = 600000;      // 10 Min. Loop Pause

// Temperatur
  #include <DallasTemperature.h>
  #include <OneWire.h>
  #define COMPARE_TEMP 1                // Send temperature only if changed? 1 = Yes 0 = No
  #define ONE_WIRE_BUS 3                // Pin where dallase sensor is connected 
  #define MAX_ATTACHED_DS18B20 16
  OneWire oneWire(ONE_WIRE_BUS);        // Setup a oneWire instance to communicate with any OneWire devices (not just Maxim/Dallas temperature ICs)
  DallasTemperature sensors(&oneWire);  // Pass the oneWire reference to Dallas Temperature. 
  float lastTemperature[MAX_ATTACHED_DS18B20];
  int numSensors=0;
  bool receivedConfig = false;
  bool metric = true;
  // Initialize temperature message
  MyMessage msg(0,V_TEMP);
// End Teperatur

void before()
{
  // Startup up the OneWire library
  sensors.begin();
}

void setup()  
{ 
  // requestTemperatures() will not block current thread
    sensors.setWaitForConversion(false);

  Serial.println("Setup done...");
}

void presentation() {
  // Send the sketch version information to the gateway and Controller
    sendSketchInfo("Hochbeet", "1.0");

  // Fetch the number of attached temperature sensors  
    numSensors = sensors.getDeviceCount();

  // Present all sensors to controller
    // Temperature
    for (int i=0; i<numSensors && i<MAX_ATTACHED_DS18B20; i++) {   
     present(i, S_TEMP);
    }

}

void loop() 
{     
  // Update vom heartbeat
  sendHeartbeat();
  
  // Fetch temperatures from Dallas sensors
  sensors.requestTemperatures();

  // query conversion time and wait until conversion completed
  int16_t conversionTime = sensors.millisToWaitForConversion(sensors.getResolution());
  wait (conversionTime);
  
  // Read temperatures and send them to controller 
  for (int i=0; i<numSensors && i<MAX_ATTACHED_DS18B20; i++) {
 
    // Fetch and round temperature to one decimal
    float temperature = static_cast<float>(static_cast<int>((getControllerConfig().isMetric?sensors.getTempCByIndex(i):sensors.getTempFByIndex(i)) * 10.)) / 10.;
 
    // Only send data if temperature has changed and no error
    #if COMPARE_TEMP == 1
    if (lastTemperature[i] != temperature && temperature != -127.00 && temperature != 85.00) {
    #else
    if (temperature != -127.00 && temperature != 85.00) {
    #endif
 
      // Send in the new temperature
      send(msg.setSensor(i).set(temperature,1));
      // Save new temperatures for next compare
      lastTemperature[i]=temperature;
      }
    }

   sleep (SLEEP_TIME);
}
