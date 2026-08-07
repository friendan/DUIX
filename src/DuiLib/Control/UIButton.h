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
		~CButtonUI();

		LPCTSTR GetClass() const;
		LPVOID GetInterface(LPCTSTR pstrName);
		UINT GetControlFlags() const;
		bool PreferClientHit() const;
		void SetManager(CPaintManagerUI* pManager, CControlUI* pParent, bool bInit = true);

		bool Activate();
		void SetEnabled(bool bEnable = true);
		void DoEvent(TEventUI& event);

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
		void SetIconLib(LPCTSTR pstrLib, LPCTSTR pstrName);
		void SetIconSrc(LPCTSTR pstrPath);
		void ClearIcon();
		bool HasIcon() const;
		void SetIconSize(int nSize);
		int GetIconSize() const { return m_nIconSize; }
		void SetIconGap(int nGap);
		int GetIconGap() const { return m_nIconGap; }
		void SetIconPosition(LPCTSTR pstrPos);
		LPCTSTR GetIconPosition() const { return m_sIconPos; }
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
		LPCTSTR GetLoadingType() const { return m_sLoadingType; }
		void SetLoadingDisable(bool bDisable);
		bool IsLoadingDisable() const { return m_bLoadingDisable; }

		SIZE EstimateSize(SIZE szAvailable) override;

		void SetAttribute(LPCTSTR pstrName, LPCTSTR pstrValue);

		void SetKind(ControlKind kind) override;
		void SetOutline(bool bOutline) override;

		void PaintText(IRenderContext& ctx);

		void PaintBackgroundColor(IRenderContext& ctx);
		void PaintStatusImage(IRenderContext& ctx);
		void PaintBorder(IRenderContext& ctx);
		void PaintForegroundImage(IRenderContext& ctx);
		void PaintBackgroundImage(IRenderContext& ctx);

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
	};

}	// namespace DuiLib

#endif // __UIBUTTON_H__
