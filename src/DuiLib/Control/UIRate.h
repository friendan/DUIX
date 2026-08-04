#ifndef __UIRATE_H__
#define __UIRATE_H__

#pragma once

namespace DuiLib
{
	/// 评分：一排可点星标（支持半星、清空、只读）。
	class UILIB_API CRateUI : public CControlUI
	{
		DECLARE_DUICONTROL(CRateUI)
	public:
		CRateUI();

		LPCTSTR GetClass() const;
		LPVOID GetInterface(LPCTSTR pstrName);
		UINT GetControlFlags() const;
		bool PreferClientHit() const;

		void SetValue(double v, bool bNotify = true);
		double GetValue() const;
		void SetCount(int n);
		int GetCount() const;
		void SetAllowHalf(bool b);
		bool IsAllowHalf() const;
		void SetAllowClear(bool b);
		bool IsAllowClear() const;
		void SetReadOnly(bool b);
		bool IsReadOnly() const;
		void SetStarSize(int n);
		int GetStarSize() const;
		void SetStarGap(int n);
		int GetStarGap() const;
		void SetCharacter(LPCTSTR pstr);
		LPCTSTR GetCharacter() const;
		void SetStarColor(DWORD dw);
		DWORD GetStarColor() const;
		void SetVoidColor(DWORD dw);
		DWORD GetVoidColor() const;

		void SetAttribute(LPCTSTR pstrName, LPCTSTR pstrValue);
		SIZE EstimateSize(SIZE szAvailable);
		bool DoPaint(IRenderContext& ctx, const RECT& rcPaint, CControlUI* pStopControl);
		void DoEvent(TEventUI& event);

	protected:
		int ScaleValue(int v) const;
		double ClampValue(double v) const;
		double Quantize(double v) const;
		RECT GetStarRect(int index) const; // 0-based
		int HitStarIndex(POINT pt) const; // 1-based，未命中 -1
		double NextValueForStar(int star1) const;
		double DisplayValue() const;
		int ResolveFont() const;
		void PaintStar(IRenderContext& ctx, int index, double display);

	protected:
		double m_dbValue;
		double m_dbHover;
		int m_nCount;
		bool m_bAllowHalf;
		bool m_bAllowClear;
		bool m_bReadOnly;
		int m_nStarSize;
		int m_nStarGap;
		DWORD m_dwStarColor;
		DWORD m_dwVoidColor;
		CDuiString m_sCharacter;
	};
}

#endif // __UIRATE_H__
