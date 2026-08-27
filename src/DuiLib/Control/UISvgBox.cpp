#include "StdAfx.h"
#include "UISvgBox.h"
#include "UIBootstrapIcons.h"
#include "UILucideIcons.h"
#include "UIIconParkIcons.h"
#include "UITablerIcons.h"
#include "UIRemixIconIcons.h"
#include "UITwemojiIcons.h"
	#include <lunasvg.h>
	#include <string>
	#include <vector>
	#include <memory>
	#include "../Utils/stb_image.h"
	#include <wincodec.h>

	namespace DuiLib
{
	IMPLEMENT_DUICONTROL(CSvgBoxUI)

	static std::string DuiStringToUtf8(LPCTSTR pstr)
	{
		if( pstr == NULL || *pstr == _T('\0') ) return std::string();
#ifdef _UNICODE
		int n = ::WideCharToMultiByte(CP_UTF8, 0, pstr, -1, NULL, 0, NULL, NULL);
		if( n <= 1 ) return std::string();
		std::string s((size_t)(n - 1), '\0');
		::WideCharToMultiByte(CP_UTF8, 0, pstr, -1, &s[0], n, NULL, NULL);
		return s;
#else
		return std::string(pstr);
#endif
	}

	static CDuiString ResolveSvgFilePathLocal(LPCTSTR pstrPath)
	{
		if( pstrPath == NULL || *pstrPath == _T('\0') ) return CDuiString();
		if( (pstrPath[0] == _T('\\') || pstrPath[0] == _T('/')) ||
			(_tcslen(pstrPath) >= 2 && pstrPath[1] == _T(':')) ) {
			return CDuiString(pstrPath);
		}
		CDuiString sFile = CPaintManagerUI::GetResourcePath();
		sFile += pstrPath;
		return sFile;
	}

	enum SvgTintModeLocal { SvgTintFill = 0, SvgTintStroke = 1, SvgTintBoth = 2, SvgTintSkip = 3 };

	static SvgTintModeLocal DetectTintModeLocal(const std::string& svgUtf8)
	{
		if( svgUtf8.empty() ) return SvgTintBoth;
		auto has = [&](const char* s) -> bool {
			return svgUtf8.find(s) != std::string::npos;
		};
		if( (has("fill=\"#") || has("fill='#")) && !has("currentColor") )
			return SvgTintSkip;
		const bool bFillNone = has("fill=\"none\"") || has("fill='none'");
		const bool bStrokeCurrent = has("stroke=\"currentColor\"") || has("stroke='currentColor'");
		const bool bFillCurrent = has("fill=\"currentColor\"") || has("fill='currentColor'");
		const bool bHasStroke = has("stroke=") || has("stroke:");
		if( bStrokeCurrent || (bFillNone && bHasStroke) )
			return SvgTintStroke;
		if( bFillCurrent || !bHasStroke )
			return SvgTintFill;
		return SvgTintBoth;
	}

	static HBITMAP CreatePremultHBitmap(const lunasvg::Bitmap& bitmap)
	{
		const int w = bitmap.width();
		const int h = bitmap.height();
		if( w <= 0 || h <= 0 || bitmap.isNull() ) return NULL;

		BITMAPINFO bmi;
		::ZeroMemory(&bmi, sizeof(bmi));
		bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
		bmi.bmiHeader.biWidth = w;
		bmi.bmiHeader.biHeight = -h;
		bmi.bmiHeader.biPlanes = 1;
		bmi.bmiHeader.biBitCount = 32;
		bmi.bmiHeader.biCompression = BI_RGB;
		bmi.bmiHeader.biSizeImage = (DWORD)(w * h * 4);

		LPBYTE pDest = NULL;
		HBITMAP hBitmap = ::CreateDIBSection(NULL, &bmi, DIB_RGB_COLORS, (void**)&pDest, NULL, 0);
		if( hBitmap == NULL || pDest == NULL ) return NULL;

		const uint8_t* pSrc = bitmap.data();
		const int stride = bitmap.stride();
		const int rowBytes = w * 4;
		for( int y = 0; y < h; ++y ) {
			::memcpy(pDest + y * rowBytes, pSrc + y * stride, (size_t)rowBytes);
		}
		return hBitmap;
	}

	static CDuiString GetPathExtLowerLoc(CDuiString path)
	{
		int pos = path.ReverseFind(_T('.'));
		if( pos < 0 ) return CDuiString();
		CDuiString ext = path.Mid(pos);
		ext.MakeLower();
		return ext;
	}

	// 本地位图扩展名（favicon/普通图片）：.ico/.png/.jpg/.jpeg/.bmp/.gif/.webp
	static bool IsBitmapFileExt(LPCTSTR pstrPath)
	{
		if( pstrPath == NULL || *pstrPath == _T('\0') ) return false;
		CDuiString ext = GetPathExtLowerLoc(pstrPath);
		if( ext.IsEmpty() ) return false;
		return (ext == _T(".ico") || ext == _T(".png") || ext == _T(".jpg") || ext == _T(".jpeg")
			|| ext == _T(".bmp") || ext == _T(".gif") || ext == _T(".webp"));
	}

	// 由 RGBA（top-down, 4ch）像素 → 预乘 alpha BGRA（top-down 32bpp）DIB，按目标 w×h 等比 contain 缩放居中。
	// 成功返回 HBITMAP（调用方 DeleteObject），失败 NULL。
	static HBITMAP CreatePremultHBitmapFromRgba(const BYTE* pRgba, int iw, int ih, int w, int h)
	{
		if( pRgba == NULL || iw <= 0 || ih <= 0 || w <= 0 || h <= 0 ) return NULL;

		// 目标尺寸：等比 contain（保持源比例，居中，不拉伸变形）
		int dw = w, dh = h;
		const double scale = (double)w / iw < (double)h / ih ? (double)w / iw : (double)h / ih;
		dw = (int)(iw * scale + 0.5);
		dh = (int)(ih * scale + 0.5);
		if( dw < 1 ) dw = 1;
		if( dh < 1 ) dh = 1;
		const int ox = (w - dw) / 2;
		const int oy = (h - dh) / 2;

		BITMAPINFO bmi;
		::ZeroMemory(&bmi, sizeof(bmi));
		bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
		bmi.bmiHeader.biWidth = w;
		bmi.bmiHeader.biHeight = -h;
		bmi.bmiHeader.biPlanes = 1;
		bmi.bmiHeader.biBitCount = 32;
		bmi.bmiHeader.biCompression = BI_RGB;
		bmi.bmiHeader.biSizeImage = (DWORD)(w * h * 4);

		LPBYTE pDest = NULL;
		HBITMAP hBmp = ::CreateDIBSection(NULL, &bmi, DIB_RGB_COLORS, (void**)&pDest, NULL, 0);
		if( hBmp == NULL || pDest == NULL ) return NULL;
		::ZeroMemory(pDest, (size_t)w * h * 4);

		// 双线性缩放（预乘 BGRA，top-down），落在目标矩形并居中
		for( int y = 0; y < dh; ++y ) {
			const double sy = (y + 0.5) * ih / dh - 0.5;
			int y0 = (int)sy;
			double fy = sy - y0;
			if( y0 < 0 ) { y0 = 0; fy = 0; }
			int y1 = y0 + 1;
			if( y1 >= ih ) y1 = ih - 1;
			for( int x = 0; x < dw; ++x ) {
				const double sx = (x + 0.5) * iw / dw - 0.5;
				int x0 = (int)sx;
				double fx = sx - x0;
				if( x0 < 0 ) { x0 = 0; fx = 0; }
				int x1 = x0 + 1;
				if( x1 >= iw ) x1 = iw - 1;

				const BYTE* p00 = pRgba + (y0 * iw + x0) * 4;
				const BYTE* p10 = pRgba + (y0 * iw + x1) * 4;
				const BYTE* p01 = pRgba + (y1 * iw + x0) * 4;
				const BYTE* p11 = pRgba + (y1 * iw + x1) * 4;
				BYTE r,g,b,a;
				for( int c = 0; c < 4; ++c ) {
					double v = (p00[c]*(1-fx) + p10[c]*fx)*(1-fy) + (p01[c]*(1-fx) + p11[c]*fx)*fy;
					if( v < 0 ) v = 0; if( v > 255 ) v = 255;
					BYTE bv = (BYTE)(v + 0.5);
					if( c == 0 ) r = bv; else if( c == 1 ) g = bv; else if( c == 2 ) b = bv; else a = bv;
				}
				const int px = ox + x, py = oy + y;
				int dstIdx = (py * w + px) * 4;
				pDest[dstIdx + 0] = (BYTE)((UINT)(b * a) / 255);
				pDest[dstIdx + 1] = (BYTE)((UINT)(g * a) / 255);
				pDest[dstIdx + 2] = (BYTE)((UINT)(r * a) / 255);
				pDest[dstIdx + 3] = a;
			}
		}
		return hBmp;
	}

	static HBITMAP CreatePremultHBitmapFromRgba(const BYTE* pRgba, int iw, int ih, int w, int h);
	static HBITMAP CreatePremultHBitmapFromMemory(const BYTE* pData, size_t nLen, int w, int h);
	static bool DecodeViaWic(const BYTE* pData, size_t nLen, int w, int h, HBITMAP* ppHBitmap);

	// 从文件字节：先 stbi 解码，失败回退 WIC → 预乘 DIB
	static HBITMAP CreatePremultHBitmapFromFile(LPCTSTR pstrPath, int w, int h)
	{
		if( pstrPath == NULL || w <= 0 || h <= 0 ) return NULL;
		std::string utf8 = DuiStringToUtf8(pstrPath);
		if( utf8.empty() ) return NULL;
		int iw = 0, ih = 0, n = 0;
		BYTE* pRgba = stbi_load(utf8.c_str(), &iw, &ih, &n, 4);
		if( pRgba != NULL && iw > 0 && ih > 0 ) {
			HBITMAP hBmp = CreatePremultHBitmapFromRgba(pRgba, iw, ih, w, h);
			stbi_image_free(pRgba);
			return hBmp;
		}
		if( pRgba ) stbi_image_free(pRgba);
		// stb 解不出（如某些 ICO）→ 读文件字节走 memory 的 WIC 回退
		HANDLE hFile = ::CreateFile(pstrPath, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
		if( hFile == INVALID_HANDLE_VALUE ) return NULL;
		DWORD dwSize = ::GetFileSize(hFile, NULL);
		HBITMAP hRes = NULL;
		if( dwSize > 0 && dwSize != INVALID_FILE_SIZE ) {
			std::vector<BYTE> buf((size_t)dwSize);
			DWORD rd = 0;
			if( ::ReadFile(hFile, buf.data(), dwSize, &rd, NULL) && rd == dwSize )
				hRes = CreatePremultHBitmapFromMemory(buf.data(), (size_t)dwSize, w, h);
		}
		::CloseHandle(hFile);
		return hRes;
	}

	// 从内存字节：先 stbi 解码，失败（如 ICO/特殊编码）回退 WIC → 预乘 DIB
	static HBITMAP CreatePremultHBitmapFromMemory(const BYTE* pData, size_t nLen, int w, int h)
	{
		if( pData == NULL || nLen == 0 || w <= 0 || h <= 0 ) return NULL;
		int iw = 0, ih = 0, n = 0;
		BYTE* pRgba = stbi_load_from_memory(pData, (int)nLen, &iw, &ih, &n, 4);
		if( pRgba != NULL && iw > 0 && ih > 0 ) {
			HBITMAP hBmp = CreatePremultHBitmapFromRgba(pRgba, iw, ih, w, h);
			stbi_image_free(pRgba);
			return hBmp;
		}
		if( pRgba ) stbi_image_free(pRgba);
		// stb 解不出 → WIC 回退
		HBITMAP hBmp = NULL;
		if( DecodeViaWic(pData, nLen, w, h, &hBmp) )
			return hBmp;
		return NULL;
	}

	// WIC 回退解码（stb_image 解不出时用，如某些 ICO）：取 RGBA 像素后走 CreatePremultHBitmapFromRgba。
	// 返回 true 且 *ppHBitmap 非空表示成功；否则 false。
	static bool DecodeViaWic(const BYTE* pData, size_t nLen, int w, int h, HBITMAP* ppHBitmap)
	{
		if( ppHBitmap ) *ppHBitmap = NULL;
		if( pData == NULL || nLen == 0 || ppHBitmap == NULL ) return false;

		IWICImagingFactory* pFactory = NULL;
		HRESULT hr = ::CoCreateInstance(CLSID_WICImagingFactory, NULL, CLSCTX_INPROC_SERVER,
			IID_IWICImagingFactory, (void**)&pFactory);
		if( FAILED(hr) || pFactory == NULL ) return false;

		IWICStream* pStream = NULL;
		IWICBitmapDecoder* pDecoder = NULL;
		IWICBitmapFrameDecode* pFrame = NULL;
		IWICFormatConverter* pConv = NULL;
		BYTE* pRgba = NULL;
		bool bOk = false;
		int iw = 0, ih = 0;

		do {
			if( FAILED(pFactory->CreateStream(&pStream)) || pStream == NULL ) break;
			if( FAILED(pStream->InitializeFromMemory(const_cast<BYTE*>(pData), (DWORD)nLen)) ) break;
			if( FAILED(pFactory->CreateDecoderFromStream(pStream, NULL, WICDecodeMetadataCacheOnDemand, &pDecoder))
				|| pDecoder == NULL ) break;
			if( FAILED(pDecoder->GetFrame(0, &pFrame)) || pFrame == NULL ) break;
			UINT ww = 0, hh = 0;
			if( FAILED(pFrame->GetSize(&ww, &hh)) || ww == 0 || hh == 0 ) break;
			iw = (int)ww; ih = (int)hh;
			if( FAILED(pFactory->CreateFormatConverter(&pConv)) || pConv == NULL ) break;
			if( FAILED(pConv->Initialize(pFrame, GUID_WICPixelFormat32bppRGBA,
				WICBitmapDitherTypeNone, NULL, 0.0, WICBitmapPaletteTypeCustom)) ) break;
			pRgba = (BYTE*)malloc((size_t)iw * ih * 4);
			if( pRgba == NULL ) break;
			if( FAILED(pConv->CopyPixels(NULL, (UINT)iw * 4, (UINT)iw * ih * 4, pRgba)) ) break;
			HBITMAP hBmp = CreatePremultHBitmapFromRgba(pRgba, iw, ih, w, h);
			if( hBmp != NULL ) {
				*ppHBitmap = hBmp;
				bOk = true;
			}
		} while (0);

		if( pRgba ) free(pRgba);
		if( pConv ) pConv->Release();
		if( pFrame ) pFrame->Release();
		if( pDecoder ) pDecoder->Release();
		if( pStream ) pStream->Release();
		if( pFactory ) pFactory->Release();
		return bOk;
	}

	CSvgBoxUI::CSvgBoxUI()
		: m_dwColor(0)
		, m_dwHoverColor(0)
		, m_dwActiveColor(0)
		, m_dwDisabledColor(0)
		, m_uButtonState(0)
		, m_hCacheBitmap(NULL)
		, m_nCacheW(0)
		, m_nCacheH(0)
		, m_dwCacheColor(0)
	{
	}

	CSvgBoxUI::~CSvgBoxUI()
	{
		ClearCache();
	}

	LPCTSTR CSvgBoxUI::GetClass() const
	{
		return _T("SvgBoxUI");
	}

	LPVOID CSvgBoxUI::GetInterface(LPCTSTR pstrName)
	{
		if( _tcsicmp(pstrName, DUI_CTR_SVGBOX) == 0 ) return static_cast<CSvgBoxUI*>(this);
		return CControlUI::GetInterface(pstrName);
	}

	UINT CSvgBoxUI::GetControlFlags() const
	{
		// 与 PreferClientHit 对齐：有热态时带 SETCURSOR，供 WM_SETCURSOR；勿回调 PreferClientHit
		if( !IsEnabled() ) return 0;
		if( m_wCursor != 0 ) return UIFLAG_SETCURSOR;
		if( m_dwHoverColor != 0 || m_dwActiveColor != 0 || HasStateVisual() )
			return UIFLAG_SETCURSOR;
		return 0;
	}

	bool CSvgBoxUI::PreferClientHit() const
	{
		if( !IsEnabled() ) return false;
		if( m_dwHoverColor != 0 || m_dwActiveColor != 0 ) return true;
		return CControlUI::PreferClientHit();
	}

	void CSvgBoxUI::SyncControlStateFromButton()
	{
		// 仅同步交互态；保留基类 FOCUSED / SELECTED 等
		const UINT kMask = UISTATE_HOT | UISTATE_PUSHED | UISTATE_CAPTURED | UISTATE_DISABLED;
		m_uControlState = (m_uControlState & ~kMask) | (m_uButtonState & kMask);
	}

	void CSvgBoxUI::EnsureInteractiveCursor()
	{
		// 已显式 cursor= 时不覆盖；有悬停/按下视觉或可点反馈时默认手型
		if( GetCursor() != 0 ) return;
		if( m_dwHoverColor != 0 || m_dwActiveColor != 0 || HasStateVisual() )
			SetCursor(DUI_HAND);
	}

	CDuiString CSvgBoxUI::ResolveFilePath(LPCTSTR pstrPath)
	{
		if( pstrPath == NULL || *pstrPath == _T('\0') ) return CDuiString();
		if( (pstrPath[0] == _T('\\') || pstrPath[0] == _T('/')) ||
			(_tcslen(pstrPath) >= 2 && pstrPath[1] == _T(':')) ) {
			return CDuiString(pstrPath);
		}
		CDuiString sFile = CPaintManagerUI::GetResourcePath();
		sFile += pstrPath;
		return sFile;
	}

	DWORD CSvgBoxUI::ParseColorValue(LPCTSTR pstrValue)
	{
		DWORD clr = 0;
		if( pstrValue != NULL && ParseColorString(pstrValue, clr) ) return clr;
		return 0;
	}

	CSvgBoxUI::TintMode CSvgBoxUI::DetectTintMode(const std::string& svgUtf8)
	{
		if( svgUtf8.empty() ) return TintBoth;

		auto has = [&](const char* s) -> bool {
			return svgUtf8.find(s) != std::string::npos;
		};

		// Twemoji 等多色图标：保留原色，不单色着色
		if( (has("fill=\"#") || has("fill='#")) && !has("currentColor") )
			return TintSkip;

		const bool bFillNone = has("fill=\"none\"") || has("fill='none'");
		const bool bStrokeCurrent = has("stroke=\"currentColor\"") || has("stroke='currentColor'");
		const bool bFillCurrent = has("fill=\"currentColor\"") || has("fill='currentColor'");
		const bool bHasStroke = has("stroke=") || has("stroke:");

		// Lucide / Tabler Outline / IconPark：描边为主
		if( bStrokeCurrent || (bFillNone && bHasStroke) )
			return TintStroke;
		// Bootstrap / Remix / Tabler Filled：填充为主
		if( bFillCurrent || !bHasStroke )
			return TintFill;
		return TintBoth;
	}

	void CSvgBoxUI::ClearCache()
	{
		if( m_hCacheBitmap != NULL ) {
			// 删除 HBITMAP 前丢掉 D2D 纹理缓存，避免句柄复用后命中旧图
			IRenderDevice* pDev = GetRenderDevice();
			if( pDev != NULL ) pDev->InvalidateBitmapGpu(m_hCacheBitmap);
			::DeleteObject(m_hCacheBitmap);
			m_hCacheBitmap = NULL;
		}
		m_nCacheW = 0;
		m_nCacheH = 0;
		m_dwCacheColor = 0;
	}

	void CSvgBoxUI::LoadFromFile(LPCTSTR pstrPath)
	{
		m_sSvgPath = pstrPath ? pstrPath : _T("");
		m_sSvgData.Empty();
		m_sSvgUtf8.clear();
		m_vBitmapData.clear();
		ClearCache();
		Invalidate();
	}

	void CSvgBoxUI::LoadFromMemory(const BYTE* pData, size_t nLen)
	{
		// 二进制图像（.ico/.png/.jpg/.bmp/.gif/.webp）内存数据，按位图显示
		m_vBitmapData.assign(pData, pData + (nLen > 0 ? nLen : 0));
		m_sSvgPath.Empty();
		m_sSvgData.Empty();
		m_sSvgUtf8.clear();
		ClearCache();
		Invalidate();
	}

	void CSvgBoxUI::LoadFromResource(LPCTSTR pstrType, UINT nID)
	{
		// 从资源读取二进制位图数据（默认 RT_RCDATA；也可显式给资源类型）
		if( pstrType == NULL ) pstrType = RT_RCDATA;
		HINSTANCE hInst = CPaintManagerUI::GetInstance();
		if( hInst == NULL ) hInst = (HINSTANCE)::GetModuleHandle(NULL);
		HRSRC hRes = ::FindResource(hInst, MAKEINTRESOURCE(nID), pstrType);
		if( hRes == NULL ) return;
		HGLOBAL hGlob = ::LoadResource(hInst, hRes);
		DWORD dwSize = ::SizeofResource(hInst, hRes);
		const BYTE* pData = (hGlob != NULL) ? static_cast<const BYTE*>(::LockResource(hGlob)) : NULL;
		if( pData == NULL || dwSize == 0 ) return;
		LoadFromMemory(pData, dwSize);
	}

	void CSvgBoxUI::LoadFromData(LPCTSTR pstrSvgContent)
	{
		m_sSvgData = pstrSvgContent ? pstrSvgContent : _T("");
		m_sSvgPath.Empty();
		m_sSvgUtf8.clear();
		m_vBitmapData.clear();
		ClearCache();
		Invalidate();
	}

	void CSvgBoxUI::LoadFromUtf8Data(const char* utf8Svg)
	{
		m_sSvgUtf8 = utf8Svg ? utf8Svg : "";
		m_sSvgPath.Empty();
		m_sSvgData.Empty();
		m_vBitmapData.clear();
		ClearCache();
		Invalidate();
	}

	void CSvgBoxUI::SetColor(DWORD dwColor, bool bInvalidate)
	{
		if( m_dwColor == dwColor ) return;
		m_dwColor = dwColor;
		ClearCache();
		if( bInvalidate ) Invalidate();
	}

	DWORD CSvgBoxUI::GetColor() const
	{
		return m_dwColor;
	}

	void CSvgBoxUI::SetHoverColor(DWORD dwColor, bool bInvalidate)
	{
		if( m_dwHoverColor == dwColor ) return;
		m_dwHoverColor = dwColor;
		ClearCache();
		EnsureInteractiveCursor();
		if( bInvalidate ) Invalidate();
	}

	DWORD CSvgBoxUI::GetHoverColor() const
	{
		return m_dwHoverColor;
	}

	void CSvgBoxUI::SetActiveColor(DWORD dwColor, bool bInvalidate)
	{
		if( m_dwActiveColor == dwColor ) return;
		m_dwActiveColor = dwColor;
		ClearCache();
		EnsureInteractiveCursor();
		if( bInvalidate ) Invalidate();
	}

	DWORD CSvgBoxUI::GetActiveColor() const
	{
		return m_dwActiveColor;
	}

	void CSvgBoxUI::SetDisabledColor(DWORD dwColor, bool bInvalidate)
	{
		if( m_dwDisabledColor == dwColor ) return;
		m_dwDisabledColor = dwColor;
		ClearCache();
		if( bInvalidate ) Invalidate();
	}

	DWORD CSvgBoxUI::GetDisabledColor() const
	{
		return m_dwDisabledColor;
	}

	void CSvgBoxUI::ApplyParentButtonState(UINT uButtonState)
	{
		const UINT kMask = UISTATE_HOT | UISTATE_PUSHED | UISTATE_DISABLED | UISTATE_CAPTURED;
		const UINT uNew = (m_uButtonState & ~kMask) | (uButtonState & kMask);
		if( uNew == m_uButtonState ) return;
		m_uButtonState = uNew;
		SyncControlStateFromButton();
	}

	void CSvgBoxUI::SetEnabled(bool bEnable)
	{
		// 状态未变时不要 Invalidate：嵌在 Button 绘制中每次 Sync 都会调到这里，
		// 否则会刷出「仅图标矩形」脏区，下一帧圆角 clip 对小矩形，四角露出窗口白底。
		if( m_bEnabled == bEnable ) {
			if( bEnable ) m_uButtonState &= ~UISTATE_DISABLED;
			else m_uButtonState |= UISTATE_DISABLED;
			SyncControlStateFromButton();
			return;
		}
		CControlUI::SetEnabled(bEnable);
		if( bEnable ) m_uButtonState &= ~UISTATE_DISABLED;
		else m_uButtonState |= UISTATE_DISABLED;
		SyncControlStateFromButton();
	}

	void CSvgBoxUI::Invalidate()
	{
		// 挂在 Button / 圆角父控件下时，只脏图标矩形会让下一帧 RoundClip 作用在小区域上，
		// GetUpdateRect 包围盒还会清到旁钮；提升为父控件整区刷新。
		CControlUI* pParent = GetParent();
		if( pParent != NULL ) {
			const bool bButtonParent = (pParent->GetInterface(DUI_CTR_BUTTON) != NULL);
			SIZE radius = pParent->GetBorderRadius();
			if( bButtonParent || radius.cx > 0 || radius.cy > 0 ) {
				pParent->Invalidate();
				return;
			}
		}
		CControlUI::Invalidate();
	}

	void CSvgBoxUI::PaintIcon(IRenderContext& ctx, const RECT& rcPaint)
	{
		if( !::IntersectRect(&m_rcPaint, &rcPaint, &m_rcItem) ) return;
		PaintStatusImage(ctx);
	}

	DWORD CSvgBoxUI::GetPaintColor() const
	{
		if( !IsEnabled() || (m_uButtonState & UISTATE_DISABLED) != 0 ) {
			if( m_dwDisabledColor != 0 ) return m_dwDisabledColor;
			return m_dwColor;
		}
		if( (m_uButtonState & UISTATE_PUSHED) != 0 && m_dwActiveColor != 0 )
			return m_dwActiveColor;
		if( (m_uButtonState & UISTATE_HOT) != 0 && m_dwHoverColor != 0 )
			return m_dwHoverColor;
		return m_dwColor;
	}

	void CSvgBoxUI::DoEvent(TEventUI& event)
	{
		if( !IsMouseEnabled() && event.Type > UIEVENT__MOUSEBEGIN && event.Type < UIEVENT__MOUSEEND ) {
			if( !BubbleEvent(event) ) CControlUI::DoEvent(event);
			return;
		}

		if( event.Type == UIEVENT_BUTTONDOWN || event.Type == UIEVENT_DBLCLICK )
		{
			if( ::PtInRect(&m_rcItem, event.ptMouse) && IsEnabled() ) {
				m_uButtonState |= UISTATE_PUSHED | UISTATE_CAPTURED;
				SyncControlStateFromButton();
				Invalidate();
			}
			return;
		}
		if( event.Type == UIEVENT_MOUSEMOVE )
		{
			if( (m_uButtonState & UISTATE_CAPTURED) != 0 ) {
				if( ::PtInRect(&m_rcItem, event.ptMouse) )
					m_uButtonState |= UISTATE_PUSHED;
				else
					m_uButtonState &= ~UISTATE_PUSHED;
				SyncControlStateFromButton();
				Invalidate();
			}
			return;
		}
		if( event.Type == UIEVENT_BUTTONUP )
		{
			if( (m_uButtonState & UISTATE_CAPTURED) != 0 ) {
				m_uButtonState &= ~(UISTATE_PUSHED | UISTATE_CAPTURED);
				SyncControlStateFromButton();
				Invalidate();
				if( ::PtInRect(&m_rcItem, event.ptMouse) && IsEnabled() && m_pManager != NULL )
					m_pManager->SendNotify(this, DUI_MSGTYPE_CLICK);
			}
			return;
		}
		if( event.Type == UIEVENT_MOUSEENTER )
		{
			if( IsEnabled() ) {
				m_uButtonState |= UISTATE_HOT;
				SyncControlStateFromButton();
				Invalidate();
			}
			return;
		}
		if( event.Type == UIEVENT_MOUSELEAVE )
		{
			if( IsEnabled() ) {
				m_uButtonState &= ~UISTATE_HOT;
				SyncControlStateFromButton();
				Invalidate();
			}
			return;
		}
		CControlUI::DoEvent(event);
	}

	void CSvgBoxUI::SetAttribute(LPCTSTR pstrName, LPCTSTR pstrValue)
	{
		if( _tcsicmp(pstrName, _T("src")) == 0 ) {
			LoadFromFile(pstrValue);
		}
		else if( _tcsicmp(pstrName, _T("data")) == 0 ) {
			LoadFromData(pstrValue);
		}
		else if( _tcsicmp(pstrName, _T("bsicon")) == 0 ) {
			LoadFromUtf8Data(BootstrapIcons::GetIcon(pstrValue));
		}
		else if( _tcsicmp(pstrName, _T("iconpark")) == 0 ) {
			LoadFromUtf8Data(IconParkIcons::GetIcon(pstrValue));
		}
		else if( _tcsicmp(pstrName, _T("lucide")) == 0 ) {
			LoadFromUtf8Data(LucideIcons::GetIcon(pstrValue));
		}
		else if( _tcsicmp(pstrName, _T("tabler-filled")) == 0 ) {
			LoadFromUtf8Data(TablerFilledIcons::GetIcon(pstrValue));
		}
		else if( _tcsicmp(pstrName, _T("tabler-outline")) == 0 ) {
			LoadFromUtf8Data(TablerOutlineIcons::GetIcon(pstrValue));
		}
		else if( _tcsicmp(pstrName, _T("remixicon")) == 0 ) {
			LoadFromUtf8Data(RemixIconIcons::GetIcon(pstrValue));
		}
		else if( _tcsicmp(pstrName, _T("twicon")) == 0 ) {
			LoadFromUtf8Data(TwemojiIcons::GetIcon(pstrValue));
		}
		else if( _tcsicmp(pstrName, _T("color")) == 0
			|| _tcsicmp(pstrName, _T("color-hover")) == 0
			|| _tcsicmp(pstrName, _T("color-active")) == 0
			|| _tcsicmp(pstrName, _T("color-disabled")) == 0
			|| _tcsicmp(pstrName, _T("tint")) == 0
			|| _tcsicmp(pstrName, _T("tint-hover")) == 0
			|| _tcsicmp(pstrName, _T("tint-active")) == 0
			|| _tcsicmp(pstrName, _T("tint-disabled")) == 0
			|| _tcsicmp(pstrName, _T("fill")) == 0 ) {
			// 记录 var(--token)，热切主题时由 RefreshThemeVarAttributes 重解
			if( pstrValue != NULL && _tcsnicmp(pstrValue, _T("var("), 4) == 0 ) {
				CDuiString key;
				key.Format(_T("_tvar:%s"), pstrName);
				AddCustomAttribute(key.GetData(), pstrValue);
			}
			if( _tcsicmp(pstrName, _T("color")) == 0 || _tcsicmp(pstrName, _T("tint")) == 0
				|| _tcsicmp(pstrName, _T("fill")) == 0 )
				SetColor(ParseColorValue(pstrValue));
			else if( _tcsicmp(pstrName, _T("color-hover")) == 0 || _tcsicmp(pstrName, _T("tint-hover")) == 0 )
				SetHoverColor(ParseColorValue(pstrValue));
			else if( _tcsicmp(pstrName, _T("color-active")) == 0 || _tcsicmp(pstrName, _T("tint-active")) == 0 )
				SetActiveColor(ParseColorValue(pstrValue));
			else
				SetDisabledColor(ParseColorValue(pstrValue));
			EnsureInteractiveCursor();
		}
		else {
			CControlUI::SetAttribute(pstrName, pstrValue);
			EnsureInteractiveCursor();
		}
	}

	static bool LoadSvgDocument(const CDuiString& sPath, const CDuiString& sData, const std::string& sUtf8,
		std::unique_ptr<lunasvg::Document>& document, std::string& sProbe)
	{
		document.reset();
		sProbe.clear();
		if( !sPath.IsEmpty() ) {
			BYTE* pSvg = NULL;
			DWORD nSvg = 0;
			if( CPaintManagerUI::LoadResourceData(sPath.GetData(), &pSvg, &nSvg) && pSvg != NULL && nSvg > 0 ) {
				sProbe.assign(reinterpret_cast<const char*>(pSvg), (size_t)nSvg);
				delete[] pSvg;
				document = lunasvg::Document::loadFromData(sProbe);
			}
			else {
				CDuiString sFile = ResolveSvgFilePathLocal(sPath.GetData());
				document = lunasvg::Document::loadFromFile(DuiStringToUtf8(sFile.GetData()));
			}
		}
		else if( !sUtf8.empty() ) {
			sProbe = sUtf8;
			document = lunasvg::Document::loadFromData(sUtf8);
		}
		else if( !sData.IsEmpty() ) {
			sProbe = DuiStringToUtf8(sData.GetData());
			document = lunasvg::Document::loadFromData(sProbe);
		}
		return document.get() != NULL;
	}

	static void ApplySvgTint(lunasvg::Document& document, const std::string& sProbe, DWORD dwColor)
	{
		if( dwColor == 0 ) return;
		const SvgTintModeLocal mode = sProbe.empty() ? SvgTintBoth : DetectTintModeLocal(sProbe);
		if( mode == SvgTintSkip ) return;
		const BYTE r = DuiColorR(dwColor);
		const BYTE g = DuiColorG(dwColor);
		const BYTE b = DuiColorB(dwColor);
		char style[256];
		if( mode == SvgTintStroke ) {
			sprintf_s(style, sizeof(style),
				"* { fill: none; stroke: #%02x%02x%02x; }", r, g, b);
		}
		else if( mode == SvgTintFill ) {
			sprintf_s(style, sizeof(style),
				"* { fill: #%02x%02x%02x; stroke: none; }", r, g, b);
		}
		else {
			sprintf_s(style, sizeof(style),
				"* { fill: #%02x%02x%02x; stroke: #%02x%02x%02x; }",
				r, g, b, r, g, b);
		}
		document.applyStyleSheet(style);
	}

	static bool RenderSvgToLunaBitmap(const CDuiString& sPath, const CDuiString& sData, const std::string& sUtf8,
		int w, int h, DWORD dwColor, lunasvg::Bitmap& out)
	{
		out = lunasvg::Bitmap();
		if( w <= 0 || h <= 0 ) return false;
		std::unique_ptr<lunasvg::Document> document;
		std::string sProbe;
		if( !LoadSvgDocument(sPath, sData, sUtf8, document, sProbe) ) return false;
		ApplySvgTint(*document, sProbe, dwColor);
		out = document->renderToBitmap(w, h);
		return !out.isNull();
	}

	static int GetImageEncoderClsid(const WCHAR* format, CLSID* pClsid)
	{
		UINT num = 0, size = 0;
		Gdiplus::GetImageEncodersSize(&num, &size);
		if( size == 0 ) return -1;
		Gdiplus::ImageCodecInfo* pInfo = (Gdiplus::ImageCodecInfo*)(malloc(size));
		if( pInfo == NULL ) return -1;
		Gdiplus::GetImageEncoders(num, size, pInfo);
		for( UINT i = 0; i < num; ++i ) {
			if( wcscmp(pInfo[i].MimeType, format) == 0 ) {
				*pClsid = pInfo[i].Clsid;
				free(pInfo);
				return (int)i;
			}
		}
		free(pInfo);
		return -1;
	}

	static Gdiplus::Bitmap* CreateGdipBitmapFromLuna(const lunasvg::Bitmap& src, bool bStraightAlpha)
	{
		const int w = src.width();
		const int h = src.height();
		if( w <= 0 || h <= 0 || src.isNull() ) return NULL;
		Gdiplus::Bitmap* pBmp = new Gdiplus::Bitmap(w, h,
			bStraightAlpha ? PixelFormat32bppARGB : PixelFormat32bppPARGB);
		if( pBmp == NULL || pBmp->GetLastStatus() != Gdiplus::Ok ) {
			delete pBmp;
			return NULL;
		}
		Gdiplus::BitmapData bd;
		Gdiplus::Rect rc(0, 0, w, h);
		if( pBmp->LockBits(&rc, Gdiplus::ImageLockModeWrite,
			bStraightAlpha ? PixelFormat32bppARGB : PixelFormat32bppPARGB, &bd) != Gdiplus::Ok ) {
			delete pBmp;
			return NULL;
		}
		const uint8_t* pSrc = src.data();
		const int srcStride = src.stride();
		for( int y = 0; y < h; ++y ) {
			BYTE* pDst = (BYTE*)bd.Scan0 + y * bd.Stride;
			const uint8_t* pRow = pSrc + y * srcStride;
			if( !bStraightAlpha ) {
				memcpy(pDst, pRow, (size_t)w * 4);
				continue;
			}
			for( int x = 0; x < w; ++x ) {
				// lunasvg / GDI+ PARGB：预乘 BGRA
				BYTE b = pRow[x * 4 + 0];
				BYTE g = pRow[x * 4 + 1];
				BYTE r = pRow[x * 4 + 2];
				const BYTE a = pRow[x * 4 + 3];
				if( a > 0 && a < 255 ) {
					r = (BYTE)((r * 255) / a);
					g = (BYTE)((g * 255) / a);
					b = (BYTE)((b * 255) / a);
				}
				else if( a == 0 ) {
					r = g = b = 0;
				}
				pDst[x * 4 + 0] = b;
				pDst[x * 4 + 1] = g;
				pDst[x * 4 + 2] = r;
				pDst[x * 4 + 3] = a;
			}
		}
		pBmp->UnlockBits(&bd);
		return pBmp;
	}

	static bool SavePngToMemory(Gdiplus::Bitmap* pBmp, std::vector<BYTE>& out)
	{
		out.clear();
		if( pBmp == NULL ) return false;
		CLSID clsid;
		if( GetImageEncoderClsid(L"image/png", &clsid) < 0 ) return false;
		IStream* pStm = NULL;
		if( FAILED(::CreateStreamOnHGlobal(NULL, TRUE, &pStm)) || pStm == NULL ) return false;
		if( pBmp->Save(pStm, &clsid, NULL) != Gdiplus::Ok ) {
			pStm->Release();
			return false;
		}
		HGLOBAL hMem = NULL;
		if( FAILED(::GetHGlobalFromStream(pStm, &hMem)) || hMem == NULL ) {
			pStm->Release();
			return false;
		}
		SIZE_T n = ::GlobalSize(hMem);
		const void* p = ::GlobalLock(hMem);
		if( p == NULL || n == 0 ) {
			if( p ) ::GlobalUnlock(hMem);
			pStm->Release();
			return false;
		}
		out.resize((size_t)n);
		memcpy(out.data(), p, (size_t)n);
		::GlobalUnlock(hMem);
		pStm->Release();
		return !out.empty();
	}

	static bool WriteIcoFromPngImages(LPCTSTR pstrPath,
		const std::vector<std::vector<BYTE> >& pngs, const std::vector<int>& sizes)
	{
		if( pstrPath == NULL || pngs.empty() || pngs.size() != sizes.size() ) return false;
		const WORD nCount = (WORD)pngs.size();
		if( nCount == 0 ) return false;

#pragma pack(push, 1)
		struct ICONDIR { WORD idReserved; WORD idType; WORD idCount; };
		struct ICONDIRENTRY {
			BYTE bWidth; BYTE bHeight; BYTE bColorCount; BYTE bReserved;
			WORD wPlanes; WORD wBitCount; DWORD dwBytesInRes; DWORD dwImageOffset;
		};
#pragma pack(pop)

		HANDLE hFile = ::CreateFile(pstrPath, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
		if( hFile == INVALID_HANDLE_VALUE ) return false;

		ICONDIR dir = { 0, 1, nCount };
		DWORD written = 0;
		if( !::WriteFile(hFile, &dir, sizeof(dir), &written, NULL) ) {
			::CloseHandle(hFile);
			::DeleteFile(pstrPath);
			return false;
		}

		DWORD offset = (DWORD)(sizeof(ICONDIR) + sizeof(ICONDIRENTRY) * nCount);
		std::vector<ICONDIRENTRY> ents(nCount);
		for( WORD i = 0; i < nCount; ++i ) {
			const int s = sizes[i];
			ICONDIRENTRY& ent = ents[i];
			::ZeroMemory(&ent, sizeof(ent));
			ent.bWidth = (s >= 256) ? 0 : (BYTE)s;
			ent.bHeight = (s >= 256) ? 0 : (BYTE)s;
			ent.wPlanes = 1;
			ent.wBitCount = 32;
			ent.dwBytesInRes = (DWORD)pngs[i].size();
			ent.dwImageOffset = offset;
			offset += ent.dwBytesInRes;
		}
		for( WORD i = 0; i < nCount; ++i ) {
			if( !::WriteFile(hFile, &ents[i], sizeof(ICONDIRENTRY), &written, NULL) ) {
				::CloseHandle(hFile);
				::DeleteFile(pstrPath);
				return false;
			}
		}
		for( WORD i = 0; i < nCount; ++i ) {
			if( pngs[i].empty()
				|| !::WriteFile(hFile, pngs[i].data(), (DWORD)pngs[i].size(), &written, NULL) ) {
				::CloseHandle(hFile);
				::DeleteFile(pstrPath);
				return false;
			}
		}
		::CloseHandle(hFile);
		return true;
	}

	static bool RenderSizeToPng(const CDuiString& sPath, const CDuiString& sData, const std::string& sUtf8,
		int size, DWORD dwColor, std::vector<BYTE>& pngOut)
	{
		pngOut.clear();
		if( size < 1 || size > 512 ) return false;
		lunasvg::Bitmap luna;
		if( !RenderSvgToLunaBitmap(sPath, sData, sUtf8, size, size, dwColor, luna) )
			return false;
		Gdiplus::Bitmap* pBmp = CreateGdipBitmapFromLuna(luna, true);
		if( pBmp == NULL ) return false;
		const bool ok = SavePngToMemory(pBmp, pngOut);
		delete pBmp;
		return ok && !pngOut.empty();
	}

	static CDuiString GetPathExtLower(LPCTSTR pstrPath)
	{
		CDuiString s;
		if( pstrPath == NULL ) return s;
		LPCTSTR pDot = _tcsrchr(pstrPath, _T('.'));
		if( pDot == NULL ) return s;
		s = pDot;
		s.MakeLower();
		return s;
	}

	bool CSvgBoxUI::EnsureCache(int w, int h, DWORD dwColor)
	{
		if( w <= 0 || h <= 0 ) return false;
		if( m_hCacheBitmap != NULL && m_nCacheW == w && m_nCacheH == h && m_dwCacheColor == dwColor )
			return true;

		ClearCache();
		if( m_sSvgPath.IsEmpty() && m_sSvgData.IsEmpty() && m_sSvgUtf8.empty() ) return false;

		// 位图源（favicon .ico/.png/.jpg/.bmp/.gif/.webp）：文件路径 via m_sSvgPath，或内存/资源 via m_vBitmapData。
		// 按图片解码并等比缩放，不做 SVG 着色；走与 SVG 相同的 m_hCacheBitmap 预乘 HBITMAP，供原样绘制。
		if( (!m_vBitmapData.empty() || (IsBitmapFileExt(m_sSvgPath.GetData()) && m_sSvgData.IsEmpty() && m_sSvgUtf8.empty()))
			&& m_sSvgData.IsEmpty() && m_sSvgUtf8.empty() ) {
			if( !m_vBitmapData.empty() )
				m_hCacheBitmap = CreatePremultHBitmapFromMemory(m_vBitmapData.data(), m_vBitmapData.size(), w, h);
			else {
				CDuiString sRes = ResolveFilePath(m_sSvgPath.GetData());
				if( sRes.IsEmpty() ) return false;
				m_hCacheBitmap = CreatePremultHBitmapFromFile(sRes.GetData(), w, h);
			}
			if( m_hCacheBitmap == NULL ) return false;
			m_nCacheW = w;
			m_nCacheH = h;
			m_dwCacheColor = dwColor;   // 位图不参与着色，仅用于缓存失效键
			return true;
		}

		lunasvg::Bitmap bitmap;
		if( !RenderSvgToLunaBitmap(m_sSvgPath, m_sSvgData, m_sSvgUtf8, w, h, dwColor, bitmap) )
			return false;

		m_hCacheBitmap = CreatePremultHBitmap(bitmap);
		if( m_hCacheBitmap == NULL ) return false;
		m_nCacheW = w;
		m_nCacheH = h;
		m_dwCacheColor = dwColor;
		return true;
	}

	HBITMAP CSvgBoxUI::RasterizeToHBitmap(const char* utf8Svg, size_t nBytes,
		int width, int height, DWORD dwTintColor, int* pOutW, int* pOutH)
	{
		if( utf8Svg == NULL || nBytes == 0 ) return NULL;
		std::string sUtf8(utf8Svg, nBytes);
		std::unique_ptr<lunasvg::Document> document = lunasvg::Document::loadFromData(sUtf8);
		if( !document ) return NULL;

		int w = width;
		int h = height;
		if( w <= 0 ) w = (int)(document->width() + 0.5);
		if( h <= 0 ) h = (int)(document->height() + 0.5);
		if( w <= 0 ) w = 256;
		if( h <= 0 ) h = 256;
		if( w > 4096 ) w = 4096;
		if( h > 4096 ) h = 4096;

		DWORD dwTint = dwTintColor;
		if( dwTint == (DWORD)-1 ) dwTint = 0;

		CDuiString sEmpty;
		lunasvg::Bitmap bitmap;
		if( !RenderSvgToLunaBitmap(sEmpty, sEmpty, sUtf8, w, h, dwTint, bitmap) )
			return NULL;
		HBITMAP hBmp = CreatePremultHBitmap(bitmap);
		if( hBmp == NULL ) return NULL;
		if( pOutW ) *pOutW = bitmap.width();
		if( pOutH ) *pOutH = bitmap.height();
		return hBmp;
	}

	HBITMAP CSvgBoxUI::RasterizeToHBitmap(LPCTSTR pstrSvg,
		int width, int height, DWORD dwTintColor, int* pOutW, int* pOutH)
	{
		if( pstrSvg == NULL || *pstrSvg == _T('\0') ) return NULL;
		std::string sUtf8 = DuiStringToUtf8(pstrSvg);
		return RasterizeToHBitmap(sUtf8.c_str(), sUtf8.size(), width, height, dwTintColor, pOutW, pOutH);
	}

	bool CSvgBoxUI::ExportToIcoFile(LPCTSTR pstrPath, DWORD dwTintColor) const
	{
		// Windows 壳图标常规上限 256；多帧覆盖托盘/列表/桌面/资源管理器等场景
		static const int kDefaultSizes[] = { 16, 24, 32, 48, 64, 128, 256 };
		return ExportToIcoFile(pstrPath, kDefaultSizes, 7, dwTintColor);
	}

	bool CSvgBoxUI::ExportToIcoFile(LPCTSTR pstrPath, const int* pSizes, int nCount, DWORD dwTintColor) const
	{
		if( pstrPath == NULL || *pstrPath == _T('\0') || pSizes == NULL || nCount <= 0 )
			return false;
		if( m_sSvgPath.IsEmpty() && m_sSvgData.IsEmpty() && m_sSvgUtf8.empty() ) return false;

		DWORD dwColor = dwTintColor;
		if( dwTintColor == (DWORD)-1 )
			dwColor = GetPaintColor();

		std::vector<std::vector<BYTE> > pngs;
		std::vector<int> sizes;
		pngs.reserve((size_t)nCount);
		sizes.reserve((size_t)nCount);

		for( int i = 0; i < nCount; ++i ) {
			int s = pSizes[i];
			if( s < 1 ) continue;
			if( s > 512 ) s = 512;
			// 去重（同尺寸只留一份）
			bool bDup = false;
			for( size_t j = 0; j < sizes.size(); ++j ) {
				if( sizes[j] == s ) { bDup = true; break; }
			}
			if( bDup ) continue;

			std::vector<BYTE> png;
			if( !RenderSizeToPng(m_sSvgPath, m_sSvgData, m_sSvgUtf8, s, dwColor, png) )
				continue;
			sizes.push_back(s);
			pngs.push_back(png);
		}
		if( pngs.empty() ) return false;
		return WriteIcoFromPngImages(pstrPath, pngs, sizes);
	}

	bool CSvgBoxUI::ExportToFile(LPCTSTR pstrPath, int width, int height, DWORD dwTintColor, int jpegQuality) const
	{
		if( pstrPath == NULL || *pstrPath == _T('\0') ) return false;
		if( m_sSvgPath.IsEmpty() && m_sSvgData.IsEmpty() && m_sSvgUtf8.empty() ) return false;

		CDuiString ext = GetPathExtLower(pstrPath);
		if( ext == _T(".ico") ) {
			// .ico 走专用接口：未指定尺寸 → 多尺寸；指定则单尺寸正方形
			int s = width;
			if( s <= 0 ) s = height;
			if( s <= 0 )
				return ExportToIcoFile(pstrPath, dwTintColor);
			if( s > 512 ) s = 512;
			return ExportToIcoFile(pstrPath, &s, 1, dwTintColor);
		}

		int w = width;
		int h = height;
		if( w <= 0 ) w = (int)(m_rcItem.right - m_rcItem.left);
		if( h <= 0 ) h = (int)(m_rcItem.bottom - m_rcItem.top);
		if( w <= 0 ) w = GetFixedWidth();
		if( h <= 0 ) h = GetFixedHeight();
		if( w <= 0 ) w = 256;
		if( h <= 0 ) h = 256;

		DWORD dwColor = dwTintColor;
		if( dwTintColor == (DWORD)-1 )
			dwColor = GetPaintColor();

		lunasvg::Bitmap luna;
		if( !RenderSvgToLunaBitmap(m_sSvgPath, m_sSvgData, m_sSvgUtf8, w, h, dwColor, luna) )
			return false;

		const bool bJpg = (ext == _T(".jpg") || ext == _T(".jpeg"));
		const bool bBmp = (ext == _T(".bmp"));

		Gdiplus::Bitmap* pBmp = NULL;
		if( bJpg ) {
			Gdiplus::Bitmap* pSrc = CreateGdipBitmapFromLuna(luna, true);
			if( pSrc == NULL ) return false;
			pBmp = new Gdiplus::Bitmap(w, h, PixelFormat24bppRGB);
			if( pBmp == NULL || pBmp->GetLastStatus() != Gdiplus::Ok ) {
				delete pSrc;
				delete pBmp;
				return false;
			}
			Gdiplus::Graphics g(pBmp);
			g.Clear(Gdiplus::Color(255, 255, 255));
			g.DrawImage(pSrc, 0, 0, w, h);
			delete pSrc;
		}
		else {
			pBmp = CreateGdipBitmapFromLuna(luna, true);
			if( pBmp == NULL ) return false;
		}

		bool ok = false;
		CLSID clsid;
		const WCHAR* mime = L"image/png";
		if( bJpg ) mime = L"image/jpeg";
		else if( bBmp ) mime = L"image/bmp";
		if( GetImageEncoderClsid(mime, &clsid) >= 0 ) {
#ifdef _UNICODE
			const WCHAR* wsz = pstrPath;
#else
			WCHAR wsz[MAX_PATH];
			::MultiByteToWideChar(CP_ACP, 0, pstrPath, -1, wsz, MAX_PATH);
#endif
			if( bJpg ) {
				if( jpegQuality < 1 ) jpegQuality = 1;
				if( jpegQuality > 100 ) jpegQuality = 100;
				Gdiplus::EncoderParameters params;
				params.Count = 1;
				params.Parameter[0].Guid = Gdiplus::EncoderQuality;
				params.Parameter[0].Type = Gdiplus::EncoderParameterValueTypeLong;
				params.Parameter[0].NumberOfValues = 1;
				ULONG quality = (ULONG)jpegQuality;
				params.Parameter[0].Value = &quality;
				ok = (pBmp->Save(wsz, &clsid, &params) == Gdiplus::Ok);
			}
			else {
				ok = (pBmp->Save(wsz, &clsid, NULL) == Gdiplus::Ok);
			}
		}
		delete pBmp;
		return ok;
	}

	void CSvgBoxUI::PaintStatusImage(IRenderContext& ctx)
	{
		if( !IsEnabled() ) m_uButtonState |= UISTATE_DISABLED;
		else m_uButtonState &= ~UISTATE_DISABLED;

		const int w = (int)(m_rcItem.right - m_rcItem.left);
		const int h = (int)(m_rcItem.bottom - m_rcItem.top);
		const DWORD dwColor = GetPaintColor();
		if( !EnsureCache(w, h, dwColor) || m_hCacheBitmap == NULL ) return;

		RECT rcBmpPart = { 0, 0, m_nCacheW, m_nCacheH };
		RECT rcCorners = { 0, 0, 0, 0 };
		ctx.DrawImage(m_hCacheBitmap, m_rcItem, m_rcPaint, rcBmpPart, rcCorners, true, ScaleImageFade());
	}
}
