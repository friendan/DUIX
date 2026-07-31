#include "StdAfx.h"
#include "UIHorizontalLayout.h"

namespace DuiLib
{
	IMPLEMENT_DUICONTROL(CHorizontalLayoutUI)

	CHorizontalLayoutUI::CHorizontalLayoutUI() : CLinearLayoutUI(LAYOUT_HORIZONTAL) {}

	LPCTSTR CHorizontalLayoutUI::GetClass() const
	{
		return _T("HorizontalLayoutUI");
	}

	LPVOID CHorizontalLayoutUI::GetInterface(LPCTSTR pstrName)
	{
		if( _tcsicmp(pstrName, DUI_CTR_HORIZONTALLAYOUT) == 0 ) return static_cast<CHorizontalLayoutUI*>(this);
		return CLinearLayoutUI::GetInterface(pstrName);
	}

	void CHorizontalLayoutUI::SetAttribute(LPCTSTR pstrName, LPCTSTR pstrValue)
	{
		if( _tcsicmp(pstrName, _T("sepwidth")) == 0 ) SetSepSize(_ttoi(pstrValue));
		else CLinearLayoutUI::SetAttribute(pstrName, pstrValue);
	}
}