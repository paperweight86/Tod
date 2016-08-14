#pragma once

#include "2DRenderers.h"
#include "Colour.h"
#include "Rect.h"

#include <array>

namespace tod
{
	enum EResourceType
	{
		eResourceType_RectangleGeometry,
		eResourceType_PathGeometry,
		eResourceType_ShapeGeometry,
		eResourceType_SolidBrush,
		eResourceType_TextFormat,
		eResourceType_EllipseGeometry,

		eResourceType_Bitmap,

		eResourceType_RenderTarget,
		
		FORCE_UINT32 = uint32_max
	};

	class I2DRenderer
	{
	protected:
		E2DRenderer m_eType;
	public:
		I2DRenderer( ) : m_eType(e2DRenderer_Unknown) {}
		virtual ~I2DRenderer( ) {}
		virtual E2DRenderer GetType( ) const { return m_eType; };

		virtual bool Initialise( ptr hwnd, uint32 width, uint32 height ) = 0;
		virtual void Deinitialise( ) = 0;

		virtual void SetClearColor(SColour colour) = 0;

		virtual void Update( uint32 dt ) = 0;
		virtual void BeginDraw( ) = 0;
		virtual void EndDraw( ) = 0;

		virtual rhandle CreateRectangleGeometry( SRect rect ) = 0;
		virtual rhandle CreateEllipseGeometry( const float2& pos, const float2& size ) = 0;
		virtual rhandle CreateGeometry( const float2* vertices, uint32 vertexCount, SRect* rect = 0 ) = 0;
		virtual rhandle CreateFillGeometry( const float2* vertices, uint32 vertexCount, SRect* rect = 0 ) = 0;

		virtual rhandle CreateBrush( const SColour& colour ) = 0;

		virtual rhandle CreateImage( uint16 width, uint16 height, uint8* data ) = 0;
		virtual rhandle CreateImageFromRenderTarget(rhandle hRenderTarget) = 0;

		virtual void    UpdateImage( rhandle img, uint16 width, uint16 height, uint8* data ) = 0;
		virtual void	UpdateGeometry(rhandle geo, const float2* vertices, uint32 vertexCount) = 0;

		virtual rhandle CreateRenderTarget(bool present = true) = 0;
		virtual void	SetRenderTarget(rhandle renderTarget) = 0;

		virtual void DrawGeometry( rhandle hGeometry, rhandle hBrush ) = 0;
		virtual void DrawGeometry(rhandle hGeometry, rhandle hBrush, float2 translation) = 0;
		virtual void DrawFillGeometry( rhandle hGeometry, rhandle hBrush ) = 0;
		virtual void DrawFillGeometry( rhandle hGeometry, rhandle hBrush, float2 translation ) = 0;
		virtual void DrawFillGeometry(rhandle hGeometry, rhandle hBrush, float2 translation, float2 scale) = 0;
		virtual void DrawFillGeometry(rhandle hGeometry, rhandle hBrush, float2 translation, float2 scale, float rotAngle,
									  float2 rotCenter = float2(0.0f, 0.0f)) = 0;
		virtual void DrawTextString(wstr string, SRect rect, rhandle hBrush) = 0;

		virtual void DrawBitmap( rhandle hBitmap, float2 pos, float2 scale ) = 0;
		virtual void DrawBitmap( rhandle hBitmap, float2 pos, float2 scale, float opacity ) = 0;

		virtual void DrawRectangle(rhandle hBrush, SRect rect, float2 offset = float2(0.0f, 0.0f), float2 scale = float2(1.0f, 1.0f), float stroke = 0.0f) = 0;

		virtual bool SavePngImage(rhandle hRenderTarget, wstr path) = 0;

		virtual void DestroyResource(rhandle hResource) = 0;
	};
}
