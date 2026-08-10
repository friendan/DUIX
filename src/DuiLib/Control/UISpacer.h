#ifndef __UISPACER_H__
#define __UISPACER_H__

#pragma once

namespace DuiLib
{
	class UILIB_API CSpacerUI : public CControlUI
	{
		DECLARE_DUICONTROL(CSpacerUI)
	public:
		CSpacerUI();

		LPCTSTR GetClass() const;
		LPVOID GetInterface(LPCTSTR pstrName);
		UINT GetControlFlags() const;
		bool DoPaint(IRenderContext& ctx, const RECT& rcPaint, CControlUI* pStopControl);
	};
}

#endif // __UISPACER_H__