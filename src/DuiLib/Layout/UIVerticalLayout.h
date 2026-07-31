#ifndef __UIVERTICALLAYOUT_H__
#define __UIVERTICALLAYOUT_H__

#pragma once

namespace DuiLib
{
	class UILIB_API CVerticalLayoutUI : public CLinearLayoutUI
	{
		DECLARE_DUICONTROL(CVerticalLayoutUI)
	public:
		CVerticalLayoutUI();

		LPCTSTR GetClass() const;
		LPVOID GetInterface(LPCTSTR pstrName);

		void SetSepHeight(int iHeight) { SetSepSize(iHeight); }
		int GetSepHeight() const { return GetSepSize(); }
		void SetAttribute(LPCTSTR pstrName, LPCTSTR pstrValue);
	};
}
#endif // __UIVERTICALLAYOUT_H__