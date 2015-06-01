#pragma once

namespace tod
{
	struct SRect
	{
		float x,y,w,h;

		SRect( ) : x(0.f), y(0.f), w(0.f), h(0.f) {}
		SRect( float _x, float _y, float _w, float _h ) : x(_x), y(_y), w(_w), h(_h) {}
	};
}
