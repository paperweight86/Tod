#pragma once

#include "I2DRenderer.h"
#include "types.h"

#include <list>
#include <map>
#include <unordered_map>

struct ID2D1Factory;
struct ID2D1HwndRenderTarget;
struct ID2D1RectangleGeometry;
struct ID2D1SolidColorBrush;
struct ID2D1Geometry;
struct ID2D1Brush;
struct IDWriteFactory;
struct IDWriteTextFormat;
struct ID2D1Bitmap;

namespace tod
{
	struct SColourHasher 
	{
		size_t operator()(const SColour& col) const 
		{

			return ((((  std::hash<float>()(col.r)
					^ (std::hash<float>()(col.g) << 1)) >> 1)
				    ^ (std::hash<float>()(col.b) << 1)) >> 1)
					^ (std::hash<float>()(col.a) << 1);
		}
	};


	class CD2D1Renderer : public I2DRenderer
	{
	private:
		ptr m_hwnd;

		ID2D1Factory* m_pDirect2dFactory;
		ID2D1HwndRenderTarget* m_pRenderTarget;
		ID2D1RectangleGeometry* m_pD2D1Rect;
		ID2D1SolidColorBrush* m_pBlackBrush;
		ID2D1SolidColorBrush* m_pMidGrayBrush;

		std::map<rhandle,ID2D1Geometry*> m_mGeometry;
		uint32 m_ui32NextGeometryID;
		std::map<rhandle,ID2D1Brush*> m_mBrushes;
		std::unordered_map<SColour, rhandle, SColourHasher> m_mBrushCache;
		uint32 m_ui32NextBrushID;

		std::map<rhandle, ID2D1Bitmap*> m_mBitmaps;
		uint32 m_uiNextBitmapId;

		IDWriteFactory*	   m_pDWriteFactory;
		IDWriteTextFormat* m_pTextFormat;

		uint32 m_width;
		uint32 m_height;

	public:
		CD2D1Renderer( );
		~CD2D1Renderer( );

		// IUIRenderer
		virtual bool Initialise( ptr hwnd, uint32 width, uint32 height );
		virtual void Deinitialise( );

		virtual void Update( uint32 dt );
		virtual void BeginDraw( );
		virtual void EndDraw( );

		virtual rhandle CreateRectangleGeometry( SRect rect );
		virtual rhandle CreateGeometry( const float2* vertices, uint32 vertexCount, SRect* rect = 0 );
		virtual rhandle CreateFillGeometry( const float2* vertices, uint32 vertexCount, SRect* rect = 0 );
		virtual rhandle CreateEllipseGeometry( const float2& pos, const float2& size );

		virtual rhandle CreateBrush( const SColour& colour );
		virtual rhandle CreateBrushCached(const SColour& colour);
		
		virtual rhandle CreateImage(uint16 width, uint16 height, uint8* data);
		virtual void    UpdateImage(rhandle img, uint16 width, uint16 height, uint8* data);

		virtual void DrawGeometry( rhandle hGeometry, rhandle hBrush );
		virtual void DrawFillGeometry( rhandle hGeometry, rhandle hBrush );
		virtual void DrawFillGeometry( rhandle hGeometry, rhandle hBrush, float2 translation );
		virtual void DrawTextString(wstr string, SRect rect);

		virtual void DrawBitmap( rhandle hBitmap, float2 pos, float2 scale );
		virtual void DrawBitmap( rhandle hBitmap, float2 pos, float2 scale, float opacity );

		virtual void DrawRectangle( rhandle hBrush, SRect rect, float2 offset = float2(0.0f, 0.0f), float2 scale = float2(1.0f, 1.0f), float stroke = 0.0f );

		virtual void DestroyResource(rhandle hResource);
	private:
		bool CreateDeviceIndependentResources();
		bool CreateDeviceResources();
		void DestroyDeviceResources();
	};
}
