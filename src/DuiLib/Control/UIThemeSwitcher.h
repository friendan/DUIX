#ifndef __UITHEMESWITCHER_H__
#define __UITHEMESWITCHER_H__
#pragma once

namespace DuiLib
{
	/// 主题切换触发器：显示当前主题，点击弹出两列预览/编辑窗
	class UILIB_API CThemeSwitcherUI : public CButtonUI
	{
		DECLARE_DUICONTROL(CThemeSwitcherUI)
	public:
		CThemeSwitcherUI();

		LPCTSTR GetClass() const;
		LPVOID GetInterface(LPCTSTR pstrName);

		void SetThemesFilter(LPCTSTR pstrThemes);
		LPCTSTR GetThemesFilter() const;
		LPCTSTR GetThemeId() const;

		/// 图标色是否随主题自动刷新（默认 true）；false 时保留皮肤里的 icon-tint*
		void SetTintAuto(bool bAuto);
		bool IsTintAuto() const;

		void SyncFromManager();
		void OpenPicker();

		bool Activate();
		void SetAttribute(LPCTSTR pstrName, LPCTSTR pstrValue);
		void DoInit();

	protected:
		CDuiString m_sThemesFilter;
		CDuiString m_sThemeId;
		bool m_bTintAuto;
	};

	/// 主题选择 / 自定义预览窗（由 ThemeSwitcher 打开）
	class UILIB_API CThemePickerWnd : public WindowImplBase
	{
	public:
		CThemePickerWnd(CThemeSwitcherUI* pOwner, LPCTSTR pstrThemesFilter);
		~CThemePickerWnd();

		static void Open(HWND hOwner, CThemeSwitcherUI* pSwitcher, LPCTSTR pstrThemesFilter);

		virtual void OnFinalMessage(HWND hWnd);
		virtual CDuiString GetSkinFile();
		virtual LPCTSTR GetWindowClassName() const;
		virtual void InitWindow();
		virtual void Notify(TNotifyUI& msg);
		DUI_DECLARE_MESSAGE_MAP()
		virtual void OnClick(TNotifyUI& msg);
		virtual LRESULT MessageHandler(UINT uMsg, WPARAM wParam, LPARAM lParam, bool& bHandled);

	private:
		bool ThemeAllowed(LPCTSTR id) const;
		void RebuildThemeList();
		void SelectTheme(LPCTSTR id, bool bPreviewApply);
		void RebuildTokenList();
		void RebuildMiniPreview();
		void SetEditMode(bool bEdit);
		void OnTokenRowClick(CControlUI* pRow);
		void OnTokenHexClick(CControlUI* pHex);
		void BeginHexEdit(LPCTSTR token);
		void CommitHexEdit(bool bApply);
		void FocusHexEdit();
		void OnPaletteColor(DWORD dwColor, bool bChanging);
		bool DoNewTheme();
		bool DoImport();
		bool DoSaveAs();
		void CommitAndClose(bool bOk);
		CTheme* GetSelectedTheme() const;

	private:
		CThemeSwitcherUI* m_pOwner;
		CDuiString m_sThemesFilter;
		CDuiString m_sEnterThemeId;
		CDuiString m_sSelectedId;
		CDuiString m_sEditToken;
		CDuiString m_sLastHexClick;
		DWORD m_dwLastHexClick;
		bool m_bEditMode;
		bool m_bHexEditing;
		bool m_bCommitted;
		CVerticalLayoutUI* m_pThemeList;
		CVerticalLayoutUI* m_pTokenList;
		CHorizontalLayoutUI* m_pMiniPreview;
		CColorPaletteUI* m_pPalette;
		static CThemePickerWnd* s_pActive;
	};
}

#endif // __UITHEMESWITCHER_H__
