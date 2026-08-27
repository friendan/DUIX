#ifndef __UIHOTKEY_H__
#define __UIHOTKEY_H__
#pragma once

// HOTKEYF_WIN 等扩展位；RegisterHotKey 用 CHotKeyUI::HotKeyToRegisterMods 映射。
#ifndef HOTKEYF_WIN
#define HOTKEYF_WIN 0x10
#endif

namespace DuiLib{
	class CHotKeyUI;

	class UILIB_API CHotKeyWnd : public CWindowWnd
	{
	public:
		CHotKeyWnd(void);

	public:
		void Init(CHotKeyUI * pOwner);
		RECT CalPos();
		void CloseAndDetach();
		LPCTSTR GetWindowClassName() const;
		void OnFinalMessage(HWND hWnd);
		LPCTSTR GetSuperClassName() const;
		LRESULT HandleMessage(UINT uMsg, WPARAM wParam, LPARAM lParam);
		LRESULT OnKillFocus(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
		LRESULT OnEditChanged(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	public:
		void SetHotKey(WORD wVirtualKeyCode, WORD wModifiers);
		void GetHotKey(WORD &wVirtualKeyCode, WORD &wModifiers) const;
		DWORD GetHotKey(void) const;
		CDuiString GetHotKeyName();
		void SetRules(WORD wInvalidComb, WORD wModifiers);
		CDuiString GetKeyName(UINT vk, BOOL fExtended);
	protected:
		CHotKeyUI * m_pOwner;
		HBRUSH m_hBkBrush;
		bool m_bInit;
	};

	class UILIB_API CHotKeyUI : public CLabelUI
	{
		DECLARE_DUICONTROL(CHotKeyUI)
		friend CHotKeyWnd;
	public:
		CHotKeyUI();
		~CHotKeyUI();
		LPCTSTR GetClass() const;
		LPVOID GetInterface(LPCTSTR pstrName);
		UINT GetControlFlags() const;
		void SetEnabled(bool bEnable = true);
		void SetText(LPCTSTR pstrText);
		LPCTSTR GetImage();
		void SetImage(LPCTSTR pStrImage);
		LPCTSTR GetHoverImage();
		void SetHoverImage(LPCTSTR pStrImage);
		LPCTSTR GetFocusImage();
		void SetFocusImage(LPCTSTR pStrImage);
		LPCTSTR GetDisabledImage();
		void SetDisabledImage(LPCTSTR pStrImage);
		void SetNativeBackgroundColor(DWORD dwBackgroundColor);
		DWORD GetNativeBackgroundColor() const;

		void SetPos(RECT rc, bool bNeedInvalidate = true);
		void SetVisible(bool bVisible = true);
		void SetInternVisible(bool bVisible = true);
		SIZE EstimateSize(SIZE szAvailable);
		void DoEvent(TEventUI& event);
		void SetAttribute(LPCTSTR pstrName, LPCTSTR pstrValue);

		void PaintStatusImage(IRenderContext& ctx);
		void PaintText(IRenderContext& ctx);
	public:
		void GetHotKey(WORD &wVirtualKeyCode, WORD &wModifiers) const;
		DWORD GetHotKey(void) const;
		void SetHotKey(WORD wVirtualKeyCode, WORD wModifiers);
		/// 只读展示：不创建原生 HOTKEY 子窗、不出现 I 形光标；由外部/对话框捕获按键后 SetHotKey
		void SetReadOnly(bool bReadOnly);
		bool IsReadOnly() const { return m_bReadOnly; }
		/// 显示名（如「Ctrl + S」）；vk/mod 均为 0 时返回空串
		static CDuiString FormatHotKeyName(WORD wVirtualKeyCode, WORD wModifiers);
		/// HOTKEYF_* → RegisterHotKey 的 fsModifiers（忽略 HOTKEYF_EXT；可选 MOD_NOREPEAT）
		static UINT HotKeyToRegisterMods(WORD wHotKeyModifiers, bool bNoRepeat = false);
		/// RegisterHotKey 的 fsModifiers → HOTKEYF_*（忽略 MOD_NOREPEAT）
		static WORD RegisterModsToHotKey(UINT uRegisterMods);
		/// 比较用修饰键掩码（忽略 HOTKEYF_EXT）
		static WORD HotKeyCompareMask(WORD wModifiers);
		/// 是否同一快捷键（忽略 EXT）
		static bool IsSameHotKey(WORD vk1, WORD mod1, WORD vk2, WORD mod2);
		/// 字母 A–Z / 数字 0–9 / 小键盘数字（裸键时通常需修饰键）
		static bool IsLetterOrDigitKey(WORD vk);
		/// RequireModifier 规则：无修饰键且为字母/数字 → 应拦截；F1~F12、Home 等返回 false
		static bool IsBareLetterOrDigit(WORD vk, WORD mod);
		/// 在 pm 控件树查找已占用快捷键的控件（Button::SetShortcutKey / Control Alt-shortcut）
		/// pExclude 为正在编辑的控件（不计冲突）；pConflictText 可写提示文案
		static CControlUI* FindShortcutConflict(CPaintManagerUI* pm, WORD vk, WORD mod,
			CControlUI* pExclude = NULL, CDuiString* pConflictText = NULL);

	protected:
		CHotKeyWnd * m_pWindow;
		UINT m_uButtonState;
		CDuiString m_sImage;
		CDuiString m_sHoverImage;
		CDuiString m_sFocusImage;
		CDuiString m_sDisabledImage;
		DWORD m_dwHotKeybkColor;
		bool m_bNativeBkColorCustom;
		bool m_bReadOnly;

	protected:
		WORD m_wVirtualKeyCode;
		WORD m_wModifiers;
	};
}


#endif