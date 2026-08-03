#ifndef __IRENDERCONTEXT_H__
#define __IRENDERCONTEXT_H__

#pragma once

#include "IRenderResource.h"

namespace DuiLib {

	class CPaintManagerUI;
	struct tagTDrawInfo;
	typedef struct tagTDrawInfo TDrawInfo;
	struct tagTImageInfo;
	typedef struct tagTImageInfo TImageInfo;

	// 渲染上下文抽象：控件通过此接口绘制，便于后续替换 GDI/GDI+ / Direct2D / Skia。
	// 迁移期保留 GetDC() 作为逃生口（RichEdit/ActiveX 等原生 HDC 路径）。
	class UILIB_API IRenderContext
	{
	public:
		virtual ~IRenderContext() {}

		virtual HDC GetDC() const = 0;
		virtual CPaintManagerUI* GetManager() const = 0;

		// 裁剪栈（后端可用 HRGN / D2D layer / SkCanvas clip）
		virtual void PushClip(const RECT& rc) = 0;
		// width/height：CSS 圆角半径
		virtual void PushRoundClip(const RECT& rcClip, const RECT& rcRound, int width, int height) = 0;
		virtual void PopClip() = 0;
		// 临时恢复到压入前裁剪（浮层子控件绘制前后）
		virtual void SuspendClip() = 0;
		virtual void ResumeClip() = 0;

		// 状态栈（GDI: SaveDC/RestoreDC）
		virtual int SaveState() = 0;
		virtual void RestoreState(int nSaved) = 0;

		// 图元
		virtual void DrawColor(const RECT& rc, DWORD color) = 0;
		virtual void DrawGradient(const RECT& rc, DWORD dwFirst, DWORD dwSecond, bool bVertical, int nSteps) = 0;
		virtual void DrawLine(const RECT& rc, int nSize, DWORD dwPenColor, int nStyle = PS_SOLID) = 0;
		virtual void DrawRect(const RECT& rc, int nSize, DWORD dwPenColor, int nStyle = PS_SOLID) = 0;
		virtual void DrawRoundRect(const RECT& rc, int nSize, int width, int height, DWORD dwPenColor, int nStyle = PS_SOLID) = 0;
		// 填充圆角矩形；width/height 为 CSS 圆角半径
		virtual void FillRoundRect(const RECT& rc, int width, int height, DWORD dwColor) = 0;

		// 文字
		virtual void DrawText(RECT& rc, LPCTSTR pstrText, DWORD dwColor, int iFont, UINT uStyle) = 0;
		virtual void DrawText(RECT& rc, LPCTSTR pstrText, DWORD dwColor, int iFont, UINT uStyle, DWORD dwTextBKColor) = 0;
		virtual void DrawHtmlText(RECT& rc, LPCTSTR pstrText, DWORD dwColor, RECT* pLinks, CDuiString* sLinks, int& nLinkRects, int iFont, UINT uStyle) = 0;
		virtual SIZE GetTextSize(LPCTSTR pstrText, int iFont, UINT uStyle) = 0;

		// 图片（走 PaintManager 缓存 / 皮肤字符串）
		virtual bool DrawImageInfo(const RECT& rcItem, const RECT& rcPaint, const TDrawInfo* pDrawInfo, HINSTANCE instance = NULL) = 0;
		virtual bool DrawImageString(const RECT& rcItem, const RECT& rcPaint, LPCTSTR pStrImage, LPCTSTR pStrModify = NULL, HINSTANCE instance = NULL) = 0;
		// 底层位图绘制（GDI 互操作逃生口；新代码优先用 TImageInfo 重载）
#if DUILIB_GDI_INTEROP
		virtual void DrawImage(HBITMAP hBitmap, const RECT& rc, const RECT& rcPaint, const RECT& rcBmpPart, const RECT& rcCorners, bool bAlpha, UINT uFade = 255, bool hole = false, bool xtiled = false, bool ytiled = false) = 0;
#endif
		// 资源抽象入口：GDI 读 hBitmap；D2D/Skia 可读 pBackend
		virtual void DrawImage(const TImageInfo* pImageInfo, const RECT& rc, const RECT& rcPaint, const RECT& rcBmpPart, const RECT& rcCorners, UINT uFade = 255, bool hole = false, bool xtiled = false, bool ytiled = false) = 0;

		// GDI+ Image 绘制（Loading/Gif/Ring）；D2D/Skia 可忽略或转纹理
		virtual void DrawGdiplusImage(void* pGdiplusImage, int x, int y, int cx, int cy) = 0;
		virtual void DrawGdiplusImageRotated(void* pGdiplusImage, const RECT& rc, float angleDegrees) = 0;
		// 从另一 native target（GDI 下为 HDC）拉伸拷贝
		virtual void StretchBlit(void* srcNative, int x, int y, int cx, int cy, int xSrc, int ySrc, int cxSrc, int cySrc, int mode = HALFTONE) = 0;
	};

	// RAII：压入矩形裁剪
	class UILIB_API CRenderClipScope
	{
	public:
		CRenderClipScope(IRenderContext& ctx, const RECT& rc);
		CRenderClipScope(IRenderContext& ctx, const RECT& rcClip, const RECT& rcRound, int width, int height);
		~CRenderClipScope();

	private:
		IRenderContext* m_pCtx;
		CRenderClipScope(const CRenderClipScope&);
		CRenderClipScope& operator=(const CRenderClipScope&);
	};

	// RAII：在绘制周期内绑定当前 IRenderContext
	class UILIB_API CRenderContextScope
	{
	public:
		CRenderContextScope(CPaintManagerUI* pManager, IRenderContext* pCtx);
		~CRenderContextScope();

	private:
		CPaintManagerUI* m_pManager;
		IRenderContext* m_pOld;
		CRenderContextScope(const CRenderContextScope&);
		CRenderContextScope& operator=(const CRenderContextScope&);
	};

} // namespace DuiLib

#endif // __IRENDERCONTEXT_H__
