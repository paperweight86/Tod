#pragma once

#include "I2DRenderer.h"

namespace tod
{
	class C2DRendererFactory
	{
	private:
		C2DRendererFactory();
	public:
		~C2DRendererFactory();
		static bool Create( I2DRenderer** ppUIRdr, E2DRenderer eType );
	};
}
