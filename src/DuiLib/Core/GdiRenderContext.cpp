#include "StdAfx.h"
#include "GdiRenderContext.h"
#include "IRenderDevice.h"
#include "UIRender.h"
#include <math.h>

namespace DuiLib {

	/////////////////////////////////////////////////////////////////////////////////////
	//
	// CRenderContextScope / CRenderClipScope

	CRenderContextScope::CRenderContextScope(CPaintManagerUI* pManager, IRenderContext* pCtx)
		: m_pManager(pManager)
		, m_pOld(NULL)
	{
		if( m_pManager != NULL ) {
			m_pOld = m_pManager->GetRenderContext();
			m_pManager->SetRenderContext(pCtx);
		}
		IRenderDevice* pDevice = GetRenderDevice();
		if( pDevice != NULL && pCtx != NULL ) pDevice->BeginFrame(pCtx);
	}

	CRenderContextScope::~CRenderContextScope()
	{
		IRenderContext* pCtx = (m_pManager != NULL) ? m_pManager->GetRenderContext() : NULL;
		IRenderDevice* pDevice = GetRenderDevice();
		if( pDevice != NULL && pCtx != NULL ) pDevice->EndFrame(pCtx);
		if( m_pManager != NULL ) {
			m_pManager->SetRenderContext(m_pOld);
		}
	}

	CRenderClipScope::CRenderClipScope(IRenderContext& ctx, const RECT& rc)
		: m_pCtx(&ctx)
	{
		m_pCtx->PushClip(rc);
	}

	CRenderClipScope::CRenderClipScope(IRenderContext& ctx, const RECT& rcClip, const RECT& rcRound, int width, int height)
		: m_pCtx(&ctx)
	{
		m_pCtx->PushRoundClip(rcClip, rcRound, width, height);
	}

	CRenderClipScope::~CRenderClipScope()
	{
		if( m_pCtx != NULL ) m_pCtx->PopClip();
	}

	/////////////////////////////////////////////////////////////////////////////////////
	//
	// CGdiRenderContext

	CGdiRenderContext::CGdiRenderContext(HDC hDC, CPaintManagerUI* pManager)
		: m_hDC(hDC)
		, m_pManager(pManager)
	{
	}

	CGdiRenderContext::~CGdiRenderContext()
	{
		while( m_aClipStack.GetSize() > 0 ) PopClip();
	}

	HDC CGdiRenderContext::GetDC() const
	{
		return m_hDC;
	}

	CPaintManagerUI* CGdiRenderContext::GetManager() const
	{
		return m_pManager;
	}

	void CGdiRenderContext::PushClip(const RECT& rc)
	{
		CRenderClip* pClip = new CRenderClip;
		CRenderClip::GenerateClip(m_hDC, rc, *pClip);
		m_aClipStack.Add(pClip);
	}

	void CGdiRenderContext::PushRoundClip(const RECT& rcClip, const RECT& rcRound, int width, int height)
	{
		CRenderClip* pClip = new CRenderClip;
		CRenderClip::GenerateRoundClip(m_hDC, rcClip, rcRound, width, height, *pClip);
		m_aClipStack.Add(pClip);
	}

	void CGdiRenderContext::PopClip()
	{
		int nCount = m_aClipStack.GetSize();
		if( nCount <= 0 ) return;
		CRenderClip* pClip = static_cast<CRenderClip*>(m_aClipStack.GetAt(nCount - 1));
		m_aClipStack.Remove(nCount - 1);
		delete pClip;
	}

	void CGdiRenderContext::SuspendClip()
	{
		int nCount = m_aClipStack.GetSize();
		if( nCount <= 0 ) return;
		CRenderClip* pClip = static_cast<CRenderClip*>(m_aClipStack.GetAt(nCount - 1));
		CRenderClip::UseOldClipBegin(m_hDC, *pClip);
	}

	void CGdiRenderContext::ResumeClip()
	{
		int nCount = m_aClipStack.GetSize();
		if( nCount <= 0 ) return;
		CRenderClip* pClip = static_cast<CRenderClip*>(m_aClipStack.GetAt(nCount - 1));
		CRenderClip::UseOldClipEnd(m_hDC, *pClip);
	}

	int CGdiRenderContext::SaveState()
	{
		return ::SaveDC(m_hDC);
	}

	void CGdiRenderContext::RestoreState(int nSaved)
	{
		::RestoreDC(m_hDC, nSaved);
	}

	void CGdiRenderContext::DrawColor(const RECT& rc, DWORD color)
	{
		CRenderEngine::DrawColor(m_hDC, rc, color);
	}

	void CGdiRenderContext::DrawGradient(const RECT& rc, DWORD dwFirst, DWORD dwSecond, bool bVertical, int nSteps)
	{
		CRenderEngine::DrawGradient(m_hDC, rc, dwFirst, dwSecond, bVertical, nSteps);
	}

	void CGdiRenderContext::DrawLine(const RECT& rc, int nSize, DWORD dwPenColor, int nStyle)
	{
		CRenderEngine::DrawLine(m_hDC, rc, nSize, dwPenColor, nStyle);
	}

	void CGdiRenderContext::DrawRect(const RECT& rc, int nSize, DWORD dwPenColor, int nStyle)
	{
		CRenderEngine::DrawRect(m_hDC, rc, nSize, dwPenColor, nStyle);
	}

	void CGdiRenderContext::DrawRoundRect(const RECT& rc, int nSize, int width, int height, DWORD dwPenColor, int nStyle)
	{
		CRenderEngine::DrawRoundRect(m_hDC, rc, nSize, width, height, dwPenColor, nStyle);
	}

	void CGdiRenderContext::FillRoundRect(const RECT& rc, int width, int height, DWORD dwColor)
	{
		CRenderEngine::FillRoundRect(m_hDC, rc, width, height, dwColor);
	}

	void CGdiRenderContext::DrawText(RECT& rc, LPCTSTR pstrText, DWORD dwColor, int iFont, UINT uStyle)
	{
		CRenderEngine::DrawText(m_hDC, m_pManager, rc, pstrText, dwColor, iFont, uStyle);
	}

	void CGdiRenderContext::DrawText(RECT& rc, LPCTSTR pstrText, DWORD dwColor, int iFont, UINT uStyle, DWORD dwTextBKColor)
	{
		CRenderEngine::DrawText(m_hDC, m_pManager, rc, pstrText, dwColor, iFont, uStyle, dwTextBKColor);
	}

	void CGdiRenderContext::DrawHtmlText(RECT& rc, LPCTSTR pstrText, DWORD dwColor, RECT* pLinks, CDuiString* sLinks, int& nLinkRects, int iFont, UINT uStyle)
	{
		CRenderEngine::DrawHtmlText(m_hDC, m_pManager, rc, pstrText, dwColor, pLinks, sLinks, nLinkRects, iFont, uStyle);
	}

	SIZE CGdiRenderContext::GetTextSize(LPCTSTR pstrText, int iFont, UINT uStyle)
	{
		return CRenderEngine::GetTextSize(m_hDC, m_pManager, pstrText, iFont, uStyle);
	}

	bool CGdiRenderContext::DrawImageInfo(const RECT& rcItem, const RECT& rcPaint, const TDrawInfo* pDrawInfo, HINSTANCE instance)
	{
		return CRenderEngine::DrawImageInfo(m_hDC, m_pManager, rcItem, rcPaint, pDrawInfo, instance);
	}

	bool CGdiRenderContext::DrawImageString(const RECT& rcItem, const RECT& rcPaint, LPCTSTR pStrImage, LPCTSTR pStrModify, HINSTANCE instance)
	{
		return CRenderEngine::DrawImageString(m_hDC, m_pManager, rcItem, rcPaint, pStrImage, pStrModify, instance);
	}

#if DUILIB_GDI_INTEROP
	void CGdiRenderContext::DrawImage(HBITMAP hBitmap, const RECT& rc, const RECT& rcPaint, const RECT& rcBmpPart, const RECT& rcCorners, bool bAlpha, UINT uFade, bool hole, bool xtiled, bool ytiled)
	{
		CRenderEngine::DrawImage(m_hDC, hBitmap, rc, rcPaint, rcBmpPart, rcCorners, bAlpha, uFade, hole, xtiled, ytiled);
	}
#endif

	void CGdiRenderContext::DrawImage(const TImageInfo* pImageInfo, const RECT& rc, const RECT& rcPaint, const RECT& rcBmpPart, const RECT& rcCorners, UINT uFade, bool hole, bool xtiled, bool ytiled)
	{
		if( pImageInfo == NULL || pImageInfo->hBitmap == NULL ) return;
		// 后续后端可走 pImageInfo->pBackend；GDI 仍用 hBitmap
		bool bAlpha = (m_pManager != NULL && m_pManager->IsLayered()) ? true : pImageInfo->bAlpha;
		CRenderEngine::DrawImage(m_hDC, pImageInfo->hBitmap, rc, rcPaint, rcBmpPart, rcCorners, bAlpha, uFade, hole, xtiled, ytiled);
	}

	void CGdiRenderContext::DrawGdiplusImage(void* pGdiplusImage, int x, int y, int cx, int cy)
	{
		if( pGdiplusImage == NULL || m_hDC == NULL ) return;
		Gdiplus::Image* pImage = reinterpret_cast<Gdiplus::Image*>(pGdiplusImage);
		Gdiplus::Graphics graphics(m_hDC);
		graphics.DrawImage(pImage, x, y, cx, cy);
	}

	void CGdiRenderContext::DrawGdiplusImageRotated(void* pGdiplusImage, const RECT& rc, float angleDegrees)
	{
		if( pGdiplusImage == NULL || m_hDC == NULL ) return;
		Gdiplus::Image* pImage = reinterpret_cast<Gdiplus::Image*>(pGdiplusImage);
		int iWidth = rc.right - rc.left;
		int iHeight = rc.bottom - rc.top;
		Gdiplus::PointF centerPos((Gdiplus::REAL)(rc.left + iWidth / 2), (Gdiplus::REAL)(rc.top + iHeight / 2));
		if( (iWidth % 2) == 0 ) centerPos.X -= 0.5f;
		if( (iHeight % 2) == 0 ) centerPos.Y -= 0.5f;

		Gdiplus::Graphics graphics(m_hDC);
		graphics.TranslateTransform(centerPos.X, centerPos.Y);
		graphics.RotateTransform(angleDegrees);
		graphics.TranslateTransform(-centerPos.X, -centerPos.Y);
		graphics.DrawImage(pImage, rc.left, rc.top, iWidth, iHeight);
	}

	void CGdiRenderContext::StretchBlit(void* srcNative, int x, int y, int cx, int cy, int xSrc, int ySrc, int cxSrc, int cySrc, int mode)
	{
		if( srcNative == NULL || m_hDC == NULL ) return;
		HDC hSrc = reinterpret_cast<HDC>(srcNative);
		::SetStretchBltMode(m_hDC, mode);
		::StretchBlt(m_hDC, x, y, cx, cy, hSrc, xSrc, ySrc, cxSrc, cySrc, SRCCOPY);
	}

	/////////////////////////////////////////////////////////////////////////////////////
	//
	// CGdiRenderSurface

	typedef BOOL (__stdcall *PFUNCUPDATELAYEREDWINDOW)(HWND, HDC, POINT*, SIZE*, HDC, POINT*, COLORREF, BLENDFUNCTION*, DWORD);

	CGdiRenderSurface::CGdiRenderSurface()
		: m_hDC(NULL)
		, m_hBitmap(NULL)
		, m_hOldBitmap(NULL)
		, m_pBits(NULL)
		, m_nWidth(0)
		, m_nHeight(0)
		, m_bSelected(false)
	{
	}

	CGdiRenderSurface::~CGdiRenderSurface()
	{
		Destroy();
	}

	bool CGdiRenderSurface::Ensure(int width, int height, void* refNative)
	{
		if( width <= 0 || height <= 0 ) return false;
		if( IsValid() && m_nWidth == width && m_nHeight == height ) return true;

		Destroy();
		HDC hRefDC = reinterpret_cast<HDC>(refNative);
		m_hDC = ::CreateCompatibleDC(hRefDC);
		if( m_hDC == NULL ) return false;
		void* pNative = NULL;
		if( !GetRenderDevice()->CreatePixelBuffer(width, height, &m_pBits, &pNative) || pNative == NULL ) {
			::DeleteDC(m_hDC);
			m_hDC = NULL;
			m_pBits = NULL;
			return false;
		}
		m_hBitmap = reinterpret_cast<HBITMAP>(pNative);
		m_hOldBitmap = (HBITMAP)::SelectObject(m_hDC, m_hBitmap);
		m_bSelected = true;
		m_nWidth = width;
		m_nHeight = height;
		return true;
	}

	void CGdiRenderSurface::Destroy()
	{
		if( m_hDC != NULL && m_bSelected ) {
			::SelectObject(m_hDC, m_hOldBitmap);
			m_bSelected = false;
		}
		if( m_hBitmap != NULL ) {
			GetRenderDevice()->DestroyPixelBuffer(m_hBitmap);
			m_hBitmap = NULL;
		}
		if( m_hDC != NULL ) {
			::DeleteDC(m_hDC);
			m_hDC = NULL;
		}
		m_hOldBitmap = NULL;
		m_pBits = NULL;
		m_nWidth = 0;
		m_nHeight = 0;
	}

	bool CGdiRenderSurface::IsValid() const
	{
		return m_hDC != NULL && m_hBitmap != NULL;
	}

	int CGdiRenderSurface::GetWidth() const
	{
		return m_nWidth;
	}

	int CGdiRenderSurface::GetHeight() const
	{
		return m_nHeight;
	}

	BYTE* CGdiRenderSurface::GetBits()
	{
		return m_pBits;
	}

	void* CGdiRenderSurface::GetNativeTarget()
	{
		return m_hDC;
	}

	void CGdiRenderSurface::ClearPaintRect(const RECT& rcPaint, const RECT& rcClient)
	{
		if( m_pBits == NULL ) return;
		DWORD dwWidth = (DWORD)(rcClient.right - rcClient.left);
		if( dwWidth == 0 ) return;
		for( LONG y = rcClient.bottom - rcPaint.bottom; y < rcClient.bottom - rcPaint.top; ++y ) {
			for( LONG x = rcPaint.left; x < rcPaint.right; ++x ) {
				int i = (y * (LONG)dwWidth + x) * 4;
				*(DWORD*)(&m_pBits[i]) = 0;
			}
		}
	}

	void CGdiRenderSurface::ClearAll()
	{
		if( m_pBits == NULL || m_nWidth <= 0 || m_nHeight <= 0 ) return;
		::ZeroMemory(m_pBits, (size_t)m_nWidth * (size_t)m_nHeight * 4);
	}

	void CGdiRenderSurface::FixLayeredAlpha(const RECT& rcPaint, const RECT& rcClient)
	{
		if( m_pBits == NULL ) return;
		DWORD dwWidth = (DWORD)(rcClient.right - rcClient.left);
		if( dwWidth == 0 ) return;
		for( LONG y = rcClient.bottom - rcPaint.bottom; y < rcClient.bottom - rcPaint.top; ++y ) {
			for( LONG x = rcPaint.left; x < rcPaint.right; ++x ) {
				int i = (y * (LONG)dwWidth + x) * 4;
				if( (m_pBits[i + 3] == 0) && (m_pBits[i + 0] != 0 || m_pBits[i + 1] != 0 || m_pBits[i + 2] != 0) )
					m_pBits[i + 3] = 255;
			}
		}
	}

	namespace {

		float RoundCornerCoverage(float px, float py, float w, float h, float rx, float ry)
		{
			if( rx < 0.5f ) rx = 0.5f;
			if( ry < 0.5f ) ry = 0.5f;
			if( rx > w * 0.5f ) rx = w * 0.5f;
			if( ry > h * 0.5f ) ry = h * 0.5f;

			bool inCorner = false;
			float dx = 0.0f, dy = 0.0f;

			if( px < rx && py < ry ) {
				inCorner = true;
				dx = (rx - 0.5f) - px;
				dy = (ry - 0.5f) - py;
			}
			else if( px >= w - rx && py < ry ) {
				inCorner = true;
				dx = px - (w - rx - 0.5f);
				dy = (ry - 0.5f) - py;
			}
			else if( px < rx && py >= h - ry ) {
				inCorner = true;
				dx = (rx - 0.5f) - px;
				dy = py - (h - ry - 0.5f);
			}
			else if( px >= w - rx && py >= h - ry ) {
				inCorner = true;
				dx = px - (w - rx - 0.5f);
				dy = py - (h - ry - 0.5f);
			}
			if( !inCorner ) return 1.0f;

			// 椭圆归一化距离；=1 在圆弧上
			float nx = dx / rx;
			float ny = dy / ry;
			float dist = sqrtf(nx * nx + ny * ny);
			const float aa = 1.0f / ((rx < ry) ? rx : ry); // ~1px 过渡
			if( dist <= 1.0f - aa ) return 1.0f;
			if( dist >= 1.0f + aa ) return 0.0f;
			return (1.0f + aa - dist) / (2.0f * aa);
		}

		void ApplyRoundCornerMaskToBits(BYTE* pBits, int width, int height, int radiusX, int radiusY)
		{
			if( pBits == NULL || width <= 0 || height <= 0 ) return;
			if( radiusX <= 0 && radiusY <= 0 ) return;
			float rx = (float)((radiusX > 0) ? radiusX : radiusY);
			float ry = (float)((radiusY > 0) ? radiusY : radiusX);
			float fw = (float)width;
			float fh = (float)height;

			int maxRx = (int)(rx + 2.0f);
			int maxRy = (int)(ry + 2.0f);
			if( maxRx > width / 2 ) maxRx = width / 2;
			if( maxRy > height / 2 ) maxRy = height / 2;

			for( int wy = 0; wy < height; ++wy ) {
				bool nearY = (wy < maxRy) || (wy >= height - maxRy);
				if( !nearY ) continue;
				int by = height - 1 - wy; // bottom-up DIB
				BYTE* pRow = pBits + by * width * 4;
				for( int wx = 0; wx < width; ++wx ) {
					bool nearX = (wx < maxRx) || (wx >= width - maxRx);
					if( !nearX ) continue;
					float cov = RoundCornerCoverage((float)wx + 0.5f, (float)wy + 0.5f, fw, fh, rx, ry);
					if( cov >= 0.999f ) continue;
					BYTE* p = pRow + wx * 4;
					if( cov <= 0.001f ) {
						*(DWORD*)p = 0;
						continue;
					}
					// 预乘/直通都按通道乘 coverage，得到 AA 外轮廓
					p[0] = (BYTE)((float)p[0] * cov + 0.5f);
					p[1] = (BYTE)((float)p[1] * cov + 0.5f);
					p[2] = (BYTE)((float)p[2] * cov + 0.5f);
					p[3] = (BYTE)((float)p[3] * cov + 0.5f);
				}
			}
		}

	} // namespace

	void CGdiRenderSurface::ApplyRoundCornerMask(int radiusX, int radiusY)
	{
		ApplyRoundCornerMaskToBits(m_pBits, m_nWidth, m_nHeight, radiusX, radiusY);
	}

	void CGdiRenderSurface::ApplyLayeredMask(IRenderSurface* pMask, const RECT& rcPaint, const RECT& rcClient)
	{
		if( pMask == NULL || m_pBits == NULL ) return;
		BYTE* pMaskBits = pMask->GetBits();
		if( pMaskBits == NULL ) return;
		DWORD dwWidth = (DWORD)(rcClient.right - rcClient.left);
		if( dwWidth == 0 ) return;
		for( LONG y = rcClient.bottom - rcPaint.bottom; y < rcClient.bottom - rcPaint.top; ++y ) {
			for( LONG x = rcPaint.left; x < rcPaint.right; ++x ) {
				COLORREF* pOff = (COLORREF*)(m_pBits + (y * (LONG)dwWidth + x) * 4);
				COLORREF* pBg = (COLORREF*)(pMaskBits + (y * (LONG)dwWidth + x) * 4);
				BYTE A = (BYTE)((*pBg) >> 24);
				BYTE R = (BYTE)((*pOff) >> 16) * A / 255;
				BYTE G = (BYTE)((*pOff) >> 8) * A / 255;
				BYTE B = (BYTE)(*pOff) * A / 255;
				*pOff = RGB(B, G, R) + ((DWORD)A << 24);
			}
		}
	}

	bool CGdiRenderSurface::Present(const RenderPresentParams& params)
	{
		if( !IsValid() || params.hWindowDC == NULL ) return false;
		if( params.bLayered ) {
			static PFUNCUPDATELAYEREDWINDOW s_fUpdateLayeredWindow = NULL;
			if( s_fUpdateLayeredWindow == NULL ) {
				HMODULE hUser32 = ::GetModuleHandle(_T("User32.dll"));
				if( hUser32 != NULL ) {
					s_fUpdateLayeredWindow = (PFUNCUPDATELAYEREDWINDOW)::GetProcAddress(hUser32, "UpdateLayeredWindow");
				}
			}
			if( s_fUpdateLayeredWindow == NULL || params.hWnd == NULL ) return false;

			RECT rcWnd = { 0 };
			::GetWindowRect(params.hWnd, &rcWnd);
			BLENDFUNCTION bf = { AC_SRC_OVER, 0, params.nOpacity, AC_SRC_ALPHA };
			POINT ptPos = { rcWnd.left, rcWnd.top };
			SIZE sizeWnd = { (LONG)(params.rcClient.right - params.rcClient.left), (LONG)(params.rcClient.bottom - params.rcClient.top) };
			POINT ptSrc = { 0, 0 };
			return s_fUpdateLayeredWindow(params.hWnd, params.hWindowDC, &ptPos, &sizeWnd, m_hDC, &ptSrc, 0, &bf, ULW_ALPHA) != FALSE;
		}

		const RECT& rc = params.rcPaint;
		return ::BitBlt(params.hWindowDC, rc.left, rc.top, rc.right - rc.left, rc.bottom - rc.top,
			m_hDC, rc.left, rc.top, SRCCOPY) != FALSE;
	}

	/////////////////////////////////////////////////////////////////////////////////////
	//
	// CGdiRenderDevice + global device

	static IRenderDevice* g_pRenderDevice = NULL;
	static CGdiRenderDevice g_gdiRenderDevice;

	IRenderContext* CGdiRenderDevice::CreateContext(void* nativeTarget, CPaintManagerUI* pManager)
	{
		return new CGdiRenderContext(reinterpret_cast<HDC>(nativeTarget), pManager);
	}

	void CGdiRenderDevice::DestroyContext(IRenderContext* pCtx)
	{
		delete pCtx;
	}

	IRenderSurface* CGdiRenderDevice::CreateSurface()
	{
		return new CGdiRenderSurface();
	}

	void CGdiRenderDevice::DestroySurface(IRenderSurface* pSurface)
	{
		delete pSurface;
	}

	bool CGdiRenderDevice::CreatePixelBuffer(int width, int height, BYTE** ppBits, void** ppNative)
	{
		if( ppBits == NULL || ppNative == NULL || width <= 0 || height <= 0 ) return false;
		*ppBits = NULL;
		*ppNative = NULL;
		HDC hDC = ::GetDC(NULL);
		HBITMAP hBitmap = CRenderEngine::CreateARGB32Bitmap(hDC, width, height, ppBits);
		::ReleaseDC(NULL, hDC);
		if( hBitmap == NULL ) return false;
		*ppNative = hBitmap;
		return true;
	}

	void CGdiRenderDevice::DestroyPixelBuffer(void* pNative)
	{
		if( pNative != NULL ) ::DeleteObject(reinterpret_cast<HBITMAP>(pNative));
	}

	TImageInfo* CGdiRenderDevice::LoadImage(LPCTSTR pStrImage, LPCTSTR type, DWORD mask, HINSTANCE instance)
	{
		return CRenderEngine::LoadImage(pStrImage, type, mask, instance);
	}

	TImageInfo* CGdiRenderDevice::LoadImage(UINT nID, LPCTSTR type, DWORD mask, HINSTANCE instance)
	{
		return CRenderEngine::LoadImage(nID, type, mask, instance);
	}

	TImageInfo* CGdiRenderDevice::GdiplusLoadImage(LPCTSTR pStrImage, LPCTSTR type, DWORD mask, HINSTANCE instance)
	{
		return CRenderEngine::GdiplusLoadImage(pStrImage, type, mask, instance);
	}

	TImageInfo* CGdiRenderDevice::GdiplusLoadImage(UINT nID, LPCTSTR type, DWORD mask, HINSTANCE instance)
	{
		return CRenderEngine::GdiplusLoadImage(nID, type, mask, instance);
	}

	void CGdiRenderDevice::FreeImage(TImageInfo* pImageInfo, bool bDelete)
	{
		CRenderEngine::FreeImage(pImageInfo, bDelete);
	}

	bool CGdiRenderDevice::CreateNativeFont(TFontInfo* pFontInfo, int nHeightPx, void* measureNative)
	{
		if( pFontInfo == NULL ) return false;
		DestroyNativeFont(pFontInfo);

		LOGFONT lf = { 0 };
		::GetObject(::GetStockObject(DEFAULT_GUI_FONT), sizeof(LOGFONT), &lf);
		if( !pFontInfo->sFontName.IsEmpty() ) {
			_tcsncpy(lf.lfFaceName, pFontInfo->sFontName.GetData(), LF_FACESIZE);
		}
		lf.lfCharSet = DEFAULT_CHARSET;
		lf.lfHeight = nHeightPx;
		lf.lfQuality = CLEARTYPE_QUALITY;
		if( pFontInfo->bBold ) lf.lfWeight = FW_BOLD;
		if( pFontInfo->bUnderline ) lf.lfUnderline = TRUE;
		if( pFontInfo->bItalic ) lf.lfItalic = TRUE;
		if( pFontInfo->bStrikeout ) lf.lfStrikeOut = TRUE;

		HFONT hFont = ::CreateFontIndirect(&lf);
		if( hFont == NULL ) return false;

		pFontInfo->hFont = hFont;
		pFontInfo->pBackend = NULL;
		pFontInfo->nBackend = RENDER_BACKEND_GDI;
		::ZeroMemory(&pFontInfo->tm, sizeof(pFontInfo->tm));

		HDC hMeasureDC = reinterpret_cast<HDC>(measureNative);
		if( hMeasureDC != NULL ) {
			HFONT hOldFont = (HFONT)::SelectObject(hMeasureDC, hFont);
			::GetTextMetrics(hMeasureDC, &pFontInfo->tm);
			::SelectObject(hMeasureDC, hOldFont);
		}
		return true;
	}

	void CGdiRenderDevice::DestroyNativeFont(TFontInfo* pFontInfo)
	{
		if( pFontInfo == NULL ) return;
		if( pFontInfo->hFont != NULL ) {
			::DeleteObject(pFontInfo->hFont);
			pFontInfo->hFont = NULL;
		}
		// 后续 D2D/Skia：在此释放 pBackend
		pFontInfo->pBackend = NULL;
		pFontInfo->nBackend = RENDER_BACKEND_GDI;
	}

	SIZE IRenderDevice::MeasureText(CPaintManagerUI* pManager, LPCTSTR pstrText, int iFont, UINT uStyle)
	{
		SIZE sz = { 0, 0 };
		if( pManager == NULL ) return sz;
		return RenderMeasureTextSize(pManager, pstrText, iFont, uStyle);
	}

	CTempRenderContextScope::CTempRenderContextScope(CPaintManagerUI* pManager)
		: m_pCtx(NULL)
	{
		HDC hDC = (pManager != NULL) ? pManager->GetPaintDC() : NULL;
		m_pCtx = GetRenderDevice()->CreateContext(hDC, pManager);
	}

	CTempRenderContextScope::~CTempRenderContextScope()
	{
		if( m_pCtx != NULL ) {
			GetRenderDevice()->DestroyContext(m_pCtx);
			m_pCtx = NULL;
		}
	}

	void RenderMeasureText(CPaintManagerUI* pManager, RECT& rc, LPCTSTR pstrText, DWORD dwColor, int iFont, UINT uStyle)
	{
		if( pManager == NULL ) return;
		CTempRenderContextScope scope(pManager);
		IRenderContext* pCtx = scope.operator->();
		if( pCtx == NULL ) return;
		pCtx->DrawText(rc, pstrText, dwColor, iFont, uStyle);
	}

	void RenderMeasureHtmlText(CPaintManagerUI* pManager, RECT& rc, LPCTSTR pstrText, DWORD dwColor, int iFont, UINT uStyle)
	{
		if( pManager == NULL ) return;
		CTempRenderContextScope scope(pManager);
		IRenderContext* pCtx = scope.operator->();
		if( pCtx == NULL ) return;
		int nLinks = 0;
		pCtx->DrawHtmlText(rc, pstrText, dwColor, NULL, NULL, nLinks, iFont, uStyle);
	}

	SIZE RenderMeasureTextSize(CPaintManagerUI* pManager, LPCTSTR pstrText, int iFont, UINT uStyle)
	{
		SIZE sz = { 0, 0 };
		if( pManager == NULL ) return sz;
		CTempRenderContextScope scope(pManager);
		IRenderContext* pCtx = scope.operator->();
		if( pCtx == NULL ) return sz;
		return pCtx->GetTextSize(pstrText, iFont, uStyle);
	}

#if DUILIB_RENDER_BACKEND == DUILIB_RENDER_D2D
	bool EnableD2dRenderDevice();
#endif

	IRenderDevice* GetRenderDevice()
	{
		if( g_pRenderDevice != NULL ) return g_pRenderDevice;
#if DUILIB_RENDER_BACKEND == DUILIB_RENDER_D2D
		static bool s_triedD2d = false;
		if( !s_triedD2d ) {
			s_triedD2d = true;
			if( EnableD2dRenderDevice() && g_pRenderDevice != NULL )
				return g_pRenderDevice;
		}
#endif
		return &g_gdiRenderDevice;
	}

	void SetRenderDevice(IRenderDevice* pDevice)
	{
		g_pRenderDevice = pDevice;
	}

	COwnedRenderContextScope::COwnedRenderContextScope(CPaintManagerUI* pManager, void* nativeTarget)
		: m_pManager(pManager)
		, m_pCtx(NULL)
		, m_pOld(NULL)
	{
		IRenderDevice* pDevice = GetRenderDevice();
		m_pCtx = pDevice->CreateContext(nativeTarget, pManager);
		if( m_pManager != NULL ) {
			m_pOld = m_pManager->GetRenderContext();
			m_pManager->SetRenderContext(m_pCtx);
		}
		if( m_pCtx != NULL ) pDevice->BeginFrame(m_pCtx);
	}

	COwnedRenderContextScope::~COwnedRenderContextScope()
	{
		IRenderDevice* pDevice = GetRenderDevice();
		if( m_pCtx != NULL ) pDevice->EndFrame(m_pCtx);
		if( m_pManager != NULL ) m_pManager->SetRenderContext(m_pOld);
		if( m_pCtx != NULL ) {
			pDevice->DestroyContext(m_pCtx);
			m_pCtx = NULL;
		}
	}

} // namespace DuiLib
