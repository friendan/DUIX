#include "StdAfx.h"
#include "UITabLayout.h"

namespace DuiLib
{
	IMPLEMENT_DUICONTROL(CTabLayoutUI)
	CTabLayoutUI::CTabLayoutUI() : m_iCurSel(-1), m_iDeferredSel(-1)
	{
	}

	LPCTSTR CTabLayoutUI::GetClass() const
	{
		return _T("TabLayoutUI");
	}

	LPVOID CTabLayoutUI::GetInterface(LPCTSTR pstrName)
	{
		if( _tcsicmp(pstrName, DUI_CTR_TABLAYOUT) == 0 ) return static_cast<CTabLayoutUI*>(this);
		return CContainerUI::GetInterface(pstrName);
	}

	void CTabLayoutUI::RequestLayout()
	{
		// 自身 NeedUpdate：下一帧用当前 rc 再 SetPos，给新激活页铺尺寸。
		// 仅 NeedParentUpdate 时，若父未脏且本控件未脏，子页会一直停在空矩形。
		NeedUpdate();
		NeedParentUpdate();
	}

	bool CTabLayoutUI::Add(CControlUI* pControl)
	{
		bool ret = CContainerUI::Add(pControl);
		if( !ret ) return ret;

		// 动态加入时父级可见，但子项可能被 CContainerUI::Add 标成 InternVisible=false
		pControl->SetInternVisible(true);

		if(m_iCurSel == -1 && pControl->IsVisible())
		{
			m_iCurSel = GetItemIndex(pControl);
		}
		else
		{
			pControl->SetVisible(false);
		}

		if( m_iDeferredSel >= 0 && m_iDeferredSel < GetCount() )
		{
			SelectItem(m_iDeferredSel);
			m_iDeferredSel = -1;
		}
		else
		{
			RequestLayout();
		}

		return ret;
	}

	bool CTabLayoutUI::AddAt(CControlUI* pControl, int iIndex)
	{
		bool ret = CContainerUI::AddAt(pControl, iIndex);
		if( !ret ) return ret;

		pControl->SetInternVisible(true);

		if(m_iCurSel == -1 && pControl->IsVisible())
		{
			m_iCurSel = GetItemIndex(pControl);
		}
		else if( m_iCurSel != -1 && iIndex <= m_iCurSel )
		{
			m_iCurSel += 1;
			pControl->SetVisible(false);
		}
		else
		{
			pControl->SetVisible(false);
		}

		RequestLayout();
		return ret;
	}

	bool CTabLayoutUI::Remove(CControlUI* pControl)
	{
		if( pControl == NULL) return false;

		int index = GetItemIndex(pControl);
		bool ret = CContainerUI::Remove(pControl);
		if( !ret ) return false;

		if( m_iCurSel == index)
		{
			if( GetCount() > 0 )
			{
				m_iCurSel=0;
				CControlUI* pSel = GetItemAt(m_iCurSel);
				if( pSel != NULL ) {
					pSel->SetInternVisible(true);
					pSel->SetVisible(true);
				}
			}
			else
				m_iCurSel=-1;
			if( m_pManager != NULL ) {
				m_pManager->SendNotify(this, DUI_MSGTYPE_TABSELECT, m_iCurSel, index);
			}
			RequestLayout();
		}
		else if( m_iCurSel > index )
		{
			m_iCurSel -= 1;
		}

		return ret;
	}

	void CTabLayoutUI::RemoveAll()
	{
		int iOldSel = m_iCurSel;
		m_iCurSel = -1;
		CContainerUI::RemoveAll();
		if( m_pManager != NULL && iOldSel != -1 ) {
			m_pManager->SendNotify(this, DUI_MSGTYPE_TABSELECT, m_iCurSel, iOldSel);
		}
		RequestLayout();
	}

	int CTabLayoutUI::GetCurSel() const
	{
		return m_iCurSel;
	}

	bool CTabLayoutUI::SelectItem(int iIndex)
	{
		if( iIndex < 0 || iIndex >= m_items.GetSize() ) return false;
		if( iIndex == m_iCurSel ) {
			// 同页再选：仍确保可见并重新布局（动态 Add 后下标已对但页从未 SetPos）
			CControlUI* pSel = GetItemAt(iIndex);
			if( pSel != NULL ) {
				pSel->SetInternVisible(true);
				if( !pSel->IsVisible() )
					pSel->SetVisible(true);
			}
			RequestLayout();
			return true;
		}

		int iOldSel = m_iCurSel;
		m_iCurSel = iIndex;
		for( int it = 0; it < m_items.GetSize(); it++ )
		{
			if( it == iIndex ) {
				CControlUI* p = GetItemAt(it);
				p->SetInternVisible(true);
				p->SetVisible(true);
				// 不在此 SetFocus：右键菜单 / WebBrowser 等会在切页时崩或抢焦点
			}
			else GetItemAt(it)->SetVisible(false);
		}
		RequestLayout();

		if( m_pManager != NULL ) {
			m_pManager->SendNotify(this, DUI_MSGTYPE_TABSELECT, m_iCurSel, iOldSel);
		}
		return true;
	}

	bool CTabLayoutUI::SelectItem( CControlUI* pControl )
	{
		int iIndex=GetItemIndex(pControl);
		if (iIndex==-1)
			return false;
		else
			return SelectItem(iIndex);
	}

	bool CTabLayoutUI::MoveItem(int iFrom, int iTo)
	{
		if( iFrom == iTo ) return true;
		if( iFrom < 0 || iTo < 0 || iFrom >= GetCount() || iTo >= GetCount() ) return false;

		CControlUI* pControl = GetItemAt(iFrom);
		if( pControl == NULL ) return false;
		CControlUI* pSelected = (m_iCurSel >= 0 && m_iCurSel < GetCount()) ? GetItemAt(m_iCurSel) : NULL;

		SetAutoDestroy(false);
		if( !CContainerUI::Remove(pControl) ) {
			SetAutoDestroy(true);
			return false;
		}
		// 目标：移动后位于 iTo（与 CTabBarUI::MoveTab 相同）
		int insertAt = iTo;
		if( insertAt > GetCount() ) insertAt = GetCount();
		if( !CContainerUI::AddAt(pControl, insertAt) ) {
			SetAutoDestroy(true);
			return false;
		}
		SetAutoDestroy(true);

		if( pSelected != NULL )
			m_iCurSel = GetItemIndex(pSelected);
		else
			m_iCurSel = -1;

		RequestLayout();
		return true;
	}

	void CTabLayoutUI::SetAttribute(LPCTSTR pstrName, LPCTSTR pstrValue)
	{
		if( _tcsicmp(pstrName, _T("selected-id")) == 0 ) {
			int iSel = _ttoi(pstrValue);
			if( !SelectItem(iSel) )
				m_iDeferredSel = iSel;
		}
		return CContainerUI::SetAttribute(pstrName, pstrValue);
	}

	void CTabLayoutUI::SetPos(RECT rc, bool bNeedInvalidate)
	{
		CControlUI::SetPos(rc, bNeedInvalidate);
		rc = m_rcItem;

		RECT rcPadding = GetPadding();
		// Adjust for padding
		rc.left += rcPadding.left;
		rc.top += rcPadding.top;
		rc.right -= rcPadding.right;
		rc.bottom -= rcPadding.bottom;

		for( int it = 0; it < m_items.GetSize(); it++ ) {
			CControlUI* pControl = static_cast<CControlUI*>(m_items[it]);
			if( !pControl->IsVisible() ) continue;
			if( pControl->IsAbsolute() ) {
				SetAbsolutePos(it);
				continue;
			}

			if( it != m_iCurSel ) continue;

			RECT rcMargin = pControl->GetMargin();
			RECT rcPage = rc;
			rcPage.left += rcMargin.left;
			rcPage.top += rcMargin.top;
			rcPage.right -= rcMargin.right;
			rcPage.bottom -= rcMargin.bottom;

			// 当前页铺满 TabLayout 客户区（不按子控件 EstimateSize 收缩）
			SIZE sz = { MAX(0, rcPage.right - rcPage.left), MAX(0, rcPage.bottom - rcPage.top) };
			if( sz.cx < pControl->GetMinWidth() ) sz.cx = pControl->GetMinWidth();
			if( sz.cx > pControl->GetMaxWidth() ) sz.cx = pControl->GetMaxWidth();
			if( sz.cy < pControl->GetMinHeight() ) sz.cy = pControl->GetMinHeight();
			if( sz.cy > pControl->GetMaxHeight() ) sz.cy = pControl->GetMaxHeight();

			RECT rcCtrl = { rcPage.left, rcPage.top, rcPage.left + sz.cx, rcPage.top + sz.cy };
			pControl->SetPos(rcCtrl);
		}
	}

	SIZE CTabLayoutUI::EstimateSize(SIZE szAvailable)
	{
		// 与 VBox/HBox 一致：未设宽高时返回 0，由父布局拉伸；勿按子页内容收缩
		return CControlUI::EstimateSize(szAvailable);
	}
}
