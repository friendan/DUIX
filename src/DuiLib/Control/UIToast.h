#ifndef __UITOAST_H__
#define __UITOAST_H__

#pragma once

namespace DuiLib {

	/// Toast 对齐位置（Screen* 相对工作区，Window* 相对 owner 窗口）
	enum ToastAlign
	{
		ToastAlign_ScreenTopLeft = 0,
		ToastAlign_ScreenTopCenter,
		ToastAlign_ScreenTopRight,
		ToastAlign_ScreenBottomLeft,
		ToastAlign_ScreenBottomCenter,
		ToastAlign_ScreenBottomRight,
		ToastAlign_ScreenCenter,
		ToastAlign_WindowTopLeft,
		ToastAlign_WindowTopCenter,
		ToastAlign_WindowTopRight,
		ToastAlign_WindowBottomLeft,
		ToastAlign_WindowBottomCenter,
		ToastAlign_WindowBottomRight,
		ToastAlign_WindowCenter,
	};

	/// 点击整条 Toast 回调（hToast 为该条 HWND；pUserData / pUser 来自 Options）
	typedef void (CALLBACK *ToastClickCallback)(HWND hToast, LPCTSTR pUserData, void* pUser);

	/// 关闭原因（埋点）
	enum ToastDismissReason
	{
		ToastDismiss_Timeout = 0,  ///< 倒计时结束
		ToastDismiss_Manual,       ///< × / 点击关闭 / Dismiss / DismissAll
		ToastDismiss_Evicted,      ///< SetMaxCount 超出被顶掉
	};

	/// 关闭回调（HWND 仍有效；reason 见 ToastDismissReason）
	typedef void (CALLBACK *ToastDismissCallback)(HWND hToast, ToastDismissReason reason, LPCTSTR pUserData, void* pUser);

	/// Toast 配置（链式设置）
	class UILIB_API CToastOptions
	{
		friend class CToast;
		friend class CToastWnd;
	public:
		CToastOptions();

		CToastOptions& Title(LPCTSTR text);
		CToastOptions& Text(LPCTSTR text);
		CToastOptions& Kind(ControlKind kind);
		CToastOptions& Duration(int ms);
		CToastOptions& ShowClose(bool show);
		CToastOptions& ShowIcon(bool show);
		/// 自定义图标，覆盖 kind 默认 tabler-filled；lib 同 SvgBox 属性名（bsicon/lucide/tabler-filled/…）
		CToastOptions& Icon(LPCTSTR lib, LPCTSTR name);
		CToastOptions& Align(ToastAlign align);
		CToastOptions& Owner(HWND hOwner);
		CToastOptions& MinWidth(int w);
		CToastOptions& MaxWidth(int w);
		CToastOptions& Gap(int gap);
		/// 高度；0=自动（单行约 44，标题+正文约 68；超宽时换行增高）
		CToastOptions& Height(int h);
		/// 悬停时暂停倒计时（默认 true）
		CToastOptions& PauseOnHover(bool pause);
		/// 整条点击回调；设置后鼠标为手型
		CToastOptions& OnClick(ToastClickCallback fn, void* pUser = NULL);
		/// 关闭回调（超时 / 手动 / 被顶掉）
		CToastOptions& OnDismiss(ToastDismissCallback fn, void* pUser = NULL);
		/// 回调附带的用户字符串（如详情 id）
		CToastOptions& UserData(LPCTSTR data);
		/// 点击回调后是否自动关闭（默认 true）
		CToastOptions& ClickDismiss(bool dismiss);

	private:
		CDuiString m_sTitle;
		CDuiString m_sText;
		ControlKind m_kind;
		int m_nDuration;
		bool m_bShowClose;
		bool m_bShowIcon;
		CDuiString m_sIconLib;
		CDuiString m_sIconName;
		ToastAlign m_align;
		HWND m_hOwner;
		int m_nMinWidth;
		int m_nMaxWidth;
		int m_nGap;
		int m_nHeight;
		bool m_bPauseOnHover;
		ToastClickCallback m_fnOnClick;
		void* m_pClickUser;
		ToastDismissCallback m_fnOnDismiss;
		void* m_pDismissUser;
		CDuiString m_sUserData;
		bool m_bClickDismiss;
	};

	/// Toast 便捷 API（独立弹出 HWND，不抢焦点）
	class UILIB_API CToast
	{
	public:
		static HWND Show(LPCTSTR text, const CToastOptions& opts = CToastOptions());
		static HWND Show(LPCTSTR title, LPCTSTR text, const CToastOptions& opts = CToastOptions());

		static HWND ShowSuccess(LPCTSTR text, int duration = 3000);
		static HWND ShowDanger(LPCTSTR text, int duration = 5000);
		static HWND ShowWarning(LPCTSTR text, int duration = 4000);
		static HWND ShowInfo(LPCTSTR text, int duration = 3000);

		/// 每个对齐组（Align；Window* 另按 Owner）最大同时条数；0=无上限。超出时关掉该组最旧的
		static void SetMaxCount(int n);
		static int GetMaxCount();

		/// 关闭单条 Toast（传入 Show 返回的 HWND）
		static void Dismiss(HWND hToast);
		static void DismissAll();
	};

} // namespace DuiLib

#endif // __UITOAST_H__
