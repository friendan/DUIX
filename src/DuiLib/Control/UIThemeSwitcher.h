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

		/// 注册主题到 CThemeManager；若本控件 themes 白名单非空，同时加入显示列表
		/// displayName 非空则覆盖显示名（左列文案）
		bool AddTheme(CTheme* pTheme, bool bOwn = true, LPCTSTR displayName = NULL);
		/// 从 :root 文件加载并注册（不切换当前主题）；白名单非空时加入显示列表。失败返回 NULL
		CTheme* AddThemeFile(LPCTSTR path, LPCTSTR idOverride = NULL, LPCTSTR displayName = NULL);
		/// 设置已注册主题的显示名（选择窗左列 / tooltip）
		bool SetThemeDisplayName(LPCTSTR id, LPCTSTR displayName);
		LPCTSTR GetThemeDisplayName(LPCTSTR id) const;
		/// 把已注册主题 id 加入 themes 白名单；可选同时设显示名
		bool IncludeTheme(LPCTSTR id, LPCTSTR displayName = NULL);
		/// 从 themes 白名单移除（不卸载 CThemeManager 中的主题）
		bool ExcludeTheme(LPCTSTR id);
		/// 白名单为空=全部；否则是否包含该 id
		bool IsThemeListed(LPCTSTR id) const;

		/// 图标色是否随主题自动刷新（默认 true）；false 时保留皮肤里的 icon-tint*
		void SetTintAuto(bool bAuto);
		bool IsTintAuto() const;

		/// 预览窗是否模态（默认 false：可切回主窗看实时预览；true 则 ShowModal 禁用主窗）
		void SetModal(bool bModal);
		bool IsModal() const;

		void SyncFromManager();
		void OpenPicker();

		bool Activate();
		void SetAttribute(LPCTSTR pstrName, LPCTSTR pstrValue);
		void DoInit();

	protected:
		CDuiString m_sThemesFilter;
		CDuiString m_sThemeId;
		bool m_bTintAuto;
		bool m_bModal;
	};

	/// 主题选择 / 自定义预览窗（由 ThemeSwitcher 打开）
	class UILIB_API CThemePickerWnd : public WindowImplBase
	{
	public:
		enum EditMode {
			EDITMODE_FAMILY = 0,
			EDITMODE_SIMPLE = 1,
			EDITMODE_ADVANCED = 2
		};

		CThemePickerWnd(CThemeSwitcherUI* pOwner, LPCTSTR pstrThemesFilter);
		~CThemePickerWnd();

		static void Open(HWND hOwner, CThemeSwitcherUI* pSwitcher, LPCTSTR pstrThemesFilter, bool bModal = false);

		virtual void OnFinalMessage(HWND hWnd);
		virtual CDuiString GetSkinFile();
		virtual LPCTSTR GetWindowClassName() const;
		virtual void InitWindow();
		virtual void Notify(TNotifyUI& msg);
		DUI_DECLARE_MESSAGE_MAP()
		virtual void OnClick(TNotifyUI& msg);

	private:
		bool ThemeAllowed(LPCTSTR id) const;
		static bool IsBuiltinThemeId(LPCTSTR id);
		bool EnsureEditableTheme();
		void RebuildThemeList();
		void SelectTheme(LPCTSTR id, bool bPreviewApply);
		void RebuildTokenList();
		void RebuildSeedPanel();
		void RebuildFamilyChips();
		void RebuildMiniPreview();
		void UpdateTokenRowColor(LPCTSTR token, DWORD dwColor);
		void UpdateSeedRowColor(LPCTSTR token, DWORD dwColor);
		void UpdateTokenRowSelection();
		void UpdateBrightLabel();
		void UpdateHueLabel();
		void SetEditMode(bool bEdit);
		void SetPickerMode(int mode);
		void UpdateModeUI();
		static bool IsSeedToken(LPCTSTR token);
		void ExpandFromSeeds(bool bRefreshUI);
		void ApplyFamilyTheme(bool bChanging);
		void OnTokenRowClick(CControlUI* pRow);
		void OnTokenHexClick(CControlUI* pHex);
		void BeginHexEdit(LPCTSTR token);
		void CommitHexEdit(bool bApply);
		void FocusHexEdit();
		void ApplyTokenColor(LPCTSTR token, DWORD dwColor, bool bChanging);
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
		CDuiString m_sFamilyId;
		DWORD m_dwLastHexClick;
		int m_nEditMode;
		int m_nBrightness;
		int m_nHueShift;
		bool m_bEditMode;
		bool m_bHexEditing;
		bool m_bCommitted;
		bool m_bFamilyUIGuard;
		bool m_bModal;
		CVerticalLayoutUI* m_pThemeList;
		CVerticalLayoutUI* m_pTokenList;
		CVerticalLayoutUI* m_pSeedPanel;
		CVerticalLayoutUI* m_pFamilyPanel;
		CHorizontalLayoutUI* m_pFamilyChips;
		CHorizontalLayoutUI* m_pMiniPreview;
		CColorPaletteUI* m_pPalette;
		CSegmentedUI* m_pModeSeg;
		CSliderUI* m_pBrightSlider;
		CSliderUI* m_pHueSlider;
		CLabelUI* m_pEditHint;
		CLabelUI* m_pBrightLabel;
		CLabelUI* m_pHueLabel;
		static CThemePickerWnd* s_pActive;
	};
}

#endif // __UITHEMESWITCHER_H__