#ifndef __WEBBROWSERCEFENGINE_H__
#define __WEBBROWSERCEFENGINE_H__

#pragma once

#include "IWebBrowserEngine.h"

namespace DuiLib
{
	/// CEF 占位引擎：Create 失败。应用应 Register("cef", MyCreate) 覆盖（builtin 不会盖掉已注册实现）。
	/// 外接实现建议覆盖：Stop / GetUrl / ExecuteScript / DoMessageLoopWork（CefDoMessageLoopWork），
	/// 并通过 SetHostEvents 上报 OnLoadError / OnDownloadStarting / OnFaviconChanged 等。
	class CWebBrowserCefEngine : public IWebBrowserEngine
	{
	public:
		virtual LPCTSTR GetName() const { return _T("cef"); }
		virtual bool Create(CControlUI* /*pOwner*/, HWND /*hParent*/, const RECT& /*rc*/)
		{
			OutputDebugString(_T("DuiLib: CEF engine stub — Register your IWebBrowserEngine via CWebBrowserEngineFactory::Register(\"cef\", ...).\n"));
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
