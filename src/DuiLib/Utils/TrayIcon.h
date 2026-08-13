#ifndef __UITRAYICON_H__
#define __UITRAYICON_H__

#pragma once
#include <ShellAPI.h>

#ifndef NIIF_USER
#define NIIF_USER 0x00000004
#endif
#ifndef NIIF_LARGE_ICON
#define NIIF_LARGE_ICON 0x00000020
#endif
#ifndef NIIF_NOSOUND
#define NIIF_NOSOUND 0x00000010
#endif
#ifndef NIS_HIDDEN
#define NIS_HIDDEN 0x00000001
#endif
#ifndef NIM_SETVERSION
#define NIM_SETVERSION 0x00000004
#endif
#ifndef NOTIFYICON_VERSION
#define NOTIFYICON_VERSION 3
#endif
#ifndef NOTIFYICON_VERSION_4
#define NOTIFYICON_VERSION_4 4
#endif

namespace DuiLib
{
	/// 系统托盘图标封装（Shell_NotifyIcon）。回调默认消息见 UIMSG_TRAYICON。
	class UILIB_API CTrayIcon
	{
	public:
		CTrayIcon();
		~CTrayIcon();

		/// 从资源 ID 创建（兼容旧接口）。uIconIDResource=0 或加载失败时自动用程序/窗口图标。
		void CreateTrayIcon(HWND hRecvWnd, UINT uIconIDResource = 0, LPCTSTR pToolTipText = NULL, UINT uMessage = 0);
		/// 未指定图标：自动取程序 ICO（窗口小图标 → 大图标 → exe 内嵌）
		bool Create(HWND hRecvWnd, UINT uId = 1, LPCTSTR pToolTipText = NULL, UINT uMessage = 0);
		/// 显式 HICON。hIcon=NULL 时回退程序图标。bOwnIcon=true 时析构/换图标会 DestroyIcon。
		bool Create(HWND hRecvWnd, UINT uId, HICON hIcon, LPCTSTR pToolTipText = NULL, UINT uMessage = 0, bool bOwnIcon = false);
		/// 从模块资源加载；uIconResource=0 或失败则程序图标
		bool Create(HWND hRecvWnd, UINT uId, UINT uIconResource, LPCTSTR pToolTipText = NULL, UINT uMessage = 0, HINSTANCE hInst = NULL);
		/// 从 .ico 文件创建；失败回退程序图标
		bool CreateFromFile(HWND hRecvWnd, UINT uId, LPCTSTR pIconFile, LPCTSTR pToolTipText = NULL, UINT uMessage = 0);

		void DeleteTrayIcon();
		/// 任务栏重启后（WM_TASKBARCREATED）重新 NIM_ADD
		bool Recreate();

		bool SetTooltipText(LPCTSTR pToolTipText);
		bool SetTooltipText(UINT uStringResource, HINSTANCE hInst = NULL);
		CDuiString GetTooltipText() const;

		/// bOwnIcon：是否接管 hIcon 生命周期（换图标/析构时 DestroyIcon）
		bool SetIcon(HICON hIcon, bool bOwnIcon = false);
		bool SetIcon(LPCTSTR pIconFile);
		bool SetIcon(UINT uIconResource, HINSTANCE hInst = NULL);
		/// 换成程序/窗口图标（自持有）
		bool SetIconFromApplication(HWND hWnd = NULL);
		HICON GetIcon() const;

		/// 显示 / 隐藏（优先 NIF_STATE；失败则 Delete/Recreate）
		bool Show();
		bool Hide();
		void SetShowIcon();
		void SetHideIcon();
		void RemoveIcon();

		bool IsEnabled() const { return m_bEnabled; }
		bool Enabled() { return m_bEnabled; }
		/// 是否在托盘区可见（未 Hide）
		bool IsVisible() const { return m_bEnabled && m_bVisible; }

		HWND GetNotifyWnd() const { return m_hWnd; }
		UINT GetIconId() const { return m_trayData.uID; }
		UINT GetCallbackMessage() const { return m_uMessage; }

		/// 气球通知。dwInfoFlags: NIIF_INFO / NIIF_WARNING / NIIF_ERROR / NIIF_NONE / NIIF_USER …
		bool ShowBalloon(LPCTSTR pTitle, LPCTSTR pText, DWORD dwInfoFlags = NIIF_INFO, UINT uTimeoutMs = 10000, HICON hBalloonIcon = NULL);
		bool HideBalloon();

		/// 可选：NIM_SETVERSION。推荐 NOTIFYICON_VERSION_4（配合 DecodeNotify）。默认保持旧回调布局以兼容现有窗口。
		bool SetNotifyVersion(UINT uVersion = NOTIFYICON_VERSION_4);
		UINT GetNotifyVersion() const { return m_uNotifyVersion; }

		/// 解析 UIMSG_TRAYICON（或自定义回调）的 wParam/lParam。
		/// 返回事件码：WM_LBUTTONUP / WM_RBUTTONUP / WM_LBUTTONDBLCLK / NIN_BALLOONUSERCLICK …
		static UINT DecodeNotifyMsg(WPARAM wParam, LPARAM lParam, UINT uNotifyVersion = 0);
		static UINT DecodeNotifyIconId(WPARAM wParam, LPARAM lParam, UINT uNotifyVersion = 0);
		/// VERSION_4 时为光标屏幕坐标；旧版返回 GetCursorPos
		static POINT DecodeNotifyPos(WPARAM wParam, LPARAM lParam, UINT uNotifyVersion = 0);

		/// RegisterWindowMessage(_T("TaskbarCreated"))，任务栏崩溃/重启后需 Recreate
		static UINT GetTaskbarCreatedMsg();
		/// 取程序图标（返回值需 DestroyIcon；失败 NULL）。优先窗口 WM_GETICON/类图标，再 ExtractIconEx(exe)。
		static HICON LoadApplicationIcon(HWND hWnd = NULL, bool bSmall = true);

		/// 最小化到托盘：Hide + 去任务栏（TOOLWINDOW / 清 APPWINDOW + ITaskbarList::DeleteTab）。
		static void HideWindowFromTaskbar(HWND hWnd);
		/// 若曾 HideWindowFromTaskbar：恢复扩展样式并重新挂任务栏（幂等）。
		/// 任意路径 Show 窗口时也会自动调用（见 CWindowWnd），一般不必手写。
		static bool RestoreWindowToTaskbarIfNeeded(HWND hWnd);
		/// 便捷还原：RestoreWindowToTaskbarIfNeeded + ShowWindow + 可选前台
		static void ShowWindowOnTaskbar(HWND hWnd, bool bActivate = true);
		/// 是否处于 HideWindowFromTaskbar 状态（按窗口 Prop 判断）
		static bool IsWindowHiddenFromTaskbar(HWND hWnd);

	private:
		void ResetData();
		void ReleaseOwnedIcon();
		bool ApplyAdd();
		void CopyTip(LPCTSTR pTip);
		void CopyInfoTitle(LPCTSTR pTitle);
		void CopyInfoText(LPCTSTR pText);
		HICON LoadIconRes(UINT uRes, HINSTANCE hInst) const;
		HICON LoadIconFile(LPCTSTR pFile) const;

		bool m_bEnabled;
		bool m_bVisible;
		bool m_bOwnIcon;
		HWND m_hWnd;
		UINT m_uMessage;
		UINT m_uNotifyVersion;
		HICON m_hIcon;
		NOTIFYICONDATA m_trayData;
	};
}

#endif // __UITRAYICON_H__
