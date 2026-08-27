#ifndef __UIBUTTON_H__
#define __UIBUTTON_H__

#pragma once

namespace DuiLib
{
	class CSvgBoxUI;
	class CLoadingUI;

	class UILIB_API CButtonUI : public CLabelUI
	{
		DECLARE_DUICONTROL(CButtonUI)

	public:
		CButtonUI();
		~CButtonUI() override;

		LPCTSTR GetClass() const override;
		LPVOID GetInterface(LPCTSTR pstrName) override;
		UINT GetControlFlags() const override;
		bool PreferClientHit() const override;
		void SetManager(CPaintManagerUI* pManager, CControlUI* pParent, bool bInit = true) override;

		bool Activate() override;
		void SetEnabled(bool bEnable = true) override;
		void DoEvent(TEventUI& event) override;

		virtual LPCTSTR GetImage();
		virtual void SetImage(LPCTSTR pStrImage);
		virtual LPCTSTR GetHoverImage();
		virtual void SetHoverImage(LPCTSTR pStrImage);
		virtual LPCTSTR GetActiveImage();
		virtual void SetActiveImage(LPCTSTR pStrImage);
		virtual LPCTSTR GetFocusImage();
		virtual void SetFocusImage(LPCTSTR pStrImage);
		virtual LPCTSTR GetDisabledImage();
		virtual void SetDisabledImage(LPCTSTR pStrImage);
		virtual LPCTSTR GetHoverForegroundImage();
		virtual void SetHoverForegroundImage(LPCTSTR pStrImage);
		virtual LPCTSTR GetActiveForegroundImage();
		virtual void SetActiveForegroundImage(LPCTSTR pStrImage);
		void SetStateCount(int nCount);
		int GetStateCount() const;
		virtual LPCTSTR GetStateImage();
		virtual void SetStateImage(LPCTSTR pStrImage);

		void BindTabIndex(int _BindTabIndex);
		void BindTabLayoutName(LPCTSTR _TabLayoutName);
		void BindTriggerTabSel(int _SetSelectIndex = -1);
		void RemoveBindTabIndex();
		int	 GetBindTabLayoutIndex();
		LPCTSTR GetBindTabLayoutName();

		void SetHoverFont(int index);
		int GetHoverFont() const;
		void SetActiveFont(int index);
		int GetActiveFont() const;
		void SetFocusedFont(int index);
		int GetFocusedFont() const;

		void SetFocusedColor(DWORD dwColor);
		DWORD GetFocusedColor() const;

		/// SVG 图标库 / 文件（属性名同 SvgBox：bsicon、lucide…）；icon / icon-src：SVG 或 PNG/BMP/JPG
		virtual void SetIconLib(LPCTSTR pstrLib, LPCTSTR pstrName);
		virtual void SetIconSrc(LPCTSTR pstrPath);
		/// 内存位图图标（接管 hBitmap；失败时 DeleteObject）。用于 EXE/外壳图标等。
		bool SetIconBitmap(HBITMAP hBitmap, int nWidth, int nHeight, bool bAlpha = true);
		/// 内存字节图标：PNG/JPG/BMP/GIF/WEBP、ICO、SVG（网站 favicon 等）。失败静默返回 false。
		/// 未挂 Manager 时先缓存，SetManager 后自动应用。
		virtual bool SetIconFromMemory(const BYTE* pData, DWORD dwSize);
		/// HICON → 预乘 alpha 的 32 位 HBITMAP（供 SetIconBitmap；失败 NULL）
		/// DrawIconEx + 必要时黑白双缓冲重建 alpha；并裁透明边铺满
		static HBITMAP CreateBitmapFromHIcon(HICON hIcon, int cx, int cy);
		virtual void ClearIcon();
		bool HasIcon() const;
		virtual void SetIconSize(int nSize);
		int GetIconSize() const { return m_nIconSize; }
		void SetIconGap(int nGap);
		int GetIconGap() const { return m_nIconGap; }
		void SetIconPosition(LPCTSTR pstrPos);
		LPCTSTR GetIconPosition() const { return m_sIconPos.GetData(); }
		void SetIconTint(DWORD dwColor);
		void SetIconTintHover(DWORD dwColor);
		void SetIconTintActive(DWORD dwColor);
		void SetIconTintDisabled(DWORD dwColor);
		void SetIconTintFocus(DWORD dwColor);
		/// 光栅：跟随文字色/kind（等同 SVG 默认）；`icon-tint="auto"`
		void SetIconTintAuto(bool bAuto);
		bool IsIconTintAuto() const { return m_bIconTintAuto; }

		/// 加载中：图标位显示 Loading；默认自动 SetEnabled(false)，结束时恢复原先 enabled
		void SetLoading(bool bLoading);
		bool IsLoading() const;
		void SetLoadingType(LPCTSTR pstrType);
		LPCTSTR GetLoadingType() const { return m_sLoadingType.GetData(); }
		void SetLoadingDisable(bool bDisable);
		bool IsLoadingDisable() const { return m_bLoadingDisable; }

		/// 右键「修改文本」：默认 false
		void SetEditTextEnabled(bool bEnable);
		bool IsEditTextEnabled() const { return m_bEditText; }
		/// 右键「设置/清除快捷键」：默认 false（与 edit-text 独立）
		void SetEditHotKeyEnabled(bool bEnable);
		bool IsEditHotKeyEnabled() const { return m_bEditHotKey; }

		/// 副标题（第二行：快捷键/说明）；空则单行。默认色/字号可省略
		void SetSubText(LPCTSTR pstrText);
		LPCTSTR GetSubText() const { return m_sSubText.GetData(); }
		void SetSubColor(DWORD dwColor);
		DWORD GetSubColor() const { return m_dwSubColor; }
		void SetSubFont(int index);
		int GetSubFont() const { return m_iSubFont; }
		void SetSubGap(int nGap);
		int GetSubGap() const { return m_nSubGap; }
		bool HasSubText() const { return !m_sSubText.IsEmpty(); }

		/// 与 sub-text 配套的快捷键（仅存储；不自动注册加速键 / RegisterHotKey）
		/// scope：0=程序（HOTKEYBOX_SCOPE_APP，默认），1=全局（HOTKEYBOX_SCOPE_GLOBAL）
		void SetShortcutKey(WORD wVirtualKeyCode, WORD wModifiers, int scope = 0);
		void GetShortcutKey(WORD& wVirtualKeyCode, WORD& wModifiers) const;
		void GetShortcutKey(WORD& wVirtualKeyCode, WORD& wModifiers, int& scope) const;
		int GetShortcutScope() const { return m_nShortcutScope; }
		void ClearShortcutKey();
		bool HasShortcutKey() const { return m_wShortcutVk != 0 || m_wShortcutMod != 0; }

		SIZE EstimateSize(SIZE szAvailable) override;

		void SetAttribute(LPCTSTR pstrName, LPCTSTR pstrValue) override;

		void SetKind(ControlKind kind) override;
		void SetOutline(bool bOutline) override;

		void PaintText(IRenderContext& ctx) override;

		void PaintBackgroundColor(IRenderContext& ctx) override;
		void PaintStatusImage(IRenderContext& ctx) override;
		void PaintBorder(IRenderContext& ctx) override;
		void PaintForegroundImage(IRenderContext& ctx) override;
		void PaintBackgroundImage(IRenderContext& ctx) override;

	protected:
		enum IconKind { IconNone = 0, IconSvg = 1, IconRaster = 2 };

		void SyncControlStateFromButton();
		void EnsureIcon();
		void EnsureRasterIcon();
		void EnsureLoading();
		bool IsIconAttr(LPCTSTR pstrName) const;
		static bool IsRasterImagePath(LPCTSTR pstrPath);
		void ShowSvgIcon();
		void ShowRasterIcon(LPCTSTR pstrPath);
		void RefreshRasterIconImage();
		void ReleaseMemIcon();
		void ClearPendingIconMemory();
		bool ApplyIconFromMemory(const BYTE* pData, DWORD dwSize);
		void ClearRasterTintCache();
		bool EnsureRasterTintCache(DWORD dwColor);
		void PaintRasterIcon(IRenderContext& ctx, const RECT& rcIcon);
		bool ShouldTintRasterIcon() const;
		void SyncIconAppearance();
		void SyncLoadingAppearance();
		void RestoreIconAfterLoading();
		DWORD ResolveIconColor() const;
		DWORD ResolvePaintIconColor() const;
		/// 根据内容区计算图标矩形，并收缩 rcText
		bool LayoutIconAndText(const RECT& rcContent, RECT& rcIcon, RECT& rcText) const;
		/// 主行 + 可选副行的文字块尺寸（逻辑像素测量结果已含 DPI 字体）
		SIZE MeasureTitleBlock(int iMainFont) const;
		int ResolvePaintMainFont() const;
		int ResolvePaintSubFont(int iMainFont) const;
		DWORD ResolveSubTextColor(DWORD clrMain) const;
		void ShowEditTextMenu(POINT ptClient);
		bool HandleEditTextMenuClick(WPARAM wParam);

		friend class CButtonEditTextMenuFilter;

		UINT m_uButtonState;

		int		m_iHoverFont;
		int		m_iActiveFont;
		int		m_iFocusedFont;

		DWORD m_dwFocusedColor;

		CDuiString m_sImage;
		CDuiString m_sHoverImage;
		CDuiString m_sHoverForegroundImage;
		CDuiString m_sActiveImage;
		CDuiString m_sActiveForegroundImage;
		CDuiString m_sFocusImage;
		CDuiString m_sDisabledImage;
		int m_nStateCount;
		CDuiString m_sStateImage;

		int			m_iBindTabIndex;
		CDuiString	m_sBindTabLayoutName;

		CSvgBoxUI* m_pIcon;
		CControlUI* m_pRasterIcon;
		CLoadingUI* m_pLoading;
		IconKind m_eIconKind;
		CDuiString m_sRasterPath;
		CDuiString m_sMemIconKey;
		bool m_bRasterMemKey;
		BYTE* m_pPendingIconData;
		DWORD m_dwPendingIconSize;
		HBITMAP m_hRasterTint;
		DWORD m_dwRasterTintColor;
		int m_nRasterTintW;
		int m_nRasterTintH;
		CDuiString m_sLoadingType;
		bool m_bLoading;
		bool m_bLoadingDisable;       // loading 时是否自动禁用（默认 true）
		bool m_bEnabledBeforeLoading; // 进入 loading 前的 enabled
		int m_nIconSize;
		int m_nIconGap;
		CDuiString m_sIconPos; // left / right / top / bottom
		DWORD m_dwIconTint;
		DWORD m_dwIconTintHover;
		DWORD m_dwIconTintActive;
		DWORD m_dwIconTintDisabled;
		DWORD m_dwIconTintFocus;
		bool m_bIconTint;
		bool m_bIconTintAuto; // 光栅跟随文字色；SVG 始终着色
		bool m_bEditText;              // 右键「修改文本」
		bool m_bEditHotKey;            // 右键「设置/清除快捷键」
		bool m_bEditTextMenuPending;   // 已弹出内置菜单，等待 WM_MENUCLICK
		CDuiString m_sSubText;         // 第二行说明/快捷键
		DWORD m_dwSubColor;            // 0=自动（主题次要色 / 主色半透明）
		int m_iSubFont;                // -1=与主行同 font
		int m_nSubGap;                 // 主副行间距（逻辑像素）
		WORD m_wShortcutVk;            // 快捷键虚拟键（0=无）
		WORD m_wShortcutMod;           // HOTKEYF_* 修饰键
		int m_nShortcutScope;          // HOTKEYBOX_SCOPE_APP / GLOBAL
	};

}	// namespace DuiLib

#endif // __UIBUTTON_H__
