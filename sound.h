#ifndef SOUND_H
#define SOUND_H

#include <AL/al.h>
#include <AL/alc.h>	// audio library context
#include <iostream>
#include <fstream>
#include "definitions.h"
#include "utils.h"

namespace Sound {

	bool createContext();
	void playSound();

	ALuint source, buffer;

	ALCdevice* device;
	ALCcontext* context;

	float pitch, gain;
	float[3] velocity;
	float[6] orientation;

	bool createContext() {

		device = alcOpenDevice(NULL);
		context = alcCreateContext(device, NULL);
		alcMakeContextCurrent(context);

		if (!device || !context) { 
			return false;
		}

		else { return true; }

	}



};




#endif
