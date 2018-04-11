#include <Arduino.h>
#include <SimbleeCOM.h>

#define debug

// Current range at 0 packet loss: 14 m

int total = 0;
const int rate = 102;
const float diffMax = 10.0;

int limp = 0;
int* tempData = new int[4];
float* inData = new float[4];
float* data = new float[4];

void processLimp() {
	limp = 0;
	if (abs(data[0] - data[1]) > diffMax) {
		limp += 1;
	}
	if (abs(data[2] - data[3]) > diffMax) {
		limp += 2;
	}
}

void processData() {
	for (int i = 0; i < 4; i++) {
		data[i] = inData[i];
	}
}

void convertData(const char* data) {
	for (int i = 0; i < 4; i++) {
		tempData[i] = 0;
		for (int j = 0; j < 4; j++) {
			tempData[i] |= data[i * 4 + j] << (24 - j * 8);
		}
		inData[i] = (float) tempData[i];
	}
}

void setup() {
	Serial.begin(9600);
	SimbleeCOM.mode = LOW_LATENCY;
	SimbleeCOM.begin();
}

void loop() {
	total++;
//  delay(1.0/(double)rate * 1000);
	delay(rate);
}

void SimbleeCOM_onReceive(unsigned int esn, const char* payload, int len,
		int rssi) {
	total = 0;
	convertData(payload);
#ifdef debug
	Serial.printf("%d ", rssi);
	Serial.printf("%d ", (total - 1) * 10);
	for (int i = 0; i < len; i++) {
		Serial.printf("%d ", inData[i]);
	}
	Serial.printf("\n");
#endif
	processData();
	processLimp();
	Serial.printf("%d\n", limp);
}
