#include "StdAfx.h"
#include "UIVerticalLayout.h"

namespace DuiLib
{
	IMPLEMENT_DUICONTROL(CVerticalLayoutUI)

	CVerticalLayoutUI::CVerticalLayoutUI() : CLinearLayoutUI(LAYOUT_VERTICAL) {}

	LPCTSTR CVerticalLayoutUI::GetClass() const
	{
		return _T("VerticalLayoutUI");
	}

	LPVOID CVerticalLayoutUI::GetInterface(LPCTSTR pstrName)
	{
		if( _tcsicmp(pstrName, DUI_CTR_VERTICALLAYOUT) == 0 ) return static_cast<CVerticalLayoutUI*>(this);
		return CLinearLayoutUI::GetInterface(pstrName);
	}

	void CVerticalLayoutUI::SetAttribute(LPCTSTR pstrName, LPCTSTR pstrValue)
	{
		if( _tcsicmp(pstrName, _T("sepheight")) == 0 ) SetSepSize(_ttoi(pstrValue));
		else CLinearLayoutUI::SetAttribute(pstrName, pstrValue);
	}
}