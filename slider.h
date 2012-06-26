#ifndef SLIDER_H
#define SLIDER_H
#include "definitions.h"

#define slider_w 255.0
#define slider_h 31.0 

#define RATIO 1.0/256.0	// the slider texture is 256-by-256

vertex sliders[11*4] = {

	vertex( WIN_W - slider_w - 2.0, 5.0, 0.0, 1.0 ),
	vertex( WIN_W - slider_w - 2.0, 5.0 + slider_h, 0.0, 1.0 - RATIO*slider_h ),
	vertex( WIN_W - 2.0, 5.0 + slider_h, 1.0, 1.0 - RATIO*slider_h),
	vertex( WIN_W - 2.0, 5.0, 1.0, 1.0 ),

	vertex( WIN_W - slider_w - 2.0, 5.0 + slider_h*1, 0.0, 1.0 ),
	vertex( WIN_W - slider_w - 2.0, 5.0 + slider_h*2, 0.0, 1.0 - RATIO*slider_h ),
	vertex( WIN_W - 2.0, 5.0 + slider_h*2, 1.0, 1.0 - RATIO*slider_h),
	vertex( WIN_W - 2.0, 5.0 + slider_h*1, 1.0, 1.0 ),

	vertex( WIN_W - slider_w - 2.0, 5.0 + slider_h*2, 0.0, 1.0 ),
	vertex( WIN_W - slider_w - 2.0, 5.0 + slider_h*3, 0.0, 1.0 - RATIO*slider_h ),
	vertex( WIN_W - 2.0, 5.0 + slider_h*3, 1.0, 1.0 - RATIO*slider_h),
	vertex( WIN_W - 2.0, 5.0 + slider_h*2, 1.0, 1.0 ),

	vertex( WIN_W - slider_w - 2.0, 5.0 + slider_h*3, 0.0, 1.0 ),
	vertex( WIN_W - slider_w - 2.0, 5.0 + slider_h*4, 0.0, 1.0 - RATIO*slider_h ),
	vertex( WIN_W - 2.0, 5.0 + slider_h*4, 1.0, 1.0 - RATIO*slider_h),
	vertex( WIN_W - 2.0, 5.0 + slider_h*3, 1.0, 1.0 ),

	vertex( WIN_W - slider_w - 2.0, 5.0 + slider_h*4, 0.0, 1.0 ),
	vertex( WIN_W - slider_w - 2.0, 5.0 + slider_h*5, 0.0, 1.0 - RATIO*slider_h ),
	vertex( WIN_W - 2.0, 5.0 + slider_h*5, 1.0, 1.0 - RATIO*slider_h),
	vertex( WIN_W - 2.0, 5.0 + slider_h*4, 1.0, 1.0 ),

	vertex( WIN_W - slider_w - 2.0, 5.0 + slider_h*5, 0.0, 1.0 ),
	vertex( WIN_W - slider_w - 2.0, 5.0 + slider_h*6, 0.0, 1.0 - RATIO*slider_h ),
	vertex( WIN_W - 2.0, 5.0 + slider_h*6, 1.0, 1.0 - RATIO*slider_h),
	vertex( WIN_W - 2.0, 5.0 + slider_h*5, 1.0, 1.0 ),

	vertex( WIN_W - slider_w - 2.0, 5.0 + slider_h*6, 0.0, 1.0 ),
	vertex( WIN_W - slider_w - 2.0, 5.0 + slider_h*7, 0.0, 1.0 - RATIO*slider_h ),
	vertex( WIN_W - 2.0, 5.0 + slider_h*7, 1.0, 1.0 - RATIO*slider_h),
	vertex( WIN_W - 2.0, 5.0 + slider_h*6, 1.0, 1.0 ),

	vertex( WIN_W - slider_w - 2.0, 5.0 + slider_h*7, 0.0, 1.0 ),
	vertex( WIN_W - slider_w - 2.0, 5.0 + slider_h*8, 0.0, 1.0 - RATIO*slider_h ),
	vertex( WIN_W - 2.0, 5.0 + slider_h*8, 1.0, 1.0 - RATIO*slider_h),
	vertex( WIN_W - 2.0, 5.0 + slider_h*7, 1.0, 1.0 ),

	vertex( WIN_W - slider_w - 2.0, 5.0 + slider_h*8, 0.0, 1.0 ),
	vertex( WIN_W - slider_w - 2.0, 5.0 + slider_h*9, 0.0, 1.0 - RATIO*slider_h ),
	vertex( WIN_W - 2.0, 5.0 + slider_h*9, 1.0, 1.0 - RATIO*slider_h),
	vertex( WIN_W - 2.0, 5.0 + slider_h*8, 1.0, 1.0 ),

	vertex( WIN_W - slider_w - 2.0, 5.0 + slider_h*9, 0.0, 1.0 ),
	vertex( WIN_W - slider_w - 2.0, 5.0 + slider_h*10, 0.0, 1.0 - RATIO*slider_h ),
	vertex( WIN_W - 2.0, 5.0 + slider_h*10, 1.0, 1.0 - RATIO*slider_h),
	vertex( WIN_W - 2.0, 5.0 + slider_h*9, 1.0, 1.0 ),

	vertex( WIN_W - slider_w - 2.0, 5.0 + slider_h*10, 0.0, 1.0 ),
	vertex( WIN_W - slider_w - 2.0, 5.0 + slider_h*11, 0.0, 1.0 - RATIO*slider_h ),
	vertex( WIN_W - 2.0, 5.0 + slider_h*11, 1.0, 1.0 - RATIO*slider_h),
	vertex( WIN_W - 2.0, 5.0 + slider_h*10, 1.0, 1.0 )

};

#endif
