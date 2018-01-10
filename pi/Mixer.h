// Mixer.h

#ifndef MIXER_H_
#define MIXER_H_

#include <alsa/asoundlib.h>
#include <stdio.h>
#include <string>
#include <fcntl.h>
#include "GPIO_Manager.h"
#include <cmath>

using namespace std;

// Use the newer ALSA API 
#define ALSA_PCM_NEW_HW_PARAMS_API

class Mixer
{
	public:
		// Create a new object of type Mixer
		Mixer(GPIO_Manager* gpio_manager);

		// Set the volume of the Raspberry Pi
		// - Argument is the volume out of ten (ten being max)
		// - Returns 0 if successful, -1 if not
		int setVolume(int vol);

		// Get the volume of the Raspberry Pi
		// - Returns volume out of ten if successful
		// - Returns -1 if not successful
		int getVolume(void);

		// Play back an audio file with the passed through string
		// - Arguments are file name as a string and length of audio in microseconds
		// - Return value is 0 for success or a negative error code
		int play(string file, int length);

		// Capture an audio file
		// - Argument is the name of the created audio file
		// - Return value is the length of the file or a negative error code
		int capture(string file);

	private:
		GPIO_Manager* gpio;
		long getdBValue(int vol, long maxdB, long mindB);
		int getVolValue(long vol, long maxdB, long mindB);
};

#endif // MIXER_H_