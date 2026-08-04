#ifndef __UIEMPTY_H__
#define __UIEMPTY_H__

#pragma once

namespace DuiLib
{
	/// 空状态：居中插画 + 描述 + 可选操作区（子控件）。
	class UILIB_API CEmptyUI : public CVerticalLayoutUI
	{
		DECLARE_DUICONTROL(CEmptyUI)
	public:
		CEmptyUI();
		~CEmptyUI();

		LPCTSTR GetClass() const;
		LPVOID GetInterface(LPCTSTR pstrName);

		void SetDescription(LPCTSTR pstr);
		LPCTSTR GetDescription() const;
		void SetImage(LPCTSTR pstr);
		LPCTSTR GetImage() const;
		void SetImageSize(SIZE sz);
		SIZE GetImageSize() const;
		void SetShowImage(bool b);
		bool IsShowImage() const;

		void SetAttribute(LPCTSTR pstrName, LPCTSTR pstrValue);
		void DoInit();
		bool DoPaint(IRenderContext& ctx, const RECT& rcPaint, CControlUI* pStopControl);

	protected:
		int ScaleValue(int v) const;
		void EnsureBuilt();
		void PaintDefaultIllustration(IRenderContext& ctx, const RECT& rc);

	protected:
		bool m_bBuilt;
		bool m_bShowImage;
		CDuiString m_sDescription;
		CDuiString m_sImage;
		SIZE m_szImage;
		CControlUI* m_pImageHost;
		CLabelUI* m_pDesc;
		CHorizontalLayoutUI* m_pExtra;
	};
}

#endif // __UIEMPTY_H__
