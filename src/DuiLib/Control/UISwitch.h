#ifndef __UISWITCH_H__
#define __UISWITCH_H__

#pragma once

namespace DuiLib
{
	/// 开关：轨道 + 滑块；行为同 CheckBox（无 group 时点击切换）。
	class UILIB_API CSwitchUI : public COptionUI
	{
		DECLARE_DUICONTROL(CSwitchUI)
	public:
		CSwitchUI();

		LPCTSTR GetClass() const;
		LPVOID GetInterface(LPCTSTR pstrName);

		void SetCheck(bool bCheck);
		bool GetCheck() const;

		void SetTrackSize(SIZE sz);
		SIZE GetTrackSize() const;
		void SetTrackGap(int nGap);
		int GetTrackGap() const;
		void SetThumbInset(int nInset);
		int GetThumbInset() const;

		void SetCheckedText(LPCTSTR pstrText);
		LPCTSTR GetCheckedText() const;
		void SetUncheckedText(LPCTSTR pstrText);
		LPCTSTR GetUncheckedText() const;

		void SetAttribute(LPCTSTR pstrName, LPCTSTR pstrValue);
		SIZE EstimateSize(SIZE szAvailable);
		void PaintBackgroundColor(IRenderContext& ctx);
		void PaintStatusImage(IRenderContext& ctx);
		void PaintBorder(IRenderContext& ctx);
		void PaintText(IRenderContext& ctx);

	protected:
		bool IsNativeSwitchStyle() const;
		RECT GetTrackRect() const;
		RECT GetThumbRect(const RECT& rcTrack) const;
		void PaintNativeSwitch(IRenderContext& ctx);

	protected:
		SIZE m_szTrack;
		int m_nTrackGap;
		int m_nThumbInset;
		DWORD m_dwTrackColor;
		DWORD m_dwTrackColorChecked;
		DWORD m_dwTrackColorHover;
		DWORD m_dwTrackColorCheckedHover;
		DWORD m_dwTrackColorDisabled;
		DWORD m_dwTrackColorCheckedDisabled;
		DWORD m_dwThumbColor;
		DWORD m_dwThumbColorDisabled;
		DWORD m_dwInnerTextColor;
		DWORD m_dwInnerTextColorUnchecked;
		CDuiString m_sCheckedText;
		CDuiString m_sUncheckedText;
	};
}

#endif // __UISWITCH_H__
