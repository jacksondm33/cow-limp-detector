#include <Arduino.h>
#include <SimbleeCOM.h>

//#define debug

// FL, FR, BL, BR

const int dataLength = 8;
const int inputDelay = 50;
const int* pins = new int[4] { 3, 4, 2, 6 };

const int devMax = 10;
const int weightThresh = 1;

int** data = new int* [4] { new int[dataLength], new int[dataLength],
        new int[dataLength], new int[dataLength]
};
int* means = new int[4];
int* devs = new int[4];
char* outData = new char[4];
int* tempData = new int[4];

volatile int pos = 0;

volatile bool prevFlag = false;
volatile bool flag = false;

volatile int startTime = 0;

int cycle = 0;
int cyclePos = 0;
const int cycleLength = 7;
int* cycleData = new int[4];

const int*** cycles = new const int** [2] { new const int* [cycleLength] {
        new const int[4] { 0, 0, 1, 0 }, new const int[4] { 0, 2, 1, 0 },
        new const int[4] { 0, 2, 0, 0 }, new const int[4] { 0, 2, 0, 4 },
        new const int[4] { 0, 0, 0, 4 }, new const int[4] { 3, 0, 0, 4 },
        new const int[4] { 3, 0, 0, 0 }
    }, new const int* [cycleLength] {
        new const int[4] { 0, 0, 0, 2 }, new const int[4] { 1, 0, 0, 2 },
        new const int[4] { 1, 0, 0, 0 }, new const int[4] { 1, 0, 3, 0 },
        new const int[4] { 0, 0, 3, 0 }, new const int[4] { 0, 4, 3, 0 },
        new const int[4] { 0, 4, 0, 0 }
    }
};
const int** cycleDataLengths = new const int* [2] {
    new const int[4] { 2, 3, 2, 3 }, new const int[4] { 3, 2, 3, 2 }
};

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

void getCycle (int* data)
{
    if (data[2] > weightThresh && data[3] < weightThresh)
        cycle = 0;

    else if (data[3] > weightThresh && data[2] < weightThresh)
        cycle = 1;
}

void calcData (int* data)
{
    for (int i = 0; i < 4; i++) {
        if (abs (data[i]) < weightThresh)
            data[i] = 0;
        if (cycles[cycle][pos][i] > 0)
            cycleData[cycles[cycle][cyclePos][i] - 1] += data[i];
    }
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

void averageData()
{
    for (int i = 0; i < 4; i++)
        cycleData[i] /= cycleDataLengths[cycle][i];
}

void outputData()
{
    if (prevFlag && !flag) {
        if (cyclePos == 0)
            getCycle (means);
        calcData (means);
        cyclePos++;
        if (cyclePos == cycleLength) {
            averageData();
            convertData (cycleData);
            sendData();
            cyclePos = 0;
        }
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
    Serial.printf ("%d\n", means[1]);
#endif

    delay (inputDelay);
}
