#pragma once
#include <gl\glew.h>
#include "stdafx.h"

/*
	Code copied from Richard S. Wright Jr.
	Original copyright notice below.
*/
/* Copyright (c) 2005-2010, Richard S. Wright Jr.
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, 
are permitted provided that the following conditions are met:

Redistributions of source code must retain the above copyright notice, this list 
of conditions and the following disclaimer.

Redistributions in binary form must reproduce the above copyright notice, this list 
of conditions and the following disclaimer in the documentation and/or other 
materials provided with the distribution.

Neither the name of Richard S. Wright Jr. nor the names of other contributors may be used 
to endorse or promote products derived from this software without specific prior 
written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY 
EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES 
OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT 
SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, 
INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED 
TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR 
BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN 
CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN 
ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

namespace meddlingwithfire {
	typedef struct
	{
		GLbyte	identsize;              // Size of ID field that follows header (0)
		GLbyte	colorMapType;           // 0 = None, 1 = paletted
		GLbyte	imageType;              // 0 = none, 1 = indexed, 2 = rgb, 3 = grey, +8=rle
		unsigned short	colorMapStart;          // First colour map entry
		unsigned short	colorMapLength;         // Number of colors
		unsigned char 	colorMapBits;   // bits per palette entry
		unsigned short	xstart;                 // image x origin
		unsigned short	ystart;                 // image y origin
		unsigned short	width;                  // width in pixels
		unsigned short	height;                 // height in pixels
		GLbyte	bits;                   // bits per pixel (8 16, 24, 32)
		GLbyte	descriptor;             // image descriptor
	} TGAHEADER;
	
	class TargaReader
	{
		public:
			TargaReader();
			GLbyte* readTGABits(const char* szFileName, GLint* iWidth, GLint* iHeight, GLint* iComponents, GLenum* eFormat, GLbyte *pData = NULL);
	};
}