#include <SPI.h>
#include <Ethernet.h>
#include <HttpClient.h>
#include <Cosm.h>

#include "DS18B20.h"

#include "cosm.key"

// MAC address for your Ethernet shield
byte mac[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED };

// Your Cosm key to let you upload data
char cosmKey[] = PRIVATE_COSM_KEY;  // defined in cosm.key

// Analog pin which we're monitoring (0 and 1 are used by the Ethernet shield)
int sensorPin = 2;

// Define the strings for our datastream IDs
char indoorSensorId[] = "indoor";
char outdoorSensorId[] = "outdoor";

CosmDatastream datastreams[] = {
  CosmDatastream(indoorSensorId, strlen(indoorSensorId), DATASTREAM_FLOAT),
  CosmDatastream(outdoorSensorId, strlen(outdoorSensorId), DATASTREAM_FLOAT),
};
// Finally, wrap the datastreams into a feed
CosmFeed feed(102125, datastreams, 2 /* number of datastreams */);

EthernetClient client;
CosmClient cosmclient(client);

// ----------------------------------------------------------------------------

DS18B20 ds(20);
byte numberOfSensors;



void setup()
{
	// put your setup code here, to run once:
	Serial.begin(9600);

	Serial.println("Starting multiple datastream upload to Cosm...");
	Serial.println();

	while (Ethernet.begin(mac) != 1)
	{
	Serial.println("Error getting IP address via DHCP, trying again...");
	delay(15000);
	}

	numberOfSensors = ds.search();

	Serial.print("Found sensors: ");
	Serial.println(numberOfSensors);
}

void loop()
{
	if (numberOfSensors < 2)
	{
		Serial.println("Not enough sensors. Need at least two sensors.");
		numberOfSensors = ds.search();
		delay(3000);
		return;
	}

	float indoorTemp = ds.startAndWaitForTemperature(0);
	float outdoorTemp = ds.startAndWaitForTemperature(1);

	if (indoorTemp < -100 || outdoorTemp < -100)
	{
		Serial.println("Invalid temperature");
		delay(3000);
		return;
	}

	datastreams[0].setFloat(indoorTemp);

	Serial.print("Read indoor sensor value ");
	Serial.println(datastreams[0].getFloat());

	datastreams[1].setFloat(outdoorTemp);
	Serial.print("Read outdoor sensor value ");
	Serial.println(datastreams[1].getFloat());

	Serial.println("Uploading it to Cosm");
	int ret = cosmclient.put(feed, cosmKey);
	Serial.print("cosmclient.put returned ");
	Serial.println(ret);

	Serial.println();

	delay(30000);
}

