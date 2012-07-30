#include "slider.h"

const vertex sliders[11*4] = {

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

bufferObject generateSliderVBOs() {

	bufferObject ret;

	const int slidercount = 11;
	glGenBuffers(1, &ret.VBOid);
	glBindBuffer(GL_ARRAY_BUFFER, ret.VBOid);
	glBufferData(GL_ARRAY_BUFFER, 4*slidercount*sizeof(vertex), sliders, GL_STATIC_DRAW);

	GLushort *indices = new GLushort[6*slidercount];	// three indices per triangle, two triangles per line, BUFSIZE lines

	int i = 0;
	int j = 0;

	while (i < 6*slidercount) {

		indices[i] = j;
		indices[i+1] = j+1;
		indices[i+2] = j+3;
		indices[i+3] = j+1;
		indices[i+4] = j+2;
		indices[i+5] = j+3;

		i += 6;
		j += 4;


	}

	glGenBuffers(1, &ret.IBOid);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ret.IBOid);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, 3*2*slidercount*(sizeof(GLushort)), indices, GL_STATIC_DRAW);

	delete [] indices;

	return ret;


}


