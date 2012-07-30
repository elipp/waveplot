#include "sound.h"


bool createALContext() {


	





}

int main(int argc, char* argv[]) {


	if (argc < 2) {
		std::cout << "No input\n";
		return 1;

	}

	const char* filename = argv[1];
	std::ifstream input(filename, std::ios::binary);

	if (!input.is_open()) {

		std::cout << "Couldn't open file " << filename << ": no such file or directory.\n";
		return 1;

	}

	WAVHEADERINFO info = readHeaderData(input);

	ALCdevice* device = alcOpenDevice(NULL);
	if (!device) { 
		std::cout << "No device\n";
		return 1;

	alSourcei(source1, AL_BUFFER, buffer1);
	delete [] buffer;
	
	ALenum err = alGetError();
	if (err != AL_NO_ERROR) {

		std::cout << "Error occurred. Error code: " << (int)err << "\n";
		return 1;

	}
	
	alSourcePlay(source1);

	int a;
	std::cin >> a;

	alDeleteSources(1, &source1);
	alDeleteBuffers(1, &buffer1);
	alcDestroyContext(context);
	alcCloseDevice(device);

	return 0;

}
