#ifndef __UIRENDER_H__
#define __UIRENDER_H__

#pragma once

#ifdef USE_XIMAGE_EFFECT
class CxImage;
#endif

namespace DuiLib {
	/////////////////////////////////////////////////////////////////////////////////////
	//

	// GDI 后端内部裁剪实现；控件绘制请使用 IRenderContext::PushClip / CRenderClipScope。
	class UILIB_API CRenderClip
	{
	public:
		~CRenderClip();
		RECT rcItem;
		HDC hDC;
		HRGN hRgn;
		HRGN hOldRgn;

		static void GenerateClip(HDC hDC, RECT rc, CRenderClip& clip);
		// 圆角裁剪；width/height 为 CSS 半径（内部转成 CreateRoundRectRgn 椭圆直径）
		static void GenerateRoundClip(HDC hDC, RECT rc, RECT rcItem, int width, int height, CRenderClip& clip);
		static void UseOldClipBegin(HDC hDC, CRenderClip& clip);
		static void UseOldClipEnd(HDC hDC, CRenderClip& clip);
	};

	/////////////////////////////////////////////////////////////////////////////////////
	//

	class UILIB_API CRenderEngine
	{
	public:
		// 图片加载（GDI 实现；业务侧优先走 IRenderDevice::LoadImage）
#ifdef USE_XIMAGE_EFFECT
		static CxImage *LoadGifImageX(STRINGorID bitmap, LPCTSTR type = NULL, DWORD mask = 0);
#endif
		static TImageInfo* LoadImage(STRINGorID bitmap, LPCTSTR type = NULL, DWORD mask = 0, HINSTANCE instance = NULL);
		static void FreeImage(TImageInfo* pImageInfo, bool bDelete = true);
		static TImageInfo* LoadImage(LPCTSTR pStrImage, LPCTSTR type = NULL, DWORD mask = 0, HINSTANCE instance = NULL);
		static TImageInfo* LoadImage(UINT nID, LPCTSTR type = NULL, DWORD mask = 0, HINSTANCE instance = NULL);

		static void DrawImage(HDC hDC, HBITMAP hBitmap, const RECT& rc, const RECT& rcPaint, const RECT& rcBmpPart, const RECT& rcCorners, bool bAlpha, UINT uFade = 255, bool hole = false, bool xtiled = false, bool ytiled = false);
		static bool DrawImageInfo(HDC hDC, CPaintManagerUI* pManager, const RECT& rcItem, const RECT& rcPaint, const TDrawInfo* pDrawInfo, HINSTANCE instance = NULL);
		static bool DrawImageString(HDC hDC, CPaintManagerUI* pManager, const RECT& rcItem, const RECT& rcPaint, LPCTSTR pStrImage, LPCTSTR pStrModify = NULL, HINSTANCE instance = NULL);

		// Gdiplus绘制
		static TImageInfo* GdiplusLoadImage(STRINGorID bitmap, LPCTSTR type = NULL, DWORD mask = 0, HINSTANCE instance = NULL);
		static void GdiplusDrawImage(HDC hDC, Gdiplus::Image* image, const RECT& rc, const RECT& rcPaint, const RECT& rcBmpPart, bool bAlpha, UINT uFade = 255, UINT uRotate = 0);
		static void GdiplusDrawText(HDC hDC, CPaintManagerUI* pManager, RECT& rc, LPCTSTR pstrText, DWORD dwColor, int iFont, UINT uStyle);

		// 以下函数中的颜色参数alpha值无效
		// 图元绘制
		static void DrawColor(HDC hDC, const RECT& rc, DWORD color);
		static void DrawGradient(HDC hDC, const RECT& rc, DWORD dwFirst, DWORD dwSecond, bool bVertical, int nSteps);
		static void DrawLine(HDC hDC, const RECT& rc, int nSize, DWORD dwPenColor,int nStyle = PS_SOLID);
		static void DrawRect(HDC hDC, const RECT& rc, int nSize, DWORD dwPenColor,int nStyle = PS_SOLID);
		// width/height 为 CSS 圆角半径（GDI RoundRect 内部用直径 = 半径×2）
		static void DrawRoundRect(HDC hDC, const RECT& rc, int width, int height, int nSize, DWORD dwPenColor,int nStyle = PS_SOLID);
		static void FillRoundRect(HDC hDC, const RECT& rc, int width, int height, DWORD dwColor);

		// 字体绘制
		static void DrawText(HDC hDC, CPaintManagerUI* pManager, RECT& rc, LPCTSTR pstrText,DWORD dwColor, int iFont, UINT uStyle, DWORD dwTextBKColor);
		static void DrawText(HDC hDC, CPaintManagerUI* pManager, RECT& rc, LPCTSTR pstrText, DWORD dwColor, int iFont, UINT uStyle);
		static void DrawHtmlText(HDC hDC, CPaintManagerUI* pManager, RECT& rc, LPCTSTR pstrText, DWORD dwColor, RECT* pLinks, CDuiString* sLinks, int& nLinkRects, int iFont, UINT uStyle);

		// 辅助函数
		static void CheckAlphaColor(DWORD& dwColor);
		static DWORD AdjustColor(DWORD dwColor, short H, short S, short L);

		static HBITMAP CreateARGB32Bitmap(HDC hDC, int cx, int cy, BYTE** pBits);
		static void AdjustImage(bool bUseHSL, TImageInfo* imageInfo, short H, short S, short L);

		static HBITMAP GenerateBitmap(CPaintManagerUI* pManager, RECT rc, CControlUI* pStopControl = NULL, DWORD dwFilterColor = 0);
		static HBITMAP GenerateBitmap(CPaintManagerUI* pManager, CControlUI* pControl, RECT rc, DWORD dwFilterColor = 0);
		static SIZE GetTextSize(HDC hDC, CPaintManagerUI* pManager , LPCTSTR pstrText, int iFont, UINT uStyle);

	};

	// 按对齐方式计算图片目标区域（GDI / D2D 共用）
	UILIB_API bool MakeImageDest(const RECT& rcControl, const CDuiSize& szImage, const CDuiString& sAlign, const RECT& rcPadding, RECT& rcDest);

} // namespace DuiLib

#endif // __UIRENDER_H__
