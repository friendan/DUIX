#include "StdAfx.h"
#include "D2dRenderDevice.h"
#include "UIRender.h"
#include "DuiExitTrace.h"

#include <d2d1.h>
#include <d2d1_1.h>
#include <d2d1helper.h>
#include <dwrite.h>
#include <d3d11.h>
#include <dxgi1_2.h>
#include <dcomp.h>
#include <dwmapi.h>
#include <map>

#ifndef WS_EX_NOREDIRECTIONBITMAP
#define WS_EX_NOREDIRECTIONBITMAP 0x00200000L
#endif
#ifndef DWMWA_FORCE_ICONIC_REPRESENTATION
#define DWMWA_FORCE_ICONIC_REPRESENTATION 7
#endif
#ifndef DWMWA_HAS_ICONIC_BITMAP
#define DWMWA_HAS_ICONIC_BITMAP 10
#endif
#ifndef DWMWA_DISALLOW_PEEK
#define DWMWA_DISALLOW_PEEK 11
#endif
#ifndef DWMWA_TRANSITIONS_FORCEDISABLED
#define DWMWA_TRANSITIONS_FORCEDISABLED 3
#endif
#ifndef DWMWA_FREEZE_REPRESENTATION
#define DWMWA_FREEZE_REPRESENTATION 15
#endif

#pragma comment(lib, "d2d1.lib")
#pragma comment(lib, "dwrite.lib")
#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "dcomp.lib")
#pragma comment(lib, "dwmapi.lib")

namespace DuiLib {

	namespace {

		const int kMaxD2dClips = 64;

		// 非分层 Present 走 GDI BitBlt：RT 用软件实现。
		// TYPE_DEFAULT 常落硬件 DXGI，任务栏悬停会整栏图标闪白。
		D2D1_RENDER_TARGET_PROPERTIES GdiCompatRtProps()
		{
			return D2D1::RenderTargetProperties(
				D2D1_RENDER_TARGET_TYPE_SOFTWARE,
				D2D1::PixelFormat(DXGI_FORMAT_B8G8R8A8_UNORM, D2D1_ALPHA_MODE_PREMULTIPLIED),
				0.0f, 0.0f,
				D2D1_RENDER_TARGET_USAGE_GDI_COMPATIBLE,
				D2D1_FEATURE_LEVEL_DEFAULT);
		}

		D2D1_COLOR_F ToColorF(DWORD color)
		{
			// DuiLib DWORD = CSS RRGGBBAA
			D2D1_COLOR_F c;
			c.r = DuiColorR(color) / 255.0f;
			c.g = DuiColorG(color) / 255.0f;
			c.b = DuiColorB(color) / 255.0f;
			c.a = DuiColorA(color) / 255.0f;
			if( c.a <= 0.0f && (color & 0xFFFFFF00u) != 0 ) c.a = 1.0f;
			return c;
		}

		D2D1_RECT_F ToRectF(const RECT& rc)
		{
			return D2D1::RectF((FLOAT)rc.left, (FLOAT)rc.top, (FLOAT)rc.right, (FLOAT)rc.bottom);
		}

		DWRITE_TEXT_ALIGNMENT ToDWriteAlign(UINT uStyle)
		{
			if( uStyle & DT_CENTER ) return DWRITE_TEXT_ALIGNMENT_CENTER;
			if( uStyle & DT_RIGHT ) return DWRITE_TEXT_ALIGNMENT_TRAILING;
			return DWRITE_TEXT_ALIGNMENT_LEADING;
		}

		DWRITE_PARAGRAPH_ALIGNMENT ToDWriteParagraphAlign(UINT uStyle)
		{
			if( uStyle & DT_VCENTER ) return DWRITE_PARAGRAPH_ALIGNMENT_CENTER;
			if( uStyle & DT_BOTTOM ) return DWRITE_PARAGRAPH_ALIGNMENT_FAR;
			return DWRITE_PARAGRAPH_ALIGNMENT_NEAR;
		}

		// HtmlText 内联占位（x 缩进 / 图片）
		class CDWriteSpacerInline : public IDWriteInlineObject
		{
		public:
			CDWriteSpacerInline(FLOAT width, FLOAT height)
				: m_cRef(1), m_width(width), m_height(height) {}
			virtual ~CDWriteSpacerInline() {}

			IFACEMETHODIMP_(ULONG) AddRef() { return InterlockedIncrement(&m_cRef); }
			IFACEMETHODIMP_(ULONG) Release()
			{
				ULONG c = InterlockedDecrement(&m_cRef);
				if( c == 0 ) delete this;
				return c;
			}
			IFACEMETHODIMP QueryInterface(REFIID riid, void** ppv)
			{
				if( ppv == NULL ) return E_POINTER;
				if( riid == __uuidof(IUnknown) || riid == __uuidof(IDWriteInlineObject) ) {
					*ppv = static_cast<IDWriteInlineObject*>(this);
					AddRef();
					return S_OK;
				}
				*ppv = NULL;
				return E_NOINTERFACE;
			}
			COM_DECLSPEC_NOTHROW STDMETHODIMP Draw(void* clientDrawingContext, IDWriteTextRenderer*, FLOAT originX, FLOAT originY,
				BOOL isSideways, BOOL isRightToLeft, IUnknown* clientDrawingEffect)
			{
				(void)clientDrawingContext; (void)originX; (void)originY;
				(void)isSideways; (void)isRightToLeft; (void)clientDrawingEffect;
				return S_OK;
			}
			COM_DECLSPEC_NOTHROW STDMETHODIMP GetMetrics(DWRITE_INLINE_OBJECT_METRICS* metrics)
			{
				if( metrics == NULL ) return E_POINTER;
				metrics->width = m_width;
				metrics->height = m_height;
				metrics->baseline = m_height;
				metrics->supportsSideways = FALSE;
				return S_OK;
			}
			COM_DECLSPEC_NOTHROW STDMETHODIMP GetOverhangMetrics(DWRITE_OVERHANG_METRICS* overhangs)
			{
				if( overhangs == NULL ) return E_POINTER;
				ZeroMemory(overhangs, sizeof(*overhangs));
				return S_OK;
			}
			COM_DECLSPEC_NOTHROW STDMETHODIMP GetBreakConditions(DWRITE_BREAK_CONDITION* breakConditionBefore, DWRITE_BREAK_CONDITION* breakConditionAfter)
			{
				if( breakConditionBefore ) *breakConditionBefore = DWRITE_BREAK_CONDITION_NEUTRAL;
				if( breakConditionAfter ) *breakConditionAfter = DWRITE_BREAK_CONDITION_NEUTRAL;
				return S_OK;
			}

		private:
			LONG m_cRef;
			FLOAT m_width;
			FLOAT m_height;
		};

		class CDWriteImageInline : public IDWriteInlineObject
		{
		public:
			CDWriteImageInline(ID2D1RenderTarget* pRT, ID2D1Bitmap* pBitmap, FLOAT width, FLOAT height, const D2D1_RECT_F& rcSrc)
				: m_cRef(1), m_pRT(pRT), m_pBitmap(pBitmap), m_width(width), m_height(height), m_rcSrc(rcSrc)
			{
				if( m_pBitmap ) m_pBitmap->AddRef();
			}
			virtual ~CDWriteImageInline()
			{
				if( m_pBitmap ) m_pBitmap->Release();
			}

			IFACEMETHODIMP_(ULONG) AddRef() { return InterlockedIncrement(&m_cRef); }
			IFACEMETHODIMP_(ULONG) Release()
			{
				ULONG c = InterlockedDecrement(&m_cRef);
				if( c == 0 ) delete this;
				return c;
			}
			IFACEMETHODIMP QueryInterface(REFIID riid, void** ppv)
			{
				if( ppv == NULL ) return E_POINTER;
				if( riid == __uuidof(IUnknown) || riid == __uuidof(IDWriteInlineObject) ) {
					*ppv = static_cast<IDWriteInlineObject*>(this);
					AddRef();
					return S_OK;
				}
				*ppv = NULL;
				return E_NOINTERFACE;
			}
			COM_DECLSPEC_NOTHROW STDMETHODIMP Draw(void* clientDrawingContext, IDWriteTextRenderer*, FLOAT originX, FLOAT originY,
				BOOL isSideways, BOOL isRightToLeft, IUnknown* clientDrawingEffect)
			{
				(void)isSideways; (void)isRightToLeft; (void)clientDrawingEffect;
				ID2D1RenderTarget* pRT = m_pRT;
				if( pRT == NULL ) pRT = reinterpret_cast<ID2D1RenderTarget*>(clientDrawingContext);
				if( pRT == NULL || m_pBitmap == NULL ) return S_OK;
				pRT->DrawBitmap(m_pBitmap,
					D2D1::RectF(originX, originY, originX + m_width, originY + m_height),
					1.0f, D2D1_BITMAP_INTERPOLATION_MODE_LINEAR, &m_rcSrc);
				return S_OK;
			}
			COM_DECLSPEC_NOTHROW STDMETHODIMP GetMetrics(DWRITE_INLINE_OBJECT_METRICS* metrics)
			{
				if( metrics == NULL ) return E_POINTER;
				metrics->width = m_width;
				metrics->height = m_height;
				metrics->baseline = m_height;
				metrics->supportsSideways = FALSE;
				return S_OK;
			}
			COM_DECLSPEC_NOTHROW STDMETHODIMP GetOverhangMetrics(DWRITE_OVERHANG_METRICS* overhangs)
			{
				if( overhangs == NULL ) return E_POINTER;
				ZeroMemory(overhangs, sizeof(*overhangs));
				return S_OK;
			}
			COM_DECLSPEC_NOTHROW STDMETHODIMP GetBreakConditions(DWRITE_BREAK_CONDITION* breakConditionBefore, DWRITE_BREAK_CONDITION* breakConditionAfter)
			{
				if( breakConditionBefore ) *breakConditionBefore = DWRITE_BREAK_CONDITION_NEUTRAL;
				if( breakConditionAfter ) *breakConditionAfter = DWRITE_BREAK_CONDITION_NEUTRAL;
				return S_OK;
			}

		private:
			LONG m_cRef;
			ID2D1RenderTarget* m_pRT;
			ID2D1Bitmap* m_pBitmap;
			FLOAT m_width;
			FLOAT m_height;
			D2D1_RECT_F m_rcSrc;
		};

	} // namespace

	/////////////////////////////////////////////////////////////////////////////////////
	//
	// CD2dRenderContext

	CD2dRenderContext::CD2dRenderContext(HDC hDC, CPaintManagerUI* pManager, ID2D1Factory* pFactory, IDWriteFactory* pDWrite)
		: m_gdiFallback(hDC, pManager)
		, m_pFactory(pFactory)
		, m_pDWrite(pDWrite)
		, m_pDCRT(NULL)
		, m_pRT(NULL)
		, m_pBoundSurface(NULL)
		, m_pBrush(NULL)
		, m_bInDraw(false)
		, m_bPixelsDirty(false)
		, m_pHwndInterop(NULL)
		, m_hHwndInteropDC(NULL)
		, m_nD2dClipDepth(0)
		, m_nBindWidth(0)
		, m_nBindHeight(0)
	{
		ZeroMemory(m_clipStack, sizeof(m_clipStack));
		ZeroMemory(m_d2dClipIsLayer, sizeof(m_d2dClipIsLayer));
		m_nClipCount = 0;
	}

	CD2dRenderContext::~CD2dRenderContext()
	{
		FlushToGdi();
		ReleaseHwndInteropDC();
		if( m_pBrush != NULL ) {
			m_pBrush->Release();
			m_pBrush = NULL;
		}
		if( m_pDCRT != NULL ) {
			CD2dRenderDevice* pDev = GetD2dRenderDevice();
			if( pDev != NULL ) pDev->InvalidateBitmapCacheForRT(m_pDCRT);
		}
		m_pRT = NULL;
		m_pBoundSurface = NULL;
		if( m_pDCRT != NULL ) {
			m_pDCRT->Release();
			m_pDCRT = NULL;
		}
	}

	ID2D1Bitmap* CD2dRenderContext::GetOrCreateBitmap(HBITMAP hBitmap, int nX, int nY, LPBYTE pBits, bool bAlpha)
	{
		CD2dRenderDevice* pDev = GetD2dRenderDevice();
		if( pDev == NULL ) return NULL;
		return pDev->GetOrCreateBitmap(m_pRT, hBitmap, nX, nY, pBits, bAlpha);
	}

	ID2D1Bitmap* CD2dRenderContext::GetOrCreateBitmapFromGdiplus(void* pGdiplusImage)
	{
		CD2dRenderDevice* pDev = GetD2dRenderDevice();
		if( pDev == NULL ) return NULL;
		return pDev->GetOrCreateBitmapFromGdiplus(m_pRT, pGdiplusImage);
	}

	void CD2dRenderContext::ClearBitmapCache()
	{
		// 缓存已提升到 Device，Context 不再持有
	}

	void CD2dRenderContext::DrawBitmapRect(ID2D1Bitmap* pBitmap, const RECT& rcDest, const RECT& rcSrc, float opacity)
	{
		if( pBitmap == NULL || m_pRT == NULL ) return;
		if( rcDest.right <= rcDest.left || rcDest.bottom <= rcDest.top ) return;
		if( rcSrc.right <= rcSrc.left || rcSrc.bottom <= rcSrc.top ) return;
		if( opacity <= 0.0f ) return;
		if( opacity > 1.0f ) opacity = 1.0f;
		// 1:1 blit (RichEdit GDI offscreen, native icons): NEAREST keeps ClearType sharp.
		// Scaled images still use LINEAR.
		const LONG destW = rcDest.right - rcDest.left;
		const LONG destH = rcDest.bottom - rcDest.top;
		const LONG srcW = rcSrc.right - rcSrc.left;
		const LONG srcH = rcSrc.bottom - rcSrc.top;
		const D2D1_BITMAP_INTERPOLATION_MODE mode =
			(destW == srcW && destH == srcH)
				? D2D1_BITMAP_INTERPOLATION_MODE_NEAREST_NEIGHBOR
				: D2D1_BITMAP_INTERPOLATION_MODE_LINEAR;
		m_pRT->DrawBitmap(
			pBitmap,
			ToRectF(rcDest),
			opacity,
			mode,
			ToRectF(rcSrc));
	}

	bool CD2dRenderContext::DrawImageD2d(ID2D1Bitmap* pBitmap, const RECT& rc, const RECT& rcPaint, const RECT& rcBmpPart, const RECT& rcCorners, UINT uFade, bool hole, bool xtiled, bool ytiled)
	{
		if( pBitmap == NULL ) return false;
		float opacity = (uFade >= 255) ? 1.0f : ((float)uFade / 255.0f);
		RECT rcDest = { 0 };
		RECT rcSrc = { 0 };

		auto BlitClipped = [&](const RECT& dest, int srcL, int srcT, int srcW, int srcH) {
			if( srcW <= 0 || srcH <= 0 ) return;
			RECT rcDraw = { 0 };
			if( !::IntersectRect(&rcDraw, &rcPaint, &dest) ) return;
			const int destW = dest.right - dest.left;
			const int destH = dest.bottom - dest.top;
			if( destW <= 0 || destH <= 0 ) return;
			// Prefer integer 1:1 mapping when buffer matches dest (avoids MulDiv
			// shrink/stretch that forces LINEAR upsample and blurs RichEdit text).
			if( srcW == destW && srcH == destH ) {
				rcSrc.left = srcL + (rcDraw.left - dest.left);
				rcSrc.top = srcT + (rcDraw.top - dest.top);
				rcSrc.right = srcL + (rcDraw.right - dest.left);
				rcSrc.bottom = srcT + (rcDraw.bottom - dest.top);
			}
			else {
				rcSrc.left = srcL + ::MulDiv(rcDraw.left - dest.left, srcW, destW);
				rcSrc.top = srcT + ::MulDiv(rcDraw.top - dest.top, srcH, destH);
				rcSrc.right = srcL + ::MulDiv(rcDraw.right - dest.left, srcW, destW);
				rcSrc.bottom = srcT + ::MulDiv(rcDraw.bottom - dest.top, srcH, destH);
			}
			if( rcSrc.right <= rcSrc.left || rcSrc.bottom <= rcSrc.top ) return;
			DrawBitmapRect(pBitmap, rcDraw, rcSrc, opacity);
		};

		// center
		if( !hole ) {
			rcDest.left = rc.left + rcCorners.left;
			rcDest.top = rc.top + rcCorners.top;
			rcDest.right = rc.right - rcCorners.right;
			rcDest.bottom = rc.bottom - rcCorners.bottom;
			int srcL = rcBmpPart.left + rcCorners.left;
			int srcT = rcBmpPart.top + rcCorners.top;
			int srcW = rcBmpPart.right - rcBmpPart.left - rcCorners.left - rcCorners.right;
			int srcH = rcBmpPart.bottom - rcBmpPart.top - rcCorners.top - rcCorners.bottom;
			if( srcW > 0 && srcH > 0 && rcDest.right > rcDest.left && rcDest.bottom > rcDest.top ) {
				if( !xtiled && !ytiled ) {
					BlitClipped(rcDest, srcL, srcT, srcW, srcH);
				}
				else if( xtiled && ytiled ) {
					for( LONG y = rcDest.top; y < rcDest.bottom; y += srcH ) {
						LONG h = srcH;
						if( y + h > rcDest.bottom ) h = rcDest.bottom - y;
						for( LONG x = rcDest.left; x < rcDest.right; x += srcW ) {
							LONG w = srcW;
							if( x + w > rcDest.right ) w = rcDest.right - x;
							RECT d = { x, y, x + w, y + h };
							BlitClipped(d, srcL, srcT, w, h);
						}
					}
				}
				else if( xtiled ) {
					for( LONG x = rcDest.left; x < rcDest.right; x += srcW ) {
						LONG w = srcW;
						if( x + w > rcDest.right ) w = rcDest.right - x;
						RECT d = { x, rcDest.top, x + w, rcDest.bottom };
						BlitClipped(d, srcL, srcT, w, srcH);
					}
				}
				else {
					for( LONG y = rcDest.top; y < rcDest.bottom; y += srcH ) {
						LONG h = srcH;
						if( y + h > rcDest.bottom ) h = rcDest.bottom - y;
						RECT d = { rcDest.left, y, rcDest.right, y + h };
						BlitClipped(d, srcL, srcT, srcW, h);
					}
				}
			}
		}

		// corners & edges
		if( rcCorners.left > 0 && rcCorners.top > 0 ) {
			RECT d = { rc.left, rc.top, rc.left + rcCorners.left, rc.top + rcCorners.top };
			BlitClipped(d, rcBmpPart.left, rcBmpPart.top, rcCorners.left, rcCorners.top);
		}
		if( rcCorners.top > 0 ) {
			RECT d = { rc.left + rcCorners.left, rc.top, rc.right - rcCorners.right, rc.top + rcCorners.top };
			BlitClipped(d, rcBmpPart.left + rcCorners.left, rcBmpPart.top,
				rcBmpPart.right - rcBmpPart.left - rcCorners.left - rcCorners.right, rcCorners.top);
		}
		if( rcCorners.right > 0 && rcCorners.top > 0 ) {
			RECT d = { rc.right - rcCorners.right, rc.top, rc.right, rc.top + rcCorners.top };
			BlitClipped(d, rcBmpPart.right - rcCorners.right, rcBmpPart.top, rcCorners.right, rcCorners.top);
		}
		if( rcCorners.left > 0 ) {
			RECT d = { rc.left, rc.top + rcCorners.top, rc.left + rcCorners.left, rc.bottom - rcCorners.bottom };
			BlitClipped(d, rcBmpPart.left, rcBmpPart.top + rcCorners.top,
				rcCorners.left, rcBmpPart.bottom - rcBmpPart.top - rcCorners.top - rcCorners.bottom);
		}
		if( rcCorners.right > 0 ) {
			RECT d = { rc.right - rcCorners.right, rc.top + rcCorners.top, rc.right, rc.bottom - rcCorners.bottom };
			BlitClipped(d, rcBmpPart.right - rcCorners.right, rcBmpPart.top + rcCorners.top,
				rcCorners.right, rcBmpPart.bottom - rcBmpPart.top - rcCorners.top - rcCorners.bottom);
		}
		if( rcCorners.left > 0 && rcCorners.bottom > 0 ) {
			RECT d = { rc.left, rc.bottom - rcCorners.bottom, rc.left + rcCorners.left, rc.bottom };
			BlitClipped(d, rcBmpPart.left, rcBmpPart.bottom - rcCorners.bottom, rcCorners.left, rcCorners.bottom);
		}
		if( rcCorners.bottom > 0 ) {
			RECT d = { rc.left + rcCorners.left, rc.bottom - rcCorners.bottom, rc.right - rcCorners.right, rc.bottom };
			BlitClipped(d, rcBmpPart.left + rcCorners.left, rcBmpPart.bottom - rcCorners.bottom,
				rcBmpPart.right - rcBmpPart.left - rcCorners.left - rcCorners.right, rcCorners.bottom);
		}
		if( rcCorners.right > 0 && rcCorners.bottom > 0 ) {
			RECT d = { rc.right - rcCorners.right, rc.bottom - rcCorners.bottom, rc.right, rc.bottom };
			BlitClipped(d, rcBmpPart.right - rcCorners.right, rcBmpPart.bottom - rcCorners.bottom, rcCorners.right, rcCorners.bottom);
		}
		return true;
	}

	bool CD2dRenderContext::EnsureDCRenderTarget()
	{
		if( m_pDCRT != NULL ) return true;
		if( m_pFactory == NULL ) return false;

		D2D1_RENDER_TARGET_PROPERTIES props = GdiCompatRtProps();
		HRESULT hr = m_pFactory->CreateDCRenderTarget(&props, &m_pDCRT);
		return SUCCEEDED(hr) && m_pDCRT != NULL;
	}

	IRenderSurface* CD2dRenderContext::FindBoundSurface() const
	{
		CPaintManagerUI* pManager = GetManager();
		if( pManager == NULL ) return NULL;
		HDC hDC = m_gdiFallback.GetDC();
		IRenderSurface* pSurfaces[2] = {
			pManager->GetOffscreenSurface(),
			pManager->GetBackgroundSurface()
		};
		for( int i = 0; i < 2; ++i ) {
			IRenderSurface* pSurf = pSurfaces[i];
			if( pSurf == NULL || pSurf->GetBackendTarget() == NULL ) continue;
			if( pSurf->IsWindowTarget() ) return pSurf;
			if( pSurf->GetNativeTarget() == hDC ) return pSurf;
		}
		return NULL;
	}

	void CD2dRenderContext::ReleaseHwndInteropDC()
	{
		if( m_pHwndInterop != NULL ) {
			if( m_hHwndInteropDC != NULL )
				m_pHwndInterop->ReleaseDC(NULL);
			m_pHwndInterop->Release();
			m_pHwndInterop = NULL;
			m_hHwndInteropDC = NULL;
		}
		// GetDC 失败回退时可能临时 Pop 了 D2D clip；归还 DC 后按逻辑栈补回
		if( m_bInDraw && m_pRT != NULL && m_nD2dClipDepth < m_nClipCount ) {
			for( int i = m_nD2dClipDepth; i < m_nClipCount; ++i )
				ApplyClipEntry(m_clipStack[i]);
		}
	}

	HDC CD2dRenderContext::AcquireHwndInteropDC()
	{
		if( m_hHwndInteropDC != NULL ) return m_hHwndInteropDC;
		if( m_pRT == NULL ) return NULL;
		ID2D1GdiInteropRenderTarget* pInterop = NULL;
		HRESULT hr = m_pRT->QueryInterface(__uuidof(ID2D1GdiInteropRenderTarget), (void**)&pInterop);
		if( FAILED(hr) || pInterop == NULL ) return NULL;
		HDC hDC = NULL;
		hr = pInterop->GetDC(D2D1_DC_INITIALIZE_MODE_COPY, &hDC);
		if( FAILED(hr) || hDC == NULL ) {
			pInterop->Release();
			return NULL;
		}
		m_pHwndInterop = pInterop;
		m_hHwndInteropDC = hDC;
		return hDC;
	}

	void CD2dRenderContext::FlushToGdi(bool bSyncPixels) const
	{
		CD2dRenderContext* pThis = const_cast<CD2dRenderContext*>(this);
		pThis->ReleaseHwndInteropDC();
		if( !pThis->m_bInDraw || pThis->m_pRT == NULL ) return;
		while( pThis->m_nD2dClipDepth > 0 ) {
			pThis->PopOneClip();
		}

		// BitBlt 前去掉 GDI 裁剪：若仍挂着 CreateRoundRectRgn，会把 D2D AA 圆角裁成锯齿
		HDC hGdi = pThis->m_gdiFallback.GetDC();
		int nGdiSave = 0;
		if( bSyncPixels && hGdi != NULL )
			nGdiSave = ::SaveDC(hGdi);
		if( bSyncPixels && hGdi != NULL )
			::SelectClipRgn(hGdi, NULL);

		const bool bComp = (pThis->m_pBoundSurface != NULL && pThis->m_pBoundSurface->IsLayeredComposition());
		bool bCopied = false;
		// BitmapRT：优先在 EndDraw 前用 GdiInterop；失败则 EndDraw 后走 GetBitmap 回退
		if( !bComp && bSyncPixels && pThis->m_pBoundSurface != NULL )
			bCopied = pThis->m_pBoundSurface->CopyBackendToPixels();

		pThis->m_pRT->EndDraw();
		pThis->m_bInDraw = false;

		if( bSyncPixels && pThis->m_pBoundSurface != NULL ) {
			if( bComp )
				pThis->m_pBoundSurface->CopyBackendToPixels();
			else if( !bCopied )
				pThis->m_pBoundSurface->CopyBackendToPixelsViaBitmap();
		}

		if( nGdiSave != 0 && hGdi != NULL )
			::RestoreDC(hGdi, nGdiSave);

		if( bSyncPixels )
			pThis->m_bPixelsDirty = false;
	}

	bool CD2dRenderContext::EnsureD2dDraw()
	{
		ReleaseHwndInteropDC();
		if( m_bInDraw ) return true;

		m_pBoundSurface = FindBoundSurface();
		if( m_pBoundSurface != NULL ) {
			m_pRT = reinterpret_cast<ID2D1RenderTarget*>(m_pBoundSurface->GetBackendTarget());
			if( m_pRT == NULL ) {
				m_pBoundSurface = NULL;
			}
			else {
				m_pRT->BeginDraw();
				m_bInDraw = true;
				m_pBoundSurface->OnBackendBeginDraw();
				if( m_bPixelsDirty )
					m_pBoundSurface->CopyPixelsToBackend();
				m_bPixelsDirty = false;
				m_nD2dClipDepth = 0;
				m_nBindWidth = m_pBoundSurface->GetWidth();
				m_nBindHeight = m_pBoundSurface->GetHeight();
				for( int i = 0; i < m_nClipCount; ++i )
					ApplyClipEntry(m_clipStack[i]);
				return true;
			}
		}

		if( !EnsureDCRenderTarget() ) return false;
		m_pRT = m_pDCRT;

		HDC hDC = m_gdiFallback.GetDC();
		if( hDC == NULL ) return false;

		int width = m_nBindWidth;
		int height = m_nBindHeight;
		HBITMAP hBmp = (HBITMAP)::GetCurrentObject(hDC, OBJ_BITMAP);
		if( hBmp != NULL ) {
			BITMAP bm = { 0 };
			if( ::GetObject(hBmp, sizeof(bm), &bm) && bm.bmWidth > 0 && bm.bmHeight > 0 ) {
				width = bm.bmWidth;
				height = bm.bmHeight;
			}
		}
		if( width <= 0 || height <= 0 ) {
			width = ::GetDeviceCaps(hDC, HORZRES);
			height = ::GetDeviceCaps(hDC, VERTRES);
		}
		if( width <= 0 ) width = 1;
		if( height <= 0 ) height = 1;
		m_nBindWidth = width;
		m_nBindHeight = height;

		RECT rcBind = { 0, 0, width, height };
		HRESULT hr = m_pDCRT->BindDC(hDC, &rcBind);
		if( FAILED(hr) ) return false;
		m_pDCRT->BeginDraw();
		m_bInDraw = true;
		m_bPixelsDirty = false;
		m_nD2dClipDepth = 0;
		for( int i = 0; i < m_nClipCount; ++i )
			ApplyClipEntry(m_clipStack[i]);
		return true;
	}

	ID2D1SolidColorBrush* CD2dRenderContext::GetBrush(DWORD argb)
	{
		if( m_pRT == NULL ) return NULL;
		D2D1_COLOR_F c = ToColorF(argb);
		if( m_pBrush == NULL ) {
			if( FAILED(m_pRT->CreateSolidColorBrush(c, &m_pBrush)) ) return NULL;
		}
		else {
			m_pBrush->SetColor(c);
		}
		return m_pBrush;
	}

	ID2D1RenderTarget* CD2dRenderContext::GetRenderTarget() const
	{
		return m_pRT;
	}

	void CD2dRenderContext::OnBeginFrame()
	{
		m_bPixelsDirty = false;
		EnsureD2dDraw();
	}

	void CD2dRenderContext::OnEndFrame()
	{
		// 非分层必须 sync→GDI 再 BitBlt Present；跳过并用 BitmapRT DrawBitmap 直出会黑屏。见 AGENTS.md
		// 分层零拷贝：EndDraw 留在 DXGI 上，Present 直接 Commit；仅在 GDI 已脏时回写
		bool bSync = true;
		if( m_pBoundSurface != NULL && m_pBoundSurface->IsLayeredComposition() )
			bSync = m_bPixelsDirty;
		else if( m_pBoundSurface != NULL && m_pBoundSurface->IsWindowTarget() )
			bSync = false;
		FlushToGdi(bSync);
	}

	HDC CD2dRenderContext::GetDC() const
	{
		CD2dRenderContext* pThis = const_cast<CD2dRenderContext*>(this);
		// HWND 直出：不要 EndDraw（会提前 Present），改为在 RT 上借 GDI DC
		if( pThis->m_pBoundSurface != NULL && pThis->m_pBoundSurface->IsWindowTarget() ) {
			if( !pThis->m_bInDraw )
				pThis->EnsureD2dDraw();
			HDC hInterop = pThis->AcquireHwndInteropDC();
			if( hInterop != NULL ) return hInterop;

			// GdiInterop 不可用：结束当前 HWND 帧并禁用直出，回退离屏 GDI DC
			pThis->ReleaseHwndInteropDC();
			if( pThis->m_bInDraw && pThis->m_pRT != NULL ) {
				while( pThis->m_nD2dClipDepth > 0 )
					pThis->PopOneClip();
				pThis->m_pRT->EndDraw();
				pThis->m_bInDraw = false;
			}
			pThis->m_pBoundSurface->DisableWindowTarget();
			pThis->m_pBoundSurface = NULL;
			pThis->m_pRT = NULL;
			pThis->m_bPixelsDirty = true;
			return m_gdiFallback.GetDC();
		}

		// 离屏 BitmapRT：同样在 BeginDraw 期间借 GDI DC，避免 Flush/EndDraw 打断帧
		// （RichEdit TxDraw 等）；Win8+ 允许在 clip 栈上 GetDC。
		if( pThis->m_pBoundSurface != NULL && pThis->m_pBoundSurface->GetBackendTarget() != NULL
			&& !pThis->m_pBoundSurface->IsLayeredComposition() ) {
			if( !pThis->m_bInDraw )
				pThis->EnsureD2dDraw();
			HDC hInterop = pThis->AcquireHwndInteropDC();
			if( hInterop != NULL ) return hInterop;

			// GetDC 失败（常见于裁剪栈不平衡）：弹出裁剪后重试一次
			while( pThis->m_nD2dClipDepth > 0 )
				pThis->PopOneClip();
			hInterop = pThis->AcquireHwndInteropDC();
			if( hInterop != NULL ) {
				// 裁剪已弹出；GDI 侧仍有 clip。归还后 EnsureD2dDraw 会按 m_nClipCount 重建
				return hInterop;
			}
		}

		FlushToGdi();
		pThis->m_bPixelsDirty = true;
		return m_gdiFallback.GetDC();
	}

	void CD2dRenderContext::ReleaseNativeDC()
	{
		ReleaseHwndInteropDC();
	}

	HDC CD2dRenderContext::GetGdiPaintDC() const
	{
		// 先结束 D2D 帧并同步到 GDI，再在真正的 GDI DC 上 TxDraw。
		// 禁止走 AcquireHwndInteropDC：预乘 BGRA 上的 ClearType 会发灰发糊。
		CD2dRenderContext* pThis = const_cast<CD2dRenderContext*>(this);
		pThis->FlushToGdi();
		pThis->m_bPixelsDirty = true;
		return m_gdiFallback.GetDC();
	}

	CPaintManagerUI* CD2dRenderContext::GetManager() const
	{
		return m_gdiFallback.GetManager();
	}

	void CD2dRenderContext::ApplyClipEntry(const TD2dClipEntry& entryIn)
	{
		if( m_pRT == NULL ) return;
		if( m_nD2dClipDepth >= kMaxD2dClips ) return;
		TD2dClipEntry& entry = const_cast<TD2dClipEntry&>(entryIn);
		if( entry.bRound && m_pFactory != NULL && entry.radiusX > 0.0f && entry.radiusY > 0.0f ) {
			ID2D1RoundedRectangleGeometry* pGeom = NULL;
			// 几何必须用完整控件矩形（rcRound），再与 rcBound 内容范围求交——对齐 GDI CombineRgn
			D2D1_ROUNDED_RECT rr = D2D1::RoundedRect(ToRectF(entry.rcRound), entry.radiusX, entry.radiusY);
			if( SUCCEEDED(m_pFactory->CreateRoundedRectangleGeometry(rr, &pGeom)) && pGeom != NULL ) {
				ID2D1Layer* pLayer = NULL;
				if( SUCCEEDED(m_pRT->CreateLayer(&pLayer)) && pLayer != NULL ) {
					m_pRT->PushLayer(
						D2D1::LayerParameters(ToRectF(entry.rcBound), pGeom, D2D1_ANTIALIAS_MODE_PER_PRIMITIVE),
						pLayer);
					pLayer->Release();
					pGeom->Release();
					entry.bLayer = true;
					m_d2dClipIsLayer[m_nD2dClipDepth] = true;
					m_nD2dClipDepth++;
					return;
				}
				pGeom->Release();
			}
		}
		entry.bLayer = false;
		m_pRT->PushAxisAlignedClip(ToRectF(entry.rcBound), D2D1_ANTIALIAS_MODE_PER_PRIMITIVE);
		m_d2dClipIsLayer[m_nD2dClipDepth] = false;
		m_nD2dClipDepth++;
	}

	void CD2dRenderContext::PopOneClip()
	{
		if( m_pRT == NULL || m_nD2dClipDepth <= 0 ) return;
		// 必须按实际 Push 类型 Pop，不能用 m_clipStack[].bLayer（Resume/回退路径会与之脱节）
		if( m_d2dClipIsLayer[m_nD2dClipDepth - 1] )
			m_pRT->PopLayer();
		else
			m_pRT->PopAxisAlignedClip();
		m_nD2dClipDepth--;
	}

	void CD2dRenderContext::PushClip(const RECT& rc)
	{
		m_gdiFallback.PushClip(rc);
		if( m_nClipCount < kMaxD2dClips ) {
			TD2dClipEntry& e = m_clipStack[m_nClipCount++];
			e.rcBound = rc;
			e.rcRound = rc;
			e.bRound = false;
			e.radiusX = e.radiusY = 0.0f;
			e.bLayer = false;
			if( m_bInDraw && m_pRT != NULL )
				ApplyClipEntry(e);
		}
	}

	void CD2dRenderContext::PushRoundClip(const RECT& rcClip, const RECT& rcRound, int width, int height)
	{
		// D2D 已用 PushLayer 做 AA 圆角时，GDI 侧不要 CreateRoundRectRgn：
		// 帧内 FlushToGdi→BitBlt 若碰上锯齿 RGN，会把抗锯齿边缘裁掉（首帧常见，拖动重绘后消失）。
		if( m_bInDraw && m_pRT != NULL )
			m_gdiFallback.PushClip(rcClip);
		else
			m_gdiFallback.PushRoundClip(rcClip, rcRound, width, height);
		if( m_nClipCount < kMaxD2dClips ) {
			TD2dClipEntry& e = m_clipStack[m_nClipCount++];
			RECT rcBound = { 0 };
			if( !::IntersectRect(&rcBound, &rcClip, &rcRound) )
				rcBound = rcRound;
			e.rcBound = rcBound;
			e.rcRound = rcRound;
			// width/height 为 CSS 半径（与属性 border-radius 一致）
			e.bRound = (width > 0 || height > 0);
			e.radiusX = (FLOAT)width;
			e.radiusY = (FLOAT)height;
			e.bLayer = false;
			if( m_bInDraw && m_pRT != NULL )
				ApplyClipEntry(e);
		}
	}

	void CD2dRenderContext::PopClip()
	{
		m_gdiFallback.PopClip();
		if( m_bInDraw && m_pRT != NULL && m_nD2dClipDepth > 0 )
			PopOneClip();
		if( m_nClipCount > 0 ) m_nClipCount--;
	}

	void CD2dRenderContext::SuspendClip()
	{
		m_gdiFallback.SuspendClip();
		// 与 GDI 一致：只暂时去掉栈顶一层，避免清空全部裁剪后状态错乱
		if( m_bInDraw && m_pRT != NULL && m_nD2dClipDepth > 0 )
			PopOneClip();
	}

	void CD2dRenderContext::ResumeClip()
	{
		m_gdiFallback.ResumeClip();
		if( m_bInDraw && m_pRT != NULL && m_nD2dClipDepth < m_nClipCount )
			ApplyClipEntry(m_clipStack[m_nClipCount - 1]);
	}

	// SaveDC 哨兵：D2D 帧内无 GDI 状态可保存时返回此值
	static const int kD2dSaveStateNop = 0x7F00D2D1;

	int CD2dRenderContext::SaveState()
	{
		// ColorPalette 等仅成对 Save/Restore，不依赖 GDI 裁剪态
		if( m_bInDraw && m_pRT != NULL )
			return kD2dSaveStateNop;
		FlushToGdi();
		return m_gdiFallback.SaveState();
	}

	void CD2dRenderContext::RestoreState(int nSaved)
	{
		if( nSaved == kD2dSaveStateNop )
			return;
		FlushToGdi();
		m_gdiFallback.RestoreState(nSaved);
	}

	void CD2dRenderContext::DrawColor(const RECT& rc, DWORD color)
	{
		if( !EnsureD2dDraw() ) {
			m_gdiFallback.DrawColor(rc, color);
			return;
		}
		ID2D1SolidColorBrush* pBrush = GetBrush(color);
		if( pBrush == NULL ) {
			FlushToGdi();
			m_gdiFallback.DrawColor(rc, color);
			return;
		}
		m_pRT->FillRectangle(ToRectF(rc), pBrush);
	}

	void CD2dRenderContext::DrawGradient(const RECT& rc, DWORD dwFirst, DWORD dwSecond, bool bVertical, int nSteps)
	{
		(void)nSteps;
		if( !EnsureD2dDraw() ) {
			m_gdiFallback.DrawGradient(rc, dwFirst, dwSecond, bVertical, nSteps);
			return;
		}

		D2D1_GRADIENT_STOP stops[2];
		stops[0].color = ToColorF(dwFirst);
		stops[0].position = 0.0f;
		stops[1].color = ToColorF(dwSecond);
		stops[1].position = 1.0f;

		ID2D1GradientStopCollection* pStops = NULL;
		if( FAILED(m_pRT->CreateGradientStopCollection(stops, 2, &pStops)) || pStops == NULL ) {
			FlushToGdi();
			m_gdiFallback.DrawGradient(rc, dwFirst, dwSecond, bVertical, nSteps);
			return;
		}

		D2D1_LINEAR_GRADIENT_BRUSH_PROPERTIES props;
		if( bVertical ) {
			props.startPoint = D2D1::Point2F((FLOAT)rc.left, (FLOAT)rc.top);
			props.endPoint = D2D1::Point2F((FLOAT)rc.left, (FLOAT)rc.bottom);
		}
		else {
			props.startPoint = D2D1::Point2F((FLOAT)rc.left, (FLOAT)rc.top);
			props.endPoint = D2D1::Point2F((FLOAT)rc.right, (FLOAT)rc.top);
		}

		ID2D1LinearGradientBrush* pBrush = NULL;
		HRESULT hr = m_pRT->CreateLinearGradientBrush(props, pStops, &pBrush);
		pStops->Release();
		if( FAILED(hr) || pBrush == NULL ) {
			FlushToGdi();
			m_gdiFallback.DrawGradient(rc, dwFirst, dwSecond, bVertical, nSteps);
			return;
		}
		m_pRT->FillRectangle(ToRectF(rc), pBrush);
		pBrush->Release();
	}

	void CD2dRenderContext::DrawLine(const RECT& rc, int nSize, DWORD dwPenColor, int nStyle)
	{
		(void)nStyle;
		if( !EnsureD2dDraw() ) {
			m_gdiFallback.DrawLine(rc, nSize, dwPenColor, nStyle);
			return;
		}
		ID2D1SolidColorBrush* pBrush = GetBrush(dwPenColor);
		if( pBrush == NULL ) {
			FlushToGdi();
			m_gdiFallback.DrawLine(rc, nSize, dwPenColor, nStyle);
			return;
		}
		FLOAT stroke = (nSize > 0) ? (FLOAT)nSize : 1.0f;
		FLOAT x0 = (FLOAT)rc.left;
		FLOAT y0 = (FLOAT)rc.top;
		FLOAT x1 = (FLOAT)rc.right;
		FLOAT y1 = (FLOAT)rc.bottom;
		const bool bAxisHair = (stroke <= 1.0f) && (rc.left == rc.right || rc.top == rc.bottom);
		D2D1_ANTIALIAS_MODE oldAA = m_pRT->GetAntialiasMode();
		if( bAxisHair ) {
			// 轴对齐 1px 用 ALIASED，避免 PER_PRIMITIVE 把细线糊成约 2px
			m_pRT->SetAntialiasMode(D2D1_ANTIALIAS_MODE_ALIASED);
		}
		else if( stroke <= 1.0f ) {
			if( rc.left == rc.right ) { x0 += 0.5f; x1 += 0.5f; }
			else if( rc.top == rc.bottom ) { y0 += 0.5f; y1 += 0.5f; }
		}
		m_pRT->DrawLine(D2D1::Point2F(x0, y0), D2D1::Point2F(x1, y1), pBrush, stroke);
		if( bAxisHair )
			m_pRT->SetAntialiasMode(oldAA);
	}

	void CD2dRenderContext::DrawRect(const RECT& rc, int nSize, DWORD dwPenColor, int nStyle)
	{
		(void)nStyle;
		if( !EnsureD2dDraw() ) {
			m_gdiFallback.DrawRect(rc, nSize, dwPenColor, nStyle);
			return;
		}
		ID2D1SolidColorBrush* pBrush = GetBrush(dwPenColor);
		if( pBrush == NULL ) {
			FlushToGdi();
			m_gdiFallback.DrawRect(rc, nSize, dwPenColor, nStyle);
			return;
		}
		FLOAT stroke = (nSize > 0) ? (FLOAT)nSize : 1.0f;
		m_pRT->DrawRectangle(ToRectF(rc), pBrush, stroke);
	}

	void CD2dRenderContext::DrawRoundRect(const RECT& rc, int nSize, int width, int height, DWORD dwPenColor, int nStyle)
	{
		(void)nStyle;
		if( !EnsureD2dDraw() ) {
			m_gdiFallback.DrawRoundRect(rc, nSize, width, height, dwPenColor, nStyle);
			return;
		}
		ID2D1SolidColorBrush* pBrush = GetBrush(dwPenColor);
		if( pBrush == NULL ) {
			FlushToGdi();
			m_gdiFallback.DrawRoundRect(rc, nSize, width, height, dwPenColor, nStyle);
			return;
		}
		FLOAT stroke = (nSize > 0) ? (FLOAT)nSize : 1.0f;
		D2D1_ROUNDED_RECT rr = D2D1::RoundedRect(ToRectF(rc), (FLOAT)width, (FLOAT)height);
		m_pRT->DrawRoundedRectangle(rr, pBrush, stroke);
	}

	void CD2dRenderContext::FillRoundRect(const RECT& rc, int width, int height, DWORD dwColor)
	{
		if( width <= 0 && height <= 0 ) {
			DrawColor(rc, dwColor);
			return;
		}
		if( !EnsureD2dDraw() ) {
			m_gdiFallback.FillRoundRect(rc, width, height, dwColor);
			return;
		}
		ID2D1SolidColorBrush* pBrush = GetBrush(dwColor);
		if( pBrush == NULL ) {
			FlushToGdi();
			m_gdiFallback.FillRoundRect(rc, width, height, dwColor);
			return;
		}
		FLOAT radiusX = (FLOAT)((width > 0) ? width : height);
		FLOAT radiusY = (FLOAT)((height > 0) ? height : width);
		D2D1_ROUNDED_RECT rr = D2D1::RoundedRect(ToRectF(rc), radiusX, radiusY);
		m_pRT->SetAntialiasMode(D2D1_ANTIALIAS_MODE_PER_PRIMITIVE);
		m_pRT->FillRoundedRectangle(rr, pBrush);
	}

	IDWriteTextFormat* CD2dRenderContext::CreateTextFormat(int iFont) const
	{
		if( m_pDWrite == NULL ) return NULL;
		CPaintManagerUI* pManager = GetManager();
		if( pManager == NULL ) return NULL;
		TFontInfo* pFontInfo = pManager->GetFontInfo(iFont);
		if( pFontInfo == NULL ) return NULL;

		// 与 RebuildFont/CreateNativeFont 一致：DWrite 字号也要按 DPI Scale
		int nLogical = pFontInfo->iSize;
		if( nLogical < 0 ) nLogical = -nLogical;
		FLOAT fontSize = (FLOAT)pManager->GetDPIObj()->Scale(nLogical);
		if( fontSize < 1.0f ) fontSize = (FLOAT)pFontInfo->tm.tmHeight;
		if( fontSize < 1.0f ) fontSize = 12.0f;

		IDWriteTextFormat* pFormat = NULL;
		HRESULT hr = m_pDWrite->CreateTextFormat(
			pFontInfo->sFontName.IsEmpty() ? L"Segoe UI" : pFontInfo->sFontName.GetData(),
			NULL,
			pFontInfo->bBold ? DWRITE_FONT_WEIGHT_BOLD : DWRITE_FONT_WEIGHT_NORMAL,
			pFontInfo->bItalic ? DWRITE_FONT_STYLE_ITALIC : DWRITE_FONT_STYLE_NORMAL,
			DWRITE_FONT_STRETCH_NORMAL,
			fontSize,
			L"en-us",
			&pFormat);
		return SUCCEEDED(hr) ? pFormat : NULL;
	}

	bool CD2dRenderContext::TextLooksLikeHtml(LPCTSTR pstrText)
	{
		return (pstrText != NULL && _tcschr(pstrText, _T('<')) != NULL);
	}

	bool CD2dRenderContext::MeasureTextDWrite(RECT& rc, LPCTSTR pstrText, IDWriteTextFormat* pFormat, UINT uStyle) const
	{
		if( pstrText == NULL || pFormat == NULL || m_pDWrite == NULL ) return false;
		pFormat->SetTextAlignment(ToDWriteAlign(uStyle));
		pFormat->SetParagraphAlignment(ToDWriteParagraphAlign(uStyle));
		pFormat->SetWordWrapping((uStyle & DT_SINGLELINE) ? DWRITE_WORD_WRAPPING_NO_WRAP : DWRITE_WORD_WRAPPING_WRAP);

		FLOAT maxW = (uStyle & DT_SINGLELINE) ? 16384.0f : (FLOAT)((rc.right - rc.left > 1) ? (rc.right - rc.left) : 1);
		FLOAT maxH = 16384.0f;
		IDWriteTextLayout* pLayout = NULL;
		UINT32 len = (UINT32)_tcslen(pstrText);
		if( FAILED(m_pDWrite->CreateTextLayout(pstrText, len, pFormat, maxW, maxH, &pLayout)) || pLayout == NULL )
			return false;

		DWRITE_TEXT_METRICS metrics = { 0 };
		HRESULT hr = pLayout->GetMetrics(&metrics);
		pLayout->Release();
		if( FAILED(hr) ) return false;
		rc.right = rc.left + (LONG)(metrics.widthIncludingTrailingWhitespace + 0.999f);
		rc.bottom = rc.top + (LONG)(metrics.height + 0.999f);
		return true;
	}

	void CD2dRenderContext::DrawText(RECT& rc, LPCTSTR pstrText, DWORD dwColor, int iFont, UINT uStyle)
	{
		if( pstrText == NULL || *pstrText == _T('\0') ) return;

		IDWriteTextFormat* pFormat = CreateTextFormat(iFont);
		if( (uStyle & DT_CALCRECT) != 0 ) {
			if( MeasureTextDWrite(rc, pstrText, pFormat, uStyle) ) {
				if( pFormat != NULL ) pFormat->Release();
				return;
			}
			if( pFormat != NULL ) pFormat->Release();
			FlushToGdi();
			m_gdiFallback.DrawText(rc, pstrText, dwColor, iFont, uStyle);
			return;
		}

		if( pFormat == NULL || !EnsureD2dDraw() ) {
			if( pFormat != NULL ) pFormat->Release();
			FlushToGdi();
			m_gdiFallback.DrawText(rc, pstrText, dwColor, iFont, uStyle);
			return;
		}

		pFormat->SetTextAlignment(ToDWriteAlign(uStyle));
		pFormat->SetParagraphAlignment(ToDWriteParagraphAlign(uStyle));
		pFormat->SetWordWrapping((uStyle & DT_SINGLELINE) ? DWRITE_WORD_WRAPPING_NO_WRAP : DWRITE_WORD_WRAPPING_WRAP);

		ID2D1SolidColorBrush* pBrush = GetBrush(dwColor);
		if( pBrush == NULL ) {
			pFormat->Release();
			FlushToGdi();
			m_gdiFallback.DrawText(rc, pstrText, dwColor, iFont, uStyle);
			return;
		}

		UINT32 len = (UINT32)_tcslen(pstrText);
		D2D1_DRAW_TEXT_OPTIONS opts = ((uStyle & DT_NOCLIP) != 0)
			? D2D1_DRAW_TEXT_OPTIONS_NONE
			: D2D1_DRAW_TEXT_OPTIONS_CLIP;
		m_pRT->DrawText(
			pstrText, len, pFormat, ToRectF(rc), pBrush,
			opts, DWRITE_MEASURING_MODE_NATURAL);
		pFormat->Release();
	}

	void CD2dRenderContext::DrawText(RECT& rc, LPCTSTR pstrText, DWORD dwColor, int iFont, UINT uStyle, DWORD dwTextBKColor)
	{
		DrawColor(rc, dwTextBKColor);
		DrawText(rc, pstrText, dwColor, iFont, uStyle);
	}

	void CD2dRenderContext::DrawHtmlText(RECT& rc, LPCTSTR pstrText, DWORD dwColor, RECT* pLinks, CDuiString* sLinks, int& nLinkRects, int iFont, UINT uStyle)
	{
		if( !TextLooksLikeHtml(pstrText) ) {
			DrawText(rc, pstrText, dwColor, iFont, uStyle);
			nLinkRects = 0;
			return;
		}
		if( DrawHtmlTextDWrite(rc, pstrText, dwColor, pLinks, sLinks, nLinkRects, iFont, uStyle) )
			return;
		FlushToGdi();
		m_gdiFallback.DrawHtmlText(rc, pstrText, dwColor, pLinks, sLinks, nLinkRects, iFont, uStyle);
	}

	bool CD2dRenderContext::DrawHtmlTextDWrite(RECT& rc, LPCTSTR pstrText, DWORD dwColor, RECT* pLinks, CDuiString* sLinks, int& nLinkRects, int iFont, UINT uStyle)
	{
		if( pstrText == NULL || m_pDWrite == NULL ) return false;

		struct TRange {
			UINT32 start;
			UINT32 length;
			bool bold;
			bool italic;
			bool underline;
			bool selected;
			DWORD color;
			int fontId;
			FLOAT fontSize;
			CDuiString fontFace;
			bool link;
			CDuiString href;
		};
		struct TInline {
			UINT32 pos;
			enum { Spacer, Image } type;
			FLOAT width;
			FLOAT height;
			FLOAT srcLeft;
			FLOAT srcTop;
			FLOAT srcRight;
			FLOAT srcBottom;
			CDuiString imageName;
		};

		CDuiString sPlain;
		CStdPtrArray aRanges;
		CStdPtrArray aInlines;
		bool bold = false, italic = false, underline = false, inLink = false, inRaw = false, inSelected = false;
		DWORD curColor = dwColor;
		int curFontId = iFont;
		FLOAT curFontSize = 0.0f;
		CDuiString curFontFace;
		int paraExtra = 0;
		CDuiString curHref;
		UINT32 plainPos = 0;
		FLOAT lineHeight = 16.0f;
		CPaintManagerUI* pManager = GetManager();
		auto ScaledFontSize = [&](TFontInfo* pFi) -> FLOAT {
			if( pFi == NULL ) return 12.0f;
			int nLogical = pFi->iSize;
			if( nLogical < 0 ) nLogical = -nLogical;
			FLOAT sz = (pManager != NULL) ? (FLOAT)pManager->GetDPIObj()->Scale(nLogical) : (FLOAT)nLogical;
			if( sz < 1.0f ) sz = (FLOAT)pFi->tm.tmHeight;
			if( sz < 1.0f ) sz = 12.0f;
			return sz;
		};
		{
			TFontInfo* pFi = pManager ? pManager->GetFontInfo(iFont) : NULL;
			if( pFi != NULL ) {
				lineHeight = (FLOAT)(pFi->tm.tmHeight + pFi->tm.tmExternalLeading);
				curFontSize = ScaledFontSize(pFi);
			}
			if( lineHeight < 1.0f ) lineHeight = 16.0f;
			if( curFontSize <= 0.0f ) curFontSize = 12.0f;
		}

		auto PushRange = [&](UINT32 start, UINT32 len) {
			if( len == 0 ) return;
			TRange* p = new TRange;
			p->start = start;
			p->length = len;
			p->bold = bold;
			p->italic = italic;
			p->underline = underline || inLink;
			p->selected = inSelected;
			p->color = curColor;
			p->fontId = curFontId;
			p->fontSize = curFontSize;
			p->fontFace = curFontFace;
			p->link = inLink;
			p->href = curHref;
			aRanges.Add(p);
		};
		auto FreeAll = [&]() {
			for( int i = 0; i < aRanges.GetSize(); ++i )
				delete static_cast<TRange*>(aRanges.GetAt(i));
			aRanges.Empty();
			for( int i = 0; i < aInlines.GetSize(); ++i )
				delete static_cast<TInline*>(aInlines.GetAt(i));
			aInlines.Empty();
		};
		auto AppendObjectReplacement = [&]() {
			sPlain += (TCHAR)0xFFFC;
			plainPos++;
		};

		LPCTSTR p = pstrText;
		UINT32 runStart = 0;
		while( *p != _T('\0') ) {
			TCHAR open = (*p == _T('<') || *p == _T('{')) ? *p : 0;
			if( open == 0 ) {
				sPlain += *p++;
				plainPos++;
				continue;
			}
			TCHAR close = (open == _T('<')) ? _T('>') : _T('}');
			LPCTSTR pTag = p + 1;
			LPCTSTR pEnd = pTag;
			while( *pEnd && *pEnd != close ) pEnd++;
			if( *pEnd != close ) {
				FreeAll();
				return false;
			}

			// raw 模式：除 </r> 外整段标签当字面量
			if( inRaw ) {
				CDuiString sTagRaw(pTag, (int)(pEnd - pTag));
				sTagRaw.MakeLower();
				if( sTagRaw == _T("/r") ) {
					if( plainPos > runStart )
						PushRange(runStart, plainPos - runStart);
					inRaw = false;
					runStart = plainPos;
					p = pEnd + 1;
					continue;
				}
				sPlain += open;
				plainPos++;
				for( LPCTSTR q = pTag; q < pEnd; ++q ) {
					sPlain += *q;
					plainPos++;
				}
				sPlain += close;
				plainPos++;
				p = pEnd + 1;
				continue;
			}

			if( plainPos > runStart )
				PushRange(runStart, plainPos - runStart);
			runStart = plainPos;

			CDuiString sTag(pTag, (int)(pEnd - pTag));
			sTag.MakeLower();
			bool ok = true;

			if( sTag == _T("b") ) bold = true;
			else if( sTag == _T("/b") ) bold = false;
			else if( sTag == _T("i") ) italic = true;
			else if( sTag == _T("/i") ) italic = false;
			else if( sTag == _T("u") ) underline = true;
			else if( sTag == _T("/u") ) underline = false;
			else if( sTag == _T("r") ) inRaw = true;
			else if( sTag == _T("/r") ) inRaw = false;
			else if( sTag == _T("s") ) inSelected = true;
			else if( sTag == _T("/s") ) inSelected = false;
			else if( sTag == _T("n") || sTag == _T("n/") ) {
				sPlain += _T('\n');
				plainPos++;
				runStart = plainPos;
				if( paraExtra > 0 ) {
					TInline* pin = new TInline;
					pin->pos = plainPos;
					pin->type = TInline::Spacer;
					pin->width = 0.0f;
					pin->height = (FLOAT)paraExtra;
					aInlines.Add(pin);
					AppendObjectReplacement();
					runStart = plainPos;
				}
			}
			else if( sTag.GetLength() >= 1 && sTag.GetAt(0) == _T('c') && (sTag.GetLength() == 1 || sTag.GetAt(1) == _T(' ') || sTag.GetAt(1) == _T('#')) ) {
				LPCTSTR pc = sTag.GetData() + 1;
				DWORD clr = 0;
				if( ParseColorStringToken(pc, clr) ) curColor = clr;
			}
			else if( sTag == _T("/c") ) curColor = dwColor;
			else if( sTag.GetLength() >= 1 && sTag.GetAt(0) == _T('a') && (sTag.GetLength() == 1 || sTag.GetAt(1) == _T(' ') || sTag.GetAt(1) == _T('=')) ) {
				inLink = true;
				curHref.Empty();
				LPCTSTR pa = sTag.GetData() + 1;
				while( *pa == _T(' ') ) pa++;
				if( *pa != _T('\0') ) curHref = pa;
			}
			else if( sTag == _T("/a") ) {
				inLink = false;
				curHref.Empty();
			}
			else if( sTag.GetLength() >= 1 && sTag.GetAt(0) == _T('f') ) {
				LPCTSTR pf = sTag.GetData() + 1;
				while( *pf == _T(' ') ) pf++;
				if( _istdigit(*pf) || (*pf == _T('-') && _istdigit(*(pf + 1))) ) {
					curFontId = (int)_tcstol(pf, NULL, 10);
					curFontFace.Empty();
					TFontInfo* pFi = pManager ? pManager->GetFontInfo(curFontId) : NULL;
					if( pFi != NULL )
						curFontSize = ScaledFontSize(pFi);
				}
				else if( pManager != NULL && *pf != _T('\0') ) {
					CDuiString sFontName;
					while( *pf && *pf != _T(' ') ) {
						sFontName += *pf++;
					}
					while( *pf == _T(' ') ) pf++;
					int iFontSize = 10;
					if( _istdigit(*pf) )
						iFontSize = (int)_tcstol(pf, const_cast<LPTSTR*>(&pf), 10);
					while( *pf == _T(' ') ) pf++;
					CDuiString sAttr = pf;
					sAttr.MakeLower();
					bool bBold = sAttr.Find(_T("bold")) >= 0;
					bool bUnderline = sAttr.Find(_T("underline")) >= 0;
					bool bItalic = sAttr.Find(_T("italic")) >= 0;
					bool bStrike = sAttr.Find(_T("strikeout")) >= 0;
					HFONT hFont = pManager->GetFont(sFontName.GetData(), iFontSize, bBold, bUnderline, bItalic, bStrike);
					if( hFont == NULL ) {
						static int s_htmlFontId = MAX_FONT_ID;
						hFont = pManager->AddFont(s_htmlFontId--, sFontName.GetData(), iFontSize, bBold, bUnderline, bItalic, bStrike);
					}
					TFontInfo* pFi = pManager->GetFontInfo(hFont);
					if( pFi == NULL ) {
						ok = false;
					}
					else {
						curFontFace = sFontName;
						curFontSize = (pManager != NULL) ? (FLOAT)pManager->GetDPIObj()->Scale(iFontSize) : (FLOAT)iFontSize;
						if( curFontSize < 1.0f ) curFontSize = (FLOAT)iFontSize;
						if( pFi->tm.tmHeight > 0 )
							lineHeight = (FLOAT)(pFi->tm.tmHeight + pFi->tm.tmExternalLeading);
						bold = bBold || bold;
						italic = bItalic || italic;
						underline = bUnderline || underline;
						curFontId = iFont; // face 优先走 fontFace
					}
				}
				else {
					ok = false;
				}
			}
			else if( sTag == _T("/f") ) {
				curFontId = iFont;
				curFontFace.Empty();
				TFontInfo* pFi = pManager ? pManager->GetFontInfo(iFont) : NULL;
				if( pFi != NULL )
					curFontSize = ScaledFontSize(pFi);
			}
			else if( sTag.GetLength() >= 1 && sTag.GetAt(0) == _T('p') ) {
				LPCTSTR pp = sTag.GetData() + 1;
				while( *pp == _T(' ') ) pp++;
				paraExtra = (int)_tcstol(pp, NULL, 10);
				if( plainPos > 0 && sPlain.GetAt(sPlain.GetLength() - 1) != _T('\n') ) {
					sPlain += _T('\n');
					plainPos++;
					runStart = plainPos;
				}
			}
			else if( sTag == _T("/p") ) paraExtra = 0;
			else if( sTag.GetLength() >= 1 && sTag.GetAt(0) == _T('x') ) {
				LPCTSTR px = sTag.GetData() + 1;
				while( *px == _T(' ') ) px++;
				int dx = (int)_tcstol(px, NULL, 10);
				if( dx > 0 ) {
					TInline* pin = new TInline;
					pin->pos = plainPos;
					pin->type = TInline::Spacer;
					pin->width = (FLOAT)dx;
					pin->height = lineHeight;
					aInlines.Add(pin);
					AppendObjectReplacement();
					runStart = plainPos;
				}
			}
			else if( sTag.GetLength() >= 1 && sTag.GetAt(0) == _T('y') ) {
				LPCTSTR py = sTag.GetData() + 1;
				while( *py == _T(' ') ) py++;
				int dy = (int)_tcstol(py, NULL, 10);
				if( dy > 0 ) lineHeight = (FLOAT)dy;
			}
			else if( sTag.GetLength() > 1 && sTag.GetAt(0) == _T('i') && (sTag.GetAt(1) == _T(' ') || sTag.GetAt(1) == _T('\t')) ) {
				LPCTSTR pi = sTag.GetData() + 1;
				while( *pi == _T(' ') ) pi++;
				CDuiString sName;
				while( *pi && *pi != _T(' ') ) sName += *pi++;
				while( *pi == _T(' ') ) pi++;
				int iImageListNum = (int)_tcstol(pi, const_cast<LPTSTR*>(&pi), 10);
				if( iImageListNum <= 0 ) iImageListNum = 1;
				while( *pi == _T(' ') ) pi++;
				int iImageListIndex = (int)_tcstol(pi, const_cast<LPTSTR*>(&pi), 10);
				if( iImageListIndex < 0 || iImageListIndex >= iImageListNum ) iImageListIndex = 0;
				if( sName.IsEmpty() ) {
					ok = false;
				}
				else {
					TInline* pin = new TInline;
					pin->pos = plainPos;
					pin->type = TInline::Image;
					pin->width = pin->height = 0;
					pin->srcLeft = pin->srcTop = 0;
					pin->srcRight = pin->srcBottom = 0;
					pin->imageName = sName;
					// 暂存图集参数到 src*（解析尺寸后再换算）
					pin->srcLeft = (FLOAT)iImageListNum;
					pin->srcTop = (FLOAT)iImageListIndex;
					aInlines.Add(pin);
					AppendObjectReplacement();
					runStart = plainPos;
				}
			}
			else {
				ok = false;
			}

			p = pEnd + 1;
			if( !ok ) {
				FreeAll();
				return false;
			}
			runStart = plainPos;
		}
		if( plainPos > runStart )
			PushRange(runStart, plainPos - runStart);

		if( sPlain.IsEmpty() ) {
			FreeAll();
			if( (uStyle & DT_CALCRECT) != 0 ) {
				rc.right = rc.left;
				rc.bottom = rc.top;
			}
			nLinkRects = 0;
			return true;
		}

		bool bDraw = (uStyle & DT_CALCRECT) == 0;
		if( bDraw && !EnsureD2dDraw() ) {
			FreeAll();
			return false;
		}

		// 解析图片尺寸并准备 bitmap
		for( int i = 0; i < aInlines.GetSize(); ++i ) {
			TInline* pin = static_cast<TInline*>(aInlines.GetAt(i));
			if( pin->type != TInline::Image ) continue;
			if( pManager == NULL ) {
				FreeAll();
				return false;
			}
			const TImageInfo* pInfo = pManager->GetImageEx(pin->imageName.GetData());
			if( pInfo == NULL ) {
				FreeAll();
				return false;
			}
			int iImageListNum = (int)pin->srcLeft;
			int iImageListIndex = (int)pin->srcTop;
			if( iImageListNum <= 0 ) iImageListNum = 1;
			if( iImageListIndex < 0 || iImageListIndex >= iImageListNum ) iImageListIndex = 0;
			int iCellW = pInfo->nX;
			if( iImageListNum > 1 ) iCellW = pInfo->nX / iImageListNum;
			pin->width = (FLOAT)iCellW;
			pin->height = (FLOAT)pInfo->nY;
			pin->srcLeft = (FLOAT)(iCellW * iImageListIndex);
			pin->srcTop = 0.0f;
			pin->srcRight = pin->srcLeft + (FLOAT)iCellW;
			pin->srcBottom = (FLOAT)pInfo->nY;
		}

		IDWriteTextFormat* pBaseFormat = CreateTextFormat(iFont);
		if( pBaseFormat == NULL ) {
			FreeAll();
			return false;
		}

		pBaseFormat->SetTextAlignment(ToDWriteAlign(uStyle));
		pBaseFormat->SetParagraphAlignment(ToDWriteParagraphAlign(uStyle));
		pBaseFormat->SetWordWrapping((uStyle & DT_SINGLELINE) ? DWRITE_WORD_WRAPPING_NO_WRAP : DWRITE_WORD_WRAPPING_WRAP);

		FLOAT maxW = (uStyle & DT_SINGLELINE) ? 16384.0f : (FLOAT)((rc.right - rc.left > 1) ? (rc.right - rc.left) : 1);
		IDWriteTextLayout* pLayout = NULL;
		UINT32 len = (UINT32)sPlain.GetLength();
		if( FAILED(m_pDWrite->CreateTextLayout(sPlain.GetData(), len, pBaseFormat, maxW, 16384.0f, &pLayout)) || pLayout == NULL ) {
			pBaseFormat->Release();
			FreeAll();
			return false;
		}

		CStdPtrArray aBrushes;
		CStdPtrArray aInlineObjs;
		for( int i = 0; i < aRanges.GetSize(); ++i ) {
			TRange* pRange = static_cast<TRange*>(aRanges.GetAt(i));
			DWRITE_TEXT_RANGE tr = { pRange->start, pRange->length };
			if( pRange->bold ) pLayout->SetFontWeight(DWRITE_FONT_WEIGHT_BOLD, tr);
			if( pRange->italic ) pLayout->SetFontStyle(DWRITE_FONT_STYLE_ITALIC, tr);
			if( pRange->underline ) pLayout->SetUnderline(TRUE, tr);
			if( !pRange->fontFace.IsEmpty() ) {
				pLayout->SetFontFamilyName(pRange->fontFace.GetData(), tr);
				if( pRange->fontSize > 0.0f )
					pLayout->SetFontSize(pRange->fontSize, tr);
			}
			else if( pRange->fontId != iFont ) {
				IDWriteTextFormat* pFmt = CreateTextFormat(pRange->fontId);
				if( pFmt != NULL ) {
					pLayout->SetFontSize(pFmt->GetFontSize(), tr);
					WCHAR face[LF_FACESIZE] = { 0 };
					pFmt->GetFontFamilyName(face, LF_FACESIZE);
					pLayout->SetFontFamilyName(face, tr);
					pFmt->Release();
				}
			}
			if( bDraw && m_pRT != NULL && pRange->color != 0 ) {
				ID2D1SolidColorBrush* pBrush = NULL;
				if( SUCCEEDED(m_pRT->CreateSolidColorBrush(ToColorF(pRange->color), &pBrush)) && pBrush != NULL ) {
					pLayout->SetDrawingEffect(pBrush, tr);
					aBrushes.Add(pBrush);
				}
			}
		}

		for( int i = 0; i < aInlines.GetSize(); ++i ) {
			TInline* pin = static_cast<TInline*>(aInlines.GetAt(i));
			DWRITE_TEXT_RANGE tr = { pin->pos, 1 };
			IDWriteInlineObject* pObj = NULL;
			if( pin->type == TInline::Spacer ) {
				pObj = new CDWriteSpacerInline(pin->width, pin->height > 0 ? pin->height : lineHeight);
			}
			else if( pin->type == TInline::Image && pManager != NULL ) {
				const TImageInfo* pInfo = pManager->GetImageEx(pin->imageName.GetData());
				ID2D1Bitmap* pBmp = NULL;
				if( bDraw && m_pRT != NULL && pInfo != NULL && pInfo->hBitmap != NULL )
					pBmp = GetOrCreateBitmap(pInfo->hBitmap, pInfo->nX, pInfo->nY, pInfo->pBits, pInfo->bAlpha);
				D2D1_RECT_F rcSrc = D2D1::RectF(pin->srcLeft, pin->srcTop, pin->srcRight, pin->srcBottom);
				if( pBmp != NULL )
					pObj = new CDWriteImageInline(m_pRT, pBmp, pin->width, pin->height, rcSrc);
				else
					pObj = new CDWriteSpacerInline(pin->width, pin->height);
			}
			if( pObj != NULL ) {
				pLayout->SetInlineObject(pObj, tr);
				aInlineObjs.Add(pObj);
			}
		}

		DWRITE_TEXT_METRICS metrics = { 0 };
		pLayout->GetMetrics(&metrics);
		if( (uStyle & DT_CALCRECT) != 0 ) {
			rc.right = rc.left + (LONG)(metrics.widthIncludingTrailingWhitespace + 0.999f);
			rc.bottom = rc.top + (LONG)(metrics.height + 0.999f);
		}
		else if( m_pRT != NULL ) {
			DWORD dwSelBk = (pManager != NULL) ? pManager->GetDefaultSelectedBackgroundColor() : 0x316AC5FF;
			for( int i = 0; i < aRanges.GetSize(); ++i ) {
				TRange* pRange = static_cast<TRange*>(aRanges.GetAt(i));
				if( !pRange->selected || pRange->length == 0 ) continue;
				DWRITE_HIT_TEST_METRICS* pMetrics = NULL;
				UINT32 actual = 0;
				HRESULT hrHit = pLayout->HitTestTextRange(pRange->start, pRange->length, (FLOAT)rc.left, (FLOAT)rc.top, NULL, 0, &actual);
				if( hrHit == E_NOT_SUFFICIENT_BUFFER && actual > 0 ) {
					pMetrics = new DWRITE_HIT_TEST_METRICS[actual];
					if( SUCCEEDED(pLayout->HitTestTextRange(pRange->start, pRange->length, (FLOAT)rc.left, (FLOAT)rc.top, pMetrics, actual, &actual)) ) {
						ID2D1SolidColorBrush* pSelBrush = NULL;
						if( SUCCEEDED(m_pRT->CreateSolidColorBrush(ToColorF(dwSelBk), &pSelBrush)) && pSelBrush != NULL ) {
							for( UINT32 m = 0; m < actual; ++m ) {
								D2D1_RECT_F r = D2D1::RectF(
									pMetrics[m].left,
									pMetrics[m].top,
									pMetrics[m].left + pMetrics[m].width,
									pMetrics[m].top + pMetrics[m].height);
								m_pRT->FillRectangle(r, pSelBrush);
							}
							pSelBrush->Release();
						}
					}
					delete[] pMetrics;
				}
			}
			ID2D1SolidColorBrush* pDefBrush = GetBrush(dwColor);
			if( pDefBrush != NULL )
				m_pRT->DrawTextLayout(D2D1::Point2F((FLOAT)rc.left, (FLOAT)rc.top), pLayout, pDefBrush, D2D1_DRAW_TEXT_OPTIONS_CLIP);
		}

		int nOutLinks = 0;
		if( pLinks != NULL && sLinks != NULL ) {
			for( int i = 0; i < aRanges.GetSize() && nOutLinks < nLinkRects; ++i ) {
				TRange* pRange = static_cast<TRange*>(aRanges.GetAt(i));
				if( !pRange->link ) continue;
				FLOAT x, y;
				DWRITE_HIT_TEST_METRICS hm = { 0 };
				if( SUCCEEDED(pLayout->HitTestTextPosition(pRange->start, FALSE, &x, &y, &hm)) ) {
					FLOAT x2, y2;
					DWRITE_HIT_TEST_METRICS hm2 = { 0 };
					UINT32 endPos = pRange->start + (pRange->length > 0 ? pRange->length - 1 : 0);
					pLayout->HitTestTextPosition(endPos, FALSE, &x2, &y2, &hm2);
					RECT rcLink = {
						rc.left + (LONG)hm.left,
						rc.top + (LONG)hm.top,
						rc.left + (LONG)(hm2.left + hm2.width + 0.999f),
						rc.top + (LONG)(hm.top + hm.height + 0.999f)
					};
					pLinks[nOutLinks] = rcLink;
					sLinks[nOutLinks] = pRange->href;
					nOutLinks++;
				}
			}
		}
		nLinkRects = nOutLinks;

		for( int i = 0; i < aBrushes.GetSize(); ++i )
			static_cast<ID2D1SolidColorBrush*>(aBrushes.GetAt(i))->Release();
		for( int i = 0; i < aInlineObjs.GetSize(); ++i )
			static_cast<IDWriteInlineObject*>(aInlineObjs.GetAt(i))->Release();
		pLayout->Release();
		pBaseFormat->Release();
		FreeAll();
		return true;
	}

	SIZE CD2dRenderContext::GetTextSize(LPCTSTR pstrText, int iFont, UINT uStyle)
	{
		RECT rc = { 0, 0, 9999, 9999 };
		IDWriteTextFormat* pFormat = CreateTextFormat(iFont);
		if( MeasureTextDWrite(rc, pstrText, pFormat, uStyle | DT_CALCRECT) ) {
			SIZE sz = { rc.right - rc.left, rc.bottom - rc.top };
			if( pFormat != NULL ) pFormat->Release();
			return sz;
		}
		if( pFormat != NULL ) pFormat->Release();
		return m_gdiFallback.GetTextSize(pstrText, iFont, uStyle);
	}

	bool CD2dRenderContext::DrawImageInfo(const RECT& rcItem, const RECT& rcPaint, const TDrawInfo* pDrawInfo, HINSTANCE instance)
	{
		CPaintManagerUI* pManager = GetManager();
		if( pManager == NULL || pDrawInfo == NULL ) return false;

		RECT rcDest = rcItem;
		if( pDrawInfo->rcDest.left != 0 || pDrawInfo->rcDest.top != 0 ||
			pDrawInfo->rcDest.right != 0 || pDrawInfo->rcDest.bottom != 0 ) {
			rcDest.left = rcItem.left + pDrawInfo->rcDest.left;
			rcDest.top = rcItem.top + pDrawInfo->rcDest.top;
			rcDest.right = rcItem.left + pDrawInfo->rcDest.right;
			if( rcDest.right > rcItem.right ) rcDest.right = rcItem.right;
			rcDest.bottom = rcItem.top + pDrawInfo->rcDest.bottom;
			if( rcDest.bottom > rcItem.bottom ) rcDest.bottom = rcItem.bottom;
		}
		if( pDrawInfo->szImage.cx > 0 && pDrawInfo->szImage.cy > 0 ) {
			SIZE szImage = pManager->GetDPIObj()->Scale(pDrawInfo->szImage);
			RECT rcPadding = pManager->GetDPIObj()->Scale(pDrawInfo->rcPadding);
			MakeImageDest(rcItem, szImage, pDrawInfo->sAlign, rcPadding, rcDest);
		}

		if( pDrawInfo->bGdiplus || pDrawInfo->uRotate != 0 ) {
			const TImageInfo* dataRot = pManager->GetImageEx(
				pDrawInfo->sImageName.GetData(),
				pDrawInfo->sResType.IsEmpty() ? NULL : pDrawInfo->sResType.GetData(),
				pDrawInfo->dwMask, false, pDrawInfo->bGdiplus, instance);
			if( dataRot == NULL ) return false;

			if( pDrawInfo->bGdiplus && dataRot->pImage != NULL ) {
				if( !EnsureD2dDraw() ) {
					FlushToGdi();
					return m_gdiFallback.DrawImageInfo(rcItem, rcPaint, pDrawInfo, instance);
				}
				ID2D1Bitmap* pBitmap = GetOrCreateBitmapFromGdiplus(dataRot->pImage);
				if( pBitmap == NULL ) {
					FlushToGdi();
					return m_gdiFallback.DrawImageInfo(rcItem, rcPaint, pDrawInfo, instance);
				}
				float opacity = (pDrawInfo->uFade >= 255) ? 1.0f : ((float)pDrawInfo->uFade / 255.0f);
				if( pDrawInfo->uRotate != 0 ) {
					D2D1_POINT_2F center = D2D1::Point2F(
						(rcDest.left + rcDest.right) * 0.5f,
						(rcDest.top + rcDest.bottom) * 0.5f);
					m_pRT->SetTransform(D2D1::Matrix3x2F::Rotation((FLOAT)pDrawInfo->uRotate, center));
				}
				m_pRT->DrawBitmap(pBitmap, ToRectF(rcDest), opacity, D2D1_BITMAP_INTERPOLATION_MODE_LINEAR);
				if( pDrawInfo->uRotate != 0 )
					m_pRT->SetTransform(D2D1::Matrix3x2F::Identity());
				return true;
			}

			if( pDrawInfo->uRotate != 0 && dataRot->hBitmap != NULL ) {
				if( !EnsureD2dDraw() ) {
					FlushToGdi();
					return m_gdiFallback.DrawImageInfo(rcItem, rcPaint, pDrawInfo, instance);
				}
				bool bAlpha = pManager->IsLayered() ? true : dataRot->bAlpha;
				ID2D1Bitmap* pBitmap = GetOrCreateBitmap(dataRot->hBitmap, dataRot->nX, dataRot->nY, dataRot->pBits, bAlpha);
				if( pBitmap == NULL ) {
					FlushToGdi();
					return m_gdiFallback.DrawImageInfo(rcItem, rcPaint, pDrawInfo, instance);
				}
				RECT rcBmpPart = pDrawInfo->rcSource;
				if( rcBmpPart.left == 0 && rcBmpPart.right == 0 && rcBmpPart.top == 0 && rcBmpPart.bottom == 0 ) {
					rcBmpPart.right = dataRot->nX;
					rcBmpPart.bottom = dataRot->nY;
				}
				D2D1_POINT_2F center = D2D1::Point2F(
					(rcDest.left + rcDest.right) * 0.5f,
					(rcDest.top + rcDest.bottom) * 0.5f);
				m_pRT->SetTransform(D2D1::Matrix3x2F::Rotation((FLOAT)pDrawInfo->uRotate, center));
				bool ok = DrawImageD2d(pBitmap, rcDest, rcPaint, rcBmpPart, pDrawInfo->rcCorner,
					pDrawInfo->uFade, pDrawInfo->bHole, pDrawInfo->bTiledX, pDrawInfo->bTiledY);
				m_pRT->SetTransform(D2D1::Matrix3x2F::Identity());
				if( !ok ) {
					FlushToGdi();
					return m_gdiFallback.DrawImageInfo(rcItem, rcPaint, pDrawInfo, instance);
				}
				return true;
			}

			FlushToGdi();
			return m_gdiFallback.DrawImageInfo(rcItem, rcPaint, pDrawInfo, instance);
		}

		const TImageInfo* data = pManager->GetImageEx(
			pDrawInfo->sImageName.GetData(),
			pDrawInfo->sResType.IsEmpty() ? NULL : pDrawInfo->sResType.GetData(),
			pDrawInfo->dwMask, false, false, instance);
		if( data == NULL ) return false;

		RECT rcBmpPart = pDrawInfo->rcSource;
		if( rcBmpPart.left == 0 && rcBmpPart.right == 0 && rcBmpPart.top == 0 && rcBmpPart.bottom == 0 ) {
			rcBmpPart.right = data->nX;
			rcBmpPart.bottom = data->nY;
		}
		if( rcBmpPart.right > data->nX ) rcBmpPart.right = data->nX;
		if( rcBmpPart.bottom > data->nY ) rcBmpPart.bottom = data->nY;

		RECT rcTemp = { 0 };
		if( !::IntersectRect(&rcTemp, &rcDest, &rcItem) ) return true;
		if( !::IntersectRect(&rcTemp, &rcDest, &rcPaint) ) return true;

		bool bAlpha = pManager->IsLayered() ? true : data->bAlpha;
		(void)bAlpha;
		DrawImage(data, rcDest, rcPaint, rcBmpPart, pDrawInfo->rcCorner, pDrawInfo->uFade, pDrawInfo->bHole, pDrawInfo->bTiledX, pDrawInfo->bTiledY);
		return true;
	}

	bool CD2dRenderContext::DrawImageString(const RECT& rcItem, const RECT& rcPaint, LPCTSTR pStrImage, LPCTSTR pStrModify, HINSTANCE instance)
	{
		CPaintManagerUI* pManager = GetManager();
		if( pManager == NULL ) return false;
		const TDrawInfo* pDrawInfo = pManager->GetDrawInfo(pStrImage, pStrModify);
		return DrawImageInfo(rcItem, rcPaint, pDrawInfo, instance);
	}

#if DUILIB_GDI_INTEROP
	void CD2dRenderContext::DrawImage(HBITMAP hBitmap, const RECT& rc, const RECT& rcPaint, const RECT& rcBmpPart, const RECT& rcCorners, bool bAlpha, UINT uFade, bool hole, bool xtiled, bool ytiled)
	{
		if( hBitmap == NULL ) return;

		// RichEdit 等离屏文字：不透明 1:1 直接 GDI BitBlt，避免 D2D 上传/采样发糊
		const bool bOpaqueCopy =
			!bAlpha && !hole && !xtiled && !ytiled && uFade >= 255
			&& rcCorners.left == 0 && rcCorners.top == 0
			&& rcCorners.right == 0 && rcCorners.bottom == 0
			&& (rc.right - rc.left) == (rcBmpPart.right - rcBmpPart.left)
			&& (rc.bottom - rc.top) == (rcBmpPart.bottom - rcBmpPart.top);
		if( bOpaqueCopy ) {
			FlushToGdi();
			m_gdiFallback.DrawImage(hBitmap, rc, rcPaint, rcBmpPart, rcCorners, false, uFade, hole, xtiled, ytiled);
			m_bPixelsDirty = true;
			return;
		}

		if( !EnsureD2dDraw() ) {
			FlushToGdi();
			m_gdiFallback.DrawImage(hBitmap, rc, rcPaint, rcBmpPart, rcCorners, bAlpha, uFade, hole, xtiled, ytiled);
			return;
		}

		BITMAP bm = { 0 };
		::GetObject(hBitmap, sizeof(bm), &bm);
		// CreateDIBSection 有 bmBits：直接上传，避免 GetDIBits(BI_RGB) 弄丢 alpha
		LPBYTE pBits = (bm.bmBits != NULL) ? (LPBYTE)bm.bmBits : NULL;
		ID2D1Bitmap* pBitmap = GetOrCreateBitmap(hBitmap, bm.bmWidth, bm.bmHeight, pBits, bAlpha);
		if( pBitmap == NULL || !DrawImageD2d(pBitmap, rc, rcPaint, rcBmpPart, rcCorners, uFade, hole, xtiled, ytiled) ) {
			FlushToGdi();
			m_gdiFallback.DrawImage(hBitmap, rc, rcPaint, rcBmpPart, rcCorners, bAlpha, uFade, hole, xtiled, ytiled);
		}
	}
#endif

	void CD2dRenderContext::DrawImage(const TImageInfo* pImageInfo, const RECT& rc, const RECT& rcPaint, const RECT& rcBmpPart, const RECT& rcCorners, UINT uFade, bool hole, bool xtiled, bool ytiled)
	{
		if( pImageInfo == NULL || pImageInfo->hBitmap == NULL ) return;
		bool bAlpha = (GetManager() != NULL && GetManager()->IsLayered()) ? true : pImageInfo->bAlpha;
		if( !EnsureD2dDraw() ) {
			FlushToGdi();
			m_gdiFallback.DrawImage(pImageInfo, rc, rcPaint, rcBmpPart, rcCorners, uFade, hole, xtiled, ytiled);
			return;
		}

		ID2D1Bitmap* pBitmap = GetOrCreateBitmap(pImageInfo->hBitmap, pImageInfo->nX, pImageInfo->nY, pImageInfo->pBits, bAlpha);
		if( pBitmap == NULL || !DrawImageD2d(pBitmap, rc, rcPaint, rcBmpPart, rcCorners, uFade, hole, xtiled, ytiled) ) {
			FlushToGdi();
			m_gdiFallback.DrawImage(pImageInfo, rc, rcPaint, rcBmpPart, rcCorners, uFade, hole, xtiled, ytiled);
		}
	}

	void CD2dRenderContext::DrawGdiplusImage(void* pGdiplusImage, int x, int y, int cx, int cy)
	{
		if( pGdiplusImage == NULL ) return;
		if( !EnsureD2dDraw() ) {
			FlushToGdi();
			m_gdiFallback.DrawGdiplusImage(pGdiplusImage, x, y, cx, cy);
			return;
		}
		ID2D1Bitmap* pBitmap = GetOrCreateBitmapFromGdiplus(pGdiplusImage);
		if( pBitmap == NULL ) {
			FlushToGdi();
			m_gdiFallback.DrawGdiplusImage(pGdiplusImage, x, y, cx, cy);
			return;
		}
		RECT rcDest = { x, y, x + cx, y + cy };
		D2D1_SIZE_U sz = pBitmap->GetPixelSize();
		RECT rcSrc = { 0, 0, (LONG)sz.width, (LONG)sz.height };
		DrawBitmapRect(pBitmap, rcDest, rcSrc, 1.0f);
	}

	void CD2dRenderContext::DrawGdiplusImageRotated(void* pGdiplusImage, const RECT& rc, float angleDegrees)
	{
		if( pGdiplusImage == NULL ) return;
		if( !EnsureD2dDraw() ) {
			FlushToGdi();
			m_gdiFallback.DrawGdiplusImageRotated(pGdiplusImage, rc, angleDegrees);
			return;
		}
		ID2D1Bitmap* pBitmap = GetOrCreateBitmapFromGdiplus(pGdiplusImage);
		if( pBitmap == NULL ) {
			FlushToGdi();
			m_gdiFallback.DrawGdiplusImageRotated(pGdiplusImage, rc, angleDegrees);
			return;
		}
		D2D1_POINT_2F center = D2D1::Point2F(
			(rc.left + rc.right) * 0.5f,
			(rc.top + rc.bottom) * 0.5f);
		m_pRT->SetTransform(D2D1::Matrix3x2F::Rotation(angleDegrees, center));
		D2D1_SIZE_U sz = pBitmap->GetPixelSize();
		RECT rcSrc = { 0, 0, (LONG)sz.width, (LONG)sz.height };
		DrawBitmapRect(pBitmap, rc, rcSrc, 1.0f);
		m_pRT->SetTransform(D2D1::Matrix3x2F::Identity());
	}

	ID2D1Bitmap* CD2dRenderContext::CreateTempBitmapFromHBitmap(HBITMAP hBitmap, bool bAlpha)
	{
		if( hBitmap == NULL || m_pRT == NULL ) return NULL;
		BITMAP bm = { 0 };
		if( !::GetObject(hBitmap, sizeof(bm), &bm) || bm.bmWidth <= 0 || bm.bmHeight <= 0 )
			return NULL;
		const int nX = bm.bmWidth;
		const int nY = bm.bmHeight;
		BITMAPINFO bmi = {};
		bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
		bmi.bmiHeader.biWidth = nX;
		bmi.bmiHeader.biHeight = -nY; // top-down
		bmi.bmiHeader.biPlanes = 1;
		bmi.bmiHeader.biBitCount = 32;
		bmi.bmiHeader.biCompression = BI_RGB;
		BYTE* pTempBits = new BYTE[nX * nY * 4];
		HDC hScreen = ::GetDC(NULL);
		int nCopied = ::GetDIBits(hScreen, hBitmap, 0, nY, pTempBits, &bmi, DIB_RGB_COLORS);
		::ReleaseDC(NULL, hScreen);
		if( nCopied == 0 ) {
			delete[] pTempBits;
			return NULL;
		}
		// StretchBlt(SRCCOPY) 按不透明拷贝；忽略源 alpha，与 GDI 一致
		D2D1_BITMAP_PROPERTIES props = D2D1::BitmapProperties(
			D2D1::PixelFormat(DXGI_FORMAT_B8G8R8A8_UNORM,
				bAlpha ? D2D1_ALPHA_MODE_PREMULTIPLIED : D2D1_ALPHA_MODE_IGNORE));
		ID2D1Bitmap* pBitmap = NULL;
		HRESULT hr = m_pRT->CreateBitmap(
			D2D1::SizeU((UINT32)nX, (UINT32)nY),
			pTempBits,
			(UINT32)(nX * 4),
			props,
			&pBitmap);
		delete[] pTempBits;
		if( FAILED(hr) ) return NULL;
		return pBitmap;
	}

	void CD2dRenderContext::StretchBlit(void* srcNative, int x, int y, int cx, int cy, int xSrc, int ySrc, int cxSrc, int cySrc, int mode)
	{
		(void)mode;
		if( srcNative == NULL || cx <= 0 || cy <= 0 || cxSrc <= 0 || cySrc <= 0 ) return;
		if( !EnsureD2dDraw() ) {
			FlushToGdi();
			m_gdiFallback.StretchBlit(srcNative, x, y, cx, cy, xSrc, ySrc, cxSrc, cySrc, mode);
			return;
		}
		HDC hSrc = reinterpret_cast<HDC>(srcNative);
		HBITMAP hBmp = (HBITMAP)::GetCurrentObject(hSrc, OBJ_BITMAP);
		if( hBmp == NULL ) {
			FlushToGdi();
			m_gdiFallback.StretchBlit(srcNative, x, y, cx, cy, xSrc, ySrc, cxSrc, cySrc, mode);
			return;
		}
		ID2D1Bitmap* pBitmap = CreateTempBitmapFromHBitmap(hBmp, false);
		if( pBitmap == NULL ) {
			FlushToGdi();
			m_gdiFallback.StretchBlit(srcNative, x, y, cx, cy, xSrc, ySrc, cxSrc, cySrc, mode);
			return;
		}
		RECT rcDest = { x, y, x + cx, y + cy };
		RECT rcSrc = { xSrc, ySrc, xSrc + cxSrc, ySrc + cySrc };
		DrawBitmapRect(pBitmap, rcDest, rcSrc, 1.0f);
		pBitmap->Release();
	}

	/////////////////////////////////////////////////////////////////////////////////////
	//
	// CD2dRenderSurface

	CD2dRenderSurface::CD2dRenderSurface(CD2dRenderDevice* pDevice)
		: m_pDevice(pDevice)
		, m_pFactory(pDevice != NULL ? pDevice->GetD2dFactory() : NULL)
		, m_pBitmapRT(NULL)
		, m_pHwndRT(NULL)
		, m_hWndTarget(NULL)
		, m_bWindowTargetDisabled(false)
		, m_pSwapChain(NULL)
		, m_pCompCtx(NULL)
		, m_pCompTargetBmp(NULL)
		, m_pCompTarget(NULL)
		, m_pCompVisual(NULL)
		, m_hCompWnd(NULL)
		, m_nCompWidth(0)
		, m_nCompHeight(0)
		, m_bCompDisabled(false)
		, m_bCompDirectDraw(false)
		, m_bCompFrameActive(false)
		, m_bPendingClear(false)
		, m_bPostProcessOnGdi(false)
		, m_bDirtyValid(false)
	{
		::SetRectEmpty(&m_rcDirty);
	}

	CD2dRenderSurface::~CD2dRenderSurface()
	{
		DUI_EXIT_SCOPE(L"~CD2dRenderSurface");
		Destroy();
	}

	void CD2dRenderSurface::DestroyBitmapRT()
	{
		if( m_pBitmapRT != NULL ) {
			if( m_pDevice != NULL ) m_pDevice->InvalidateBitmapCacheForRT(m_pBitmapRT);
			m_pBitmapRT->Release();
			m_pBitmapRT = NULL;
		}
	}

	void CD2dRenderSurface::DestroyHwndRT()
	{
		if( m_pHwndRT != NULL ) {
			if( m_pDevice != NULL ) m_pDevice->InvalidateBitmapCacheForRT(m_pHwndRT);
			m_pHwndRT->Release();
			m_pHwndRT = NULL;
		}
		m_hWndTarget = NULL;
	}

	bool CD2dRenderSurface::EnsureBitmapRT()
	{
		if( m_pBitmapRT != NULL ) return true;
		if( m_pFactory == NULL || !m_gdiFallback.IsValid() ) return false;

		D2D1_RENDER_TARGET_PROPERTIES props = GdiCompatRtProps();

		ID2D1DCRenderTarget* pTempDC = NULL;
		HRESULT hr = m_pFactory->CreateDCRenderTarget(&props, &pTempDC);
		if( FAILED(hr) || pTempDC == NULL ) return false;

		RECT rcBind = { 0, 0, m_gdiFallback.GetWidth(), m_gdiFallback.GetHeight() };
		hr = pTempDC->BindDC(reinterpret_cast<HDC>(m_gdiFallback.GetNativeTarget()), &rcBind);
		if( FAILED(hr) ) {
			pTempDC->Release();
			return false;
		}

		D2D1_SIZE_F sizeF = D2D1::SizeF((FLOAT)m_gdiFallback.GetWidth(), (FLOAT)m_gdiFallback.GetHeight());
		D2D1_SIZE_U sizeU = D2D1::SizeU((UINT32)m_gdiFallback.GetWidth(), (UINT32)m_gdiFallback.GetHeight());
		hr = pTempDC->CreateCompatibleRenderTarget(
			sizeF, sizeU,
			D2D1::PixelFormat(DXGI_FORMAT_B8G8R8A8_UNORM, D2D1_ALPHA_MODE_PREMULTIPLIED),
			D2D1_COMPATIBLE_RENDER_TARGET_OPTIONS_GDI_COMPATIBLE,
			&m_pBitmapRT);
		pTempDC->Release();
		return SUCCEEDED(hr) && m_pBitmapRT != NULL;
	}

	bool CD2dRenderSurface::EnsureWindowTarget(HWND hWnd, int width, int height)
	{
		if( m_bWindowTargetDisabled ) return false;
		if( hWnd == NULL || width <= 0 || height <= 0 || m_pFactory == NULL ) return false;
		if( !::IsWindow(hWnd) ) return false;

		if( !m_gdiFallback.Ensure(width, height, NULL) ) return false;

		if( m_pHwndRT != NULL && m_hWndTarget == hWnd ) {
			D2D1_SIZE_U sz = m_pHwndRT->GetPixelSize();
			if( sz.width == (UINT32)width && sz.height == (UINT32)height ) return true;
			HRESULT hrResize = m_pHwndRT->Resize(D2D1::SizeU((UINT32)width, (UINT32)height));
			if( FAILED(hrResize) ) {
				DisableWindowTarget();
				return false;
			}
			return true;
		}

		DestroyHwndRT();
		DestroyBitmapRT();

		D2D1_RENDER_TARGET_PROPERTIES rtp = GdiCompatRtProps();
		D2D1_HWND_RENDER_TARGET_PROPERTIES hrtp = D2D1::HwndRenderTargetProperties(
			hWnd, D2D1::SizeU((UINT32)width, (UINT32)height), D2D1_PRESENT_OPTIONS_NONE);
		HRESULT hr = m_pFactory->CreateHwndRenderTarget(&rtp, &hrtp, &m_pHwndRT);
		if( FAILED(hr) || m_pHwndRT == NULL ) {
			m_pHwndRT = NULL;
			m_bWindowTargetDisabled = true;
			EnsureBitmapRT();
			return false;
		}
		m_hWndTarget = hWnd;
		return true;
	}

	void CD2dRenderSurface::DisableWindowTarget()
	{
		DestroyHwndRT();
		m_bWindowTargetDisabled = true;
		// 不在这里 EnsureBitmapRT：避免 Present 用一张未绘制的空 RT 把窗口盖成黑色。
		// 下一帧 Ensure()/EnsureBitmapRT 会按离屏路径重建。
	}

	void CD2dRenderSurface::ClearPaintRect(const RECT& rcPaint, const RECT& rcClient)
	{
		SetDirtyRect(rcPaint);
		m_gdiFallback.ClearPaintRect(rcPaint, rcClient);
		if( m_pHwndRT != NULL ) return;

		RECT rcDest = rcPaint;
		if( rcDest.left < rcClient.left ) rcDest.left = rcClient.left;
		if( rcDest.top < rcClient.top ) rcDest.top = rcClient.top;
		if( rcDest.right > rcClient.right ) rcDest.right = rcClient.right;
		if( rcDest.bottom > rcClient.bottom ) rcDest.bottom = rcClient.bottom;
		if( rcDest.left >= rcDest.right || rcDest.top >= rcDest.bottom ) return;
		SetDirtyRect(rcDest);

		// 零拷贝 Comp：延后到首次 BeginDraw 时 Clear DXGI
		if( m_bCompDirectDraw ) {
			m_bPendingClear = true;
			return;
		}

		if( !EnsureBitmapRT() || m_pBitmapRT == NULL ) return;
		m_pBitmapRT->BeginDraw();
		m_pBitmapRT->PushAxisAlignedClip(
			D2D1::RectF((FLOAT)rcDest.left, (FLOAT)rcDest.top, (FLOAT)rcDest.right, (FLOAT)rcDest.bottom),
			D2D1_ANTIALIAS_MODE_ALIASED);
		m_pBitmapRT->Clear(D2D1::ColorF(0, 0, 0, 0));
		m_pBitmapRT->PopAxisAlignedClip();
		m_pBitmapRT->EndDraw();
	}

	void CD2dRenderSurface::ClearAll()
	{
		m_gdiFallback.ClearAll();
		RECT rc = { 0, 0, GetWidth(), GetHeight() };
		SetDirtyRect(rc);
		if( m_pHwndRT != NULL ) return;
		if( m_bCompDirectDraw ) {
			m_bPendingClear = true;
			return;
		}
		if( !EnsureBitmapRT() || m_pBitmapRT == NULL ) return;
		m_pBitmapRT->BeginDraw();
		m_pBitmapRT->Clear(D2D1::ColorF(0, 0, 0, 0));
		m_pBitmapRT->EndDraw();
	}

	void CD2dRenderSurface::FixLayeredAlpha(const RECT& rcPaint, const RECT& rcClient)
	{
		// 零拷贝 Comp：像素已在 DXGI，D2D 预乘 alpha 无需再扫 GDI bits
		if( m_bCompDirectDraw && !m_bPostProcessOnGdi ) return;
		m_gdiFallback.FixLayeredAlpha(rcPaint, rcClient);
	}

	void CD2dRenderSurface::ApplyRoundCornerMask(int radiusX, int radiusY)
	{
		if( radiusX <= 0 && radiusY <= 0 ) return;
		// Comp 直绘时先拉回 GDI 再遮罩，Present 走 postprocess 回写
		if( m_bCompDirectDraw )
			CopyBackendToPixels();
		m_gdiFallback.ApplyRoundCornerMask(radiusX, radiusY);
		if( m_bCompDirectDraw )
			m_bPostProcessOnGdi = true;
	}

	void CD2dRenderSurface::ApplyLayeredMask(IRenderSurface* pMask, const RECT& rcPaint, const RECT& rcClient)
	{
		// Comp 帧内容在 DXGI：先把脏区拉回 GDI 再做遮罩
		if( m_bCompDirectDraw )
			CopyBackendToPixels();
		if( ApplyLayeredMaskD2d(pMask, rcPaint, rcClient) ) {
			m_bPostProcessOnGdi = true;
			return;
		}
		m_gdiFallback.ApplyLayeredMask(pMask, rcPaint, rcClient);
		m_bPostProcessOnGdi = true;
	}

	bool CD2dRenderSurface::Ensure(int width, int height, void* refNative)
	{
		if( m_pHwndRT != NULL ) {
			// 已切 HWND 直出时，Ensure 只维护 GDI 互操作缓冲
			if( !m_gdiFallback.Ensure(width, height, refNative) ) return false;
			D2D1_SIZE_U sz = m_pHwndRT->GetPixelSize();
			if( sz.width != (UINT32)width || sz.height != (UINT32)height )
				m_pHwndRT->Resize(D2D1::SizeU((UINT32)width, (UINT32)height));
			return true;
		}

		bool needNewRT = !m_gdiFallback.IsValid()
			|| m_gdiFallback.GetWidth() != width
			|| m_gdiFallback.GetHeight() != height;
		if( !m_gdiFallback.Ensure(width, height, refNative) ) {
			DestroyBitmapRT();
			return false;
		}
		if( needNewRT ) DestroyBitmapRT();
		EnsureBitmapRT();
		return m_gdiFallback.IsValid();
	}

	void CD2dRenderSurface::Destroy()
	{
		DUI_EXIT_SCOPE(L"CD2dRenderSurface::Destroy");
		{
			DUI_EXIT_SCOPE(L"Destroy: Composition");
			DestroyComposition();
		}
		{
			DUI_EXIT_SCOPE(L"Destroy: HwndRT");
			DestroyHwndRT();
		}
		{
			DUI_EXIT_SCOPE(L"Destroy: BitmapRT");
			DestroyBitmapRT();
		}
		{
			DUI_EXIT_SCOPE(L"Destroy: GdiFallback");
			m_gdiFallback.Destroy();
		}
	}

	bool CD2dRenderSurface::IsValid() const { return m_gdiFallback.IsValid() || m_pHwndRT != NULL || m_bCompDirectDraw; }
	int CD2dRenderSurface::GetWidth() const { return m_gdiFallback.GetWidth(); }
	int CD2dRenderSurface::GetHeight() const { return m_gdiFallback.GetHeight(); }
	BYTE* CD2dRenderSurface::GetBits() { return m_gdiFallback.GetBits(); }
	void* CD2dRenderSurface::GetNativeTarget() { return m_gdiFallback.GetNativeTarget(); }

	void CD2dRenderSurface::SetDirtyRect(const RECT& rc)
	{
		m_rcDirty = rc;
		m_bDirtyValid = true;
	}

	void CD2dRenderSurface::GetDirtyRect(RECT& rc) const
	{
		ResolveDirtyRect(rc);
	}

	void CD2dRenderSurface::ResolveDirtyRect(RECT& rc) const
	{
		if( m_bDirtyValid ) {
			rc = m_rcDirty;
		}
		else {
			rc.left = 0;
			rc.top = 0;
			rc.right = GetWidth();
			rc.bottom = GetHeight();
		}
		if( rc.left < 0 ) rc.left = 0;
		if( rc.top < 0 ) rc.top = 0;
		if( rc.right > GetWidth() ) rc.right = GetWidth();
		if( rc.bottom > GetHeight() ) rc.bottom = GetHeight();
		if( rc.right < rc.left ) rc.right = rc.left;
		if( rc.bottom < rc.top ) rc.bottom = rc.top;
	}

	void* CD2dRenderSurface::GetBackendTarget()
	{
		if( m_pHwndRT != NULL ) return m_pHwndRT;
		if( m_bCompDirectDraw && m_pCompCtx != NULL ) {
			if( !BindCompositionTarget() ) return m_pBitmapRT;
			return m_pCompCtx;
		}
		return m_pBitmapRT;
	}

	bool CD2dRenderSurface::CopyBackendToPixels()
	{
		if( m_pHwndRT != NULL ) return true;
		RECT rcDirty = { 0 };
		ResolveDirtyRect(rcDirty);
		if( rcDirty.left >= rcDirty.right || rcDirty.top >= rcDirty.bottom ) return true;

		if( m_bCompDirectDraw && m_pCompTargetBmp != NULL )
			return CopyCompToGdi(rcDirty);

		if( m_pBitmapRT == NULL || !m_gdiFallback.IsValid() ) return false;

		ID2D1GdiInteropRenderTarget* pInterop = NULL;
		HRESULT hr = m_pBitmapRT->QueryInterface(__uuidof(ID2D1GdiInteropRenderTarget), (void**)&pInterop);
		if( FAILED(hr) || pInterop == NULL ) return false;

		HDC hSrcDC = NULL;
		hr = pInterop->GetDC(D2D1_DC_INITIALIZE_MODE_COPY, &hSrcDC);
		if( FAILED(hr) || hSrcDC == NULL ) {
			pInterop->Release();
			return false;
		}

		HDC hDstDC = reinterpret_cast<HDC>(GetNativeTarget());
		::BitBlt(hDstDC, rcDirty.left, rcDirty.top,
			rcDirty.right - rcDirty.left, rcDirty.bottom - rcDirty.top,
			hSrcDC, rcDirty.left, rcDirty.top, SRCCOPY);
		pInterop->ReleaseDC(NULL);
		pInterop->Release();
		return true;
	}

	bool CD2dRenderSurface::CopyBackendToPixelsViaBitmap()
	{
		if( m_pHwndRT != NULL || m_bCompDirectDraw ) return false;
		if( m_pBitmapRT == NULL || m_pFactory == NULL || !m_gdiFallback.IsValid() ) return false;

		RECT rcDirty = { 0 };
		ResolveDirtyRect(rcDirty);
		if( rcDirty.left >= rcDirty.right || rcDirty.top >= rcDirty.bottom ) return true;

		ID2D1Bitmap* pBitmap = NULL;
		HRESULT hr = m_pBitmapRT->GetBitmap(&pBitmap);
		if( FAILED(hr) || pBitmap == NULL ) return false;

		D2D1_RENDER_TARGET_PROPERTIES props = GdiCompatRtProps();
		ID2D1DCRenderTarget* pDC = NULL;
		hr = m_pFactory->CreateDCRenderTarget(&props, &pDC);
		if( FAILED(hr) || pDC == NULL ) {
			pBitmap->Release();
			return false;
		}

		RECT rcBind = { 0, 0, GetWidth(), GetHeight() };
		HDC hDstDC = reinterpret_cast<HDC>(GetNativeTarget());
		hr = pDC->BindDC(hDstDC, &rcBind);
		if( SUCCEEDED(hr) ) {
			pDC->BeginDraw();
			pDC->PushAxisAlignedClip(
				D2D1::RectF((FLOAT)rcDirty.left, (FLOAT)rcDirty.top, (FLOAT)rcDirty.right, (FLOAT)rcDirty.bottom),
				D2D1_ANTIALIAS_MODE_ALIASED);
			pDC->DrawBitmap(
				pBitmap,
				D2D1::RectF(0, 0, (FLOAT)GetWidth(), (FLOAT)GetHeight()),
				1.0f,
				D2D1_BITMAP_INTERPOLATION_MODE_NEAREST_NEIGHBOR);
			pDC->PopAxisAlignedClip();
			hr = pDC->EndDraw();
		}
		pDC->Release();
		pBitmap->Release();
		return SUCCEEDED(hr);
	}

	bool CD2dRenderSurface::CopyPixelsToBackend()
	{
		if( m_pHwndRT != NULL ) return true;
		RECT rcDirty = { 0 };
		ResolveDirtyRect(rcDirty);
		if( rcDirty.left >= rcDirty.right || rcDirty.top >= rcDirty.bottom ) return true;

		if( m_bCompDirectDraw && m_pCompCtx != NULL )
			return CopyGdiToComp(rcDirty);

		if( m_pBitmapRT == NULL || !m_gdiFallback.IsValid() ) return false;

		ID2D1GdiInteropRenderTarget* pInterop = NULL;
		HRESULT hr = m_pBitmapRT->QueryInterface(__uuidof(ID2D1GdiInteropRenderTarget), (void**)&pInterop);
		if( FAILED(hr) || pInterop == NULL ) return false;

		HDC hDstDC = NULL;
		hr = pInterop->GetDC(D2D1_DC_INITIALIZE_MODE_COPY, &hDstDC);
		if( FAILED(hr) || hDstDC == NULL ) {
			pInterop->Release();
			return false;
		}

		HDC hSrcDC = reinterpret_cast<HDC>(GetNativeTarget());
		::BitBlt(hDstDC, rcDirty.left, rcDirty.top,
			rcDirty.right - rcDirty.left, rcDirty.bottom - rcDirty.top,
			hSrcDC, rcDirty.left, rcDirty.top, SRCCOPY);
		pInterop->ReleaseDC(NULL);
		pInterop->Release();
		return true;
	}

	bool CD2dRenderSurface::PrepareLayeredComposition(HWND hWnd, int width, int height)
	{
		if( m_bCompDisabled ) {
			m_bCompDirectDraw = false;
			return false;
		}
		if( !EnsureComposition(hWnd, width, height) ) {
			m_bCompDirectDraw = false;
			return false;
		}
		m_bCompDirectDraw = true;
		m_bCompFrameActive = false;
		m_bPostProcessOnGdi = false;
		UnbindCompositionTarget();
		return BindCompositionTarget();
	}

	void CD2dRenderSurface::SetLayeredCompositionEnabled(bool bEnable)
	{
		if( bEnable ) {
			m_bCompDisabled = false;
			return;
		}
		m_bCompDisabled = true;
		m_bCompDirectDraw = false;
		m_bCompFrameActive = false;
		DestroyComposition();
	}

	bool CD2dRenderSurface::CreateTopDownBitmap(ID2D1RenderTarget* pRT, int width, int height, const BYTE* pBottomUpBits, ID2D1Bitmap** ppBitmap)
	{
		if( pRT == NULL || pBottomUpBits == NULL || ppBitmap == NULL || width <= 0 || height <= 0 ) return false;
		*ppBitmap = NULL;
		UINT stride = (UINT)width * 4;
		BYTE* pTopDown = new BYTE[stride * (UINT)height];
		for( int y = 0; y < height; ++y )
			memcpy(pTopDown + y * stride, pBottomUpBits + (height - 1 - y) * stride, stride);

		D2D1_BITMAP_PROPERTIES props = D2D1::BitmapProperties(
			D2D1::PixelFormat(DXGI_FORMAT_B8G8R8A8_UNORM, D2D1_ALPHA_MODE_PREMULTIPLIED));
		HRESULT hr = pRT->CreateBitmap(D2D1::SizeU((UINT32)width, (UINT32)height), pTopDown, stride, props, ppBitmap);
		delete[] pTopDown;
		return SUCCEEDED(hr) && *ppBitmap != NULL;
	}

	bool CD2dRenderSurface::ApplyLayeredMaskD2d(IRenderSurface* pMask, const RECT& rcPaint, const RECT& rcClient)
	{
		if( pMask == NULL || m_pFactory == NULL ) return false;
		if( !EnsureBitmapRT() || m_pBitmapRT == NULL ) return false;

		BYTE* pContentBits = GetBits();
		BYTE* pMaskBits = pMask->GetBits();
		if( pContentBits == NULL || pMaskBits == NULL ) return false;

		int width = GetWidth();
		int height = GetHeight();
		if( width <= 0 || height <= 0 ) return false;

		ID2D1Bitmap* pContentBmp = NULL;
		ID2D1Bitmap* pMaskBmp = NULL;
		if( !CreateTopDownBitmap(m_pBitmapRT, width, height, pContentBits, &pContentBmp) ||
			!CreateTopDownBitmap(m_pBitmapRT, width, height, pMaskBits, &pMaskBmp) ) {
			if( pContentBmp ) pContentBmp->Release();
			if( pMaskBmp ) pMaskBmp->Release();
			return false;
		}

		ID2D1BitmapBrush* pContentBrush = NULL;
		HRESULT hr = m_pBitmapRT->CreateBitmapBrush(pContentBmp, &pContentBrush);
		if( FAILED(hr) || pContentBrush == NULL ) {
			pContentBmp->Release();
			pMaskBmp->Release();
			return false;
		}

		RECT rcDest = rcPaint;
		if( rcDest.left < rcClient.left ) rcDest.left = rcClient.left;
		if( rcDest.top < rcClient.top ) rcDest.top = rcClient.top;
		if( rcDest.right > rcClient.right ) rcDest.right = rcClient.right;
		if( rcDest.bottom > rcClient.bottom ) rcDest.bottom = rcClient.bottom;

		m_pBitmapRT->BeginDraw();
		m_pBitmapRT->SetAntialiasMode(D2D1_ANTIALIAS_MODE_ALIASED);
		m_pBitmapRT->PushAxisAlignedClip(
			D2D1::RectF((FLOAT)rcDest.left, (FLOAT)rcDest.top, (FLOAT)rcDest.right, (FLOAT)rcDest.bottom),
			D2D1_ANTIALIAS_MODE_ALIASED);
		m_pBitmapRT->Clear(D2D1::ColorF(0, 0, 0, 0));
		D2D1_RECT_F rcF = D2D1::RectF((FLOAT)rcDest.left, (FLOAT)rcDest.top, (FLOAT)rcDest.right, (FLOAT)rcDest.bottom);
		m_pBitmapRT->FillOpacityMask(
			pMaskBmp,
			pContentBrush,
			D2D1_OPACITY_MASK_CONTENT_GRAPHICS,
			&rcF,
			&rcF);
		m_pBitmapRT->PopAxisAlignedClip();
		m_pBitmapRT->SetAntialiasMode(D2D1_ANTIALIAS_MODE_PER_PRIMITIVE);
		hr = m_pBitmapRT->EndDraw();

		pContentBrush->Release();
		pContentBmp->Release();
		pMaskBmp->Release();
		if( FAILED(hr) ) return false;

		// 回写 GDI bits（UpdateLayeredWindow 仍读 GDI）
		ID2D1GdiInteropRenderTarget* pInterop = NULL;
		hr = m_pBitmapRT->QueryInterface(__uuidof(ID2D1GdiInteropRenderTarget), (void**)&pInterop);
		if( FAILED(hr) || pInterop == NULL ) return false;
		// EndDraw 后不能 GetDC；改为临时 DC RT 画到 GDI
		pInterop->Release();

		D2D1_RENDER_TARGET_PROPERTIES props = GdiCompatRtProps();
		ID2D1DCRenderTarget* pDC = NULL;
		hr = m_pFactory->CreateDCRenderTarget(&props, &pDC);
		if( FAILED(hr) || pDC == NULL ) return false;

		RECT rcBind = { 0, 0, width, height };
		hr = pDC->BindDC(reinterpret_cast<HDC>(GetNativeTarget()), &rcBind);
		if( FAILED(hr) ) {
			pDC->Release();
			return false;
		}

		ID2D1Bitmap* pResult = NULL;
		hr = m_pBitmapRT->GetBitmap(&pResult);
		if( FAILED(hr) || pResult == NULL ) {
			pDC->Release();
			return false;
		}

		pDC->BeginDraw();
		pDC->PushAxisAlignedClip(rcF, D2D1_ANTIALIAS_MODE_ALIASED);
		pDC->DrawBitmap(pResult, rcF, 1.0f, D2D1_BITMAP_INTERPOLATION_MODE_NEAREST_NEIGHBOR, rcF);
		pDC->PopAxisAlignedClip();
		hr = pDC->EndDraw();
		pResult->Release();
		pDC->Release();
		return SUCCEEDED(hr);
	}

	void CD2dRenderSurface::ApplyCompositionWindowStyle(HWND hWnd)
	{
		if( hWnd == NULL || !::IsWindow(hWnd) ) return;
		LONG_PTR ex = ::GetWindowLongPtr(hWnd, GWL_EXSTYLE);
		LONG_PTR neu = (ex | WS_EX_NOREDIRECTIONBITMAP) & ~(LONG_PTR)WS_EX_LAYERED;
		if( neu != ex )
			::SetWindowLongPtr(hWnd, GWL_EXSTYLE, neu);
	}

	void CD2dRenderSurface::ApplyLayeredWindowStyle(HWND hWnd)
	{
		if( hWnd == NULL || !::IsWindow(hWnd) ) return;
		LONG_PTR ex = ::GetWindowLongPtr(hWnd, GWL_EXSTYLE);
		LONG_PTR neu = (ex | WS_EX_LAYERED) & ~(LONG_PTR)WS_EX_NOREDIRECTIONBITMAP;
		if( neu != ex )
			::SetWindowLongPtr(hWnd, GWL_EXSTYLE, neu);
	}

	void CD2dRenderSurface::DestroyComposition()
	{
		DUI_EXIT_SCOPE(L"DestroyComposition");
		{
			DUI_EXIT_SCOPE(L"DComp UnbindCompositionTarget");
			UnbindCompositionTarget();
		}
		{
			DUI_EXIT_SCOPE(L"DComp Release CompCtx");
			if( m_pCompCtx != NULL ) {
				m_pCompCtx->Release();
				m_pCompCtx = NULL;
			}
		}
		{
			DUI_EXIT_SCOPE(L"DComp Release CompTarget");
			if( m_pCompTarget != NULL ) {
				m_pCompTarget->SetRoot(NULL);
				m_pCompTarget->Release();
				m_pCompTarget = NULL;
			}
		}
		{
			DUI_EXIT_SCOPE(L"DComp Release CompVisual");
			if( m_pCompVisual != NULL ) {
				m_pCompVisual->Release();
				m_pCompVisual = NULL;
			}
		}
		{
			DUI_EXIT_SCOPE(L"DComp Release SwapChain");
			if( m_pSwapChain != NULL ) {
				m_pSwapChain->Release();
				m_pSwapChain = NULL;
			}
		}
		m_hCompWnd = NULL;
		m_nCompWidth = 0;
		m_nCompHeight = 0;
		m_bCompDirectDraw = false;
		m_bCompFrameActive = false;
		m_bPendingClear = false;
	}

	void CD2dRenderSurface::UnbindCompositionTarget()
	{
		if( m_pCompCtx != NULL )
			m_pCompCtx->SetTarget(NULL);
		if( m_pCompTargetBmp != NULL ) {
			m_pCompTargetBmp->Release();
			m_pCompTargetBmp = NULL;
		}
		m_bCompFrameActive = false;
	}

	bool CD2dRenderSurface::BindCompositionTarget()
	{
		if( m_pCompCtx == NULL || m_pSwapChain == NULL ) return false;
		if( m_pCompTargetBmp != NULL ) {
			m_bCompFrameActive = true;
			return true;
		}

		IDXGISurface* pDxgiSurface = NULL;
		HRESULT hr = m_pSwapChain->GetBuffer(0, __uuidof(IDXGISurface), (void**)&pDxgiSurface);
		if( FAILED(hr) || pDxgiSurface == NULL ) return false;

		D2D1_BITMAP_PROPERTIES1 bp = D2D1::BitmapProperties1(
			D2D1_BITMAP_OPTIONS_TARGET | D2D1_BITMAP_OPTIONS_CANNOT_DRAW,
			D2D1::PixelFormat(DXGI_FORMAT_B8G8R8A8_UNORM, D2D1_ALPHA_MODE_PREMULTIPLIED));
		hr = m_pCompCtx->CreateBitmapFromDxgiSurface(pDxgiSurface, &bp, &m_pCompTargetBmp);
		pDxgiSurface->Release();
		if( FAILED(hr) || m_pCompTargetBmp == NULL ) {
			m_pCompTargetBmp = NULL;
			return false;
		}
		m_pCompCtx->SetTarget(m_pCompTargetBmp);
		m_bCompFrameActive = true;
		return true;
	}

	void CD2dRenderSurface::OnBackendBeginDraw()
	{
		if( !m_bPendingClear ) return;
		m_bPendingClear = false;
		RECT rcDirty = { 0 };
		ResolveDirtyRect(rcDirty);
		if( rcDirty.left >= rcDirty.right || rcDirty.top >= rcDirty.bottom ) return;

		ID2D1RenderTarget* pRT = NULL;
		if( m_bCompDirectDraw && m_pCompCtx != NULL )
			pRT = m_pCompCtx;
		else if( m_pBitmapRT != NULL )
			pRT = m_pBitmapRT;
		if( pRT == NULL ) return;

		pRT->PushAxisAlignedClip(
			D2D1::RectF((FLOAT)rcDirty.left, (FLOAT)rcDirty.top, (FLOAT)rcDirty.right, (FLOAT)rcDirty.bottom),
			D2D1_ANTIALIAS_MODE_ALIASED);
		pRT->Clear(D2D1::ColorF(0, 0, 0, 0));
		pRT->PopAxisAlignedClip();
	}

	bool CD2dRenderSurface::CopyCompToGdi(const RECT& rcDirty)
	{
		if( m_pCompTargetBmp == NULL || m_pFactory == NULL || !m_gdiFallback.IsValid() ) return false;

		D2D1_RENDER_TARGET_PROPERTIES props = GdiCompatRtProps();
		ID2D1DCRenderTarget* pDC = NULL;
		HRESULT hr = m_pFactory->CreateDCRenderTarget(&props, &pDC);
		if( FAILED(hr) || pDC == NULL ) return false;

		RECT rcBind = { 0, 0, GetWidth(), GetHeight() };
		hr = pDC->BindDC(reinterpret_cast<HDC>(GetNativeTarget()), &rcBind);
		if( FAILED(hr) ) {
			pDC->Release();
			return false;
		}

		D2D1_RECT_F rcF = D2D1::RectF((FLOAT)rcDirty.left, (FLOAT)rcDirty.top, (FLOAT)rcDirty.right, (FLOAT)rcDirty.bottom);
		pDC->BeginDraw();
		pDC->PushAxisAlignedClip(rcF, D2D1_ANTIALIAS_MODE_ALIASED);
		pDC->DrawBitmap(m_pCompTargetBmp, rcF, 1.0f, D2D1_BITMAP_INTERPOLATION_MODE_NEAREST_NEIGHBOR, &rcF);
		pDC->PopAxisAlignedClip();
		hr = pDC->EndDraw();
		pDC->Release();
		return SUCCEEDED(hr);
	}

	bool CD2dRenderSurface::CopyGdiToComp(const RECT& rcDirty)
	{
		if( m_pCompCtx == NULL || !m_gdiFallback.IsValid() ) return false;
		BYTE* pBits = GetBits();
		if( pBits == NULL ) return false;

		int width = GetWidth();
		int height = GetHeight();
		if( width <= 0 || height <= 0 ) return false;

		// 仅上传脏区：从 bottom-up DIB 裁出 top-down 子矩形
		int dw = rcDirty.right - rcDirty.left;
		int dh = rcDirty.bottom - rcDirty.top;
		if( dw <= 0 || dh <= 0 ) return true;

		UINT stride = (UINT)dw * 4;
		BYTE* pTopDown = new BYTE[stride * (UINT)dh];
		for( int y = 0; y < dh; ++y ) {
			int srcY = height - 1 - (rcDirty.top + y);
			const BYTE* pSrc = pBits + (srcY * width + rcDirty.left) * 4;
			memcpy(pTopDown + y * stride, pSrc, stride);
		}

		D2D1_BITMAP_PROPERTIES props = D2D1::BitmapProperties(
			D2D1::PixelFormat(DXGI_FORMAT_B8G8R8A8_UNORM, D2D1_ALPHA_MODE_PREMULTIPLIED));
		ID2D1Bitmap* pBmp = NULL;
		HRESULT hr = m_pCompCtx->CreateBitmap(D2D1::SizeU((UINT32)dw, (UINT32)dh), pTopDown, stride, props, &pBmp);
		delete[] pTopDown;
		if( FAILED(hr) || pBmp == NULL ) return false;

		D2D1_RECT_F rcF = D2D1::RectF((FLOAT)rcDirty.left, (FLOAT)rcDirty.top, (FLOAT)rcDirty.right, (FLOAT)rcDirty.bottom);
		m_pCompCtx->DrawBitmap(pBmp, rcF, 1.0f, D2D1_BITMAP_INTERPOLATION_MODE_NEAREST_NEIGHBOR);
		pBmp->Release();
		return true;
	}

	bool CD2dRenderSurface::EnsureComposition(HWND hWnd, int width, int height)
	{
		if( m_bCompDisabled || hWnd == NULL || width <= 0 || height <= 0 ) return false;
		if( m_pDevice == NULL || !m_pDevice->EnsureGpu() ) {
			m_bCompDisabled = true;
			return false;
		}

		IDCompositionDevice* pDComp = m_pDevice->GetDCompDevice();
		ID2D1Device* pD2dDevice = m_pDevice->GetD2dDevice();
		ID3D11Device* pD3D = m_pDevice->GetD3DDevice();
		IDXGIDevice* pDxgiDevice = m_pDevice->GetDxgiDevice();
		if( pDComp == NULL || pD2dDevice == NULL || pD3D == NULL || pDxgiDevice == NULL ) {
			m_bCompDisabled = true;
			return false;
		}

		if( m_pSwapChain != NULL && m_hCompWnd == hWnd &&
			m_nCompWidth == width && m_nCompHeight == height &&
			m_pCompCtx != NULL && m_pCompTarget != NULL && m_pCompVisual != NULL ) {
			return true;
		}

		// 尺寸变化：优先 ResizeBuffers；HWND 变化则整链重建
		if( m_pSwapChain != NULL && m_hCompWnd == hWnd && m_pCompCtx != NULL ) {
			UnbindCompositionTarget();
			HRESULT hrResize = m_pSwapChain->ResizeBuffers(2, (UINT)width, (UINT)height, DXGI_FORMAT_B8G8R8A8_UNORM, 0);
			if( SUCCEEDED(hrResize) ) {
				m_nCompWidth = width;
				m_nCompHeight = height;
				ApplyCompositionWindowStyle(hWnd);
				return true;
			}
		}

		DestroyComposition();

		DXGI_SWAP_CHAIN_DESC1 desc = {};
		desc.Width = (UINT)width;
		desc.Height = (UINT)height;
		desc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
		desc.SampleDesc.Count = 1;
		desc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
		desc.BufferCount = 2;
		desc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_SEQUENTIAL;
		desc.AlphaMode = DXGI_ALPHA_MODE_PREMULTIPLIED;
		desc.Scaling = DXGI_SCALING_STRETCH;

		IDXGIAdapter* pAdapter = NULL;
		HRESULT hr = pDxgiDevice->GetAdapter(&pAdapter);
		if( FAILED(hr) || pAdapter == NULL ) {
			m_bCompDisabled = true;
			return false;
		}
		IDXGIFactory2* pFactory = NULL;
		hr = pAdapter->GetParent(__uuidof(IDXGIFactory2), (void**)&pFactory);
		pAdapter->Release();
		if( FAILED(hr) || pFactory == NULL ) {
			m_bCompDisabled = true;
			return false;
		}

		hr = pFactory->CreateSwapChainForComposition(pD3D, &desc, NULL, &m_pSwapChain);
		pFactory->Release();
		if( FAILED(hr) || m_pSwapChain == NULL ) {
			m_pSwapChain = NULL;
			m_bCompDisabled = true;
			return false;
		}

		hr = pD2dDevice->CreateDeviceContext(D2D1_DEVICE_CONTEXT_OPTIONS_NONE, &m_pCompCtx);
		if( FAILED(hr) || m_pCompCtx == NULL ) {
			DestroyComposition();
			m_bCompDisabled = true;
			return false;
		}

		hr = pDComp->CreateTargetForHwnd(hWnd, TRUE, &m_pCompTarget);
		if( FAILED(hr) || m_pCompTarget == NULL ) {
			DestroyComposition();
			m_bCompDisabled = true;
			return false;
		}
		hr = pDComp->CreateVisual(&m_pCompVisual);
		if( FAILED(hr) || m_pCompVisual == NULL ) {
			DestroyComposition();
			m_bCompDisabled = true;
			return false;
		}
		hr = m_pCompVisual->SetContent(m_pSwapChain);
		if( FAILED(hr) ) {
			DestroyComposition();
			m_bCompDisabled = true;
			return false;
		}
		hr = m_pCompTarget->SetRoot(m_pCompVisual);
		if( FAILED(hr) ) {
			DestroyComposition();
			m_bCompDisabled = true;
			return false;
		}
		hr = pDComp->Commit();
		if( FAILED(hr) ) {
			DestroyComposition();
			m_bCompDisabled = true;
			return false;
		}

		m_hCompWnd = hWnd;
		m_nCompWidth = width;
		m_nCompHeight = height;
		ApplyCompositionWindowStyle(hWnd);
		return true;
	}

	bool CD2dRenderSurface::PresentLayeredDComp(const RenderPresentParams& params)
	{
		if( params.hWnd == NULL ) return false;
		int width = GetWidth();
		int height = GetHeight();
		if( width <= 0 || height <= 0 ) return false;
		if( !EnsureComposition(params.hWnd, width, height) ) return false;
		if( m_pCompCtx == NULL || m_pSwapChain == NULL || m_pDevice == NULL ) return false;

		IDCompositionDevice* pDComp = m_pDevice->GetDCompDevice();
		if( pDComp == NULL ) return false;

		HRESULT hr = S_OK;
		bool bZeroCopy = m_bCompDirectDraw && m_bCompFrameActive && m_pCompTargetBmp != NULL;

		if( bZeroCopy ) {
			// 遮罩等后处理改过 GDI：把脏区回传 DXGI
			if( m_bPostProcessOnGdi ) {
				m_pCompCtx->BeginDraw();
				RECT rcDirty = { 0 };
				ResolveDirtyRect(rcDirty);
				CopyGdiToComp(rcDirty);
				hr = m_pCompCtx->EndDraw();
				m_bPostProcessOnGdi = false;
				if( FAILED(hr) ) {
					UnbindCompositionTarget();
					return false;
				}
			}
			// 非 255 透明度：拉回 GDI 走 ULW 混合（零拷贝路径不做整屏重调制）
			if( params.nOpacity < 255 ) {
				CopyBackendToPixels();
				UnbindCompositionTarget();
				m_bPostProcessOnGdi = false;
				return PresentLayeredGdi(params);
			}

			// Flip 模型：用 Present1 脏矩形，未更新区域由 DWM 保留上一帧
			RECT rcDirty = { 0 };
			ResolveDirtyRect(rcDirty);
			DXGI_PRESENT_PARAMETERS presentParams = {};
			RECT dirtyRects[1] = { rcDirty };
			bool bPartial = (rcDirty.left > 0 || rcDirty.top > 0 ||
				rcDirty.right < width || rcDirty.bottom < height);
			if( bPartial && rcDirty.left < rcDirty.right && rcDirty.top < rcDirty.bottom ) {
				presentParams.DirtyRectsCount = 1;
				presentParams.pDirtyRects = dirtyRects;
				hr = m_pSwapChain->Present1(1, 0, &presentParams);
			}
			else {
				hr = m_pSwapChain->Present(1, 0);
			}
			if( FAILED(hr) ) {
				UnbindCompositionTarget();
				return false;
			}
			hr = pDComp->Commit();
			UnbindCompositionTarget();
			m_bPostProcessOnGdi = false;
			return SUCCEEDED(hr);
		}

		// 回退：从 BitmapRT blit 到 DXGI（未走零拷贝直绘时）
		if( m_pBitmapRT == NULL ) return false;
		if( !BindCompositionTarget() ) return false;

		ID2D1Bitmap* pSrcBmp = NULL;
		hr = m_pBitmapRT->GetBitmap(&pSrcBmp);
		if( FAILED(hr) || pSrcBmp == NULL ) {
			UnbindCompositionTarget();
			return false;
		}

		m_pCompCtx->BeginDraw();
		m_pCompCtx->Clear(D2D1::ColorF(0, 0, 0, 0));
		float opacity = (params.nOpacity >= 255) ? 1.0f : ((float)params.nOpacity / 255.0f);
		D2D1_RECT_F rcFull = D2D1::RectF(0, 0, (FLOAT)width, (FLOAT)height);
		m_pCompCtx->DrawBitmap(pSrcBmp, rcFull, opacity, D2D1_BITMAP_INTERPOLATION_MODE_NEAREST_NEIGHBOR, rcFull);
		hr = m_pCompCtx->EndDraw();
		pSrcBmp->Release();
		if( FAILED(hr) ) {
			UnbindCompositionTarget();
			return false;
		}

		hr = m_pSwapChain->Present(1, 0);
		if( FAILED(hr) ) {
			UnbindCompositionTarget();
			return false;
		}
		hr = pDComp->Commit();
		UnbindCompositionTarget();
		m_bPostProcessOnGdi = false;
		return SUCCEEDED(hr);
	}

	bool CD2dRenderSurface::PresentLayeredGdi(const RenderPresentParams& params)
	{
		ApplyLayeredWindowStyle(params.hWnd);
		return m_gdiFallback.Present(params);
	}

	bool CD2dRenderSurface::PresentLayered(const RenderPresentParams& params)
	{
		if( params.hWnd == NULL ) return false;

		// 优先 DXGI/DirectComposition（零拷贝或 Bitmap blit）；失败回退 ULW
		if( !m_bCompDisabled && PresentLayeredDComp(params) )
			return true;

		return PresentLayeredGdi(params);
	}

	bool CD2dRenderSurface::Present(const RenderPresentParams& params)
	{
		if( params.bLayered )
			return PresentLayered(params);

		if( m_pHwndRT != NULL ) {
			// EndFrame 已 EndDraw，内容已提交到窗口
			return true;
		}

		// 非分层：优先 BitBlt GDI 离屏缓冲。
		// BitmapRT→窗口 DC 的 DrawBitmap 在预乘 alpha / GetDC 混绘后可能把窗口盖成黑屏；
		// EndFrame 已把 D2D 内容同步到 GDI（或 GetDC 路径直接画在 GDI 上）。
		if( m_gdiFallback.IsValid() && params.hWindowDC != NULL )
			return m_gdiFallback.Present(params);

		if( m_pBitmapRT != NULL && m_pFactory != NULL && params.hWindowDC != NULL ) {
			ID2D1Bitmap* pBitmap = NULL;
			if( SUCCEEDED(m_pBitmapRT->GetBitmap(&pBitmap)) && pBitmap != NULL ) {
				D2D1_RENDER_TARGET_PROPERTIES props = GdiCompatRtProps();
				ID2D1DCRenderTarget* pDC = NULL;
				HRESULT hr = m_pFactory->CreateDCRenderTarget(&props, &pDC);
				if( SUCCEEDED(hr) && pDC != NULL ) {
					RECT rcBind = params.rcClient;
					if( rcBind.right <= rcBind.left || rcBind.bottom <= rcBind.top ) {
						rcBind.left = 0;
						rcBind.top = 0;
						rcBind.right = GetWidth();
						rcBind.bottom = GetHeight();
					}
					hr = pDC->BindDC(params.hWindowDC, &rcBind);
					if( SUCCEEDED(hr) ) {
						pDC->BeginDraw();
						pDC->PushAxisAlignedClip(ToRectF(params.rcPaint), D2D1_ANTIALIAS_MODE_PER_PRIMITIVE);
						pDC->DrawBitmap(
							pBitmap,
							D2D1::RectF(0, 0, (FLOAT)GetWidth(), (FLOAT)GetHeight()),
							1.0f,
							D2D1_BITMAP_INTERPOLATION_MODE_NEAREST_NEIGHBOR);
						pDC->PopAxisAlignedClip();
						hr = pDC->EndDraw();
					}
					pDC->Release();
					pBitmap->Release();
					if( SUCCEEDED(hr) ) return true;
				}
				else if( pBitmap != NULL ) {
					pBitmap->Release();
				}
			}
		}
		return false;
	}

	/////////////////////////////////////////////////////////////////////////////////////
	//
	// CD2dRenderDevice

	static CD2dRenderDevice g_d2dRenderDevice;

	CD2dRenderDevice::CD2dRenderDevice()
		: m_pFactory(NULL)
		, m_pFactory1(NULL)
		, m_pDWriteFactory(NULL)
		, m_pD3DDevice(NULL)
		, m_pD3DContext(NULL)
		, m_pDxgiDevice(NULL)
		, m_pD2dDevice(NULL)
		, m_pDCompDevice(NULL)
		, m_bReady(false)
		, m_bGpuReady(false)
		, m_bGpuFailed(false)
	{
	}

	CD2dRenderDevice::~CD2dRenderDevice()
	{
		DUI_EXIT_SCOPE(L"~CD2dRenderDevice (static teardown)");
		Shutdown();
	}

	bool CD2dRenderDevice::Initialize()
	{
		if( m_bReady ) return true;

		HRESULT hr = D2D1CreateFactory(D2D1_FACTORY_TYPE_SINGLE_THREADED, &m_pFactory);
		if( FAILED(hr) || m_pFactory == NULL ) {
			Shutdown();
			return false;
		}
		m_pFactory->QueryInterface(__uuidof(ID2D1Factory1), (void**)&m_pFactory1);

		hr = DWriteCreateFactory(
			DWRITE_FACTORY_TYPE_SHARED,
			__uuidof(IDWriteFactory),
			reinterpret_cast<IUnknown**>(&m_pDWriteFactory));
		if( FAILED(hr) || m_pDWriteFactory == NULL ) {
			Shutdown();
			return false;
		}

		m_bReady = true;
		return true;
	}

	void CD2dRenderDevice::ShutdownGpu()
	{
		DUI_EXIT_SCOPE(L"CD2dRenderDevice::ShutdownGpu");
		{
			DUI_EXIT_SCOPE(L"ShutdownGpu Release DCompDevice");
			if( m_pDCompDevice != NULL ) {
				m_pDCompDevice->Release();
				m_pDCompDevice = NULL;
			}
		}
		{
			DUI_EXIT_SCOPE(L"ShutdownGpu Release D2dDevice");
			if( m_pD2dDevice != NULL ) {
				m_pD2dDevice->Release();
				m_pD2dDevice = NULL;
			}
		}
		{
			DUI_EXIT_SCOPE(L"ShutdownGpu Release DxgiDevice");
			if( m_pDxgiDevice != NULL ) {
				m_pDxgiDevice->Release();
				m_pDxgiDevice = NULL;
			}
		}
		{
			DUI_EXIT_SCOPE(L"ShutdownGpu Release D3DContext");
			if( m_pD3DContext != NULL ) {
				m_pD3DContext->Release();
				m_pD3DContext = NULL;
			}
		}
		{
			DUI_EXIT_SCOPE(L"ShutdownGpu Release D3DDevice");
			if( m_pD3DDevice != NULL ) {
				m_pD3DDevice->Release();
				m_pD3DDevice = NULL;
			}
		}
		m_bGpuReady = false;
	}

	void CD2dRenderDevice::Shutdown()
	{
		DUI_EXIT_SCOPE(L"CD2dRenderDevice::Shutdown");
		ClearBitmapCache();
		ShutdownGpu();
		if( m_pDWriteFactory != NULL ) {
			m_pDWriteFactory->Release();
			m_pDWriteFactory = NULL;
		}
		if( m_pFactory1 != NULL ) {
			m_pFactory1->Release();
			m_pFactory1 = NULL;
		}
		if( m_pFactory != NULL ) {
			m_pFactory->Release();
			m_pFactory = NULL;
		}
		m_bReady = false;
		m_bGpuFailed = false;
	}

	bool CD2dRenderDevice::EnsureGpu()
	{
		if( m_bGpuReady ) return true;
		if( m_bGpuFailed || !m_bReady ) return false;
		if( m_pFactory1 == NULL ) {
			m_bGpuFailed = true;
			return false;
		}

		UINT flags = D3D11_CREATE_DEVICE_BGRA_SUPPORT;
		D3D_FEATURE_LEVEL levels[] = {
			D3D_FEATURE_LEVEL_11_0,
			D3D_FEATURE_LEVEL_10_1,
			D3D_FEATURE_LEVEL_10_0
		};
		const UINT nLevels = (UINT)(sizeof(levels) / sizeof(levels[0]));
		D3D_FEATURE_LEVEL featureLevel = D3D_FEATURE_LEVEL_10_0;
		HRESULT hr = D3D11CreateDevice(
			NULL, D3D_DRIVER_TYPE_HARDWARE, NULL, flags,
			levels, nLevels, D3D11_SDK_VERSION,
			&m_pD3DDevice, &featureLevel, &m_pD3DContext);
		if( FAILED(hr) ) {
			hr = D3D11CreateDevice(
				NULL, D3D_DRIVER_TYPE_WARP, NULL, flags,
				levels, nLevels, D3D11_SDK_VERSION,
				&m_pD3DDevice, &featureLevel, &m_pD3DContext);
		}
		if( FAILED(hr) || m_pD3DDevice == NULL ) {
			ShutdownGpu();
			m_bGpuFailed = true;
			return false;
		}

		hr = m_pD3DDevice->QueryInterface(__uuidof(IDXGIDevice), (void**)&m_pDxgiDevice);
		if( FAILED(hr) || m_pDxgiDevice == NULL ) {
			ShutdownGpu();
			m_bGpuFailed = true;
			return false;
		}

		hr = m_pFactory1->CreateDevice(m_pDxgiDevice, &m_pD2dDevice);
		if( FAILED(hr) || m_pD2dDevice == NULL ) {
			ShutdownGpu();
			m_bGpuFailed = true;
			return false;
		}

		hr = DCompositionCreateDevice(
			m_pDxgiDevice,
			__uuidof(IDCompositionDevice),
			(void**)&m_pDCompDevice);
		if( FAILED(hr) || m_pDCompDevice == NULL ) {
			ShutdownGpu();
			m_bGpuFailed = true;
			return false;
		}

		m_bGpuReady = true;
		return true;
	}

	IRenderContext* CD2dRenderDevice::CreateContext(void* nativeTarget, CPaintManagerUI* pManager)
	{
		return new CD2dRenderContext(reinterpret_cast<HDC>(nativeTarget), pManager, m_pFactory, m_pDWriteFactory);
	}

	void CD2dRenderDevice::DestroyContext(IRenderContext* pCtx)
	{
		delete pCtx;
	}

	IRenderSurface* CD2dRenderDevice::CreateSurface()
	{
		return new CD2dRenderSurface(this);
	}

	void CD2dRenderDevice::DestroySurface(IRenderSurface* pSurface)
	{
		DUI_EXIT_SCOPE(L"DestroySurface");
		delete pSurface;
	}

	bool CD2dRenderDevice::CreatePixelBuffer(int width, int height, BYTE** ppBits, void** ppNative)
	{
		CGdiRenderDevice gdi;
		return gdi.CreatePixelBuffer(width, height, ppBits, ppNative);
	}

	void CD2dRenderDevice::DestroyPixelBuffer(void* pNative)
	{
		// RichEdit 等每帧 Create/Destroy 临时位图：删 HBITMAP 前清 D2D 缓存，避免句柄复用串图与泄漏
		if( pNative != NULL )
			InvalidateBitmapCacheForImage(reinterpret_cast<HBITMAP>(pNative), NULL);
		CGdiRenderDevice gdi;
		gdi.DestroyPixelBuffer(pNative);
	}

	static void MarkImageBackendD2d(TImageInfo* pInfo)
	{
		if( pInfo == NULL ) return;
		pInfo->nBackend = RENDER_BACKEND_D2D;
	}

	TImageInfo* CD2dRenderDevice::LoadImage(LPCTSTR pStrImage, LPCTSTR type, DWORD mask, HINSTANCE instance)
	{
		TImageInfo* pInfo = CRenderEngine::LoadImage(pStrImage, type, mask, instance);
		MarkImageBackendD2d(pInfo);
		return pInfo;
	}

	TImageInfo* CD2dRenderDevice::LoadImage(UINT nID, LPCTSTR type, DWORD mask, HINSTANCE instance)
	{
		TImageInfo* pInfo = CRenderEngine::LoadImage(nID, type, mask, instance);
		MarkImageBackendD2d(pInfo);
		return pInfo;
	}

	TImageInfo* CD2dRenderDevice::GdiplusLoadImage(LPCTSTR pStrImage, LPCTSTR type, DWORD mask, HINSTANCE instance)
	{
		TImageInfo* pInfo = CRenderEngine::GdiplusLoadImage(pStrImage, type, mask, instance);
		MarkImageBackendD2d(pInfo);
		return pInfo;
	}

	TImageInfo* CD2dRenderDevice::GdiplusLoadImage(UINT nID, LPCTSTR type, DWORD mask, HINSTANCE instance)
	{
		TImageInfo* pInfo = CRenderEngine::GdiplusLoadImage(nID, type, mask, instance);
		MarkImageBackendD2d(pInfo);
		return pInfo;
	}

	void CD2dRenderDevice::ClearBitmapCache()
	{
		for( int i = 0; i < m_aBitmapCache.GetSize(); ++i ) {
			TD2dBitmapCache* pEntry = static_cast<TD2dBitmapCache*>(m_aBitmapCache.GetAt(i));
			if( pEntry != NULL ) {
				if( pEntry->pBitmap != NULL ) pEntry->pBitmap->Release();
				delete pEntry;
			}
		}
		m_aBitmapCache.Empty();
	}

	void CD2dRenderDevice::InvalidateBitmapCacheForRT(ID2D1RenderTarget* pRT)
	{
		if( pRT == NULL ) return;
		for( int i = m_aBitmapCache.GetSize() - 1; i >= 0; --i ) {
			TD2dBitmapCache* pEntry = static_cast<TD2dBitmapCache*>(m_aBitmapCache.GetAt(i));
			if( pEntry == NULL || pEntry->pRT != pRT ) continue;
			if( pEntry->pBitmap != NULL ) pEntry->pBitmap->Release();
			delete pEntry;
			m_aBitmapCache.Remove(i);
		}
	}

	void CD2dRenderDevice::InvalidateBitmapCacheForImage(HBITMAP hBitmap, void* pGdiplusKey)
	{
		for( int i = m_aBitmapCache.GetSize() - 1; i >= 0; --i ) {
			TD2dBitmapCache* pEntry = static_cast<TD2dBitmapCache*>(m_aBitmapCache.GetAt(i));
			if( pEntry == NULL ) continue;
			bool match = false;
			if( hBitmap != NULL && pEntry->hBitmap == hBitmap ) match = true;
			if( pGdiplusKey != NULL && pEntry->pGdiplusKey == pGdiplusKey ) match = true;
			if( !match ) continue;
			if( pEntry->pBitmap != NULL ) pEntry->pBitmap->Release();
			delete pEntry;
			m_aBitmapCache.Remove(i);
		}
	}

	ID2D1Bitmap* CD2dRenderDevice::GetOrCreateBitmap(ID2D1RenderTarget* pRT, HBITMAP hBitmap, int nX, int nY, LPBYTE pBits, bool bAlpha)
	{
		if( hBitmap == NULL || pRT == NULL || nX <= 0 || nY <= 0 ) return NULL;

		for( int i = 0; i < m_aBitmapCache.GetSize(); ++i ) {
			TD2dBitmapCache* pEntry = static_cast<TD2dBitmapCache*>(m_aBitmapCache.GetAt(i));
			if( pEntry != NULL && pEntry->pRT == pRT && pEntry->hBitmap == hBitmap && pEntry->pBitmap != NULL )
				return pEntry->pBitmap;
		}

		LPBYTE pPixels = pBits;
		BYTE* pTempBits = NULL;
		if( pPixels == NULL ) {
			BITMAP bm = { 0 };
			if( !::GetObject(hBitmap, sizeof(bm), &bm) || bm.bmWidth <= 0 || bm.bmHeight <= 0 ) return NULL;
			nX = bm.bmWidth;
			nY = bm.bmHeight;
			BITMAPINFO bmi = {};
			bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
			bmi.bmiHeader.biWidth = nX;
			bmi.bmiHeader.biHeight = -nY; // top-down
			bmi.bmiHeader.biPlanes = 1;
			bmi.bmiHeader.biBitCount = 32;
			bmi.bmiHeader.biCompression = BI_RGB;
			pTempBits = new BYTE[nX * nY * 4];
			HDC hScreen = ::GetDC(NULL);
			int nCopied = ::GetDIBits(hScreen, hBitmap, 0, nY, pTempBits, &bmi, DIB_RGB_COLORS);
			::ReleaseDC(NULL, hScreen);
			if( nCopied == 0 ) {
				delete[] pTempBits;
				return NULL;
			}
			pPixels = pTempBits;
		}

		D2D1_BITMAP_PROPERTIES props = D2D1::BitmapProperties(
			D2D1::PixelFormat(DXGI_FORMAT_B8G8R8A8_UNORM,
				bAlpha ? D2D1_ALPHA_MODE_PREMULTIPLIED : D2D1_ALPHA_MODE_IGNORE));
		ID2D1Bitmap* pBitmap = NULL;
		HRESULT hr = pRT->CreateBitmap(
			D2D1::SizeU((UINT32)nX, (UINT32)nY),
			pPixels,
			(UINT32)(nX * 4),
			props,
			&pBitmap);
		if( pTempBits != NULL ) delete[] pTempBits;
		if( FAILED(hr) || pBitmap == NULL ) return NULL;

		TD2dBitmapCache* pEntry = new TD2dBitmapCache;
		pEntry->hBitmap = hBitmap;
		pEntry->pGdiplusKey = NULL;
		pEntry->pRT = pRT;
		pEntry->pBitmap = pBitmap;
		m_aBitmapCache.Add(pEntry);
		return pBitmap;
	}

	// 把当前 Gdiplus 像素灌进已有 D2D 位图（Loading/Gif 会改同一指针的像素，仅靠指针键会命中脏缓存）
	static bool CopyGdiplusPixelsToD2dBitmap(ID2D1Bitmap* pBitmap, void* pGdiplusImage)
	{
		if( pBitmap == NULL || pGdiplusImage == NULL ) return false;
		Gdiplus::Image* pImage = reinterpret_cast<Gdiplus::Image*>(pGdiplusImage);
		UINT nX = pImage->GetWidth();
		UINT nY = pImage->GetHeight();
		if( nX == 0 || nY == 0 ) return false;
		D2D1_SIZE_U sz = pBitmap->GetPixelSize();
		if( sz.width != nX || sz.height != nY ) return false;

		Gdiplus::Bitmap tmp((INT)nX, (INT)nY, PixelFormat32bppPARGB);
		{
			Gdiplus::Graphics g(&tmp);
			g.SetCompositingMode(Gdiplus::CompositingModeSourceCopy);
			g.DrawImage(pImage, 0, 0, (INT)nX, (INT)nY);
		}
		Gdiplus::BitmapData bd = { 0 };
		Gdiplus::Rect lockRc(0, 0, (INT)nX, (INT)nY);
		if( tmp.LockBits(&lockRc, Gdiplus::ImageLockModeRead, PixelFormat32bppPARGB, &bd) != Gdiplus::Ok )
			return false;
		HRESULT hr = pBitmap->CopyFromMemory(NULL, bd.Scan0, (UINT32)bd.Stride);
		tmp.UnlockBits(&bd);
		return SUCCEEDED(hr);
	}

	ID2D1Bitmap* CD2dRenderDevice::GetOrCreateBitmapFromGdiplus(ID2D1RenderTarget* pRT, void* pGdiplusImage)
	{
		if( pGdiplusImage == NULL || pRT == NULL ) return NULL;

		for( int i = 0; i < m_aBitmapCache.GetSize(); ++i ) {
			TD2dBitmapCache* pEntry = static_cast<TD2dBitmapCache*>(m_aBitmapCache.GetAt(i));
			if( pEntry != NULL && pEntry->pRT == pRT && pEntry->pGdiplusKey == pGdiplusImage && pEntry->pBitmap != NULL ) {
				// 同指针的 Gdiplus 位图内容可能已变（Loading 烘焙角度 / Gif 换帧），必须回写 GPU
				if( !CopyGdiplusPixelsToD2dBitmap(pEntry->pBitmap, pGdiplusImage) ) {
					if( pEntry->pBitmap != NULL ) pEntry->pBitmap->Release();
					delete pEntry;
					m_aBitmapCache.Remove(i);
					break;
				}
				return pEntry->pBitmap;
			}
		}

		Gdiplus::Image* pImage = reinterpret_cast<Gdiplus::Image*>(pGdiplusImage);
		UINT nX = pImage->GetWidth();
		UINT nY = pImage->GetHeight();
		if( nX == 0 || nY == 0 ) return NULL;

		Gdiplus::Bitmap tmp((INT)nX, (INT)nY, PixelFormat32bppPARGB);
		{
			Gdiplus::Graphics g(&tmp);
			g.SetCompositingMode(Gdiplus::CompositingModeSourceCopy);
			g.DrawImage(pImage, 0, 0, (INT)nX, (INT)nY);
		}

		Gdiplus::BitmapData bd = { 0 };
		Gdiplus::Rect lockRc(0, 0, (INT)nX, (INT)nY);
		if( tmp.LockBits(&lockRc, Gdiplus::ImageLockModeRead, PixelFormat32bppPARGB, &bd) != Gdiplus::Ok )
			return NULL;

		D2D1_BITMAP_PROPERTIES props = D2D1::BitmapProperties(
			D2D1::PixelFormat(DXGI_FORMAT_B8G8R8A8_UNORM, D2D1_ALPHA_MODE_PREMULTIPLIED));
		ID2D1Bitmap* pBitmap = NULL;
		HRESULT hr = pRT->CreateBitmap(
			D2D1::SizeU(nX, nY),
			bd.Scan0,
			(UINT32)bd.Stride,
			props,
			&pBitmap);
		tmp.UnlockBits(&bd);
		if( FAILED(hr) || pBitmap == NULL ) return NULL;

		TD2dBitmapCache* pEntry = new TD2dBitmapCache;
		pEntry->hBitmap = NULL;
		pEntry->pGdiplusKey = pGdiplusImage;
		pEntry->pRT = pRT;
		pEntry->pBitmap = pBitmap;
		m_aBitmapCache.Add(pEntry);
		return pBitmap;
	}

	void CD2dRenderDevice::FreeImage(TImageInfo* pImageInfo, bool bDelete)
	{
		InvalidateImageGpu(pImageInfo);
		if( pImageInfo != NULL && pImageInfo->pBackend != NULL ) {
			reinterpret_cast<IUnknown*>(pImageInfo->pBackend)->Release();
			pImageInfo->pBackend = NULL;
		}
		CRenderEngine::FreeImage(pImageInfo, bDelete);
	}

	void CD2dRenderDevice::InvalidateImageGpu(TImageInfo* pImageInfo)
	{
		if( pImageInfo == NULL ) return;
		if( pImageInfo->hBitmap != NULL )
			InvalidateBitmapCacheForImage(pImageInfo->hBitmap, pImageInfo->pImage);
		else if( pImageInfo->pImage != NULL )
			InvalidateBitmapCacheForImage(NULL, pImageInfo->pImage);
	}

	void CD2dRenderDevice::InvalidateBitmapGpu(HBITMAP hBitmap)
	{
		if( hBitmap == NULL ) return;
		InvalidateBitmapCacheForImage(hBitmap, NULL);
	}

	bool CD2dRenderDevice::CreateNativeFont(TFontInfo* pFontInfo, int nHeightPx, void* measureNative)
	{
		CGdiRenderDevice gdi;
		if( !gdi.CreateNativeFont(pFontInfo, nHeightPx, measureNative) ) return false;
		if( pFontInfo == NULL ) return false;

		pFontInfo->nBackend = RENDER_BACKEND_D2D;
		// DWrite TextFormat 改为绘制/测量时临时创建，不再长期挂在 TFontInfo 上。
		pFontInfo->pBackend = NULL;
		return true;
	}

	void CD2dRenderDevice::DestroyNativeFont(TFontInfo* pFontInfo)
	{
		if( pFontInfo == NULL ) return;
		if( pFontInfo->hFont != NULL ) {
			::DeleteObject(pFontInfo->hFont);
			pFontInfo->hFont = NULL;
		}
		pFontInfo->pBackend = NULL;
		pFontInfo->nBackend = RENDER_BACKEND_D2D;
	}

	void CD2dRenderDevice::BeginFrame(IRenderContext* pCtx)
	{
		if( pCtx == NULL ) return;
		static_cast<CD2dRenderContext*>(pCtx)->OnBeginFrame();
	}

	void CD2dRenderDevice::EndFrame(IRenderContext* pCtx)
	{
		if( pCtx == NULL ) return;
		static_cast<CD2dRenderContext*>(pCtx)->OnEndFrame();
	}

	CD2dRenderDevice* GetD2dRenderDevice()
	{
		return &g_d2dRenderDevice;
	}

	bool EnableD2dRenderDevice()
	{
		CD2dRenderDevice* pDevice = GetD2dRenderDevice();
		if( pDevice == NULL ) return false;
		if( !pDevice->Initialize() ) return false;
		SetRenderDevice(pDevice);
		return true;
	}

	void EnableGdiRenderDevice()
	{
		SetRenderDevice(NULL);
	}

	namespace {
		struct TaskbarIconCache {
			HBITMAP hbm;
			int cx;
			int cy;
			TaskbarIconCache() : hbm(NULL), cx(0), cy(0) {}
		};
		std::map<HWND, TaskbarIconCache>& TaskbarIconCacheMap()
		{
			static std::map<HWND, TaskbarIconCache> s_map;
			return s_map;
		}
		HICON GetWindowIconForTaskbar(HWND hWnd)
		{
			HICON hIcon = (HICON)::SendMessage(hWnd, WM_GETICON, ICON_BIG, 0);
			if( hIcon == NULL )
				hIcon = (HICON)::SendMessage(hWnd, WM_GETICON, ICON_SMALL, 0);
			if( hIcon == NULL )
				hIcon = (HICON)(ULONG_PTR)::GetClassLongPtr(hWnd, GCLP_HICON);
			if( hIcon == NULL )
				hIcon = (HICON)(ULONG_PTR)::GetClassLongPtr(hWnd, GCLP_HICONSM);
			if( hIcon == NULL )
				hIcon = ::LoadIcon(NULL, IDI_APPLICATION);
			return hIcon;
		}
		HBITMAP CreateTaskbarIconBitmap(HWND hWnd, int cx, int cy)
		{
			if( cx < 1 ) cx = 1;
			if( cy < 1 ) cy = 1;
			BITMAPINFO bmi = {};
			bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
			bmi.bmiHeader.biWidth = cx;
			bmi.bmiHeader.biHeight = -cy;
			bmi.bmiHeader.biPlanes = 1;
			bmi.bmiHeader.biBitCount = 32;
			bmi.bmiHeader.biCompression = BI_RGB;
			void* bits = NULL;
			HBITMAP hbm = ::CreateDIBSection(NULL, &bmi, DIB_RGB_COLORS, &bits, NULL, 0);
			if( hbm == NULL ) return NULL;
			HDC hdc = ::CreateCompatibleDC(NULL);
			HGDIOBJ old = ::SelectObject(hdc, hbm);
			RECT rc = { 0, 0, cx, cy };
			::FillRect(hdc, &rc, (HBRUSH)::GetStockObject(BLACK_BRUSH));
			int side = (cx < cy) ? cx : cy;
			if( side > 256 ) side = 256;
			::DrawIconEx(hdc, (cx - side) / 2, (cy - side) / 2,
				GetWindowIconForTaskbar(hWnd), side, side, 0, NULL, DI_NORMAL);
			::SelectObject(hdc, old);
			::DeleteDC(hdc);
			return hbm;
		}
		HBITMAP EnsureTaskbarIconBitmap(HWND hWnd, int cx, int cy)
		{
			TaskbarIconCache& st = TaskbarIconCacheMap()[hWnd];
			if( st.hbm != NULL && st.cx == cx && st.cy == cy )
				return st.hbm;
			if( st.hbm != NULL ) {
				::DeleteObject(st.hbm);
				st.hbm = NULL;
			}
			st.hbm = CreateTaskbarIconBitmap(hWnd, cx, cy);
			st.cx = cx;
			st.cy = cy;
			return st.hbm;
		}
	}

	void DisableTaskbarLivePreview(HWND hWnd)
	{
		if( hWnd == NULL || !::IsWindow(hWnd) ) return;
		BOOL bTrue = TRUE;
		// 强制图标化：悬停/Peek 不再实时抓窗，任务栏只显示应用图标
		::DwmSetWindowAttribute(hWnd, DWMWA_FORCE_ICONIC_REPRESENTATION, &bTrue, sizeof(bTrue));
		::DwmSetWindowAttribute(hWnd, DWMWA_HAS_ICONIC_BITMAP, &bTrue, sizeof(bTrue));
		::DwmSetWindowAttribute(hWnd, DWMWA_DISALLOW_PEEK, &bTrue, sizeof(bTrue));
		::DwmSetWindowAttribute(hWnd, DWMWA_TRANSITIONS_FORCEDISABLED, &bTrue, sizeof(bTrue));
		::DwmSetWindowAttribute(hWnd, DWMWA_FREEZE_REPRESENTATION, &bTrue, sizeof(bTrue));
		TaskbarIconCacheMap()[hWnd] = TaskbarIconCache();
	}

	void HandleTaskbarIconicThumbnail(HWND hWnd, int cx, int cy)
	{
		HBITMAP hbm = EnsureTaskbarIconBitmap(hWnd, cx, cy);
		if( hbm != NULL )
			::DwmSetIconicThumbnail(hWnd, hbm, 0);
	}

	void HandleTaskbarIconicLivePreview(HWND hWnd)
	{
		HBITMAP hbm = EnsureTaskbarIconBitmap(hWnd, 256, 256);
		if( hbm == NULL ) return;
		POINT pt = { 0, 0 };
		::DwmSetIconicLivePreviewBitmap(hWnd, hbm, &pt, 0);
	}

	void ClearTaskbarLivePreview(HWND hWnd)
	{
		std::map<HWND, TaskbarIconCache>::iterator it = TaskbarIconCacheMap().find(hWnd);
		if( it == TaskbarIconCacheMap().end() ) return;
		if( it->second.hbm != NULL )
			::DeleteObject(it->second.hbm);
		TaskbarIconCacheMap().erase(it);
	}

} // namespace DuiLib
