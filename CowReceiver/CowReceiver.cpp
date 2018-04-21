#include <Arduino.h>
#include <SimbleeCOM.h>

//#define debug

int* weightTable = new int[1024] { NULL };

const int rate = 102;
const int diffMax = 10;

const double weightFactor = 5.0 / 6.0;
const int* OFFSETS = new int[4] { 29, 29, 36, 146 };

int limp = 0;
int* inData = new int[4];
int* data = new int[4];

int convertWeight(int rawData) {
    if (rawData < 0 or rawData >= 1024) {
        return 0;
    }
    if (weightTable[rawData] == NULL) {
        weightTable[rawData] = rawData * weightFactor;
    }
    return weightTable[rawData];
}

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
        data[i] = convertWeight(inData[i] - OFFSETS[i]);
    }
}

void convertData(const char* data) {
    for (int i = 0; i < 4; i++) {
        inData[i] = 0;
        for (int j = 0; j < 2; j++) {
            inData[i] |= data[i * 2 + j] << (8 - j * 8);
        }
    }
}

void setup() {
    Serial.begin(9600);
    SimbleeCOM.mode = LOW_LATENCY;
    SimbleeCOM.begin();
}

void loop() {
    delay(rate);
}

void SimbleeCOM_onReceive(unsigned int esn, const char* payload, int len,
        int rssi) {
    convertData(payload);
#ifdef debug
    for (int i = 0; i < 4; i++) {
        Serial.printf("%d ", inData[i]);
    }
#endif
    processData();
    processLimp();
#ifndef debug
    for (int i = 0; i < 4; i++) {
        Serial.printf("%d ", data[i]);
    }
#endif
    Serial.printf("%d\n", limp);
}
