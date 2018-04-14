#include <Arduino.h>
#include <SimbleeCOM.h>

#define debug

// Current range at 0 packet loss: 14 m

int total = 0;
const int rate = 102;
const int diffMax = 10;

int limp = 0;
int* tempData = new int[4];
int* inData = new int[4];
int* data = new int[4];

int convertWeight(int rawData) {
    return rawData * 5 / 6;
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
        data[i] = inData[i];
    }
}

void convertData(const char* data) {
    for (int i = 0; i < 4; i++) {
        tempData[i] = 0;
        for (int j = 0; j < 2; j++) {
            tempData[i] |= data[i * 2 + j] << (8 - j * 8);
        }
        inData[i] = convertWeight(tempData[i]);
    }
}

void setup() {
    Serial.begin(9600);
    SimbleeCOM.mode = LOW_LATENCY;
    SimbleeCOM.begin();
}

void loop() {
    total++;
    delay(rate);
}

void SimbleeCOM_onReceive(unsigned int esn, const char* payload, int len,
        int rssi) {
    total = 0;
    convertData(payload);
#ifdef debug
//    Serial.printf("%d ", rssi);
//    Serial.printf("%d ", (total - 1) * 10);
    for (int i = 0; i < 4; i++) {
        Serial.printf("%d ", inData[i]);
    }
    Serial.printf("        \r");
#endif
    processData();
    processLimp();
//    Serial.printf("Limp: %d\n", limp);
}
