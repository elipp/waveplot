#ifndef UTILS_H
#define UTILS_H

#ifdef _WIN32
#include <Windows.h>
#endif

#include <iostream>
#include <fstream>
#include "definitions.h"


std::size_t cpp_getfilesize(std::ifstream& input);

char* readRawWAVBuffer(std::ifstream& input, std::size_t *bufsize);
float* readSampleData_int16(std::ifstream& input, std::size_t* const numsamples); 

WAVHEADERINFO readHeaderData(std::ifstream& input);

std::size_t getNumSamples(const WAVHEADERINFO& info);

double getDuration(const WAVHEADERINFO& info);

#endif 
