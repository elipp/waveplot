#ifndef SLIDER_H
#define SLIDER_H
#include "definitions.h"
static const float slider_w = 255.0, slider_h = 31.0;

static const float RATIO=1.0/256.0;	// the slider texture is 256-by-256

bufferObject generateSliderVBOs();

#endif
