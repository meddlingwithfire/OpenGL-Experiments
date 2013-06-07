#pragma once

#include "stdafx.h"
#include "FileReader.h"

namespace meddlingwithfire
{
	FileReader::FileReader(void)
	{

	}

	/**
	* Store all the file's contents in memory, useful to pass shaders
	* source code to OpenGL
	*/
	/* Problem:
	*  We should close the input file before the return NULL; statements but this would lead to a lot of repetition (DRY)
	*   -you could solve this by using goto or by abusing switch/for/while + break or by building an if else mess
	*  better solution: let the user handle the File: char* file_read(const FILE* input)
	*/
	char* FileReader::readAllText(const char* filename)
	{
		FILE* input = fopen(filename, "rb");
		if(input == NULL)
		{
			return NULL;
		}

		if(fseek(input, 0, SEEK_END) == -1)
		{
			return NULL;
		}
		long size = ftell(input);
		if(size == -1)
		{
			return NULL;
		}
		if(fseek(input, 0, SEEK_SET) == -1)
		{
			return NULL;
		}

		/*if using c-compiler: dont cast malloc's return value*/
		char *content = (char*) malloc( (size_t) size +1  ); 
		if(content == NULL)
		{
			return NULL;
		}

		fread(content, 1, (size_t)size, input);
		if(ferror(input)) {
			free(content);
			return NULL;
		}

		fclose(input);
		content[size] = '\0';
		return content;
	}
}