#ifndef __UIAPPICON_H__
#define __UIAPPICON_H__

#pragma once

namespace DuiLib
{
	/// 手机桌面风格应用图标：图标在上、标题在下；继承 Button 的图标库 / 点击 / 悬停。
	/// text 为空且有图标时为纯图标（图标居中），未写死宽高时自动收成近似正方形。
	/// 无图标仅有 text 时为文字图标：仅图标区画主题色底 + 可换行文字（超出裁切），全文走 tooltip。
	/// 可选内置角标（数字 / 小红点），挂在图标区右上角。
	class UILIB_API CAppIconUI : public CButtonUI
	{
		DECLARE_DUICONTROL(CAppIconUI)
	public:
		CAppIconUI();

		LPCTSTR GetClass() const override;
		LPVOID GetInterface(LPCTSTR pstrName) override;
		bool PreferClientHit() const override;

		void SetManager(CPaintManagerUI* pManager, CControlUI* pParent, bool bInit = true) override;
		void SetText(LPCTSTR pstrText) override;
		void SetSubText(LPCTSTR pstrText);
		void SetIconSize(int nSize) override;
		void ClearIcon() override;
		void SetIconLib(LPCTSTR pstrLib, LPCTSTR pstrName) override;
		void SetIconSrc(LPCTSTR pstrPath) override;
		bool SetIconFromMemory(const BYTE* pData, DWORD dwSize) override;
		void OnResetDpiAssets() override;

		/// 无 SVG/光栅、仅有 text 时为文字图标模式
		bool IsTextIcon() const;
		void SetTextIconBackground(DWORD dw);
		DWORD GetTextIconBackground() const { return m_dwTextIconBk; }
		bool IsTextIconBackgroundCustom() const { return m_bTextIconBkCustom; }
		void SetTextIconColor(DWORD dw);
		DWORD GetTextIconColor() const { return m_dwTextIconFg; }
		bool IsTextIconColorCustom() const { return m_bTextIconFgCustom; }
		/// C++ 里 ClearIcon / SetIconLib / SetIconSrc / SetFileIcon 后调用，刷新文字图标/自动尺寸/自动 tip
		void RefreshLayout();

		/// 文件图标：PNG/JPG/BMP/GIF/SVG 显示内容；EXE/DLL/ICO/其它取外壳/内嵌图标
		void SetFileIcon(LPCTSTR pstrPath);
		LPCTSTR GetFileIcon() const { return m_sFileIcon.GetData(); }

		void SetBadgeCount(int n);
		int GetBadgeCount() const { return m_nBadgeCount; }
		void SetBadgeOverflow(int n);
		int GetBadgeOverflow() const { return m_nBadgeOverflow; }
		void SetBadgeShowZero(bool b);
		bool IsBadgeShowZero() const { return m_bBadgeShowZero; }
		void SetBadgeDot(bool b);
		bool IsBadgeDot() const { return m_bBadgeDot; }
		void SetBadgeHang(bool b);
		bool IsBadgeHang() const { return m_bBadgeHang; }
		void SetBadgeOffset(SIZE sz);
		SIZE GetBadgeOffset() const { return m_szBadgeOffset; }
		void SetBadgeColor(DWORD dw);
		DWORD GetBadgeColor() const { return m_dwBadgeColor; }
		void SetBadgeTextColor(DWORD dw);
		DWORD GetBadgeTextColor() const { return m_dwBadgeTextColor; }

		void SetKind(ControlKind kind) override;
		void SetAttribute(LPCTSTR pstrName, LPCTSTR pstrValue) override;
		void PaintText(IRenderContext& ctx) override;
		bool DoPaint(IRenderContext& ctx, const RECT& rcPaint, CControlUI* pStopControl) override;

	protected:
		void ApplyDefaultHoverChrome();
		void SyncAutoSize();
		void SyncTextIconToolTip();
		bool IsIconOnly() const;
		DWORD ResolveTextIconBk() const;
		DWORD ResolveTextIconFg() const;
		bool LayoutTextIconPlate(const RECT& rcContent, RECT& rcIcon) const;
		int ScaleValue(int v) const;
		bool ShouldShowBadge() const;
		CDuiString FormatBadgeCount() const;
		SIZE MeasureBadgeSize() const;
		RECT CalcIconHostRect() const;
		RECT CalcBadgeRect(const RECT& rcHost) const;
		void PaintBadge(IRenderContext& ctx);
		bool ApplyFileIcon();
		static HICON LoadFileHIcon(LPCTSTR pstrPath, int cx, int cy);
		static HBITMAP HIconToHBitmap(HICON hIcon, int cx, int cy);

	protected:
		bool m_bWidthFromSkin;
		bool m_bHeightFromSkin;
		bool m_bTipFromSkin;
		bool m_bTipAuto;
		bool m_bTextIconBkCustom;
		bool m_bTextIconFgCustom;
		DWORD m_dwTextIconBk;
		DWORD m_dwTextIconFg;
		CDuiString m_sFileIcon;
		int m_nBadgeCount;
		int m_nBadgeOverflow;
		bool m_bBadgeShowZero;
		bool m_bBadgeDot;
		bool m_bBadgeHang;
		SIZE m_szBadgeOffset;
		DWORD m_dwBadgeColor;
		DWORD m_dwBadgeTextColor;
		int m_nBadgeDotSize;
		int m_nBadgeHeight;
	};
}

#endif // __UIAPPICON_H__