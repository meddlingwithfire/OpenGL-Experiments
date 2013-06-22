#pragma once

#include "stdafx.h"
#include "TargaReader.h"

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

namespace meddlingwithfire
{
	TargaReader::TargaReader(void)
	{

	}

	////////////////////////////////////////////////////////////////////
	// Allocate memory and load targa bits. Returns pointer to new buffer,
	// height, and width of texture, and the OpenGL format of data.
	// Call free() on buffer when finished!
	// This only works on pretty vanilla targas... 8, 24, or 32 bit color
	// only, no palettes, no RLE encoding.
	// This function also takes an optional final parameter to preallocated 
	// storage for loading in the image data.
	GLbyte* TargaReader::readTGABits(const char* szFileName, GLint* iWidth, GLint* iHeight, GLint* iComponents, GLenum* eFormat, GLbyte *pData)
	{
		FILE *pFile;			// File pointer
		TGAHEADER tgaHeader;		// TGA file header
		unsigned long lImageSize;		// Size in bytes of image
		short sDepth;			// Pixel depth;
		GLbyte* pBits = NULL;          // Pointer to bits

		// Default/Failed values
		*iWidth = 0;
		*iHeight = 0;
		*eFormat = GL_RGB;
		*iComponents = GL_RGB;

		// Attempt to open the file
		pFile = fopen(szFileName, "rb");
		if(pFile == NULL)
			return NULL;

		// Read in header (binary)
		fread(&tgaHeader, 18, 1, pFile);

		// Get width, height, and depth of texture
		*iWidth = tgaHeader.width;
		*iHeight = tgaHeader.height;
		sDepth = tgaHeader.bits / 8;

		// Put some validity checks here. Very simply, I only understand
		// or care about 8, 24, or 32 bit targa's.
		if(tgaHeader.bits != 8 && tgaHeader.bits != 24 && tgaHeader.bits != 32)
			return NULL;

		// Calculate size of image buffer
		lImageSize = tgaHeader.width * tgaHeader.height * sDepth;

		// Allocate memory and check for success
		if(pData == NULL) 
			pBits = (GLbyte*)malloc(lImageSize * sizeof(GLbyte));
		else 
			pBits = pData; 

		// Read in the bits
		// Check for read error. This should catch RLE or other 
		// weird formats that I don't want to recognize
		if(fread(pBits, lImageSize, 1, pFile) != 1)
		{
			if(pBits != NULL)
				free(pBits);
			return NULL;
		}

		// Set OpenGL format expected
		switch(sDepth)
		{
			case 4:
				*eFormat = GL_BGRA;
				*iComponents = GL_RGBA;
				break;
			case 1:
				*eFormat = GL_LUMINANCE;
				*iComponents = GL_LUMINANCE;
				break;
			default:        // RGB
				// If on the iPhone, TGA's are BGR, and the iPhone does not 
				// support BGR without alpha, but it does support RGB,
				// so a simple swizzle of the red and blue bytes will suffice.
				// For faster iPhone loads however, save your TGA's with an Alpha!
				break;
		}

		// Done with File
		fclose(pFile);

		// Return pointer to image data
		return pBits;
	}
}