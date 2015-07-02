#include "stdafx.h"
#include "D2D1Renderer.h"

#include <d2d1_1.h>
#include <dwrite.h>

#include "HRTools.h"

#include <array>

namespace tod
{

CD2D1Renderer::CD2D1Renderer( ) : 
	I2DRenderer( ), 
	m_pDirect2dFactory(nullptr),
	m_pRenderTarget(nullptr),
	m_pD2D1Rect(nullptr),
	m_hwnd((ptr)nullptr),
	m_ui32NextGeometryID(0),
	m_ui32NextBrushID(0),
	m_uiNextBitmapId(0)
{
	m_eType = e2DRenderer_Direct2D;
}

CD2D1Renderer::~CD2D1Renderer( )
{
	DestroyDeviceResources();
}

// IUIRenderer
bool CD2D1Renderer::Initialise( ptr hwnd, uint32 width, uint32 height )
{
	ComponentLogFunc( );

	m_hwnd = hwnd;

	m_width = width;
	m_height = height;

	if( !CreateDeviceIndependentResources() )
	{
		Logger.Error(_T("Initialisation failed"));
		return false;
	}

	if( !CreateDeviceResources() )
	{
		Logger.Error(_T("Initialisation failed"));
		return false;
	}

	return true;
}

void CD2D1Renderer::Deinitialise( )
{
	ComponentLogFunc( );

}

void CD2D1Renderer::Update( uint32 dt )
{
	ComponentLogFunc( );
	CreateDeviceResources();
	m_pRenderTarget->BeginDraw();
	m_pRenderTarget->Clear(D2D1::ColorF(D2D1::ColorF::White));
	m_pRenderTarget->SetTransform(D2D1::Matrix3x2F::Translation(256.0f-25.0f,256.0f-25.0f));
	m_pRenderTarget->FillGeometry(m_pD2D1Rect, m_pBlackBrush);
	m_pRenderTarget->DrawGeometry(m_pD2D1Rect, m_pMidGrayBrush, 2.0f);
	HRESULT hr = m_pRenderTarget->EndDraw();
#ifdef _DEBUG
	CHECK_HR_ONFAIL_LOG(hr,_T("Failed to draw render target"));
#endif
	if (hr == D2DERR_RECREATE_TARGET)
	{
		hr = S_OK;
		DestroyDeviceResources();
	}
}

void CD2D1Renderer::BeginDraw( )
{
	//ComponentLogFunc( );
	CreateDeviceResources();
	m_pRenderTarget->BeginDraw();
	m_pRenderTarget->Clear(D2D1::ColorF(D2D1::ColorF::White));
	m_pRenderTarget->SetTransform( D2D1::Matrix3x2F::Identity() );
}

void CD2D1Renderer::EndDraw( )
{
	ComponentLogFunc( );
	HRESULT hr =  m_pRenderTarget->EndDraw();
	CHECK_HR_ONFAIL_LOG( hr, _T("Failed to draw correctly") );
}

rhandle CD2D1Renderer::CreateRectangleGeometry( SRect rect )
{
	ComponentLogFunc( );
	rhandle handle = nullrhandle;
	
	ID2D1RectangleGeometry* pNewRect = nullptr;
	D2D1_RECT_F d2drect = D2D1::RectF(rect.x, rect.y, rect.x+rect.w, rect.y+rect.h);
	HRESULT hr = m_pDirect2dFactory->CreateRectangleGeometry( d2drect, &pNewRect );

	CHECK_HR_ONFAIL_LOG(hr, _T("Failed to create Direct2D rectangle geometry"));
	if( SUCCEEDED(hr) )
	{
		handle = MAKE_RHANDLE( m_ui32NextGeometryID++, eResourceType_RectangleGeometry );
		m_mGeometry.insert( std::make_pair(handle, pNewRect) );
	}

	return handle;
}

rhandle CD2D1Renderer::CreateFillGeometry( const float2* vertices, uint32 vertexCount, SRect* rect )
{
	ComponentLogFunc( );
	rhandle handle = nullrhandle;

	ID2D1PathGeometry* pNewGeo = nullptr;
	
	HRESULT hr = m_pDirect2dFactory->CreatePathGeometry( &pNewGeo );
	CHECK_HR_ONFAIL_LOG(hr, _T("Failed to create Direct2D path geometry"));
	if( SUCCEEDED(hr) )
	{
		ID2D1GeometrySink *pSink = NULL;
		hr = pNewGeo->Open(&pSink);
		CHECK_HR_ONFAIL_LOG_RETURN_VAL( hr, _T("Failed to create Direct2D path geometry"), nullrhandle);
		pSink->SetFillMode(D2D1_FILL_MODE_WINDING);
		pSink->BeginFigure( D2D1::Point2F( vertices[0].x, vertices[0].y ), D2D1_FIGURE_BEGIN_FILLED );
		for(uint32 i = 0; i < vertexCount; ++i)
		{
			pSink->AddLine(D2D1::Point2F( vertices[i].x, vertices[i].y ));
		}
		pSink->EndFigure(D2D1_FIGURE_END_CLOSED);
		hr = pSink->Close();
		CHECK_HR_ONFAIL_LOG_RETURN_VAL( hr, _T("Failed to create Direct2D path geometry"), nullrhandle);
		handle = MAKE_RHANDLE( m_ui32NextGeometryID++, eResourceType_ShapeGeometry );
		m_mGeometry.insert( std::make_pair(handle, pNewGeo) );
	}
	
	return handle;
}

rhandle CD2D1Renderer::CreateGeometry( const float2* vertices, uint32 vertexCount, SRect* rect )
{
	ComponentLogFunc( );
	rhandle handle = nullrhandle;

	ID2D1PathGeometry* pNewGeo = nullptr;

	HRESULT hr = m_pDirect2dFactory->CreatePathGeometry( &pNewGeo );
	CHECK_HR_ONFAIL_LOG(hr, _T("Failed to create Direct2D path geometry"));
	if( SUCCEEDED(hr) )
	{
		ID2D1GeometrySink *pSink = NULL;
		hr = pNewGeo->Open(&pSink);
		CHECK_HR_ONFAIL_LOG_RETURN_VAL( hr, _T("Failed to create Direct2D path geometry"), nullrhandle);
		pSink->SetFillMode(D2D1_FILL_MODE_WINDING);
		pSink->BeginFigure( D2D1::Point2F( vertices[vertexCount-1].x, vertices[vertexCount-1].y ), D2D1_FIGURE_BEGIN_HOLLOW );
		for(uint32 i = 0; i < vertexCount; ++i)
		{
			pSink->AddLine(D2D1::Point2F( vertices[i].x, vertices[i].y ));
		}
		pSink->EndFigure(D2D1_FIGURE_END_OPEN);
		hr = pSink->Close();
		CHECK_HR_ONFAIL_LOG_RETURN_VAL( hr, _T("Failed to create Direct2D path geometry"), nullrhandle);
		handle = MAKE_RHANDLE( m_ui32NextGeometryID++, eResourceType_PathGeometry );
		m_mGeometry.insert( std::make_pair(handle, pNewGeo) );
	}
	
	return handle;
}

rhandle CD2D1Renderer::CreateEllipseGeometry( const float2& pos, const float2& size )
{
	ComponentLogFunc( );
	rhandle handle = nullrhandle;
	
	ID2D1EllipseGeometry* pNewRect = nullptr;
	HRESULT hr = m_pDirect2dFactory->CreateEllipseGeometry( D2D1::Ellipse( D2D1::Point2F(pos.x,pos.y), size.x, size.y ), &pNewRect );
	CHECK_HR_ONFAIL_LOG(hr, _T("Failed to create Direct2D rectangle geometry"));
	if( SUCCEEDED(hr) )
	{
		handle = MAKE_RHANDLE( m_ui32NextGeometryID++, eResourceType_EllipseGeometry );
		m_mGeometry.insert( std::make_pair(handle, pNewRect) );
	}

	return handle;
}

rhandle CD2D1Renderer::CreateBrush( const SColour& colour )
{
	return CreateBrushCached(colour);
}

rhandle CD2D1Renderer::CreateBrushCached(const SColour& colour)
{
	ComponentLogFunc();
	auto brushFindResult = m_mBrushCache.find(colour);
	if (brushFindResult != m_mBrushCache.end())
	{
		return brushFindResult->second;
	}
	
	Logger.Warning(_T("Brush cache miss for R %f G %f B %f A %f"), colour.r, colour.g, colour.b, colour.a);

	rhandle handle = nullrhandle;
	ID2D1SolidColorBrush* pBrush = nullptr;
	HRESULT hr = m_pRenderTarget->CreateSolidColorBrush(D2D1::ColorF(colour.r, colour.g, colour.b, colour.a), &pBrush);
	CHECK_HR_ONFAIL_LOG(hr, _T("Failed to create Direct2D solid colour brush"));
	if (SUCCEEDED(hr))
	{
		handle = MAKE_RHANDLE(m_ui32NextBrushID++, eResourceType_SolidBrush);
		m_mBrushes.insert(std::make_pair(handle, pBrush));
		m_mBrushCache.insert(std::make_pair(colour, handle));
	}

	return handle;
}

rhandle CD2D1Renderer::CreateImage(uint16 width, uint16 height, uint8* data)
{
	ComponentLogFunc();
	rhandle handle = nullrhandle;

	D2D_SIZE_U bmSize = D2D1::SizeU(width, height);
	D2D1_BITMAP_PROPERTIES bmProps;
	bmProps.dpiX = 96.0f;
	bmProps.dpiY = 96.0f;
	bmProps.pixelFormat.alphaMode = D2D1_ALPHA_MODE_PREMULTIPLIED;
	bmProps.pixelFormat.format	  = DXGI_FORMAT_R8G8B8A8_UNORM;

	ID2D1Bitmap* pNewBmp = NULL;
	HRESULT hr = m_pRenderTarget->CreateBitmap( bmSize, bmProps, &pNewBmp );
	CHECK_HR_ONFAIL_LOG(hr, _T("Failed to create Direct2D bitmap image"));

	if (SUCCEEDED(hr))
	{
		handle = MAKE_RHANDLE( m_uiNextBitmapId++, eResourceType_Bitmap );
		m_mBitmaps.insert( std::make_pair(handle, pNewBmp) );
		if (data != NULL)
		{
			D2D_RECT_U bmRect = D2D1::RectU(0, 0, width, height);
			CHECK_HR_ONFAIL_LOG( pNewBmp->CopyFromMemory(&bmRect, data, width*4), 
								 _T("Failed to initialise Direct2D bitmap image"));
		}
	}

	return handle;
}

void CD2D1Renderer::UpdateImage(rhandle img, uint16 width, uint16 height, uint8* data)
{
	auto pBmp  = m_mBitmaps[img];
	if (data != nullptr)
	{
		D2D_RECT_U bmRect = D2D1::RectU(0, 0, width, height);
		CHECK_HR_ONFAIL_LOG(pBmp->CopyFromMemory(&bmRect, data, width * 4),
							_T("Failed to initialise Direct2D bitmap image"));
	}
}


void CD2D1Renderer::DrawGeometry( rhandle hGeometry, rhandle hBrush )
{
	auto geoIterFound = m_mGeometry.find( hGeometry );
	auto bruIterFound = m_mBrushes.find( hBrush );
	assert( geoIterFound != m_mGeometry.end() );
	assert( bruIterFound != m_mBrushes.end() );
	m_pRenderTarget->DrawGeometry( (*geoIterFound).second, (*bruIterFound).second );
}

void CD2D1Renderer::DrawFillGeometry( rhandle hGeometry, rhandle hBrush )
{
	auto geoIterFound = m_mGeometry.find( hGeometry );
	auto bruIterFound = m_mBrushes.find( hBrush );
	assert( geoIterFound != m_mGeometry.end() );
	assert( bruIterFound != m_mBrushes.end() );
	m_pRenderTarget->FillGeometry( (*geoIterFound).second, (*bruIterFound).second );
}

void CD2D1Renderer::DrawFillGeometry( rhandle hGeometry, rhandle hBrush, float2 translation )
{
	D2D1::Matrix3x2F trans = D2D1::Matrix3x2F::Translation( translation.x, translation.y );
	m_pRenderTarget->SetTransform(trans);
	DrawFillGeometry( hGeometry, hBrush );
	m_pRenderTarget->SetTransform(D2D1::Matrix3x2F::Identity());
}

void CD2D1Renderer::DrawTextString( wstr string, SRect rect )
{
	D2D1_RECT_F d2d1Rect = D2D1::RectF(rect.x, rect.y, rect.x+rect.w, rect.y+rect.h);
	m_pRenderTarget->DrawTextW(string, lstrlenW(string), m_pTextFormat, d2d1Rect, m_pBlackBrush);
}

void CD2D1Renderer::DrawBitmap(rhandle hBitmap, float2 pos, float2 scale)
{
	DrawBitmap(hBitmap, pos, scale, 1.0f);
}

void CD2D1Renderer::DrawBitmap(rhandle hBitmap, float2 pos, float2 scale, float opacity)
{
	auto bmpIterFound = m_mBitmaps.find(hBitmap);
	assert(bmpIterFound != m_mBitmaps.end());
	auto size = bmpIterFound->second->GetSize();
	D2D1_RECT_F rect = D2D1::RectF(pos.x, pos.y, pos.x + size.width*scale.x, pos.y + size.height*scale.y);
	D2D1_RECT_F srcRect = D2D1::RectF(0.0f, 0.0f, size.width, size.height);
	m_pRenderTarget->DrawBitmap(bmpIterFound->second, rect, opacity, D2D1_BITMAP_INTERPOLATION_MODE_LINEAR, srcRect);
}

void CD2D1Renderer::DrawRectangle(rhandle hBrush, SRect rect, float2 offset, float2 scale, float stroke)
{
	auto brushFound = m_mBrushes.find(hBrush);
	assert(brushFound != m_mBrushes.end());
	auto size = m_pRenderTarget->GetSize();
	float right = size.width - rect.x - rect.w*scale.x;
	float bottom = size.height - rect.y - rect.h*scale.y;
	//D2D1_RECT_F d2dRect = D2D1::RectF(rect.x + offset.x, rect.y + offset.y, right, bottom);
	D2D1_RECT_F d2dRect = D2D1::RectF(rect.x, rect.y, rect.x+rect.w, rect.y+rect.h);
	m_pRenderTarget->FillRectangle(d2dRect, brushFound->second);
}

// Private
bool CD2D1Renderer::CreateDeviceIndependentResources()
{
	ComponentLogFunc( );

	HRESULT hr = S_OK;

	// Create Direct2D factory.
	hr = D2D1CreateFactory(D2D1_FACTORY_TYPE_SINGLE_THREADED, &m_pDirect2dFactory);
	CHECK_HR_ONFAIL_LOG_RETURN( hr, _T("Failed to create Direct2D factory") );

    hr = DWriteCreateFactory(
			DWRITE_FACTORY_TYPE_SHARED,
			__uuidof(IDWriteFactory),
			reinterpret_cast<IUnknown**>(&m_pDWriteFactory)
         );
	CHECK_HR_ONFAIL_LOG_RETURN( hr, _T("Failed to create DirectWrite factory") );

	// TODO: these need to be created via interface and stored (i.e. list of)
    hr = m_pDWriteFactory->CreateTextFormat( L"Tahoma", // Font family name.
											 NULL,      // Font collection (NULL sets it to use the system font collection).
										     DWRITE_FONT_WEIGHT_REGULAR,
										     DWRITE_FONT_STYLE_NORMAL,
										     DWRITE_FONT_STRETCH_NORMAL,
										     14.0f,
										     L"en-us",
										     &m_pTextFormat
										     );
	CHECK_HR_ONFAIL_LOG_RETURN( hr, _T("Failed to create Text format") );
    hr = m_pTextFormat->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_CENTER);
	CHECK_HR_ONFAIL_LOG_RETURN( hr, _T("Failed to set Text format to align center") );
    hr = m_pTextFormat->SetParagraphAlignment(DWRITE_PARAGRAPH_ALIGNMENT_CENTER);
	CHECK_HR_ONFAIL_LOG_RETURN( hr, _T("Failed to set Text format to paragraph align center") );

	return true;
}

bool CD2D1Renderer::CreateDeviceResources( )
{
	ComponentLogFunc( );

	HRESULT hr = S_OK;
	if(!m_pRenderTarget)
	{
		// Create an HwndRenderTarget to draw to the hwnd.
		RECT rc;
		GetClientRect((HWND)m_hwnd, &rc);
		D2D1_SIZE_U size = D2D1::SizeU(rc.right - rc.left, rc.bottom - rc.top);
		hr = m_pDirect2dFactory->CreateHwndRenderTarget(
			D2D1::RenderTargetProperties(),
			D2D1::HwndRenderTargetProperties((HWND)m_hwnd, size),
			&m_pRenderTarget
			);

		
		//m_pDirect2dFactory->CreateWicBitmapRenderTarget()

		CHECK_HR_ONFAIL_LOG_RETURN(hr, _T("Failed to create Direct2D render target"));

		D2D1_RECT_F rect = D2D1::RectF(0.0f,0.0f,50.0f,50.0f);
		hr = m_pDirect2dFactory->CreateRectangleGeometry( rect, &m_pD2D1Rect );

		CHECK_HR_ONFAIL_LOG_RETURN(hr, _T("Failed to create Direct2D rectangle geometry"));

		hr = m_pRenderTarget->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::Black), &m_pBlackBrush);

		CHECK_HR_ONFAIL_LOG_RETURN(hr, _T("Failed to create Direct2D black brush"));

		hr = m_pRenderTarget->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::LightSteelBlue), &m_pMidGrayBrush);
	}

	return true;
}

void CD2D1Renderer::DestroyDeviceResources()
{
	//std::for_each(m_mBitmaps.begin(), m_mBitmaps.end(), [](std::pair<rhandle, IUnknownPtr> kv) { D2D_RELEASE(kv.second); });
	D2D_RELEASE_ALL_MAP(m_mBitmaps);
	m_mBrushCache.clear();
	D2D_RELEASE_ALL_MAP(m_mBrushes);
	D2D_RELEASE_ALL_MAP(m_mGeometry);

	D2D_RELEASE(m_pD2D1Rect);
	D2D_RELEASE(m_pBlackBrush);
	D2D_RELEASE(m_pMidGrayBrush);

	D2D_RELEASE(m_pTextFormat);
	D2D_RELEASE(m_pDWriteFactory);

	D2D_RELEASE(m_pRenderTarget);
	D2D_RELEASE(m_pDirect2dFactory);
}

void CD2D1Renderer::DestroyResource(rhandle hResource)
{
	EResourceType resourceType = (EResourceType)GET_RHANDLE_TYPE(hResource);
	switch (resourceType)
	{
	case eResourceType_EllipseGeometry:
	case eResourceType_RectangleGeometry:
	case eResourceType_PathGeometry:
	case eResourceType_ShapeGeometry:
	{
		auto geoFind = m_mGeometry.find(hResource);
		D2D_RELEASE(geoFind->second);
		m_mGeometry.erase(geoFind);
		break;
	}
	case eResourceType_SolidBrush:
	{
		// Remove from brushes cache
		auto brushReverseSearch = [hResource](std::pair<SColour, rhandle> kv) -> bool { return kv.second == hResource; };
		auto cacheResult = std::find_if(m_mBrushCache.begin(), m_mBrushCache.end(), brushReverseSearch);
		if (cacheResult != m_mBrushCache.end())
			m_mBrushCache.erase(cacheResult);

		// Remove and free from main brushes list
		auto brushFind = m_mBrushes.find(hResource);
		D2D_RELEASE(brushFind->second);
		m_mBrushes.erase(brushFind);

		break;
	}
	case eResourceType_TextFormat:
	{
		// TODO: When we create text resources
		break;
	}
	case eResourceType_Bitmap:
	{
		auto bmpFind = m_mBitmaps.find(hResource);
		bmpFind->second->Release();
		m_mBitmaps.erase(bmpFind);
		break;
	}
	default:
		break;
	}
}

}