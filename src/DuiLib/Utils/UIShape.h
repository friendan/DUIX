#ifndef __UISHAPE_H__
#define __UISHAPE_H__

#pragma once

namespace DuiLib
{
	struct tagTImageInfo;
	typedef struct tagTImageInfo TImageInfo;

	/// 从预乘/非预乘 BGRA（自上而下）按 alpha 建 HRGN；调用方 DeleteObject。
	/// stride：字节/行，0 表示 width*4。
	UILIB_API HRGN CreateRegionFromAlphaBGRA(const BYTE* pBgra, int width, int height,
		int stride = 0, BYTE alphaThreshold = 16);

	/// 按目标尺寸采样后建 RGN（窗口/控件拉伸异形）。
	UILIB_API HRGN CreateScaledRegionFromAlphaBGRA(const BYTE* pBgra, int srcW, int srcH,
		int destW, int destH, int stride = 0, BYTE alphaThreshold = 16);

	/// 从 HBITMAP（优先 DIBSECTION）取 BGRA 建 RGN；失败返回 NULL。
	UILIB_API HRGN CreateRegionFromAlphaBitmap(HBITMAP hBitmap, BYTE alphaThreshold = 16);
	UILIB_API HRGN CreateScaledRegionFromAlphaBitmap(HBITMAP hBitmap, int destW, int destH,
		BYTE alphaThreshold = 16);

	/// TImageInfo（用 hBitmap）；destW/H<=0 用图原始尺寸。
	UILIB_API HRGN CreateRegionFromAlphaImage(const TImageInfo* pInfo, BYTE alphaThreshold = 16,
		int destW = 0, int destH = 0);

	/// 点测（图坐标，自上而下）；无 alpha 通道或空图视为命中。
	UILIB_API bool HitTestAlphaBGRA(const BYTE* pBgra, int width, int height, int x, int y,
		int stride = 0, BYTE alphaThreshold = 16);

	/// 控件矩形内点 → 拉伸映射到图后测 alpha。
	UILIB_API bool HitTestAlphaInDestRect(const BYTE* pBgra, int srcW, int srcH,
		const RECT& rcDest, POINT ptClient, int stride = 0, BYTE alphaThreshold = 16);

	/// 从 HBITMAP 读出自上而下 BGRA；*ppBits 由调用方 delete[]。失败返回 false。
	UILIB_API bool CopyBitmapAlphaBits(HBITMAP hBitmap, BYTE** ppBits, int* pWidth, int* pHeight,
		int* pStride);
}

#endif // __UISHAPE_H__
