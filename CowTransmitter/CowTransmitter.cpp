#include <Arduino.h>
#include <SimbleeCOM.h>

//#define debug

const int dataLength = 10;
const int inputDelay = 100;
const int* pins = new int[4] { 2, 3, 4, 6 };

const float devMax = 30;

float** data = new float*[4] { new float[dataLength], new float[dataLength],
        new float[dataLength], new float[dataLength] };
float* means = new float[4];
float* devs = new float[4];
char* outData = new char[4];
int* tempData = new int[4];

volatile int pos = 0;

volatile bool flag = false;

volatile int startTime = 0;

float convertWeight(int rawData) {
    return (float) rawData * 105.0 / 96.0;
}

float sum(float* input) {
    float total = 0;
    for (int i = 0; i < dataLength; i++) {
        total += input[i];
    }
    return total;
}

float mean(float* input) {
    return sum(input) / dataLength;
}

float standardDev(float* inputData, float mean) {
    float total = 0;
    float difference;
    for (int i = 0; i < dataLength; i++) {
        difference = inputData[i] - mean;
        difference *= difference;
        total += difference;
    }
    return sqrt(total / dataLength);
}

void sendData() {
    SimbleeCOM.send(outData, 4);
}

void convertData(float* data) {
    for (int i = 0; i < 4; i++) {
        tempData[i] = (int) data[i];
        for (int j = 0; j < 4; j++) {
            outData[i * 4 + j] = (tempData[i] >> (24 - j * 8)) & 0xFF;
        }
    }
}

void outputData() {
    if (!flag) {
        convertData(means);
        sendData();
    }
}

void getData() {
    flag = false;
    for (int i = 0; i < 4; i++) {
        data[i][pos] = convertWeight(analogRead(pins[i]));
        means[i] = mean(data[i]);
        devs[i] = standardDev(data[i], means[i]);
        if (devs[i] > devMax) {
            flag = true;
        }
    }
    pos++;
    if (pos == dataLength) {
        pos = 0;
    }
}

void setup() {
    for (int i = 0; i < 4; i++) {
        pinMode(pins[i], INPUT);
    }
    Serial.begin(9600);
    SimbleeCOM.mode = LOW_LATENCY;
    SimbleeCOM.begin();
}

void loop() {
    startTime = millis();

    getData();
    outputData();

#ifdef debug
    Serial.println(millis() - startTime);
#endif

    delay(inputDelay);
}
