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

		LPCTSTR GetClass() const override;
		LPVOID GetInterface(LPCTSTR pstrName) override;

		bool Add(CControlUI* pControl) override;
		bool AddAt(CControlUI* pControl, int iIndex) override;
		bool Remove(CControlUI* pControl) override;
		void RemoveAll() override;
		int GetCurSel() const;
		virtual bool SelectItem(int iIndex);
		virtual bool SelectItem(CControlUI* pControl);
		bool MoveItem(int iFrom, int iTo);

		void SetPos(RECT rc, bool bNeedInvalidate = true) override;
		SIZE EstimateSize(SIZE szAvailable) override;

		void SetAttribute(LPCTSTR pstrName, LPCTSTR pstrValue) override;

	protected:
		void RequestLayout();

		int m_iCurSel;
		int m_iDeferredSel;
	};
}
#endif // __UITABLAYOUT_H__
