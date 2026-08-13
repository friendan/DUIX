#ifndef __UITITLEBAR_H__
#define __UITITLEBAR_H__

#pragma once

namespace DuiLib
{
	class CButtonUI;
	class CLabelUI;

	/// 左侧内容区：吃剩余宽度，HTML 子控件全部落入此处
	class UILIB_API CTitleBarLeftUI : public CHorizontalLayoutUI
	{
		DECLARE_DUICONTROL(CTitleBarLeftUI)
	public:
		CTitleBarLeftUI();

		LPCTSTR GetClass() const;
		LPVOID GetInterface(LPCTSTR pstrName);
	};

	/// 右侧系统按钮槽：按按钮宽度收缩
	class UILIB_API CTitleBarSysUI : public CHorizontalLayoutUI
	{
		DECLARE_DUICONTROL(CTitleBarSysUI)
	public:
		CTitleBarSysUI();

		LPCTSTR GetClass() const;
		LPVOID GetInterface(LPCTSTR pstrName);
		SIZE EstimateSize(SIZE szAvailable) override;
	};

	class UILIB_API CTitleBarUI : public CHorizontalLayoutUI
	{
		DECLARE_DUICONTROL(CTitleBarUI)
	public:
		CTitleBarUI();
		~CTitleBarUI();

		LPCTSTR GetClass() const;
		LPVOID GetInterface(LPCTSTR pstrName);

		void SetTitle(LPCTSTR pstrText);
		CDuiString GetTitle() const;

		void SetShowMin(bool bShow);
		bool IsShowMin() const { return m_bShowMin; }
		/// 最小化到托盘：Hide 主窗（不进任务栏）。托盘图标由应用 CTrayIcon 自行创建。
		void SetMinimizeToTray(bool bTray);
		bool IsMinimizeToTray() const { return m_bMinimizeToTray; }
		/// 关闭到托盘：点关闭 / 系统关闭时藏窗，不销毁。真正退出用 WindowImplBase::ForceClose。
		void SetCloseToTray(bool bTray);
		bool IsCloseToTray() const { return m_bCloseToTray; }
		void SetShowMax(bool bShow);
		bool IsShowMax() const { return m_bShowMax; }
		void SetShowClose(bool bShow);
		bool IsShowClose() const { return m_bShowClose; }

		void SetBtnWidth(int nWidth);
		int GetBtnWidth() const { return m_nBtnWidth; }

		CHorizontalLayoutUI* GetLeftSlot() const { return m_pLeft; }
		CButtonUI* GetMinButton() const { return m_pMinBtn; }
		CButtonUI* GetMaxButton() const { return m_pMaxBtn; }
		CButtonUI* GetRestoreButton() const { return m_pRestoreBtn; }
		CButtonUI* GetCloseButton() const { return m_pCloseBtn; }

		/// 在 titlebarmining / titlebarmaxing / titlebarclosing 处理里调用可取消本次操作
		void CancelNotify();
		void ResetNotifyCancel();
		bool IsNotifyCanceled() const { return m_bNotifyCancel; }

		bool Add(CControlUI* pControl) override;
		bool AddAt(CControlUI* pControl, int iIndex) override;
		void SetAttribute(LPCTSTR pstrName, LPCTSTR pstrValue) override;
		void SetFixedHeight(int cy) override;
		bool IsCaptionDragHit(POINT pt) const override;
		/// 按标题栏底/标题色同步系统按钮图标与悬停（主题 chrome 可显式调用）
		void SyncSysButtonChrome();

	protected:
		void EnsureChrome();
		void ApplySysButtonIcons();
		void SyncSysButtonVisibility();
		void SyncSysButtonMetrics();
		void ApplySysButtonStyle(CButtonUI* pBtn, bool bClose);
		bool OnSysButtonNotify(void* param);
		bool QueryAllowNotify(LPCTSTR pstrMsg, WPARAM wParam, LPARAM lParam);
		void DoSysMin();
		void DoSysMax();
		void DoSysClose();
		HWND GetOwnerHWND() const;
		static bool ParseBoolValue(LPCTSTR pstrValue);
		static bool FontFamilyExists(LPCTSTR pstrFace);
		/// 优先 Segoe Fluent Icons，其次 Segoe MDL2 Assets；都没有返回 false
		static bool ResolveSegoeIconFont(CDuiString& sFace);

		CTitleBarLeftUI* m_pLeft;
		CTitleBarSysUI* m_pSys;
		CLabelUI* m_pTitleLabel;
		CButtonUI* m_pMinBtn;
		CButtonUI* m_pMaxBtn;
		CButtonUI* m_pRestoreBtn;
		CButtonUI* m_pCloseBtn;

		bool m_bShowMin;
		bool m_bShowMax;
		bool m_bShowClose;
		bool m_bMinimizeToTray;
		bool m_bCloseToTray;
		int m_nBtnWidth;
		bool m_bNotifyCancel;
		bool m_bChromeReady;
	};
}

#endif // __UITITLEBAR_H__
