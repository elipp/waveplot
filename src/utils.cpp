#include "utils.h"

std::size_t cpp_getfilesize(std::ifstream& input) {

        input.seekg (0, std::ios::end);
        std::size_t length = input.tellg();
        input.seekg(0, std::ios::beg);

        return length;

}

float* readSampleData_int16(std::ifstream& input, std::size_t* const num_samples) {

        std::size_t filesize = cpp_getfilesize(input);
       
		static const std::size_t FILE_MAX = (0x1 << 24);

		if (filesize > FILE_MAX) {
			filesize = FILE_MAX;
		}

		const std::size_t numsamples = (filesize-44)/2;
		short *sampledata = new short[numsamples];

		WAVHEADERINFO info;
		input.seekg(0, std::ios::beg);
		input.read((char*)&info, 44);
		
		// *validate header somehow*
	
		input.seekg(44, std::ios::beg); // perhaps redundant, but better to be sure

        input.read((char*)sampledata, filesize-44);

        static const float max = (float)(0x1 << 15);
		__declspec(align(16)) float *samples = new float[numsamples];
		
		// this conversion can be done with SSE.
		// - tested this, was slow as hell with SSE as well as with SSE4.
		// Even without any kind of optimization the vanilla version seems to be a lot faster

        for (unsigned int i = 0; i < numsamples; i++) 
                samples[i] = ((float)(sampledata[i]) / max);
		
		if (info.numChannels == 2) {
			samples = downMixStereoToMono(samples, numsamples);
			*num_samples = numsamples/2;

		} else { *num_samples = numsamples; }

        std::cout << "filesize: " << filesize << "\n"
                  << "# of samples: " << *num_samples << "\n";
		

		delete [] sampledata;

        return samples;
}

float* downMixStereoToMono(float *stereodata, const std::size_t& num_samples) {

	const std::size_t num_monosamples = num_samples/2;

#ifdef _WIN32
	
	__declspec(align(16)) float *monodata = new float[num_monosamples];
	// a great spot for some SSE (or even OpenCL) wizardry as well :P
	__m128 a, b;
	const __m128 half = _mm_set1_ps(0.5);	// fill whole register. mul is always faster than div 
	
	// find out how many full 16-byte cycles can be done

	// this probably expands to just one div instruction
	const std::size_t f = 8;
	const std::size_t num_full_cycles = num_samples/f;
	const std::size_t num_left = num_full_cycles%f;

	int i = 0;
	for (i = 0; i < num_full_cycles; i++) {
		a = _mm_load_ps((const float*)&stereodata[8*i]);	// the stereodata should be 16-byte aligned: SEE readSampleData_int16 
		b = _mm_load_ps((const float*)&stereodata[8*i + 4]);
		__m128 c = _mm_mul_ps(_mm_hadd_ps(a,b), half);	// horizontal add
		_mm_store_ps(&monodata[4*i], c);

	}
	for (; i < num_full_cycles + num_left; i++) {
		monodata[i] = 0.5*(stereodata[2*i]+stereodata[2*i+1]);
	}
	
#elif __linux__
	// stub.nyi.
#endif

	delete [] stereodata;

	return monodata;

}

WAVHEADERINFO readHeaderData(std::ifstream &input) {

        WAVHEADERINFO info;

        // the first 44 bytes of a wavfile is the header info.

        input.seekg(0, std::ios::beg);
        input.read((char*)&info, 44);

        return info;

}

std::size_t getNumSamples(const WAVHEADERINFO& info) {

	return ((info.chunkSize-36)/info.bitDepth);

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


void printVertex(vertex * const v) {

	printf("%f %f %f %f\n", v->x, v->y, v->u, v->v);

}