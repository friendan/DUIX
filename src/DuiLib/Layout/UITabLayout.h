#ifndef __UITABLAYOUT_H__
#define __UITABLAYOUT_H__

#pragma once

namespace DuiLib
{
	class UILIB_API CTabLayoutUI : public CContainerUI
	{
		DECLARE_DUICONTROL(CTabLayoutUI)
	public:
		CTabLayoutUI();

		LPCTSTR GetClass() const;
		LPVOID GetInterface(LPCTSTR pstrName);

		bool Add(CControlUI* pControl);
		bool AddAt(CControlUI* pControl, int iIndex);
		bool Remove(CControlUI* pControl);
		void RemoveAll();
		int GetCurSel() const;
		virtual bool SelectItem(int iIndex);
		virtual bool SelectItem(CControlUI* pControl);
		bool MoveItem(int iFrom, int iTo);

		void SetPos(RECT rc, bool bNeedInvalidate = true);
		SIZE EstimateSize(SIZE szAvailable) override;

		void SetAttribute(LPCTSTR pstrName, LPCTSTR pstrValue);

	protected:
		int m_iCurSel;
		int m_iDeferredSel;
	};
}
#endif // __UITABLAYOUT_H__
