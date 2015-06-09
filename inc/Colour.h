#pragma once

#include "types.h"
using namespace uti;

namespace tod
{
	struct SColour
	{
		float r,g,b,a;

		bool const operator==(const SColour &other) const
		{
			return r == other.r 
				&& g == other.g
				&& b == other.b
				&& a == other.a;
		}
	};

	inline void CreateColourFromRGB( SColour& col, uint32 rgba )
	{ 
		col.r = ((float)(uint8)(rgba >> 24)) / 255.0f;
		col.g = ((float)(uint8)(rgba >> 16)) / 255.0f;
		col.b = ((float)(uint8)(rgba >> 8))  / 255.0f;
		col.a = ((float)(uint8)(rgba))	     / 255.0f;
	}
};
