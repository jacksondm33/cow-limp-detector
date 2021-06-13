#include <Arduino.h>
#include <SimbleeCOM.h>

//#define debug

// FL, FR, BL, BR

const int dataLength = 8;
const int inputDelay = 50;
const int* pins = new int[4] { 3, 4, 2, 6 };

const int devMax = 30;

int** data = new int* [4] { new int[dataLength], new int[dataLength],
        new int[dataLength], new int[dataLength]
};
int* means = new int[4];
int* devs = new int[4];
char* outData = new char[4];
int* tempData = new int[4];

volatile int pos = 0;

volatile bool flag = false;

volatile int startTime = 0;

int sum (int* input)
{
    int total = 0;
    for (int i = 0; i < dataLength; i++)
        total += input[i];
    return total;
}

int mean (int* input)
{
    return sum (input) / dataLength;
}

int standardDev (int* inputData, int mean)
{
    int total = 0;
    int difference;
    for (int i = 0; i < dataLength; i++) {
        difference = inputData[i] - mean;
        difference *= difference;
        total += difference;
    }
    return sqrt (total / dataLength);
}

void sendData()
{
    SimbleeCOM.send (outData, 8);
}

void convertData (int* data)
{
    for (int i = 0; i < 4; i++) {
        tempData[i] = (int) data[i];
        for (int j = 0; j < 2; j++)
            outData[i * 2 + j] = (tempData[i] >> (8 - j * 8)) & 0xFF;
    }
}

void outputData()
{
    if (!flag) {
        convertData (means);
        sendData();
    }
}

void getData()
{
    flag = false;
    for (int i = 0; i < 4; i++) {
        data[i][pos] = analogRead (pins[i]);
        means[i] = mean (data[i]);
        devs[i] = standardDev (data[i], means[i]);
        if (devs[i] > devMax)
            flag = true;
    }
    pos++;
    if (pos == dataLength)
        pos = 0;
}

void setup()
{
    for (int i = 0; i < 4; i++)
        pinMode (pins[i], INPUT);
    Serial.begin (9600);
    SimbleeCOM.mode = LOW_LATENCY;
    SimbleeCOM.begin();
}

void loop()
{
    startTime = millis();

    getData();
    outputData();

#ifdef debug
    for (int i = 0; i < 4; i++)
        Serial.printf ("%d ", means[i]);
    Serial.printf ("\n");
#endif

    delay (inputDelay);
}
