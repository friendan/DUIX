#ifndef __DUIEXITTRACE_H__
#define __DUIEXITTRACE_H__

#pragma once

// 调试开关：需要退出路径日志时改为 1
#ifndef DUI_EXIT_TRACE
#define DUI_EXIT_TRACE 0
#endif

#if DUI_EXIT_TRACE
namespace DuiLib {

	inline void DuiExitLog(LPCWSTR msg)
	{
		wchar_t buf[320];
		::wsprintfW(buf, L"[DuiExit] %s\n", msg);
		::OutputDebugStringW(buf);
	}

	struct CDuiExitTrace
	{
		LPCWSTR name;
		ULONGLONG t0;
		explicit CDuiExitTrace(LPCWSTR n)
			: name(n)
			, t0(::GetTickCount64())
		{
			wchar_t buf[320];
			::wsprintfW(buf, L"[DuiExit] >> %s\n", name);
			::OutputDebugStringW(buf);
		}
		~CDuiExitTrace()
		{
			wchar_t buf[320];
			::wsprintfW(buf, L"[DuiExit] << %s  %I64ums\n", name,
				(unsigned __int64)(::GetTickCount64() - t0));
			::OutputDebugStringW(buf);
		}
	};

} // namespace DuiLib

#define DUI_EXIT_SCOPE_JOIN2(a,b) a##b
#define DUI_EXIT_SCOPE_JOIN(a,b) DUI_EXIT_SCOPE_JOIN2(a,b)
#define DUI_EXIT_SCOPE(name) ::DuiLib::CDuiExitTrace DUI_EXIT_SCOPE_JOIN(_dui_exit_trace_, __LINE__)(name)
#define DUI_EXIT_LOG(msg) ::DuiLib::DuiExitLog(msg)
#else
#define DUI_EXIT_SCOPE(name) ((void)0)
#define DUI_EXIT_LOG(msg) ((void)0)
#endif

#endif // __DUIEXITTRACE_H__
