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
struct IWICImagingFactory;
struct IWICBitmap;
struct ID2D1BitmapRenderTarget;
struct ID2D1RenderTarget;
struct IWICBitmapEncoder;
struct IWICStream;

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
		
		ID2D1RenderTarget* m_pRenderTarget;
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

		IWICImagingFactory*		 m_pWicFactory;
		IWICBitmap*				 m_pWicBitmap;

		ID2D1HwndRenderTarget* m_pHwndRenderTarget;
		ID2D1RenderTarget* m_pBmpRenderTarget;

		std::map<rhandle, ID2D1RenderTarget*>  m_mRenderTargets;
		std::map<rhandle, IWICBitmap*> m_mRenderTargetWicBmps;
		uint32 m_uiNextRenderTargetID;

		uint32 m_width;
		uint32 m_height;

		SColour m_clearColor;

		IWICBitmapEncoder* m_pBmpEncoder;
		IWICStream* m_pImageStream;

	public:
		CD2D1Renderer( );
		~CD2D1Renderer( );

		// IUIRenderer
		virtual bool Initialise( ptr hwnd, uint32 width, uint32 height );
		virtual void Deinitialise( );

		virtual void SetClearColor(SColour colour);

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
		virtual void	UpdateGeometry(rhandle geo, const float2* vertices, uint32 vertexCount);

		virtual rhandle CreateRenderTarget(bool present = true);
		virtual void	SetRenderTarget(rhandle renderTarget);
		virtual rhandle CreateImageFromRenderTarget(rhandle hRenderTarget);

		virtual void DrawGeometry( rhandle hGeometry, rhandle hBrush );
		virtual void DrawGeometry(rhandle hGeometry, rhandle hBrush, float2 translation);
		virtual void DrawFillGeometry( rhandle hGeometry, rhandle hBrush );
		virtual void DrawFillGeometry( rhandle hGeometry, rhandle hBrush, float2 translation );
		virtual void DrawFillGeometry(rhandle hGeometry, rhandle hBrush, float2 translation, float2 scale);
		virtual void DrawFillGeometry(rhandle hGeometry, rhandle hBrush, float2 translation, float2 scale, float rotAngle,
								  	  float2 rotCenter = float2(0.0f, 0.0f));
		virtual void DrawTextString(wstr string, SRect rect, rhandle hBrush);

		virtual void DrawBitmap( rhandle hBitmap, float2 pos, float2 scale );
		virtual void DrawBitmap( rhandle hBitmap, float2 pos, float2 scale, float opacity );

		virtual void DrawRectangle( rhandle hBrush, SRect rect, float2 offset = float2(0.0f, 0.0f), float2 scale = float2(1.0f, 1.0f), float stroke = 0.0f );

		virtual bool SavePngImage(rhandle hRenderTarget, tstr path);

		virtual void DestroyResource(rhandle hResource);
	private:
		bool CreateDeviceIndependentResources();
		bool CreateDeviceResources();
		void DestroyDeviceResources();
	};
}
