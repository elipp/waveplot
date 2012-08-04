#ifndef UTILS_H
#define UTILS_H

#include <iostream>
#include <fstream>

#ifdef _WIN32
#include <xmmintrin.h> // for sse
#include <intrin.h>	// sse3, horizontal add
#include <smmintrin.h>
#endif

#include "definitions.h"

inline std::size_t cpp_getfilesize(std::ifstream& input);

char* readRawWAVBuffer(std::ifstream& input, std::size_t *bufsize);	// useless?
float* readSampleData_int16(std::ifstream& input, std::size_t* const numsamples); 
float* convertStereoToMono(float *stereodata, const std::size_t& num_samples);

WAVHEADERINFO readHeaderData(std::ifstream& input);

std::size_t getNumSamples(const WAVHEADERINFO& info);

double getDuration(const WAVHEADERINFO& info);

void printVertex(vertex * const v);

#endif 
