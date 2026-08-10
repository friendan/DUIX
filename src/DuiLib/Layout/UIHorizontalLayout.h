#ifndef __UIHORIZONTALLAYOUT_H__
#define __UIHORIZONTALLAYOUT_H__

#pragma once

namespace DuiLib
{
	class UILIB_API CHorizontalLayoutUI : public CLinearLayoutUI
	{
		DECLARE_DUICONTROL(CHorizontalLayoutUI)
	public:
		CHorizontalLayoutUI();

		LPCTSTR GetClass() const;
		LPVOID GetInterface(LPCTSTR pstrName);

		void SetSepWidth(int iWidth) { SetSepSize(iWidth); }
		int GetSepWidth() const { return GetSepSize(); }
		void SetAttribute(LPCTSTR pstrName, LPCTSTR pstrValue);
	};
}
#endif // __UIHORIZONTALLAYOUT_H__