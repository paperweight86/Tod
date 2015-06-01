#include "stdafx.h"

#include "2DRendererFactory.h"
#include "D2D1Renderer.h"

namespace tod
{

bool C2DRendererFactory::Create( I2DRenderer** pp2DRdr, E2DRenderer eType )
{
	switch( eType )
	{
	case e2DRenderer_Direct2D:
		(*pp2DRdr) = new CD2D1Renderer();
		break;
	case e2DRenderer_Unknown:
	default:
		(*pp2DRdr) = nullptr;
		break;
	}

	return (*pp2DRdr) != nullptr;
}

}
