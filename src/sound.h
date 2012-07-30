#ifndef SOUND_H
#define SOUND_H

#ifdef _WIN32
#include <al.h>
#include <alc.h>	// audio library context

#elif __linux__
#include <AL/al.h>
#include <AL/alc.>
#endif

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
	float velocity[3];
	float orientation[6];

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
