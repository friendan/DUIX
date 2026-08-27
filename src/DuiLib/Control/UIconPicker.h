#ifndef __UICONPICKER_H__
#define __UICONPICKER_H__
#pragma once

namespace DuiLib
{
	/// 图标选择触发器：显示当前选中图标，点击弹出图标选择窗。
	/// 内部通过 `CIconLibrary` / 各内置库枚举接口列图标；选中后把 `{库名, 图标名}` 写回并发 `selectchanged`。
	class UILIB_API CIconPickerUI : public CButtonUI
	{
		DECLARE_DUICONTROL(CIconPickerUI)
	public:
		enum { ICONPICKER_SWATCH_COUNT = 6 };
		CIconPickerUI();

		LPCTSTR GetClass() const;
		LPVOID GetInterface(LPCTSTR pstrName);

		/// 当前选中的图标库名（如 _T("tabler-outline")）
		void SetLibrary(LPCTSTR pstrLib);
		LPCTSTR GetLibrary() const;
		/// 当前选中的图标名（如 _T("settings")）；写入同时刷新触发按钮显示
		void SetSelectedIcon(LPCTSTR pstrName);
		LPCTSTR GetSelectedIcon() const;

		/// 打开选择窗时默认选中的图标库（如 _T("lucide")）；空 = 默认选第一个可见库
		void SetDefaultLibrary(LPCTSTR pstrLib);
		LPCTSTR GetDefaultLibrary() const;

		/// 图标显示尺寸（宽/高）；确定后写回并刷新触发按钮图标
		using CButtonUI::SetIconSize;
		void SetIconSize(int nW, int nH);
		int GetIconWidth() const;
		int GetIconHeight() const;

		/// 选择窗是否显示“大小设置”区（默认 true）；false 可固定宽高不给用户改
		void SetShowSizeSettings(bool bShow);
		bool IsShowSizeSettings() const;
		/// 选择窗是否显示“颜色自定义”区（默认 true）；false 可固定颜色不给用户改
		void SetShowColorSettings(bool bShow);
		bool IsShowColorSettings() const;

		/// 库白名单（逗号分隔，如 _T("lucide,tabler-outline")）；空 = 显示全部内置库
		void SetLibrariesFilter(LPCTSTR pstrLibs);
		LPCTSTR GetLibrariesFilter() const;
		/// 白名单为空=全部；否则是否包含该库
		bool IsLibListed(LPCTSTR lib) const;

		/// 选择窗是否模态（默认 false：可切回主窗；true 则 ShowModal 禁用主窗）
		void SetModal(bool bModal);
		bool IsModal() const;

		/// 图标尺寸允许范围（宽/高共用）。默认 8~256。
		void SetSizeRange(int nMin, int nMax);
		int GetSizeMin() const;
		int GetSizeMax() const;

		/// 图标颜色筛选（ARGB）。默认 0 = 无（用图标原色/主题默认）。
		void SetIconColor(DWORD dwColor);
		DWORD GetIconColor() const;
		/// 预设色块（绿/蓝/青/紫/橙/红）；未设置时使用内置默认值。
		void SetPresetColor(int iIndex, DWORD dwColor);
		DWORD GetPresetColor(int iIndex) const;

		/// 打开选择窗（已开则前置）
		void OpenPicker();

		bool Activate();
		void SetAttribute(LPCTSTR pstrName, LPCTSTR pstrValue);
		void DoInit();

	protected:
		/// 用当前选中刷新触发按钮的图标显示
		void SyncIcon();

	protected:
		CDuiString m_sIconLib;      // 当前库
		CDuiString m_sIconName;     // 当前图标名
		CDuiString m_sDefaultLib;   // 打开选择窗默认选中的库（空=第一个可见）
		CDuiString m_sLibsFilter;   // 库白名单
		bool m_bShowSize;           // 选择窗是否显示大小设置区
		bool m_bShowColor;          // 选择窗是否显示颜色自定义区
		bool m_bModal;
		int m_nSizeMin;             // 尺寸下限
		int m_nSizeMax;             // 尺寸上限
		int m_nIconW;               // 图标显示宽
		int m_nIconH;               // 图标显示高
		DWORD m_dwIconColor;        // 图标颜色筛选（0=无）
		DWORD m_dwPresetColors[ICONPICKER_SWATCH_COUNT];
	};

	/// 图标选择窗（由 CIconPickerUI 打开）
	class UILIB_API CIconPickerWnd : public WindowImplBase
	{
	public:
		/// 打开选择窗；pOwner 提供当前库/选中图标/白名单/颜色；nSizeMin/NMax 为尺寸范围
		CIconPickerWnd(CIconPickerUI* pOwner, LPCTSTR pstrLibsFilter, int nSizeMin, int nSizeMax, DWORD dwIconColor);
		~CIconPickerWnd();

		/// 打开选择窗；pOwner 提供当前库/选中图标/白名单/颜色；nSizeMin/NMax 为尺寸范围
		static void Open(HWND hOwner, CIconPickerUI* pPicker, int nSizeMin, int nSizeMax, DWORD dwIconColor, bool bModal = false);

		virtual void OnFinalMessage(HWND hWnd);
		virtual CDuiString GetSkinFile();
		virtual LPCTSTR GetWindowClassName() const;
		virtual void InitWindow();
		virtual void Notify(TNotifyUI& msg);
		DUI_DECLARE_MESSAGE_MAP()
		virtual void OnClick(TNotifyUI& msg);

	private:
		bool LibAllowed(LPCTSTR lib) const;
		void RebuildLibList();
		void SelectLib(LPCTSTR lib);
		void RebuildIconGrid();
		void SetIconSize(int nW, int nH);
		void BuildSizeCombos();         // 给宽/高下拉填预设
		void SyncSizeCombos();          // 下拉选中对齐当前 W/H
		void SetSizeMode(bool bCustom); // 在 预设(combo) 与 自定义(edit) 之间切换
		bool IsPresetPair() const;      // 宽/高是否都是预设值
		void SyncSizeEdits();           // 把当前 W/H 写回两个输入框
		void SelectColor(DWORD dwColor);           // 设置图标颜色并刷新网格预览
		void SyncCustomColorControls();           // 用当前 m_dwIconColor 同步自定义面板(色板/预览/hex)
		void SetCustomColorPanel(bool bShow);     // 显示/隐藏“自定义颜色”面板
		void CommitHexColor();                    // 从 hex 输入框读取自定义色并应用
		void ApplySwatchColors();                 // 用纯色板常量填各色块的填充
		void SyncColorSwatches();                  // 高亮当前选中的颜色块
		void ApplyCellColor(CButtonUI* pBtn) const;// 给某个网格按钮应用当前颜色
		void ReapplyGridCellColors();           // 只重上色网格现有按钮（不重建）常用于拖动改色
		int ClampSize(int v) const;     // 校验：界于 [SizeMin, SizeMax]
		void ApplySizeInputs();         // 从两个输入框读取并应用（带回退）
		void ApplySearchFilter();
		bool SelectIconByName(LPCTSTR name, bool bPreview);
		void CommitAndClose(bool bOk);

	private:
		CIconPickerUI* m_pOwner;
		CDuiString m_sLibsFilter;
		CDuiString m_sCurLib;        // 选择窗当前显示的库
		CDuiString m_sSelectedName;  // 当前选中的图标名
		int m_nIconW;                // 图标宽
		int m_nIconH;                // 图标高
		int m_nSizeMin;              // 尺寸下限
		int m_nSizeMax;              // 尺寸上限
		bool m_bModal;
		CVerticalLayoutUI* m_pLibList;
		CFlowLayoutUI* m_pIconGrid;
		CEditUI* m_pSearch;
		CComboUI* m_pSizeComboW;
		CComboUI* m_pSizeComboH;
		CEditUI* m_pSizeW;
		CEditUI* m_pSizeH;
		CControlUI* m_pComboSizeGrp;
		CControlUI* m_pEditSizeGrp;
		CButtonUI* m_pBtnSizeMode;
		bool m_bCustomSize;
		DWORD m_dwIconColor;
		CEditUI* m_pColorEdit;          // 自定义颜色 hex 输入框（在自定义色面板内）
		CColorPaletteUI* m_pColorPicker;   // 自定义色 HSL 调色板
		CLabelUI* m_pColorPreview;      // 自定义色预览色块
		CControlUI* m_pCustomColorPanel;   // “自定义”折叠面板
		bool m_bCustomActive;           // 当前处于“自定义色”模式（非预设/无）
		CLabelUI* m_pStatus;
		bool m_bShowSize;
		bool m_bShowColor;
		DWORD m_dwPresetColors[CIconPickerUI::ICONPICKER_SWATCH_COUNT];
		static CIconPickerWnd* s_pActive;
	};
}

#endif // __UICONPICKER_H__
