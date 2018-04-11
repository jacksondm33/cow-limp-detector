#include <Arduino.h>
#include <SimbleeCOM.h>

//#define debug

const int dataLength = 10;
const int inputDelay = 100;
const int* pins = new int[4] { 2, 3, 4, 6 };

const float devMax = 30;
const float weightThresh = 1;

float** data = new float*[4] { new float[dataLength], new float[dataLength],
		new float[dataLength], new float[dataLength] };
float* means = new float[4];
float* devs = new float[4];
char* outData = new char[4];
int* tempData = new int[4];

volatile int pos = 0;

volatile bool prevFlag = false;
volatile bool flag = false;

volatile int startTime = 0;

int cycle = 0;
int cyclePos = 0;
const int cycleLength = 7;
float* cycleData = new float[4];

const int*** cycles = new int**[2] { new int*[cycleLength] { new int[4] { 0, 0,
		1, 0 }, new int[4] { 0, 2, 1, 0 }, new int[4] { 0, 2, 0, 0 },
		new int[4] { 0, 2, 0, 4 }, new int[4] { 0, 0, 0, 4 }, new int[4] { 3, 0,
				0, 4 }, new int[4] { 3, 0, 0, 0 } }, new int*[cycleLength] {
		new int[4] { 0, 0, 0, 2 }, new int[4] { 1, 0, 0, 2 }, new int[4] { 1, 0,
				0, 0 }, new int[4] { 1, 0, 3, 0 }, new int[4] { 0, 0, 3, 0 },
		new int[4] { 0, 4, 3, 0 }, new int[4] { 0, 4, 0, 0 } } };
const int** cycleDataLengths = new int*[2] { new int[4] { 2, 3, 2, 3 },
		new int[4] { 3, 2, 3, 2 } };

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

void getCycle(float* data) {
	if (data[2] > weightThresh && data[3] < weightThresh) {
		cycle = 0;
	} else if (data[3] > weightThresh && data[2] < weightThresh) {
		cycle = 1;
	}
}

void calcData(float* data) {
	for (int i = 0; i < 4; i++) {
		if (abs(data[i]) < weightThresh) {
			data[i] = 0;
		}
		if (cycles[cycle][pos][i] > 0) {
			cycleData[cycles[cycle][cyclePos][i] - 1] += data[i];
		}
	}
}

void averageData() {
	for (int i = 0; i < 4; i++) {
		cycleData[i] /= cycleDataLengths[cycle][i];
	}
}

void outputData() {
	if (prevFlag && !flag) {
		if (cyclePos == 0) {
			getCycle(means);
		}
		calcData(means);
		cyclePos++;
		if (cyclePos == cycleLength) {
			averageData();
			convertData(cycleData);
			sendData();
			cyclePos = 0;
		}
	}
}

void getData() {
	prevFlag = flag;
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
