#ifndef __UISLIDER_H__
#define __UISLIDER_H__

#pragma once

namespace DuiLib
{
	class UILIB_API CSliderUI : public CProgressUI
	{
		DECLARE_DUICONTROL(CSliderUI)
	public:
		CSliderUI();

		LPCTSTR GetClass() const;
		UINT GetControlFlags() const;
		LPVOID GetInterface(LPCTSTR pstrName);

		void SetEnabled(bool bEnable = true);

		int GetChangeStep();
		void SetChangeStep(int step);
		void SetThumbSize(SIZE szXY);
		RECT GetThumbRect() const;
		LPCTSTR GetThumbImage() const;
		void SetThumbImage(LPCTSTR pStrImage);
		LPCTSTR GetThumbHoverImage() const;
		void SetThumbHoverImage(LPCTSTR pStrImage);
		LPCTSTR GetThumbActiveImage() const;
		void SetThumbActiveImage(LPCTSTR pStrImage);

		void DoEvent(TEventUI& event);
		void SetAttribute(LPCTSTR pstrName, LPCTSTR pstrValue);
		void PaintForegroundImage(IRenderContext& ctx);

		void SetValue(int nValue);
		void SetCanSendMove(bool bCanSend);
		bool GetCanSendMove() const;
	protected:
		SIZE m_szThumb;
		UINT m_uButtonState;
		int m_nStep;

		CDuiString m_sThumbImage;
		CDuiString m_sThumbHoverImage;
		CDuiString m_sThumbActiveImage;

		CDuiString m_sImageModify;
		bool	   m_bSendMove;
	};
}

#endif // __UISLIDER_H__