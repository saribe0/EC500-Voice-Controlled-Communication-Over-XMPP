// GPIO_Manager.cpp

#include "GPIO_Manager.h"

GPIO_Manager::GPIO_Manager()
{
	// Try to initialize wiring Pi
	if(wiringPiSetup() < 0) 
	{
		cout << "Unable to setup wiringPi" << endl;
	}

	// Sets LED pins to outputs
	pinMode(GREEN_LED, OUTPUT);
	pinMode(RED_LED, OUTPUT);
	pinMode(YELLOW_LED, OUTPUT);
	pinMode(READY_LED, OUTPUT);

	// Set Button pins to inputs
	pinMode(START_CAPTURE, INPUT);
	pinMode(STOP_CAPTURE, INPUT);
}

int GPIO_Manager::setLED(int led, int mode)
{
	switch(led)
	{
		// Green LED
		case 0: digitalWrite (GREEN_LED, mode);
				break;
		// Red LED
		case 1: digitalWrite (RED_LED, mode);
				break;
		// Yellow LED
		case 2: digitalWrite (YELLOW_LED, mode);
				break;
		case 3: digitalWrite (READY_LED, mode);
				break;
		default: return -1;
	}

	return 0;
}

bool GPIO_Manager::startPressed(void) 
{
	if (digitalRead(START_CAPTURE) == 1) 
	{
		return false;
	}
	else 
	{
		cout << "Start Pressed" << endl;
		return true;
	}
}

bool GPIO_Manager::stopPressed(void) 
{
	if (digitalRead(STOP_CAPTURE) == 1) 
	{
		return false;
	}
	else 
	{
		cout << "Stop Pressed" << endl;
		return true;
	}
}





