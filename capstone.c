//try both change at the same time

#include <AccelStepper.h>

AccelStepper stepperLarge(1, 10, 11);
AccelStepper stepperSmall(1, 5, 6);

//-------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------
const float oneRPM = 400/60; // 400 is the pulse per rev
const int maxRPM = 120;
const int minRPM = 10;

bool startRandom = true;

// int timeLarge, timeSmall;
int randTime;
int speedLarge, speedSmall;

unsigned long currentTime, deltaTime;

long randNumLarge, randNumSmall;
int randPosNevLarge, randPosNevSmall;

const int timeRangeForRandom = 5; // run time will be random from 1 to 6 sec
//--------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------


void setup()
{
	stepperLarge.setMaxSpeed(2000);
	stepperLarge.setAcceleration(2000);

	stepperSmall.setMaxSpeed(2000);
	stepperSmall.setAcceleration(2000);	
}

void loop()
{
	randomMode();
}

void randomMode()
{
	currentTime = millis();

	if (startRandom == true)
	{
		randPosNevLarge = - 1 + rand() % (3); // random between -1 and 1
		randPosNevSmall = - 1 + rand() % (3); // random between -1 and 1

		randNumLarge = randPosNevLarge * rand();
		randNumSmall = randPosNevSmall * rand();

		randTime = (abs(randNumLarge % timeRangeForRandom) + 1);

		speedLarge = (randNumLarge % maxRPM);
		speedSmall = (randNumSmall % maxRPM)

		if (speedLarge > -10 && speedLarge < 0)
			speedLarge = -10;
		if (speedLarge < 10 && speedLarge > 0)
			speedLarge = 10;
		if (speedSmall > -10 && speedSmall < 0)
			speedSmall = -10;
		if (speedSmall < 10 && speedSmall > 0)
			speedSmall = 10;

		stepperLarge.setSpeed(speedLarge * oneRPM);
		stepperSmall.setSpeed(speedSmall * oneRPM);
	}

	stepperLarge.runSpeed();
	stepperSmall.runSpeed();

	if (currentTime - deltaTime > randTime * 1000)
	{
		startRandom = true;
		deltaTime = currentTime;
	}
	else
		startRandom = false;
}
