// GPIO_Manager.h

#ifndef GPIO_MANAGER_H_
#define GPIO_MANAGER_H_

#include <stdio.h>
#include <string>
#include <wiringPi.h>
#include <iostream>

#define START_CAPTURE 3
#define STOP_CAPTURE 0
#define GREEN_LED 1
#define RED_LED 5
#define YELLOW_LED 4
#define READY_LED 6

using namespace std;

class GPIO_Manager
{
	public:
		// Initialize a GPIO Manager
		GPIO_Manager(void);

		// Interface with LEDs
		// - Input is the LED (0 = Green, 1 = Red, 2 = Yellow, 3 = Ready)
		// - 	and the mode (0 = off, 1 = on)
		// - Output is 0 for success, -1 for failure
		int setLED(int led, int mode);

		// Check if input is on
		bool startPressed(void);
		bool stopPressed(void);
};

	

#endif // GPIO_MANAGER_H_