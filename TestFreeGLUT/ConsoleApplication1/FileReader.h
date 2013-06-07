#include "stdafx.h"
#pragma once

namespace meddlingwithfire {
	class FileReader
	{
		public:
			FileReader();
			char* readAllText(const char* filename);
	};
}