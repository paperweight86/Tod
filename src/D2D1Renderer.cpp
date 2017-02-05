#include "stdafx.h"
#include "D2D1Renderer.h"

#include <d2d1_1.h>
#include <dwrite.h>
#include <wincodec.h>

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
	m_uiNextBitmapId(0),
	m_uiNextRenderTargetID(0),
	m_pBmpEncoder(nullptr),
	m_pImageStream(nullptr)
{
	m_eType = e2DRenderer_Direct2D;
	CreateColourFromRGB(m_clearColor, 0x00000000);
}

CD2D1Renderer::~CD2D1Renderer( )
{
	DestroyDeviceResources();
}

// IUIRenderer
bool CD2D1Renderer::Initialise( ptr hwnd, uint32 width, uint32 height )
{
	m_hwnd = hwnd;

	m_width = width;
	m_height = height;

	if( !CreateDeviceIndependentResources() )
	{
		log::err_out("Initialisation failed");
		return false;
	}

	if( !CreateDeviceResources() )
	{
		log::err_out("Initialisation failed");
		return false;
	}

	return true;
}

void CD2D1Renderer::Deinitialise( )
{
}

void CD2D1Renderer::Update( uint32 dt )
{
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

void CD2D1Renderer::SetClearColor(SColour colour)
{
	m_clearColor = colour;
}

void CD2D1Renderer::BeginDraw()
{
	//ComponentLogFunc( );
	CreateDeviceResources();
	m_pRenderTarget->BeginDraw();
	m_pRenderTarget->Clear(D2D1::ColorF(m_clearColor.r, m_clearColor.g, m_clearColor.b, m_clearColor.a));
	m_pRenderTarget->SetTransform( D2D1::Matrix3x2F::Identity() );
}

void CD2D1Renderer::EndDraw()
{
	HRESULT hr = m_pRenderTarget->EndDraw();
	CHECK_HR_ONFAIL_LOG(hr, _T("Failed to draw correctly"));
}

rhandle CD2D1Renderer::CreateRectangleGeometry( SRect rect )
{
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
		D2D_RELEASE(pSink);
		handle = MAKE_RHANDLE( m_ui32NextGeometryID++, eResourceType_ShapeGeometry );
		m_mGeometry.insert( std::make_pair(handle, pNewGeo) );
	}
	
	return handle;
}

rhandle CD2D1Renderer::CreateGeometry( const float2* vertices, uint32 vertexCount, SRect* rect )
{
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
		D2D_RELEASE(pSink);
		CHECK_HR_ONFAIL_LOG_RETURN_VAL( hr, _T("Failed to create Direct2D path geometry"), nullrhandle);
		handle = MAKE_RHANDLE( m_ui32NextGeometryID++, eResourceType_PathGeometry );
		m_mGeometry.insert( std::make_pair(handle, pNewGeo) );
	}
	
	return handle;
}

rhandle CD2D1Renderer::CreateEllipseGeometry( const float2& pos, const float2& size )
{
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
	auto brushFindResult = m_mBrushCache.find(colour);
	if (brushFindResult != m_mBrushCache.end())
	{
		return brushFindResult->second;
	}
	
	log::wrn_out("Brush cache miss for R %f G %f B %f A %f", colour.r, colour.g, colour.b, colour.a);

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

rhandle CD2D1Renderer::CreateImageFromRenderTarget(rhandle hRenderTarget)
{
	auto foundRt = m_mRenderTargets.find(hRenderTarget);
	if (foundRt == m_mRenderTargets.end())
	{
		log::err_out("Unable to create image from render target - no such render target 0x%016llx", hRenderTarget);
		return nullrhandle;
	}

	auto foundWic = m_mRenderTargetWicBmps.find(hRenderTarget);
	if (foundWic == m_mRenderTargetWicBmps.end())
	{
		log::err_out(_T("Unable to create image from render target - render target 0x%016llx has no associated bitmap"), hRenderTarget);
		return nullrhandle;
	}

	rhandle handle = MAKE_RHANDLE(uint32_max, eResourceType_Bitmap);

	auto foundBmp = m_mBitmaps.find(handle);
	if (foundBmp != m_mBitmaps.end())
	{
		foundBmp->second->Release();
		m_mBitmaps.erase(foundBmp);
	}

	ID2D1Bitmap* pBmp = nullptr;
	m_pRenderTarget->CreateBitmapFromWicBitmap(foundWic->second, &pBmp);
	m_mBitmaps.insert(std::make_pair(handle, pBmp));

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

void CD2D1Renderer::UpdateGeometry(rhandle geo, const float2* vertices, uint32 vertexCount)
{
	auto found = m_mGeometry.find(geo);
	if (found == m_mGeometry.end())
	{
		log::err_out(_T("Unknown geometry 0x%016llx"), geo);
		return;
	}

	// Direct2D geometry is fucking stupid and totally immuatable so time to delete and alloc up some new geometry
	// cos thats fucking optimal...
	D2D_RELEASE(found->second);

	rhandle handle = found->first;
	m_mGeometry.erase(found);

	ID2D1PathGeometry* pNewGeo = nullptr;

	HRESULT hr = m_pDirect2dFactory->CreatePathGeometry(&pNewGeo);
	CHECK_HR_ONFAIL_LOG(hr, _T("Failed to create Direct2D path geometry"));
	if (SUCCEEDED(hr))
	{
		ID2D1GeometrySink *pSink = NULL;
		hr = pNewGeo->Open(&pSink);
		CHECK_HR_ONFAIL_LOG_RETURN_VOID(hr, _T("Failed to create Direct2D path geometry"), nullrhandle);
		pSink->SetFillMode(D2D1_FILL_MODE_WINDING);
		pSink->BeginFigure(D2D1::Point2F(vertices[0].x, vertices[0].y), D2D1_FIGURE_BEGIN_FILLED);
		for (uint32 i = 0; i < vertexCount; ++i)
		{
			pSink->AddLine(D2D1::Point2F(vertices[i].x, vertices[i].y));
		}
		pSink->EndFigure(D2D1_FIGURE_END_CLOSED);
		hr = pSink->Close();
		CHECK_HR_ONFAIL_LOG_RETURN_VOID(hr, _T("Failed to create Direct2D path geometry"), nullrhandle);
		D2D_RELEASE(pSink);
		m_mGeometry.insert(std::make_pair(handle, pNewGeo));
	}
}

rhandle CD2D1Renderer::CreateRenderTarget(bool present)
{
	rhandle resource = nullrhandle;
	HRESULT hr = 0;
	resource = MAKE_RHANDLE(m_uiNextRenderTargetID++, eResourceType_RenderTarget);
	if (present)
	{
		// Create an HwndRenderTarget to draw to the hwnd.
		RECT rc;
		GetClientRect((HWND)m_hwnd, &rc);
		ID2D1HwndRenderTarget* pHwndRt = nullptr;
		D2D1_SIZE_U size = D2D1::SizeU(rc.right - rc.left, rc.bottom - rc.top);
		hr = m_pDirect2dFactory->CreateHwndRenderTarget(
			D2D1::RenderTargetProperties(),
			D2D1::HwndRenderTargetProperties((HWND)m_hwnd, size),
			&pHwndRt
			);

		CHECK_HR_ONFAIL_LOG_RETURN_VAL(hr, _T("Failed to create Direct2D render target"), nullrhandle);
		m_mRenderTargets.insert(std::make_pair(resource, static_cast<ID2D1RenderTarget*>(pHwndRt)));
	}
	else
	{
		IWICBitmap* pWicBmp = nullptr;
		hr = m_pWicFactory->CreateBitmap(m_width, m_height, GUID_WICPixelFormat32bppPBGRA, WICBitmapCacheOnLoad, &pWicBmp);

		CHECK_HR_ONFAIL_LOG_RETURN_VAL(hr, _T("Failed to create WIC bitmap render target"), nullrhandle);

		ID2D1RenderTarget* pRt = nullptr;
		auto rtprop = D2D1::RenderTargetProperties();
		hr = m_pDirect2dFactory->CreateWicBitmapRenderTarget(pWicBmp, &rtprop, &pRt);

		CHECK_HR_ONFAIL_LOG_RETURN_VAL(hr, _T("Failed to create Direct2d WIC bitmap render target"), nullrhandle);

		m_mRenderTargets.insert(std::make_pair(resource, pRt));
		m_mRenderTargetWicBmps.insert(std::make_pair(resource, pWicBmp));
	}

	return resource;
}

void CD2D1Renderer::SetRenderTarget(rhandle renderTarget)
{
	if (renderTarget == nullrhandle)
	{
		m_pRenderTarget = nullptr;
		return;
	}

	auto found = m_mRenderTargets.find(renderTarget);
	if (found != m_mRenderTargets.end())
	{
		m_pRenderTarget = found->second;
	}
	else
	{
		log::err_out("Unable to set render target to resource at rhandle 0x%016llx - no such render target", renderTarget);
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

void CD2D1Renderer::DrawGeometry(rhandle hGeometry, rhandle hBrush, float2 translation)
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

void CD2D1Renderer::DrawFillGeometry(rhandle hGeometry, rhandle hBrush, float2 translation, float2 scale)
{
	D2D1::Matrix3x2F trans = D2D1::Matrix3x2F::Translation(translation.x, translation.y);
	D2D1::Matrix3x2F scaleMat = D2D1::Matrix3x2F::Scale(scale.x, scale.y);
	m_pRenderTarget->SetTransform(scaleMat * trans);
	DrawFillGeometry(hGeometry, hBrush);
	m_pRenderTarget->SetTransform(D2D1::Matrix3x2F::Identity());
}

void CD2D1Renderer::DrawFillGeometry(rhandle hGeometry, rhandle hBrush, float2 translation, float2 scale, float rotAngle, float2 rotCenter)
{
	D2D1::Matrix3x2F trans = D2D1::Matrix3x2F::Translation(translation.x, translation.y);
	D2D1::Matrix3x2F scaleMat = D2D1::Matrix3x2F::Scale(scale.x, scale.y);
	D2D1::Matrix3x2F rot = D2D1::Matrix3x2F::Rotation(rotAngle, D2D1::Point2F(rotCenter.x, rotCenter.y));
	m_pRenderTarget->SetTransform(scaleMat * rot * trans);
	DrawFillGeometry(hGeometry, hBrush);
	m_pRenderTarget->SetTransform(D2D1::Matrix3x2F::Identity());
}


void CD2D1Renderer::DrawTextString(wstr string, SRect rect, rhandle hBrush)
{
	auto foundBrush = m_mBrushes.find(hBrush);
	if (foundBrush == m_mBrushes.end())
	{
		log::err_out("Unable to draw text string - no such brush 0x%016llx", hBrush);
		return;
	}
	D2D1_RECT_F d2d1Rect = D2D1::RectF(rect.x, rect.y, rect.x+rect.w, rect.y+rect.h);
	m_pRenderTarget->DrawText(string, lstrlenW(string), m_pTextFormat, d2d1Rect, foundBrush->second);
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

bool CD2D1Renderer::SavePngImage(rhandle hRenderTarget, wstr path)
{
	auto found = m_mRenderTargetWicBmps.find(hRenderTarget);
	if (found == m_mRenderTargetWicBmps.end())
	{
		log::err_out("Unknown render target 0x%016llx", hRenderTarget);
		return false;
	}
	HRESULT hr = m_pWicFactory->CreateStream(&m_pImageStream);
	CHECK_HR_ONFAIL_LOG_RETURN(hr, _T("Failed to create WIC Stream"));



	hr = m_pImageStream->InitializeFromFilename(path, GENERIC_WRITE);
	CHECK_HR_ONFAIL_LOG_RETURN(hr, _T("Failed to initialise Wic image stream"));
	
	IWICBitmapEncoder* pBmpEncoder = nullptr;
	hr = m_pWicFactory->CreateEncoder(GUID_ContainerFormatPng, NULL, &pBmpEncoder);
	CHECK_HR_ONFAIL_LOG_RETURN(hr, _T("Failed to create encoder for png when saving png"));

	hr = pBmpEncoder->Initialize(m_pImageStream, WICBitmapEncoderNoCache);
	CHECK_HR_ONFAIL_LOG_RETURN(hr, _T("Failed to initialise png encoder when saving png"));

	IWICBitmapFrameEncode* pFrameEncoder = nullptr;
	hr = pBmpEncoder->CreateNewFrame(&pFrameEncoder, NULL);
	CHECK_HR_ONFAIL_LOG_RETURN(hr, _T("Failed to create new encoder frame when saving png"));

	hr = pFrameEncoder->Initialize(NULL);
	CHECK_HR_ONFAIL_LOG_RETURN(hr, _T("Failed to initialise encoder frame when saving png"));

	hr = pFrameEncoder->SetSize(m_width, m_height);
	CHECK_HR_ONFAIL_LOG_RETURN(hr, _T("Failed to initialise encoder frame size when saving png"));

	auto pFmt = GUID_WICPixelFormat32bppPBGRA;
	hr = pFrameEncoder->SetPixelFormat(&pFmt);
	CHECK_HR_ONFAIL_LOG_RETURN(hr, _T("Failed to pixed format of encoder frame size when saving png"));

	hr = pFrameEncoder->WriteSource(found->second, NULL);
	CHECK_HR_ONFAIL_LOG_RETURN(hr, _T("Failed to write encoder frame when saving png"));

	hr = pFrameEncoder->Commit();
	CHECK_HR_ONFAIL_LOG_RETURN(hr, _T("Failed to commit encoder frame when saving png"));

	hr = pBmpEncoder->Commit();
	CHECK_HR_ONFAIL_LOG_RETURN(hr, _T("Failed to commit bmp encoder frame when saving png"));

	// Tidy up resources
	pFrameEncoder->Release();
	pBmpEncoder->Release();
	m_pImageStream->Release();

	return true;
}

// Private
bool CD2D1Renderer::CreateDeviceIndependentResources()
{
	HRESULT hr = S_OK;

	// Create Direct2D factory.
	hr = D2D1CreateFactory(D2D1_FACTORY_TYPE_SINGLE_THREADED, &m_pDirect2dFactory);
	CHECK_HR_ONFAIL_LOG_RETURN( hr, _T("Failed to create Direct2D factory") );

	CoInitialize(nullptr);

	hr = CoCreateInstance( CLSID_WICImagingFactory, nullptr,
						   CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&m_pWicFactory));

	CHECK_HR_ONFAIL_LOG_RETURN( hr, _T("Failed to create WIC Imaging Factory") );

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
	HRESULT hr = S_OK;
	if(!m_pRenderTarget)
	{
		D2D1_RECT_F rect = D2D1::RectF(0.0f,0.0f,50.0f,50.0f);
		hr = m_pDirect2dFactory->CreateRectangleGeometry( rect, &m_pD2D1Rect );

		CHECK_HR_ONFAIL_LOG_RETURN(hr, _T("Failed to create Direct2D rectangle geometry"));

		//hr = m_pRenderTarget->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::Black), &m_pBlackBrush);

		//CHECK_HR_ONFAIL_LOG_RETURN(hr, _T("Failed to create Direct2D black brush"));

		//hr = m_pRenderTarget->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::LightSteelBlue), &m_pMidGrayBrush);

		//CHECK_HR_ONFAIL_LOG_RETURN(hr, _T("Failed to create Direct2D light steel blue brush"));
	}

	return true;
}

void CD2D1Renderer::DestroyDeviceResources()
{
	D2D_RELEASE_ALL_MAP(m_mBitmaps);
	m_mBrushCache.clear();
	D2D_RELEASE_ALL_MAP(m_mBrushes);
	D2D_RELEASE_ALL_MAP(m_mGeometry);

	D2D_RELEASE(m_pD2D1Rect);
	D2D_RELEASE(m_pBlackBrush);
	D2D_RELEASE(m_pMidGrayBrush);

	D2D_RELEASE(m_pTextFormat);
	D2D_RELEASE(m_pDWriteFactory);

	D2D_RELEASE(m_pDirect2dFactory);

	D2D_RELEASE_ALL_MAP(m_mRenderTargets);
	D2D_RELEASE_ALL_MAP(m_mRenderTargetWicBmps);

	if(m_pWicFactory != nullptr)
		m_pWicFactory->Release();
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
		m_pRenderTarget->Flush();
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
	case eResourceType_RenderTarget:
	{
		auto rtFind = m_mRenderTargets.find( hResource );
		D2D_RELEASE(rtFind->second);
		m_mRenderTargets.erase(rtFind);
		break;
	}
	default:
		break;
	}
}

}