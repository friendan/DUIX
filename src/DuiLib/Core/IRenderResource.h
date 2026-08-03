#ifndef __IRENDERRESOURCE_H__
#define __IRENDERRESOURCE_H__

#pragma once

namespace DuiLib {

	class CPaintManagerUI;

	// 编译期渲染后端（可在 CMake / 预处理器覆盖）
#ifndef DUILIB_RENDER_GDI
#	define DUILIB_RENDER_GDI  0
#endif
#ifndef DUILIB_RENDER_D2D
#	define DUILIB_RENDER_D2D  1
#endif
#ifndef DUILIB_RENDER_BACKEND
#	define DUILIB_RENDER_BACKEND DUILIB_RENDER_GDI
#endif

	// 允许 HBITMAP/HFONT/GetDC 互操作（RichEdit/ActiveX/迁移期）
#ifndef DUILIB_GDI_INTEROP
#	define DUILIB_GDI_INTEROP 1
#endif

	// 布局测量：按当前 Device 建临时 Context（不绑定 Manager、不 BeginFrame）
	UILIB_API void RenderMeasureText(CPaintManagerUI* pManager, RECT& rc, LPCTSTR pstrText, DWORD dwColor, int iFont, UINT uStyle);
	UILIB_API void RenderMeasureHtmlText(CPaintManagerUI* pManager, RECT& rc, LPCTSTR pstrText, DWORD dwColor, int iFont, UINT uStyle);
	UILIB_API SIZE RenderMeasureTextSize(CPaintManagerUI* pManager, LPCTSTR pstrText, int iFont, UINT uStyle);

} // namespace DuiLib

#endif // __IRENDERRESOURCE_H__
