#include "utils.h"

std::size_t cpp_getfilesize(std::ifstream& input) {

        input.seekg (0, std::ios::end);
        std::size_t length = input.tellg();
        input.seekg(0, std::ios::beg);

        return length;

}

float* readSampleData_int16(std::ifstream& input, std::size_t* const num_samples) {

        std::size_t filesize = cpp_getfilesize(input);
        const std::size_t numsamples = (filesize-44)/2;
	*num_samples = numsamples;	// acts as the second return value
        short *sampledata = new short[numsamples];

        input.seekg(44, std::ios::beg); // perhaps redundant, but better to be sure

        input.read((char*)sampledata, filesize-44);

        float middle = (float)(0x1 << 15);

        std::cout << "filesize: " << filesize << "\n"
                  << "# of samples: " << numsamples << "\n";

        float *samples = new float[numsamples];

        for (int i = 0; i < numsamples; i++) {
                samples[i] = ((float)(sampledata[i]) / middle) - 1.0;
		//std::cout << sampledata[i] << " ";
        }
		delete [] sampledata;

        return samples;
}


WAVHEADERINFO readHeaderData(std::ifstream &input) {

        WAVHEADERINFO info;

        // the first 44 bytes of a wavfile is the header info.

        input.seekg(0, std::ios::beg);
        input.read((char*)&info, 44);

        return info;

}

std::size_t getNumSamples(const WAVHEADERINFO& info) {

	return ((info.chunksize-36)/info.bitDepth);

}

double getDuration(const WAVHEADERINFO& info) {

	return ((double)getNumSamples(info)/(double)info.sampleRate);

}


char* readRawWAVBuffer(std::ifstream& input, std::size_t *bufsize) {

	*bufsize = cpp_getfilesize(input)-44;

	char* buffer = new char[(*bufsize)];
	input.seekg(44, std::ios::beg);
	input.read(buffer, (*bufsize));
	input.seekg(0, std::ios::beg);

	return buffer;

}
