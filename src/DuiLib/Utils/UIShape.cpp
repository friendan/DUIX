#include "StdAfx.h"
#include "UIShape.h"
#include "Core/UIManager.h"

namespace DuiLib
{
	namespace
	{
		HRGN BuildRegionFromRuns(const BYTE* pBgra, int width, int height, int stride,
			BYTE alphaThreshold, int destW, int destH)
		{
			if( pBgra == NULL || width < 1 || height < 1 || destW < 1 || destH < 1 )
				return NULL;
			if( stride <= 0 ) stride = width * 4;

			const bool bScale = (destW != width || destH != height);
			const int scanW = bScale ? destW : width;
			const int scanH = bScale ? destH : height;

			const int maxRects = scanH * ((scanW / 2) + 2);
			const DWORD cbHeader = sizeof(RGNDATAHEADER);
			const DWORD cbData = (DWORD)maxRects * sizeof(RECT);
			RGNDATA* pData = (RGNDATA*)malloc(cbHeader + cbData);
			if( pData == NULL ) return NULL;
			memset(pData, 0, cbHeader);
			pData->rdh.dwSize = cbHeader;
			pData->rdh.iType = RDH_RECTANGLES;
			pData->rdh.nCount = 0;
			pData->rdh.nRgnSize = 0;
			pData->rdh.rcBound.left = 0;
			pData->rdh.rcBound.top = 0;
			pData->rdh.rcBound.right = scanW;
			pData->rdh.rcBound.bottom = scanH;

			RECT* pRect = (RECT*)pData->Buffer;
			for( int y = 0; y < scanH; ++y ) {
				int sy = bScale ? (y * height / scanH) : y;
				if( sy >= height ) sy = height - 1;
				const BYTE* row = pBgra + sy * stride;
				int x = 0;
				while( x < scanW ) {
					while( x < scanW ) {
						int sx = bScale ? (x * width / scanW) : x;
						if( sx >= width ) sx = width - 1;
						if( row[sx * 4 + 3] >= alphaThreshold ) break;
						++x;
					}
					if( x >= scanW ) break;
					const int x0 = x;
					while( x < scanW ) {
						int sx = bScale ? (x * width / scanW) : x;
						if( sx >= width ) sx = width - 1;
						if( row[sx * 4 + 3] < alphaThreshold ) break;
						++x;
					}
					if( (int)pData->rdh.nCount >= maxRects ) break;
					RECT& rc = pRect[pData->rdh.nCount++];
					rc.left = x0;
					rc.top = y;
					rc.right = x;
					rc.bottom = y + 1;
				}
			}

			HRGN hRgn = NULL;
			if( pData->rdh.nCount > 0 )
				hRgn = ::ExtCreateRegion(NULL, cbHeader + pData->rdh.nCount * sizeof(RECT), pData);
			free(pData);
			return hRgn;
		}
	}

	HRGN CreateRegionFromAlphaBGRA(const BYTE* pBgra, int width, int height, int stride, BYTE alphaThreshold)
	{
		return BuildRegionFromRuns(pBgra, width, height, stride, alphaThreshold, width, height);
	}

	HRGN CreateScaledRegionFromAlphaBGRA(const BYTE* pBgra, int srcW, int srcH,
		int destW, int destH, int stride, BYTE alphaThreshold)
	{
		return BuildRegionFromRuns(pBgra, srcW, srcH, stride, alphaThreshold, destW, destH);
	}

	bool CopyBitmapAlphaBits(HBITMAP hBitmap, BYTE** ppBits, int* pWidth, int* pHeight, int* pStride)
	{
		if( ppBits == NULL || hBitmap == NULL ) return false;
		*ppBits = NULL;
		if( pWidth ) *pWidth = 0;
		if( pHeight ) *pHeight = 0;
		if( pStride ) *pStride = 0;

		DIBSECTION ds = { 0 };
		if( ::GetObject(hBitmap, sizeof(ds), &ds) < (int)sizeof(BITMAP) )
			return false;
		const int w = ds.dsBm.bmWidth;
		const int h = abs(ds.dsBm.bmHeight);
		if( w < 1 || h < 1 ) return false;

		if( ds.dsBm.bmBits != NULL && ds.dsBm.bmBitsPixel == 32 ) {
			const int stride = ds.dsBm.bmWidthBytes > 0 ? ds.dsBm.bmWidthBytes : (w * 4);
			BYTE* pOut = new BYTE[(size_t)stride * h];
			if( pOut == NULL ) return false;
			const bool topDown = (ds.dsBmih.biHeight < 0) || (ds.dsBm.bmHeight < 0);
			const BYTE* pSrc = (const BYTE*)ds.dsBm.bmBits;
			if( topDown ) {
				memcpy(pOut, pSrc, (size_t)stride * h);
			}
			else {
				for( int y = 0; y < h; ++y )
					memcpy(pOut + y * stride, pSrc + (h - 1 - y) * stride, (size_t)w * 4);
			}
			*ppBits = pOut;
			if( pWidth ) *pWidth = w;
			if( pHeight ) *pHeight = h;
			if( pStride ) *pStride = stride;
			return true;
		}

		BITMAPINFO bmi = { 0 };
		bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
		bmi.bmiHeader.biWidth = w;
		bmi.bmiHeader.biHeight = -h;
		bmi.bmiHeader.biPlanes = 1;
		bmi.bmiHeader.biBitCount = 32;
		bmi.bmiHeader.biCompression = BI_RGB;
		BYTE* pOut = new BYTE[(size_t)w * h * 4];
		if( pOut == NULL ) return false;
		HDC hDC = ::GetDC(NULL);
		int n = ::GetDIBits(hDC, hBitmap, 0, h, pOut, &bmi, DIB_RGB_COLORS);
		::ReleaseDC(NULL, hDC);
		if( n != h ) {
			delete[] pOut;
			return false;
		}
		*ppBits = pOut;
		if( pWidth ) *pWidth = w;
		if( pHeight ) *pHeight = h;
		if( pStride ) *pStride = w * 4;
		return true;
	}

	HRGN CreateRegionFromAlphaBitmap(HBITMAP hBitmap, BYTE alphaThreshold)
	{
		BYTE* pBits = NULL;
		int w = 0, h = 0, stride = 0;
		if( !CopyBitmapAlphaBits(hBitmap, &pBits, &w, &h, &stride) )
			return NULL;
		HRGN hRgn = CreateRegionFromAlphaBGRA(pBits, w, h, stride, alphaThreshold);
		delete[] pBits;
		return hRgn;
	}

	HRGN CreateScaledRegionFromAlphaBitmap(HBITMAP hBitmap, int destW, int destH, BYTE alphaThreshold)
	{
		BYTE* pBits = NULL;
		int w = 0, h = 0, stride = 0;
		if( !CopyBitmapAlphaBits(hBitmap, &pBits, &w, &h, &stride) )
			return NULL;
		HRGN hRgn = CreateScaledRegionFromAlphaBGRA(pBits, w, h, destW, destH, stride, alphaThreshold);
		delete[] pBits;
		return hRgn;
	}

	HRGN CreateRegionFromAlphaImage(const TImageInfo* pInfo, BYTE alphaThreshold, int destW, int destH)
	{
		if( pInfo == NULL || pInfo->hBitmap == NULL ) return NULL;
		if( destW <= 0 ) destW = pInfo->nX;
		if( destH <= 0 ) destH = pInfo->nY;
		if( destW == pInfo->nX && destH == pInfo->nY )
			return CreateRegionFromAlphaBitmap(pInfo->hBitmap, alphaThreshold);
		return CreateScaledRegionFromAlphaBitmap(pInfo->hBitmap, destW, destH, alphaThreshold);
	}

	bool HitTestAlphaBGRA(const BYTE* pBgra, int width, int height, int x, int y,
		int stride, BYTE alphaThreshold)
	{
		if( pBgra == NULL || width < 1 || height < 1 ) return true;
		if( x < 0 || y < 0 || x >= width || y >= height ) return false;
		if( stride <= 0 ) stride = width * 4;
		return pBgra[y * stride + x * 4 + 3] >= alphaThreshold;
	}

	bool HitTestAlphaInDestRect(const BYTE* pBgra, int srcW, int srcH,
		const RECT& rcDest, POINT ptClient, int stride, BYTE alphaThreshold)
	{
		if( pBgra == NULL || srcW < 1 || srcH < 1 ) return true;
		const int dw = rcDest.right - rcDest.left;
		const int dh = rcDest.bottom - rcDest.top;
		if( dw < 1 || dh < 1 ) return false;
		if( !::PtInRect(&rcDest, ptClient) ) return false;
		int x = (ptClient.x - rcDest.left) * srcW / dw;
		int y = (ptClient.y - rcDest.top) * srcH / dh;
		if( x >= srcW ) x = srcW - 1;
		if( y >= srcH ) y = srcH - 1;
		return HitTestAlphaBGRA(pBgra, srcW, srcH, x, y, stride, alphaThreshold);
	}
}
