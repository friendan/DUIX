#ifndef __UIGROUPBOX_H__
#define __UIGROUPBOX_H__

#pragma once

namespace DuiLib
{

	class UILIB_API CGroupBoxUI : public CVerticalLayoutUI
	{
		DECLARE_DUICONTROL(CGroupBoxUI)
	public:
		CGroupBoxUI();
		~CGroupBoxUI();
		 LPCTSTR GetClass() const;
		LPVOID GetInterface(LPCTSTR pstrName);
		void SetColor(DWORD dwColor);
		DWORD GetColor() const;
		void SetDisabledColor(DWORD dwColor);
		DWORD GetDisabledColor() const;
		void SetFont(int index);
		int GetFont() const;

	protected:	
		//Paint
		virtual void PaintText(IRenderContext& ctx);
		virtual void PaintBorder(IRenderContext& ctx);
		virtual void SetAttribute(LPCTSTR pstrName, LPCTSTR pstrValue);

	private:
		SIZE CalcrectSize(SIZE szAvailable);

	protected:
		DWORD m_dwColor;
		DWORD m_dwDisabledColor;
		int m_iFont;
		UINT m_uTextStyle;
	};
}
#endif // __UIGROUPBOX_H__