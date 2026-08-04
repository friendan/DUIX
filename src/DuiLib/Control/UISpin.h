#ifndef __UISPIN_H__
#define __UISPIN_H__

#pragma once

namespace DuiLib
{
	/// 数字步进（InputNumber）：Edit + 上下按钮；支持小数 / 负值 / 滚轮。
	class UILIB_API CSpinUI : public CEditUI
	{
		DECLARE_DUICONTROL(CSpinUI)
	public:
		CSpinUI();

		LPCTSTR GetClass() const;
		LPVOID GetInterface(LPCTSTR pstrName);
		UINT GetControlFlags() const;
		bool PreferClientHit() const;

		void SetValue(double v);
		double GetValue() const;
		void SetMin(double v);
		double GetMin() const;
		void SetMax(double v);
		double GetMax() const;
		void SetStep(double v);
		double GetStep() const;
		void SetPrecision(int n);
		int GetPrecision() const;
		void SetControls(bool bShow);
		bool IsControls() const;

		void StepUp();
		void StepDown();

		int GetNativeEditRightReserve() const;
		void OnNativeEditChanged();
		void SetText(LPCTSTR pstrText);
		void SetAttribute(LPCTSTR pstrName, LPCTSTR pstrValue);
		SIZE EstimateSize(SIZE szAvailable);
		void DoEvent(TEventUI& event);
		void PaintStatusImage(IRenderContext& ctx);
		void PaintText(IRenderContext& ctx);

	protected:
		int ScaleValue(int v) const;
		void SyncTextFromValue(bool bNotifyEdit = true);
		bool ParseTextToValue(LPCTSTR pstr, double& out) const;
		double ClampValue(double v) const;
		CDuiString FormatValue(double v) const;
		void LayoutButtons();
		int HitButton(POINT pt) const; // 1=up -1=down 0=none
		void ApplyIntegerEditStyle();

	protected:
		double m_dbValue;
		double m_dbMin;
		double m_dbMax;
		double m_dbStep;
		int m_nPrecision;
		bool m_bControls;
		bool m_bUpdating;
		int m_nBtnWidth;
		int m_nHoverBtn; // 1 / -1 / 0
		RECT m_rcBtnUp;
		RECT m_rcBtnDown;
	};
}

#endif // __UISPIN_H__
