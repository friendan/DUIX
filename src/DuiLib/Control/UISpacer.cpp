#include "StdAfx.h"
#include "UISpacer.h"

namespace DuiLib
{
	IMPLEMENT_DUICONTROL(CSpacerUI)

	CSpacerUI::CSpacerUI()
	{
		SetMouseEnabled(false);
	}

	LPCTSTR CSpacerUI::GetClass() const
	{
		return _T("SpacerUI");
	}

	LPVOID CSpacerUI::GetInterface(LPCTSTR pstrName)
	{
		if( _tcsicmp(pstrName, DUI_CTR_SPACER) == 0 ) return static_cast<CSpacerUI*>(this);
		return CControlUI::GetInterface(pstrName);
	}

	UINT CSpacerUI::GetControlFlags() const
	{
		return 0;
	}

	bool CSpacerUI::DoPaint(IRenderContext& ctx, const RECT& rcPaint, CControlUI* pStopControl)
	{
		return true;
	}
}