#ifndef __UIEDITBOX_H__
#define __UIEDITBOX_H__

#pragma once

#include <vector>

namespace DuiLib
{
	class CEditUI;
	class CButtonUI;
	class CEditBoxHistoryWnd;

	/// 左右槽基类：按内容收缩宽度，高度跟 EditBox
	class UILIB_API CEditBoxSlotUI : public CHorizontalLayoutUI
	{
		DECLARE_DUICONTROL(CEditBoxSlotUI)
	public:
		CEditBoxSlotUI();

		LPCTSTR GetClass() const override;
		LPVOID GetInterface(LPCTSTR pstrName) override;
		SIZE EstimateSize(SIZE szAvailable) override;
	};

	/// 左侧容器（XML: EditBoxLeft / EditBoxPrefix）
	class UILIB_API CEditBoxLeftUI : public CEditBoxSlotUI
	{
		DECLARE_DUICONTROL(CEditBoxLeftUI)
	public:
		LPCTSTR GetClass() const override;
		LPVOID GetInterface(LPCTSTR pstrName) override;
	};

	/// 右侧容器（XML: EditBoxRight / EditBoxSuffix）
	class UILIB_API CEditBoxRightUI : public CEditBoxSlotUI
	{
		DECLARE_DUICONTROL(CEditBoxRightUI)
	public:
		LPCTSTR GetClass() const override;
		LPVOID GetInterface(LPCTSTR pstrName) override;
	};

	/// 带左右功能区的输入框：统一边框在外壳；中间内嵌无边框 Edit。
	/// XML 用 <EditBoxLeft> / <EditBoxRight> 声明左右槽（可设 gap、padding 等）；
	/// 直接子控件仍默认进左侧（兼容简单图标）。内建 clearable / password-toggle 在右侧最末。
	class UILIB_API CEditBoxUI : public CHorizontalLayoutUI
	{
		DECLARE_DUICONTROL(CEditBoxUI)
		friend class CEditBoxHistoryWnd;
	public:
		CEditBoxUI();
		~CEditBoxUI();

		LPCTSTR GetClass() const override;
		LPVOID GetInterface(LPCTSTR pstrName) override;
		bool IsFocused() const override;

		CEditUI* GetEdit() const { return m_pEdit; }
		CEditBoxLeftUI* GetLeftSlot() const { return static_cast<CEditBoxLeftUI*>(m_pPrefix); }
		CEditBoxRightUI* GetRightSlot() const { return static_cast<CEditBoxRightUI*>(m_pSuffix); }
		CEditBoxLeftUI* GetPrefixSlot() const { return GetLeftSlot(); }
		CEditBoxRightUI* GetSuffixSlot() const { return GetRightSlot(); }

		void SetText(LPCTSTR pstrText) override;
		CDuiString GetText() const override;

		void SetClearable(bool b);
		bool IsClearable() const { return m_bClearable; }
		void SetPasswordToggle(bool b);
		bool IsPasswordToggle() const { return m_bPasswordToggle; }
		void SetIconBtnWidth(int n);
		int GetIconBtnWidth() const { return m_nIconBtnWidth; }
		void SetSubmitOnEnter(bool b);
		bool IsSubmitOnEnter() const { return m_bSubmitOnEnter; }
		void SetSubmitButton(LPCTSTR pstrName);
		CDuiString GetSubmitButton() const { return m_sSubmitButton; }

		void SetHistoryEnabled(bool b);
		bool IsHistoryEnabled() const { return m_bHistoryEnabled; }
		void SetHistoryMaxCount(int n);
		int GetHistoryMaxCount() const { return m_nHistoryMax; }
		bool AddHistory(LPCTSTR text);
		bool RemoveHistory(LPCTSTR text);
		bool RemoveHistoryAt(int index);
		void ClearHistory();
		/// 整表替换；`items[0]` 为最新。超 `history-max` 截断尾部。`items==NULL` 或 `count<=0` 等同清空
		void SetHistory(LPCTSTR const* items, int count);
		int GetHistoryCount() const;
		CDuiString GetHistoryItem(int index) const;
		void ShowHistoryPopup();
		void CloseHistoryPopup();
		/// 用户主动关掉历史（× / Esc / 点外侧）：本轮聚焦内不再自动弹出
		void DismissHistoryPopup();
		bool IsHistoryPopupVisible() const;
		/// 内部：原生 WC_EDIT 鼠标按下（已聚焦时 Dui 收不到 BUTTONDOWN）
		void OnInnerEditNativeClick();

		bool Add(CControlUI* pControl) override;
		bool AddAt(CControlUI* pControl, int iIndex) override;
		void SetEnabled(bool bEnable = true) override;
		void SetAttribute(LPCTSTR pstrName, LPCTSTR pstrValue) override;
		void SetFixedHeight(int cy) override;
		void SetPos(RECT rc, bool bNeedInvalidate = true) override;
		void Move(SIZE szOffset, bool bNeedInvalidate = true) override;
		void DoEvent(TEventUI& event) override;
		bool DoPaint(IRenderContext& ctx, const RECT& rcPaint, CControlUI* pStopControl) override;
		void SetManager(CPaintManagerUI* pManager, CControlUI* pParent, bool bInit = true) override;

		void SyncInnerEditChrome();

	protected:
		void EnsureChrome();
		void SyncSlotMetrics();
		void SyncSlotVisibility();
		void SyncBuiltinButtons();
		void SyncClearVisible();
		void ApplyEnabledToChild(CControlUI* pControl);
		void ApplyIconButtonStyle(CButtonUI* pBtn);
		void ApplyBuiltinIcons();
		CControlUI* ResolveSubmitButton() const;
		void TrySubmitOnEnter();
		void ApplyHistoryPick(LPCTSTR text);
		void RequestShowHistoryPopup();
		void TryShowHistoryOnFocus();
		void OpenHistoryPopupNow();
		void NotifyHistoryChanged(WPARAM reason, LPARAM lParam = 0);
		int FindBuiltinInsertIndex() const;
		bool IsBuiltinControl(CControlUI* pControl) const;
		bool IsLeftSlotControl(CControlUI* pControl) const;
		bool IsRightSlotControl(CControlUI* pControl) const;
		bool AdoptLeftSlot(CEditBoxSlotUI* pNew);
		bool AdoptRightSlot(CEditBoxSlotUI* pNew);
		int IndexOfChild(CControlUI* pControl) const;
		static void DetachFromParent(CContainerUI* pParent, CControlUI* pChild);
		bool OnChildNotify(void* param);
		bool OnEditEvent(void* param);
		static bool ParseBoolValue(LPCTSTR pstrValue);
		static bool IsEditForwardAttr(LPCTSTR pstrName);

		CEditBoxSlotUI* m_pPrefix;
		CEditBoxSlotUI* m_pSuffix;
		CEditUI* m_pEdit;
		CButtonUI* m_pClearBtn;
		CButtonUI* m_pEyeBtn;
		CEditBoxHistoryWnd* m_pHistoryWnd;
		CDuiString m_sSubmitButton;
		std::vector<CDuiString> m_aHistory;

		bool m_bClearable;
		bool m_bPasswordToggle;
		bool m_bPasswordVisible;
		bool m_bChromeReady;
		bool m_bSubmitOnEnter;
		bool m_bHistoryEnabled;
		bool m_bSkipHistoryPopup;
		int m_nIconBtnWidth;
		int m_nHistoryMax;
	};
}

#endif // __UIEDITBOX_H__
