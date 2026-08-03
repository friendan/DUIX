#ifndef __GDIRENDERCONTEXT_H__
#define __GDIRENDERCONTEXT_H__

#pragma once

#include "IRenderContext.h"

namespace DuiLib {

	class CRenderClip;

	// 默认后端：委托现有 CRenderEngine（GDI / GDI+）
	class UILIB_API CGdiRenderContext : public IRenderContext
	{
	public:
		CGdiRenderContext(HDC hDC, CPaintManagerUI* pManager);
		~CGdiRenderContext();

		HDC GetDC() const override;
		CPaintManagerUI* GetManager() const override;

		void PushClip(const RECT& rc) override;
		void PushRoundClip(const RECT& rcClip, const RECT& rcRound, int width, int height) override;
		void PopClip() override;
		void SuspendClip() override;
		void ResumeClip() override;

		int SaveState() override;
		void RestoreState(int nSaved) override;

		void DrawColor(const RECT& rc, DWORD color) override;
		void DrawGradient(const RECT& rc, DWORD dwFirst, DWORD dwSecond, bool bVertical, int nSteps) override;
		void DrawLine(const RECT& rc, int nSize, DWORD dwPenColor, int nStyle = PS_SOLID) override;
		void DrawRect(const RECT& rc, int nSize, DWORD dwPenColor, int nStyle = PS_SOLID) override;
		void DrawRoundRect(const RECT& rc, int nSize, int width, int height, DWORD dwPenColor, int nStyle = PS_SOLID) override;
		void FillRoundRect(const RECT& rc, int width, int height, DWORD dwColor) override;

		void DrawText(RECT& rc, LPCTSTR pstrText, DWORD dwColor, int iFont, UINT uStyle) override;
		void DrawText(RECT& rc, LPCTSTR pstrText, DWORD dwColor, int iFont, UINT uStyle, DWORD dwTextBKColor) override;
		void DrawHtmlText(RECT& rc, LPCTSTR pstrText, DWORD dwColor, RECT* pLinks, CDuiString* sLinks, int& nLinkRects, int iFont, UINT uStyle) override;
		SIZE GetTextSize(LPCTSTR pstrText, int iFont, UINT uStyle) override;

		bool DrawImageInfo(const RECT& rcItem, const RECT& rcPaint, const TDrawInfo* pDrawInfo, HINSTANCE instance = NULL) override;
		bool DrawImageString(const RECT& rcItem, const RECT& rcPaint, LPCTSTR pStrImage, LPCTSTR pStrModify = NULL, HINSTANCE instance = NULL) override;
#if DUILIB_GDI_INTEROP
		void DrawImage(HBITMAP hBitmap, const RECT& rc, const RECT& rcPaint, const RECT& rcBmpPart, const RECT& rcCorners, bool bAlpha, UINT uFade = 255, bool hole = false, bool xtiled = false, bool ytiled = false) override;
#endif
		void DrawImage(const TImageInfo* pImageInfo, const RECT& rc, const RECT& rcPaint, const RECT& rcBmpPart, const RECT& rcCorners, UINT uFade = 255, bool hole = false, bool xtiled = false, bool ytiled = false) override;

		void DrawGdiplusImage(void* pGdiplusImage, int x, int y, int cx, int cy) override;
		void DrawGdiplusImageRotated(void* pGdiplusImage, const RECT& rc, float angleDegrees) override;
		void StretchBlit(void* srcNative, int x, int y, int cx, int cy, int xSrc, int ySrc, int cxSrc, int cySrc, int mode = HALFTONE) override;

	private:
		HDC m_hDC;
		CPaintManagerUI* m_pManager;
		CStdPtrArray m_aClipStack;
	};

	inline IRenderContext* ResolveRenderContext(CPaintManagerUI* pManager, HDC hDC, CGdiRenderContext& localCtx)
	{
		IRenderContext* pCtx = (pManager != NULL) ? pManager->GetRenderContext() : NULL;
		if( pCtx == NULL || pCtx->GetDC() != hDC ) pCtx = &localCtx;
		return pCtx;
	}

} // namespace DuiLib

#endif // __GDIRENDERCONTEXT_H__
