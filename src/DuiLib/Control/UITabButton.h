#ifndef __UITABBUTTON_H__
#define __UITABBUTTON_H__

#pragma once

namespace DuiLib
{
	class CLabelUI;
	class CSvgBoxUI;
	class CTabBarUI;
	class CLoadingUI;

	// 标签页按钮：可选图标 + 标题 + 关闭钮；鼠标事件由父 TabBar 统一处理
	class UILIB_API CTabButtonUI : public CHorizontalLayoutUI
	{
		DECLARE_DUICONTROL(CTabButtonUI)
	public:
		CTabButtonUI();
		~CTabButtonUI();

		LPCTSTR GetClass() const;
		LPVOID GetInterface(LPCTSTR pstrName);

		void SetTabTitle(LPCTSTR pstrTitle);
		CDuiString GetTabTitle() const;
		void SetText(LPCTSTR pstrText) override;

		void SetActive(bool bActive);
		bool IsActive() const { return m_bActive; }

		void SetLocked(bool bLocked);
		bool IsLocked() const { return m_bLocked; }

		void SetCloseHover(bool bHover);
		RECT GetCloseRect() const;

		void SetUrl(LPCTSTR pstrUrl);
		CDuiString GetUrl() const { return m_sUrl; }
		void SetDir(LPCTSTR pstrDir);
		CDuiString GetDir() const { return m_sDir; }

		void SetButtonWidth(int nWidth);
		int GetButtonWidth() const;

		void SetIconSize(int nSize);
		int GetIconSize() const { return m_nIconSize; }
		/// SVG 着色；0 表示跟随标签文字色（UpdateStyle）
		void SetIconTint(DWORD dwColor);
		DWORD GetIconTint() const { return m_dwIconTint; }
		bool HasIconTint() const { return m_bIconTint; }
		/// 文件路径：BMP/PNG/JPG/JPEG，或 .svg
		void SetTabIcon(LPCTSTR pstrPath);
		/// 内存图：BMP/PNG/JPG/JPEG 编码字节（不接管 pData）
		bool SetTabIcon(const BYTE* pData, DWORD dwSize);
		/// HBITMAP：复制一份入库（不销毁传入句柄）；宽高为 0 时从位图读取
		bool SetTabIcon(HBITMAP hBitmap, int nWidth = 0, int nHeight = 0, bool bAlpha = true);
		LPCTSTR GetTabIcon() const { return m_sIconPath; }
		/// 图标库：pstrLib 为 lucide / tabler-outline / bsicon 等，与 XML 属性名一致
		void SetTabIconLib(LPCTSTR pstrLib, LPCTSTR pstrName);
		void ClearTabIcon();
		bool HasTabIcon() const;
		/// 加载中图标（CLoadingUI）；与 Svg/Raster 互斥
		void SetTabLoading(bool bLoading);
		bool IsTabLoading() const;
		void SetLoadingType(LPCTSTR pstrType);
		LPCTSTR GetLoadingType() const { return m_sLoadingType; }

		// 标题文字对齐（空=继承 TabBar 的 tab-text-align / tab-vertical-align）
		void SetTitleTextAlign(LPCTSTR pstrAlign);
		LPCTSTR GetTitleTextAlign() const { return m_sTextAlign; }
		void SetTitleVerticalAlign(LPCTSTR pstrAlign);
		LPCTSTR GetTitleVerticalAlign() const { return m_sVerticalAlign; }

		void ApplyHoverStyle(bool bHover);
		void UpdateStyle();
		bool DoPaint(IRenderContext& ctx, const RECT& rcPaint, CControlUI* pStopControl) override;
		bool IsCloseFullyVisible() const;

		void SetAttribute(LPCTSTR pstrName, LPCTSTR pstrValue) override;
		void DoInit() override;

	protected:
		void EnsureChildren();
		CTabBarUI* GetOwnerBar() const;
		void ApplyIconSize();
		void ApplyTitleTextAlign();
		void ShowSvgIcon();
		void ShowRasterIcon(LPCTSTR pstrPath, bool bMemoryKey);
		void HideLoadingIcon();
		void ApplyLoadingAppearance();
		void RefreshRasterIconImage();
		void ReleaseMemIcon();
		void ClearPendingMemIcon();
		bool InstallMemIcon(HBITMAP hBitmap, int nWidth, int nHeight, bool bAlpha);
		bool FlushPendingMemIcon();
		bool IsIconAttr(LPCTSTR pstrName) const;
		static bool IsRasterImagePath(LPCTSTR pstrPath);
		DWORD ResolveIconColor() const;

		CControlUI* m_pLeftPad;
		CSvgBoxUI* m_pIcon;
		CControlUI* m_pRasterIcon;
		CLoadingUI* m_pLoading;
		CControlUI* m_pIconGap;
		CLabelUI* m_pTitle;
		CLabelUI* m_pClose;
		bool m_bActive;
		bool m_bLocked;
		bool m_bCloseHovered;
		bool m_bHover;
		bool m_bMemIcon;
		bool m_bIconTint;
		DWORD m_dwIconTint;
		int m_nIconSize;
		CDuiString m_sUrl;
		CDuiString m_sDir;
		CDuiString m_sTextAlign;
		CDuiString m_sVerticalAlign;
		CDuiString m_sIconPath;
		CDuiString m_sMemIconKey;
		CDuiString m_sLoadingType;
		BYTE* m_pPendingIconData;
		DWORD m_dwPendingIconSize;
		HBITMAP m_hPendingIcon;
		int m_nPendingIconW;
		int m_nPendingIconH;
		bool m_bPendingIconAlpha;
	};
}

#endif // __UITABBUTTON_H__
