#ifndef __WEBBROWSERCEFENGINE_H__
#define __WEBBROWSERCEFENGINE_H__

#pragma once

#include "IWebBrowserEngine.h"

namespace DuiLib
{
	/// CEF 占位引擎：Create 失败。应用应 Register("cef", MyCreate) 覆盖（builtin 不会盖掉已注册实现）。
	/// 窗口化：Create 挂子 HWND 即可。
	/// OSR：IsOffScreen=true，PaintOffScreen 用 BlitWebBrowserOsrBuffer，HandleEvent 转发输入；
	/// 帧就绪后 pOwner->Invalidate()；空闲泵 DoMessageLoopWork（CefDoMessageLoopWork）。
	/// 宿主事件：SetHostEvents → OnLoadError / OnDownloadStarting / OnFaviconChanged 等。
	class CWebBrowserCefEngine : public IWebBrowserEngine
	{
	public:
		virtual LPCTSTR GetName() const { return _T("cef"); }
		virtual bool Create(CControlUI* /*pOwner*/, HWND /*hParent*/, const RECT& /*rc*/)
		{
			OutputDebugString(_T("DuiLib: CEF engine stub — Register your IWebBrowserEngine via CWebBrowserEngineFactory::Register(\"cef\", ...). OSR: IsOffScreen/PaintOffScreen/HandleEvent.\n"));
			return false;
		}
		virtual void Destroy() {}
		virtual void SetPos(const RECT& /*rc*/) {}
		virtual void SetVisible(bool /*bVisible*/) {}
		virtual void Navigate(LPCTSTR /*url*/) {}
		virtual void GoBack() {}
		virtual void GoForward() {}
		virtual void Refresh() {}
		virtual HWND GetHostWindow() const { return NULL; }
		virtual void* GetNative() { return NULL; }
		virtual void SetHostEvents(CWebBrowserHostEvents* /*pEvents*/) {}
	};
}

#endif
