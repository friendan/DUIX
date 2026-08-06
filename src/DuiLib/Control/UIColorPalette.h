#ifndef UI_PALLET_H
#define UI_PALLET_H
#pragma once

namespace DuiLib {
	/////////////////////////////////////////////////////////////////////////////////////
	// HSL 调色板：上方面板 = 色相(H)×明度(B)；下方条 = 饱和度(S)
	class UILIB_API CColorPaletteUI : public CControlUI
	{
		DECLARE_DUICONTROL(CColorPaletteUI)
	public:
		CColorPaletteUI();
		virtual ~CColorPaletteUI();

		/// 当前色（DuiLib DWORD = RRGGBBAA），可直接作背景色
		DWORD GetSelectColor();
		void SetSelectColor(DWORD dwColor);

		virtual LPCTSTR GetClass() const;
		virtual LPVOID GetInterface(LPCTSTR pstrName);
		virtual void SetAttribute(LPCTSTR pstrName, LPCTSTR pstrValue);

		void SetPalletHeight(int nHeight);
		int GetPalletHeight() const;
		void SetPaletteHeight(int nHeight) { SetPalletHeight(nHeight); }
		int GetPaletteHeight() const { return GetPalletHeight(); }

		void SetBarHeight(int nHeight);
		int GetBarHeight() const;

		void SetThumbImage(LPCTSTR pszImage);
		LPCTSTR GetThumbImage() const;

		virtual UINT GetControlFlags() const;
		virtual void SetPos(RECT rc, bool bNeedInvalidate = true);
		virtual void DoInit();
		virtual void DoEvent(TEventUI& event);
		virtual void PaintBackgroundColor(IRenderContext& ctx);
		virtual void PaintPallet(IRenderContext& ctx);

	protected:
		void EnsureOffscreen();
		void ReleaseOffscreen();
		void SyncCursorFromValues();
		void GetPalletBarRect(RECT& rcPallet, RECT& rcBar) const;
		void NotifyColorChanging();
		void NotifyColorChanged();
		bool ApplyPalletPoint(POINT pt);
		bool ApplyBarPoint(POINT pt);
		bool NudgeByKey(WPARAM chKey);
		void UpdatePalletData();
		void UpdateBarData();
		void PaintThumb(IRenderContext& ctx, RECT rcThumb);

	private:
		HDC			m_MemDc;
		HBITMAP		m_hMemBitmap;
		HBITMAP		m_hOldBitmap;
		BITMAP		m_bmInfo;
		BYTE		*m_pBits;
		UINT		m_uButtonState;
		bool		m_bIsInBar;
		bool		m_bIsInPallet;
		int			m_nCurH;
		int			m_nCurS;
		int			m_nCurB;

		int			m_nPalletHeight;
		int			m_nBarHeight;
		CDuiPoint		m_ptLastPalletMouse;
		CDuiPoint		m_ptLastBarMouse;
		CDuiString  m_strThumbImage;
	};
}

#endif // UI_PALLET_H
