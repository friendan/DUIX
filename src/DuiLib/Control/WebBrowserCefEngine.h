#ifndef __WEBBROWSERCEFENGINE_H__
#define __WEBBROWSERCEFENGINE_H__

#pragma once

#include "IWebBrowserEngine.h"

namespace DuiLib
{
	/// CEF 占位引擎：本期不引入 SDK，Create 始终失败（便于后续接真实实现）
	class CWebBrowserCefEngine : public IWebBrowserEngine
	{
	public:
		virtual LPCTSTR GetName() const { return _T("cef"); }
		virtual bool Create(CControlUI* /*pOwner*/, HWND /*hParent*/, const RECT& /*rc*/)
		{
			OutputDebugString(_T("DuiLib: CEF engine stub — not linked. Implement CWebBrowserCefEngine with CEF SDK.\n"));
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
