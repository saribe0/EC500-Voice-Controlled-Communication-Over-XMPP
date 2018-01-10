// Mixer.cpp

#include "Mixer.h"

// Create a new object of type Mixer
Mixer::Mixer(GPIO_Manager* gpio_manager)
{
	gpio = gpio_manager;
}

// Takes an int and converts it to a volume
// - Max int is 10, min is 1 
long Mixer::getdBValue(int vol, long maxdB, long mindB)
{
	double maxdBD = (double)maxdB / 100;
	
	if (vol < 0 || vol > 10)
		return -1;
	if (vol == 0)
		return mindB;

	double diff = 10*log(10)/log(2);
	double add = 10*log((double)vol)/log(2);

	if (vol != 1)
		return (long)((maxdBD-diff+add) * 100);
	else
		return (long)((maxdBD-diff+1) * 100);
}

int Mixer::getVolValue(long vol, long maxdB, long mindB)
{
	double volD = (double)vol / 100;
	double maxdBD = (double)maxdB / 100;
	double mindBD = (double)mindB / 100;

	double diff = 10*log(10)/log(2);
	double add = 10*log(1)/log(2);
	if (volD == mindBD)
		return 0;
	else if (volD == maxdBD - diff + 1)
		return 1;
	else
	{
		double arg = (volD - maxdBD + diff) / 10.0 * log(2);
		return (int)(exp(arg) + 0.5);
	}
}

// Set the volume of the Raspberry Pi
// - Argument is the volume out of ten (ten being max)
// - Returns 0 if successful, -1 if not
int Mixer::setVolume(int vol)
{
	cout << "*** Setting volume to " << vol << " ***" << endl;
	if(vol < 0 || vol > 10)
		return -1;

	long min, max;
	snd_mixer_t *handle;
	snd_mixer_selem_id_t *sid;
	const char *selem_name = "PCM";

	// Open an empty mixer
	if (snd_mixer_open(&handle, 0) != 0)
	{
		cout << "Could not open mixer" << endl;
		return -1;
	}

	// Attach the empty mixer to the hardware device
	if (snd_mixer_attach(handle, "default") != 0)
	{
		cout << "Could not attach mixer to hardware" << endl;
		return -1;
	}

	// Register the mixer
	if (snd_mixer_selem_register(handle, NULL, NULL) != 0)
	{
		cout << "Could not register the mixer" << endl;
		return -1;
	}

	// Load the mixer elements
	if (snd_mixer_load(handle) != 0)
	{
		cout << "Could not load mixer elements" << endl;
		return -1;
	}

	// Allocate an invalid mixer ID
	snd_mixer_selem_id_alloca(&sid);

	// Set the index of the simple mixer
	snd_mixer_selem_id_set_index(sid, 0);

	// Set the name of the simple mixer
	snd_mixer_selem_id_set_name(sid, selem_name);
	
	// Gets a simple mixer handle from a mixer and a simple id
	snd_mixer_elem_t* elem = snd_mixer_find_selem(handle, sid);
	if (elem == NULL)
	{
		cout << "Could not get a simple mixer handle" << endl;
		return -1;
	}

	// Get the range of possible volume values
	if (snd_mixer_selem_get_playback_dB_range(elem, &min, &max) != 0)
	{
		cout << "Could not get playback volume range" << endl;
		return -1;
	}

	// Set the new volume based on range
	cout << "Max: " << max << ", New: " << getdBValue(vol, max, min) << endl;
	if (snd_mixer_selem_set_playback_dB_all(elem, getdBValue(vol, max, min), 1) != 0)
	{
		cout << "Could not set playback volume" << endl;
		return -1;
	}

	// Close the mixer
	if (snd_mixer_close(handle) != 0)
	{
		cout << "Could not close mixer" << endl;
		return -1;
	}

	cout << "*** Done setting volume ***" << endl;

	return 0;
}

// Get the volume of the Raspberry Pi
// - Returns volume out of ten if successful
// - Returns -1 if not successful
int Mixer::getVolume(void)
{
	cout << "*** Getting volume ***" << endl;

	long min, max, current;
	snd_mixer_t *handle;
	snd_mixer_selem_id_t *sid;
	const char *selem_name = "PCM";

	// Open an empty mixer
	if (snd_mixer_open(&handle, 0) != 0)
	{
		cout << "Could not open mixer" << endl;
		return -1;
	}

	// Attach the empty mixer to the hardware device
	if (snd_mixer_attach(handle, "default") != 0)
	{
		cout << "Could not attach handle to mixer" << endl;
		return -1;
	}

	// Register the mixer
	if (snd_mixer_selem_register(handle, NULL, NULL) != 0)
	{
		cout << "Could not register mixer" << endl;
		return -1;
	}

	// Load the mixer elements
	if(snd_mixer_load(handle) != 0)
	{
		cout << "Could not load mixer elements" << endl;
		return -1;
	}

	// Allocate an invalid mixer ID
	snd_mixer_selem_id_alloca(&sid);

	// Set the index of the simple mixer
	snd_mixer_selem_id_set_index(sid, 0);

	// Set the name of the simple mixer
	snd_mixer_selem_id_set_name(sid, selem_name);
	
	// Gets a simple mixer handle from a mixer and a simple id
	snd_mixer_elem_t* elem = snd_mixer_find_selem(handle, sid);
	if (elem == NULL)
	{
		cout << "Could get simple mixer" << endl;
		return -1;
	}

	// Get the range of possible volume values
	if (snd_mixer_selem_get_playback_dB_range(elem, &min, &max) != 0)
	{
		cout << "Could not get volume range" << endl;
		return -1;
	}

	// Get the volume based on range
	if (snd_mixer_selem_get_playback_dB(elem, SND_MIXER_SCHN_MONO, &current) != 0)
	{
		cout << "Could not get the playback volume" << endl;
		return -1;
	}

	// Close the mixer
	if (snd_mixer_close(handle) != 0)
	{
		cout << "Could not close mixer" << endl;
		return -1;
	}

	cout << "*** Done getting volume as " << current << " ***" << endl;

	// Return the current volume out of 10
	return getVolValue(current, max, min);
}

// Play back an audio file with the passed through string
// - Arguments are file name as a string and length of audio in microseconds
// - Return value is 0 for success or a negative error code
int Mixer::play(string file, int length)
{
	int err;
	int size;
	snd_pcm_t *handle;
	snd_pcm_hw_params_t *params;
	unsigned int val = 16001;			// Must redeclare, gets overwritten
	snd_pcm_uframes_t frames = 32;		// Must redeclare, gets overwritten
	int dir;
	long loops;
	char *buffer;

	cout << "Opening PCM" << endl;

	// Open PCM device for playback
	err = snd_pcm_open(&handle, "default", SND_PCM_STREAM_PLAYBACK, 0);
	if (err < 0) 
	{
		cout << "Unable to open pcm device" << endl;
		return -1;
	}
  
	// Allocate a hardware parameters object 
	snd_pcm_hw_params_alloca(&params);

	// Fill it in with default values
	err = snd_pcm_hw_params_any(handle, params);
	if (err < 0)
	{
		cout << "Cannot initialize hw params" << endl;
		return -1;
	}

	// Set the desired hardware parameters 

	// Interleaved mode 
	err = snd_pcm_hw_params_set_access(handle, params, SND_PCM_ACCESS_RW_INTERLEAVED);
	if (err < 0)
	{
		cout << "Cannot set access type" << endl;
		return -1;
	}

	// Unsigned 8-bit 
	err = snd_pcm_hw_params_set_format(handle, params, SND_PCM_FORMAT_S16_LE);
	if (err < 0)
	{
		cout << "Cannot set sample format" << endl;
		return -1;
	}

	// Mono because mic only has one channel 
	err = snd_pcm_hw_params_set_channels(handle, params, 1);
	if (err < 0)
	{
		cout << "Cannot set channel count" << endl;
		return -1;
	}

	// Set sampling rate 
	err = snd_pcm_hw_params_set_rate_near(handle, params, &val, &dir);
	if (err < 0)
	{
		cout << "Cannot set sample rate" << endl;
		return -1;
	}

	// Set period size to frames 
	err = snd_pcm_hw_params_set_period_size_near(handle, params, &frames, &dir);
	if (err < 0)
	{
		cout << "Cannot set period size" << endl;
		return -1;
	}
	cout << frames << endl;

	// Write the parameters to the driver 
	err = snd_pcm_hw_params(handle, params);
	if (err < 0) 
	{
		cout << "Unable to set hw parameters" << endl;
		return -1;
	}

	// Use a buffer large enough to hold one period 
	snd_pcm_hw_params_get_period_size(params, &frames, &dir);

	size = frames * 2; // 2 bytes/sample, 2 channels 
	buffer = (char *) malloc(size);

	// We want to loop for 5 seconds 
	snd_pcm_hw_params_get_period_time(params, &val, &dir);

	// Length in microseconds divided by period time 
	loops = length / val;

	// Open the audio file to be played 
	int fildes = open(file.c_str(), O_RDONLY);
	if (fildes == -1)
		return -1;

	// Loop though the file, playing as it goes 
	while (loops > 0) 
	{
		loops--;
		err = read(fildes, buffer, size);
		if (err == 0) 
		{
			cout << "End of file on input" << endl;
			break;
		} 
		/*else if (err != size) 
		{
			cout << "Error, did not read all bytes" << endl;
		}*/

		err = snd_pcm_writei(handle, buffer, frames);
		if (err == -EPIPE) 
		{
			// EPIPE means underrun 
			cout << "Underrun occurred" << endl;
			snd_pcm_prepare(handle);
		} 
		else if (err < 0) 
		{
			cout << "Error from writei" << endl;
			snd_strerror(err);
		}
		else if (err != (int)frames) 
		{
			cout << "Error, did not write all frams" << endl;
		}
	}

	// Clean up and return success 
	snd_pcm_drain(handle);
	snd_pcm_close(handle);
	close(fildes);
	free(buffer);
  
	return 0;
}

// Capture an audio file
// - Argument is the name of the created audio file
// - Return value is the length of the file or a negative error code
int Mixer::capture(string file)
{
	int err;
	int size;
	snd_pcm_t *handle;
	snd_pcm_hw_params_t *params;
	unsigned int val = 16000;
	snd_pcm_uframes_t frames = 32;
	int dir;
	long loops;
	char *buffer;

	int length = 0; // Return value
  
	// Open PCM device for recording (capture)
	err = snd_pcm_open(&handle, "plughw:1,0", SND_PCM_STREAM_CAPTURE, 0);
	if (err < 0) 
	{
		cout << "Unable to open pcm device" << endl;
		return -1;
	}
  
	// Allocate a hardware parameters object 
	snd_pcm_hw_params_alloca(&params);

	// Fill it in with default values
	err = snd_pcm_hw_params_any(handle, params);
	if (err < 0)
	{
		cout << "Cannot initialize hw params" << endl;
		return -1;
	}

	// Set the desired hardware parameters

	// Interleaved mode 
	err = snd_pcm_hw_params_set_access(handle, params, SND_PCM_ACCESS_RW_INTERLEAVED);
	if (err < 0)
	{
		cout << "Cannot set access type" << endl;
		return -1;
	}

	// Signed 16-bit little-endian format 
	err = snd_pcm_hw_params_set_format(handle, params, SND_PCM_FORMAT_S16_LE);
	if (err < 0)
	{
		cout << "Cannot set sample format" << endl;
		return -1;
	}

	// Mono because mic only has one channel
	err = snd_pcm_hw_params_set_channels(handle, params, 1);
	if (err < 0)
	{
		cout << "Cannot set channel count" << endl;
		return -1;
	}

	// Sampling rate as declared in header 
	err = snd_pcm_hw_params_set_rate_near(handle, params, &val, &dir);
	if (err < 0)
	{
		cout << "Cannot set sample rate" << endl;
		return -1;
	}

	// Set period size to frames as set in heaeder. 
	err = snd_pcm_hw_params_set_period_size_near(handle, params, &frames, &dir);
	if (err < 0)
	{
		cout << "Cannot set period size" << endl;
		return -1;
	}

	// Write the parameters to the driver 
	err = snd_pcm_hw_params(handle, params);
	if (err < 0) 
	{
		cout << "Unable to set hw parameters" << endl;
		return -1;
	}

	// Use a buffer large enough to hold one period 
	snd_pcm_hw_params_get_period_size(params, &frames, &dir);
	size = frames * 2; // 2 bytes/sample, 2 channels 
	buffer = (char *) malloc(size);

	// We want to loop for a maxium of 2 minutes or an interrupt 
	snd_pcm_hw_params_get_period_time(params, &val, &dir);
	loops = 60*2*1000000 / val;

	// Open the file to write to
	// - If file exists, its length is set to 0 and it is written over
	// - If file does not exist, it is created
	mode_t mode = S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH;
	int fildes = open(file.c_str(), O_WRONLY | O_CREAT | O_TRUNC, mode);
	if (fildes == -1)
		return -1;

	// Records for two minutes or until stopped
	while (loops > 0 && gpio->stopPressed() == false) 
	{
		loops--;
		err = snd_pcm_readi(handle, buffer, frames);
		if (err == -EPIPE) 
		{
			// EPIPE means overrun 
			cout << "Overrun occurred" << endl;
			snd_pcm_prepare(handle);
		} 
		else if (err < 0) 
		{
			cout << "Error occurred from read" << endl;
		} 
		else if (err != (int)frames) 
		{
			cout << "Error, short read" << endl;
		}

		err = write(fildes, buffer, size);
		if (err != size)
		{
			cout << "Error, short write occurred" << endl;
		}

		// Update length
		length += val;
	}

	// Clean up and return length 
	snd_pcm_drain(handle);
	snd_pcm_close(handle);
	close(fildes);
	free(buffer);

	return length;
}



