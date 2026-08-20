#include "StdAfx.h"
#include "UISvgBox.h"
#include "UIEmpty.h"

namespace DuiLib {

	namespace {
		// 列对齐：优先用 ListHeaderItem 的 text-align + vertical-align
		UINT ResolveListColumnTextStyle(CListHeaderUI* pHeader, int iCol, UINT uFallback)
		{
			UINT uStyle = uFallback;
			if( pHeader == NULL || iCol < 0 || iCol >= pHeader->GetCount() )
				return uStyle;

			CControlUI* pCol = pHeader->GetItemAt(iCol);
			if( pCol == NULL ) return uStyle;
			CListHeaderItemUI* pItem = static_cast<CListHeaderItemUI*>(
				pCol->GetInterface(DUI_CTR_LISTHEADERITEM));
			if( pItem == NULL ) return uStyle;

			UINT uHdr = pItem->GetTextStyle();
			uStyle &= ~(DT_LEFT | DT_CENTER | DT_RIGHT | DT_TOP | DT_VCENTER | DT_BOTTOM);
			uStyle |= (uHdr & (DT_LEFT | DT_CENTER | DT_RIGHT | DT_TOP | DT_VCENTER | DT_BOTTOM));
			if( (uStyle & (DT_TOP | DT_VCENTER | DT_BOTTOM)) == 0 )
				uStyle |= DT_VCENTER;
			// DT_LEFT==0：不能用「无左右位」判断；仅当既无 CENTER 也无 RIGHT 时保持左对齐
			if( (uStyle & DT_CENTER) == 0 && (uStyle & DT_RIGHT) == 0 )
				; // left
			else if( (uStyle & DT_CENTER) != 0 && (uStyle & DT_RIGHT) != 0 )
				uStyle &= ~DT_RIGHT;
			uStyle |= DT_SINGLELINE;
			if( (uHdr & DT_END_ELLIPSIS) != 0 )
				uStyle |= DT_END_ELLIPSIS;
			return uStyle;
		}
	}

	void ApplyListColumnCellPadding(RECT& rcCell, CListHeaderUI* pHeader, int iCol, const TListInfoUI* pInfo)
	{
		if( pInfo == NULL ) return;
		int padL = pInfo->rcTextPadding.left;
		int padR = pInfo->rcTextPadding.right;
		int padT = pInfo->rcTextPadding.top;
		int padB = pInfo->rcTextPadding.bottom;

		if( pHeader != NULL && iCol >= 0 && iCol < pHeader->GetCount() ) {
			CControlUI* pCol = pHeader->GetItemAt(iCol);
			CListHeaderItemUI* pItem = (pCol == NULL) ? NULL :
				static_cast<CListHeaderItemUI*>(pCol->GetInterface(DUI_CTR_LISTHEADERITEM));
			if( pItem != NULL ) {
				UINT uHdr = pItem->GetTextStyle();
				const bool bLeft = ((uHdr & DT_CENTER) == 0 && (uHdr & DT_RIGHT) == 0);
				const bool bRight = ((uHdr & DT_RIGHT) != 0 && (uHdr & DT_CENTER) == 0);
				CDuiBox hp = pItem->GetPadding();
				RECT ht = pItem->GetTextPadding();
				const int hL = hp.left + ht.left;
				const int hR = hp.right + ht.right;
				// 左对齐：内容至少跟表头同左距（ListHeaderItem 默认 padding-left=8）
				if( bLeft && hL > padL ) padL = hL;
				if( bRight && hR > padR ) padR = hR;
				if( !bLeft && !bRight ) {
					if( hL > padL ) padL = hL;
					if( hR > padR ) padR = hR;
				}
			}
		}

		rcCell.left += padL;
		rcCell.right -= padR;
		rcCell.top += padT;
		rcCell.bottom -= padB;
	}

	/////////////////////////////////////////////////////////////////////////////////////
	//
	//
	IMPLEMENT_DUICONTROL(CListUI)

		CListUI::CListUI()
		: m_bScrollSelect(false)
		, m_bMultiSel(false)
		, m_iCurSel(-1)
		, m_iFirstSel(-1)
		, m_iExpandedItem(-1)
		, m_pCallback(NULL)
	{
		m_pList = new CListBodyUI(this);
		m_pHeader = new CListHeaderUI;
		m_pEmpty = NULL;
		Add(m_pHeader);
		Add(m_pList);

		// 列表配置
		m_ListInfo.nColumns = 0;
		m_ListInfo.nFont = -1;
		m_ListInfo.uTextStyle = DT_VCENTER | DT_CENTER | DT_SINGLELINE;
		m_ListInfo.dwColor = 0x000000FF;
		m_ListInfo.dwBackgroundColor = 0;
		m_ListInfo.bAlternateBk = false;
		m_ListInfo.dwAlternateBackgroundColor = 0;
		m_ListInfo.dwSelectedColor = 0x000000FF;
		m_ListInfo.dwSelectedBackgroundColor = 0xC1E3FFFF;
		m_ListInfo.dwHoverColor = 0x000000FF;
		m_ListInfo.dwHoverBackgroundColor = 0xE9F5FFFF;
		m_ListInfo.dwDisabledColor = 0xCCCCCCFF;
		m_ListInfo.dwDisabledBackgroundColor = 0xFFFFFFFF;
		m_ListInfo.dwLineColor = 0;
		m_ListInfo.bShowRowLine = false;
		m_ListInfo.bShowColumnLine = false;
		m_ListInfo.bShowHeaderColumnLine = false;
		m_ListInfo.bShowHtml = false;
		m_ListInfo.bMultiExpandable = false;
		m_ListInfo.bRSelected = false;
		::ZeroMemory(&m_ListInfo.rcTextPadding, sizeof(m_ListInfo.rcTextPadding));
		::ZeroMemory(&m_ListInfo.rcColumn, sizeof(m_ListInfo.rcColumn));

	}

	LPCTSTR CListUI::GetClass() const
	{
		return _T("ListUI");
	}

	UINT CListUI::GetControlFlags() const
	{
		return UIFLAG_TABSTOP;
	}

	LPVOID CListUI::GetInterface(LPCTSTR pstrName)
	{
		if (_tcsicmp(pstrName, DUI_CTR_LIST) == 0) return static_cast<CListUI*>(this);
		if (_tcsicmp(pstrName, _T("IList")) == 0) return static_cast<IListUI*>(this);
		if (_tcsicmp(pstrName, _T("IListOwner")) == 0) return static_cast<IListOwnerUI*>(this);
		return CVerticalLayoutUI::GetInterface(pstrName);
	}

	CControlUI* CListUI::GetItemAt(int iIndex) const
	{
		return m_pList->GetItemAt(iIndex);
	}

	int CListUI::GetItemIndex(CControlUI* pControl) const
	{
		if (pControl->GetInterface(_T("ListHeader")) != NULL) return CVerticalLayoutUI::GetItemIndex(pControl);
		// We also need to recognize header sub-items
		if (_tcsstr(pControl->GetClass(), _T("ListHeaderItemUI")) != NULL) return m_pHeader->GetItemIndex(pControl);

		return m_pList->GetItemIndex(pControl);
	}

	bool CListUI::SetItemIndex(CControlUI* pControl, int iIndex)
	{
		if (pControl->GetInterface(_T("ListHeader")) != NULL) return CVerticalLayoutUI::SetItemIndex(pControl, iIndex);
		// We also need to recognize header sub-items
		if (_tcsstr(pControl->GetClass(), _T("ListHeaderItemUI")) != NULL) return m_pHeader->SetItemIndex(pControl, iIndex);

		int iOrginIndex = m_pList->GetItemIndex(pControl);
		if (iOrginIndex == -1) return false;
		if (iOrginIndex == iIndex) return true;

		IListItemUI* pSelectedListItem = NULL;
		if (m_iCurSel >= 0) pSelectedListItem =
			static_cast<IListItemUI*>(GetItemAt(m_iCurSel)->GetInterface(_T("ListItem")));
		if (!m_pList->SetItemIndex(pControl, iIndex)) return false;
		int iMinIndex = min(iOrginIndex, iIndex);
		int iMaxIndex = max(iOrginIndex, iIndex);
		for (int i = iMinIndex; i < iMaxIndex + 1; ++i) {
			CControlUI* p = m_pList->GetItemAt(i);
			IListItemUI* pListItem = static_cast<IListItemUI*>(p->GetInterface(_T("ListItem")));
			if (pListItem != NULL) {
				pListItem->SetIndex(i);
			}
		}
		if (m_iCurSel >= 0 && pSelectedListItem != NULL) m_iCurSel = pSelectedListItem->GetIndex();
		return true;
	}

	int CListUI::GetCount() const
	{
		return m_pList->GetCount();
	}

	bool CListUI::Add(CControlUI* pControl)
	{
		if( AttachEmptyControl(pControl) ) return true;
		// Override the Add() method so we can add items specifically to
		// the intended widgets. Headers are assumed to be
		// answer the correct interface so we can add multiple list headers.
		if (pControl->GetInterface(_T("ListHeader")) != NULL) {
			if (m_pHeader != pControl && m_pHeader->GetCount() == 0) {
				CVerticalLayoutUI::Remove(m_pHeader);
				m_pHeader = static_cast<CListHeaderUI*>(pControl);
			}
			m_ListInfo.nColumns = MIN(m_pHeader->GetCount(), UILIST_MAX_COLUMNS);
			return CVerticalLayoutUI::AddAt(pControl, 0);
		}
		// We also need to recognize header sub-items
		if (_tcsstr(pControl->GetClass(), _T("ListHeaderItemUI")) != NULL) {
			bool ret = m_pHeader->Add(pControl);
			m_ListInfo.nColumns = MIN(m_pHeader->GetCount(), UILIST_MAX_COLUMNS);
			return ret;
		}
		// The list items should know about us
		IListItemUI* pListItem = static_cast<IListItemUI*>(pControl->GetInterface(_T("ListItem")));
		if (pListItem != NULL) {
			pListItem->SetOwner(this);
			pListItem->SetIndex(GetCount());
			bool bOk = m_pList->Add(pControl);
			UpdateEmptyVisibility();
			return bOk;
		}
		return CVerticalLayoutUI::Add(pControl);
	}

	bool CListUI::AddAt(CControlUI* pControl, int iIndex)
	{
		if( AttachEmptyControl(pControl) ) return true;
		// Override the AddAt() method so we can add items specifically to
		// the intended widgets. Headers and are assumed to be
		// answer the correct interface so we can add multiple list headers.
		if (pControl->GetInterface(_T("ListHeader")) != NULL) {
			if (m_pHeader != pControl && m_pHeader->GetCount() == 0) {
				CVerticalLayoutUI::Remove(m_pHeader);
				m_pHeader = static_cast<CListHeaderUI*>(pControl);
			}
			m_ListInfo.nColumns = MIN(m_pHeader->GetCount(), UILIST_MAX_COLUMNS);
			return CVerticalLayoutUI::AddAt(pControl, 0);
		}
		// We also need to recognize header sub-items
		if (_tcsstr(pControl->GetClass(), _T("ListHeaderItemUI")) != NULL) {
			bool ret = m_pHeader->AddAt(pControl, iIndex);
			m_ListInfo.nColumns = MIN(m_pHeader->GetCount(), UILIST_MAX_COLUMNS);
			return ret;
		}
		if (!m_pList->AddAt(pControl, iIndex)) return false;

		// The list items should know about us
		IListItemUI* pListItem = static_cast<IListItemUI*>(pControl->GetInterface(_T("ListItem")));
		if (pListItem != NULL) {
			pListItem->SetOwner(this);
			pListItem->SetIndex(iIndex);
		}

		for (int i = iIndex + 1; i < m_pList->GetCount(); ++i) {
			CControlUI* p = m_pList->GetItemAt(i);
			pListItem = static_cast<IListItemUI*>(p->GetInterface(_T("ListItem")));
			if (pListItem != NULL) {
				pListItem->SetIndex(i);
			}
		}

		if (m_iCurSel >= iIndex) {
			int idx = m_aSelItems.Find((LPVOID)(INT_PTR)m_iCurSel);
			if (idx != -1) {
				m_aSelItems.SetAt(idx, (LPVOID)(INT_PTR)(m_iCurSel + 1));
			}
			m_iCurSel += 1;
		}
		UpdateEmptyVisibility();
		return true;
	}

	bool CListUI::Remove(CControlUI* pControl)
	{
		if( pControl != NULL && pControl == m_pEmpty ) {
			m_pEmpty = NULL;
			bool bOk = CVerticalLayoutUI::Remove(pControl);
			UpdateEmptyVisibility();
			return bOk;
		}
		if (pControl->GetInterface(_T("ListHeader")) != NULL) return CVerticalLayoutUI::Remove(pControl);
		// We also need to recognize header sub-items
		if (_tcsstr(pControl->GetClass(), _T("ListHeaderItemUI")) != NULL) return m_pHeader->Remove(pControl);

		int iIndex = m_pList->GetItemIndex(pControl);
		if (iIndex == -1) return false;

		if (!m_pList->RemoveAt(iIndex)) return false;

		for (int i = iIndex; i < m_pList->GetCount(); ++i) {
			CControlUI* p = m_pList->GetItemAt(i);
			IListItemUI* pListItem = static_cast<IListItemUI*>(p->GetInterface(_T("ListItem")));
			if (pListItem != NULL) {
				pListItem->SetIndex(i);
			}
		}

		if (iIndex == m_iCurSel && m_iCurSel >= 0) {
			int iSel = m_iCurSel;
			m_iCurSel = -1;

			int idx = m_aSelItems.Find((LPVOID)(INT_PTR)iSel);
			if(idx != -1) {
				m_aSelItems.Remove(idx);
			}

			SelectItem(FindSelectable(iSel, false));
		}
		else if (iIndex < m_iCurSel) {
			m_iCurSel -= 1; 
			for (int i = 0; i < m_aSelItems.GetSize(); ++i)
			{
				int sel = (int)(INT_PTR)m_aSelItems.GetAt(i);
				if (iIndex < sel)
					m_aSelItems.SetAt(i, (LPVOID)(INT_PTR)(sel - 1));
			}
		}

		UpdateEmptyVisibility();
		return true;
	}

	bool CListUI::RemoveAt(int iIndex)
	{
		if (!m_pList->RemoveAt(iIndex)) return false;

		for (int i = iIndex; i < m_pList->GetCount(); ++i) {
			CControlUI* p = m_pList->GetItemAt(i);
			IListItemUI* pListItem = static_cast<IListItemUI*>(p->GetInterface(_T("ListItem")));
			if (pListItem != NULL) pListItem->SetIndex(i);
		}

		if (iIndex == m_iCurSel && m_iCurSel >= 0) {
			int iSel = m_iCurSel;
			m_iCurSel = -1;

			int idx = m_aSelItems.Find((LPVOID)(INT_PTR)iSel);
			if(idx != -1) {
				m_aSelItems.Remove(idx);
			}

			SelectItem(FindSelectable(iSel, false));
		}
		else if (iIndex < m_iCurSel) {
			m_iCurSel -= 1;

			for (int i = 0; i < m_aSelItems.GetSize(); ++i)
			{
				int sel = (int)(INT_PTR)m_aSelItems.GetAt(i);
				if (iIndex < sel)
					m_aSelItems.SetAt(i, (LPVOID)(INT_PTR)(sel - 1));
			}
		}

		UpdateEmptyVisibility();
		return true;
	}

	void CListUI::RemoveAll()
	{
		m_iCurSel = -1;
		m_iExpandedItem = -1;
		m_aSelItems.Empty();
		m_pList->RemoveAll();
		UpdateEmptyVisibility();
	}

	bool CListUI::AttachEmptyControl(CControlUI* pControl)
	{
		if( pControl == NULL ) return false;
		CEmptyUI* pEmpty = static_cast<CEmptyUI*>(pControl->GetInterface(DUI_CTR_EMPTY));
		if( pEmpty == NULL ) return false;
		SetEmptyControl(pEmpty);
		return true;
	}

	void CListUI::SetEmptyControl(CEmptyUI* pEmpty)
	{
		if( m_pEmpty == pEmpty ) {
			UpdateEmptyVisibility();
			return;
		}
		if( m_pEmpty != NULL ) {
			CEmptyUI* pOld = m_pEmpty;
			m_pEmpty = NULL;
			CVerticalLayoutUI::Remove(pOld);
		}
		m_pEmpty = pEmpty;
		if( m_pEmpty == NULL ) return;
		m_pEmpty->SetAbsolute(true);
		m_pEmpty->SetMouseEnabled(true);
		m_pEmpty->SetMouseChildEnabled(true);
		// Empty 挂在 List 容器层，不进 m_pList；勿用 CListUI::GetItemIndex（只查 body）
		if( CVerticalLayoutUI::GetItemIndex(m_pEmpty) < 0 )
			CVerticalLayoutUI::Add(m_pEmpty);
		UpdateEmptyVisibility();
		NeedUpdate();
	}

	void CListUI::UpdateEmptyVisibility()
	{
		if( m_pEmpty == NULL ) {
			if( m_pHeader != NULL && m_pManager != NULL )
				m_pManager->RemovePostPaint(m_pHeader);
			return;
		}
		const bool bEmpty = (GetCount() == 0);
		m_pEmpty->SetVisible(bEmpty);
		if( bEmpty && m_pList != NULL ) {
			RECT rcBody = m_pList->GetPos();
			if( m_pHeader != NULL && m_pHeader->IsVisible() ) {
				RECT rcHeader = m_pHeader->GetPos();
				if( rcBody.top < rcHeader.bottom )
					rcBody.top = rcHeader.bottom;
			}
			if( rcBody.right > rcBody.left && rcBody.bottom > rcBody.top )
				m_pEmpty->SetPos(rcBody, false);
		}
		// 空态 Empty 可能盖住表头，仅此时挂 PostPaint 置顶重绘
		if( m_pHeader != NULL && m_pManager != NULL ) {
			m_pManager->RemovePostPaint(m_pHeader);
			if( bEmpty && m_pHeader->IsVisible() )
				m_pManager->AddPostPaint(m_pHeader);
		}
		Invalidate();
	}

	void CListUI::SetPos(RECT rc, bool bNeedInvalidate)
	{
		CVerticalLayoutUI::SetPos(rc, bNeedInvalidate);

		if (m_pHeader != NULL) {
			// Determine general list information and the size of header columns
			m_ListInfo.nColumns = MIN(m_pHeader->GetCount(), UILIST_MAX_COLUMNS);
			// The header/columns may or may not be visible at runtime. In either case
			// we should determine the correct dimensions...

			if (!m_pHeader->IsVisible()) {
				for (int it = 0; it < m_pHeader->GetCount(); it++) {
					static_cast<CControlUI*>(m_pHeader->GetItemAt(it))->SetInternVisible(true);
				}
				m_pHeader->SetPos(CDuiRect(rc.left, 0, rc.right, 0), bNeedInvalidate);
			}

			for (int i = 0; i < m_ListInfo.nColumns; i++) {
				CControlUI* pControl = static_cast<CControlUI*>(m_pHeader->GetItemAt(i));
				if (!pControl->IsVisible()) continue;
				if (pControl->IsAbsolute()) continue;
				m_ListInfo.rcColumn[i] = pControl->GetPos();
			}
			if (!m_pHeader->IsVisible()) {
				for (int it = 0; it < m_pHeader->GetCount(); it++) {
					static_cast<CControlUI*>(m_pHeader->GetItemAt(it))->SetInternVisible(false);
				}
			}
			m_pList->SetPos(m_pList->GetPos(), bNeedInvalidate);
		}

		if( m_pEmpty != NULL && m_pEmpty->IsVisible() && m_pList != NULL ) {
			RECT rcBody = m_pList->GetPos();
			// 空态层绝不能盖住表头
			if( m_pHeader != NULL && m_pHeader->IsVisible() ) {
				RECT rcHeader = m_pHeader->GetPos();
				if( rcBody.top < rcHeader.bottom )
					rcBody.top = rcHeader.bottom;
			}
			if( rcBody.right > rcBody.left && rcBody.bottom > rcBody.top )
				m_pEmpty->SetPos(rcBody, bNeedInvalidate);
		}
	}

	void CListUI::Move(SIZE szOffset, bool bNeedInvalidate)
	{
		CVerticalLayoutUI::Move(szOffset, bNeedInvalidate);
		if (!m_pHeader->IsVisible()) m_pHeader->Move(szOffset, false);
	}

	int CListUI::GetMinSelItemIndex()
	{
		if (m_aSelItems.GetSize() <= 0)
			return -1;
		int min = (int)(INT_PTR)m_aSelItems.GetAt(0);
		int index;
		for (int i = 0; i < m_aSelItems.GetSize(); ++i)
		{
			index = (int)(INT_PTR)m_aSelItems.GetAt(i);
			if (min > index)
				min = index;
		}
		return min;
	}

	int CListUI::GetMaxSelItemIndex()
	{
		if (m_aSelItems.GetSize() <= 0)
			return -1;
		int max = (int)(INT_PTR)m_aSelItems.GetAt(0);
		int index;
		for (int i = 0; i < m_aSelItems.GetSize(); ++i)
		{
			index = (int)(INT_PTR)m_aSelItems.GetAt(i);
			if (max < index)
				max = index;
		}
		return max;
	}

	void CListUI::DoEvent(TEventUI& event)
	{
		if (!IsMouseEnabled() && event.Type > UIEVENT__MOUSEBEGIN && event.Type < UIEVENT__MOUSEEND) {
			BubbleEvent(event);
			return;
		}

		if (event.Type == UIEVENT_SETFOCUS)
		{
			m_bFocused = true;
			return;
		}
		if (event.Type == UIEVENT_KILLFOCUS)
		{
			m_bFocused = false;
			return;
		}

		switch (event.Type) {
		case UIEVENT_KEYDOWN:
			switch (event.chKey) {
			case VK_UP:
				{
					if (m_aSelItems.GetSize() > 0) {
						int index = GetMinSelItemIndex() - 1;
						UnSelectAllItems();
						index > 0 ? SelectItem(index, true) : SelectItem(0, true);
					}
				}
				return;
			case VK_DOWN:
				{
					if (m_aSelItems.GetSize() > 0) {
						int index = GetMaxSelItemIndex() + 1;
						UnSelectAllItems();
						index + 1 > m_pList->GetCount() ? SelectItem(GetCount() - 1, true) : SelectItem(index, true);
					}
				}
				return;
			case VK_PRIOR:
				PageUp();
				return;
			case VK_NEXT:
				PageDown();
				return;
			case VK_HOME:
				SelectItem(FindSelectable(0, false), true);
				return;
			case VK_END:
				SelectItem(FindSelectable(GetCount() - 1, true), true);
				return;
			case VK_RETURN:
				if (m_iCurSel != -1) GetItemAt(m_iCurSel)->Activate();
				return;
			case 0x41:// Ctrl+A 
				{
					//全选
					if (IsMultiSelect() && (GetKeyState(VK_CONTROL) & 0x8000)) {
                        UnSelectAllItems();
						SelectAllItems();
					}
					return;
				}
			}
			break;
		case UIEVENT_SCROLLWHEEL:
			{
				switch (LOWORD(event.wParam)) {
				case SB_LINEUP:
					if (m_bScrollSelect && !IsMultiSelect()) SelectItem(FindSelectable(m_iCurSel - 1, false), true);
					else LineUp();
					return;
				case SB_LINEDOWN:
					if (m_bScrollSelect && !IsMultiSelect()) SelectItem(FindSelectable(m_iCurSel + 1, true), true);
					else LineDown();
					return;
				}
			}
			break;
		}
		CVerticalLayoutUI::DoEvent(event);
	}

	CListHeaderUI* CListUI::GetHeader() const
	{
		return m_pHeader;
	}

	CContainerUI* CListUI::GetList() const
	{
		return m_pList;
	}

	bool CListUI::GetScrollSelect()
	{
		return m_bScrollSelect;
	}

	void CListUI::SetScrollSelect(bool bScrollSelect)
	{
		m_bScrollSelect = bScrollSelect;
	}

	int CListUI::GetCurSelActivate() const
	{
		return m_iCurSelActivate;
	}

	bool CListUI::SelectItemActivate(int iIndex)
	{
		if (!SelectItem(iIndex, true)) {
			return false;
		}

		m_iCurSelActivate = iIndex;
		return true;
	}

	int CListUI::GetCurSel() const
	{
		if (m_aSelItems.GetSize() <= 0) {
			return -1;
		}
		else {
			return (int)(INT_PTR)m_aSelItems.GetAt(0);
		}

		return -1;
	}

	bool CListUI::SelectItem(int iIndex, bool bTakeFocus)
	{
		// 重置多选起始序号
		m_iFirstSel = -1;
		// 取消其它选择项
		UnSelectItem(iIndex, true);
		// 判断是否合法列表项
		if (iIndex < 0) return false;
		// 已经选择
		int aIndex = m_aSelItems.Find((LPVOID)(INT_PTR)iIndex);
		if (aIndex != -1) {
			return true;
		}
		// 选择当前列表项
		CControlUI* pControl = GetItemAt(iIndex);
		if (pControl == NULL) return false;
		IListItemUI* pListItem = static_cast<IListItemUI*>(pControl->GetInterface(_T("ListItem")));
		if (pListItem == NULL) return false;
		if (!pListItem->Select(true)) {
			return false;
		}
		int iLastSel = m_iCurSel;
		m_iCurSel = iIndex;
		//如果已经选中了就无需要再重复加入by nakkler
		if(m_aSelItems.Find((LPVOID)(INT_PTR)iIndex)==-1)
            m_aSelItems.Add((LPVOID)(INT_PTR)iIndex);

        EnsureVisible(iIndex);
        if (bTakeFocus) pControl->SetFocus();
        if (m_pManager != NULL && iLastSel != m_iCurSel)
        {
            m_pManager->SendNotify(this, DUI_MSGTYPE_ITEMSELECT, iIndex);
        }
		
		return true;
	}

	bool CListUI::SelectMultiItem(int iIndex, bool bTakeFocus)
	{
		// 未开启多选
		if (!IsMultiSelect()) return SelectItem(iIndex, bTakeFocus);
		// 全部取消
		if (iIndex < 0) {
			UnSelectAllItems();
			return true;
		}

		// 多选起始序号
		if (m_iFirstSel == -1) {
			if (m_iCurSel != -1) {
				m_iFirstSel = m_iCurSel;
			}
			else {
				m_iFirstSel = iIndex;
			}
		}

		CControlUI* pControl = GetItemAt(iIndex);
		if (pControl == NULL) return false;
		if (!pControl->IsEnabled()) return false;
		IListItemUI* pListItem = static_cast<IListItemUI*>(pControl->GetInterface(_T("ListItem")));
		if (pListItem == NULL) return false;

		// 多选判断
		if ((GetKeyState(VK_CONTROL) & 0x8000)) {
			int aIndex = m_aSelItems.Find((LPVOID)(INT_PTR)iIndex);
			if (aIndex != -1) {
				if (!pListItem->SelectMulti(false)) return false;
				if (m_iCurSel == iIndex) m_iCurSel = -1;
				m_aSelItems.Remove(aIndex);
				if (m_pManager != NULL) {
					m_pManager->SendNotify(this, DUI_MSGTYPE_ITEMSELECT, -1);
				}
			}
			else {
				if (!pListItem->SelectMulti(true)) return false;

				m_iCurSel = iIndex;
				m_aSelItems.Add((LPVOID)(INT_PTR)iIndex);
				EnsureVisible(iIndex);
				if (bTakeFocus) pControl->SetFocus();
				if (m_pManager != NULL) {
					m_pManager->SendNotify(this, DUI_MSGTYPE_ITEMSELECT, iIndex);
				}
			}
		}
		else if ((GetKeyState(VK_SHIFT) & 0x8000)) {
			UnSelectAllItems();

			int iStart = m_iFirstSel;
			int iEnd = iIndex;
			if (iStart > iEnd) {
				iStart = iEnd;
				iEnd = m_iFirstSel;
			}

			for (int index = iStart; index <= iEnd; index++) {
				CControlUI* pSelControl = GetItemAt(index);
				if (pSelControl == NULL) continue;
				if (!pSelControl->IsEnabled()) continue;
				IListItemUI* pSelListItem = static_cast<IListItemUI*>(pSelControl->GetInterface(_T("ListItem")));
				if (pSelListItem == NULL) continue;
				if (!pSelListItem->SelectMulti(true)) continue;
				m_aSelItems.Add((LPVOID)(INT_PTR)index);
			}

			m_iCurSel = iIndex;
			EnsureVisible(iIndex);
			if (bTakeFocus) pControl->SetFocus();
			if (m_pManager != NULL) {
				m_pManager->SendNotify(this, DUI_MSGTYPE_ITEMSELECT, iIndex, m_iFirstSel);
			}
		}
		else {
			if (!pListItem->SelectMulti(true)) return false;

			m_iCurSel = iIndex;
			m_aSelItems.Add((LPVOID)(INT_PTR)iIndex);
			EnsureVisible(iIndex);
			if (bTakeFocus) pControl->SetFocus();
			if (m_pManager != NULL) {
				m_pManager->SendNotify(this, DUI_MSGTYPE_ITEMSELECT, iIndex);
			}
		}
		return true;
	}

	void CListUI::SetMultiSelect(bool bMultiSel)
	{
		m_bMultiSel = bMultiSel;
		if (!bMultiSel) UnSelectAllItems();
	}

	bool CListUI::IsMultiSelect() const
	{
		return m_bMultiSel;
	}

	bool CListUI::UnSelectItem(int iIndex, bool bOthers)
	{
		if (bOthers) {
			for (int i = m_aSelItems.GetSize() - 1; i >= 0; --i) {
				int iSelIndex = (int)(INT_PTR)m_aSelItems.GetAt(i);
				if (iSelIndex == iIndex) continue;
				CControlUI* pControl = GetItemAt(iSelIndex);
				if (pControl == NULL) continue;
				if (!pControl->IsEnabled()) continue;
				IListItemUI* pSelListItem = static_cast<IListItemUI*>(pControl->GetInterface(_T("ListItem")));
				if (pSelListItem == NULL) continue;
				if (!pSelListItem->SelectMulti(false)) continue;
				if (m_iCurSel == iSelIndex) m_iCurSel = -1;
				m_aSelItems.Remove(i);
			}
			if(IsMultiSelect()) {
				if (m_pManager != NULL) {
					m_pManager->SendNotify(this, DUI_MSGTYPE_ITEMSELECT, -1, -1);
				}
			}
		}
		else {
			if (iIndex < 0) return false;
			CControlUI* pControl = GetItemAt(iIndex);
			if (pControl == NULL) return false;
			if (!pControl->IsEnabled()) return false;
			IListItemUI* pListItem = static_cast<IListItemUI*>(pControl->GetInterface(_T("ListItem")));
			if (pListItem == NULL) return false;
			int aIndex = m_aSelItems.Find((LPVOID)(INT_PTR)iIndex);
			if (aIndex < 0) return false;
			if (!pListItem->SelectMulti(false)) return false;
			if (m_iCurSel == iIndex) m_iCurSel = -1;
			m_aSelItems.Remove(aIndex);
		}
		return true;
	}

	void CListUI::SelectAllItems()
	{
		for (int i = 0; i < GetCount(); ++i) {
			CControlUI* pControl = GetItemAt(i);
			if (pControl == NULL) continue;
			if (!pControl->IsVisible()) continue;
			if (!pControl->IsEnabled()) continue;
			IListItemUI* pListItem = static_cast<IListItemUI*>(pControl->GetInterface(_T("ListItem")));
			if (pListItem == NULL) continue;
			if (!pListItem->SelectMulti(true)) continue;
			int aIndex = m_aSelItems.Find((LPVOID)(INT_PTR)i);
			if (aIndex < 0) {
				m_aSelItems.Add((LPVOID)(INT_PTR)i);
			}
			m_iCurSel = i;
		}

		if(IsMultiSelect()) {
			if (m_pManager != NULL) {
				m_pManager->SendNotify(this, DUI_MSGTYPE_ITEMSELECT, -1, -1);
			}
		}
	}

	void CListUI::UnSelectAllItems()
	{
		for (int i = 0; i < m_aSelItems.GetSize(); ++i) {
			int iSelIndex = (int)(INT_PTR)m_aSelItems.GetAt(i);
			CControlUI* pControl = GetItemAt(iSelIndex);
			if (pControl == NULL) continue;
			if (!pControl->IsEnabled()) continue;
			IListItemUI* pListItem = static_cast<IListItemUI*>(pControl->GetInterface(_T("ListItem")));
			if (pListItem == NULL) continue;
			if (!pListItem->SelectMulti(false)) continue;
		}
		m_aSelItems.Empty();
		m_iCurSel = -1;
	}

	int CListUI::GetSelectItemCount() const
	{
		return m_aSelItems.GetSize();
	}

	int CListUI::GetNextSelItem(int nItem) const
	{
		if (m_aSelItems.GetSize() <= 0)
			return -1;

		if (nItem < 0) {
			return (int)(INT_PTR)m_aSelItems.GetAt(0);
		}
		int aIndex = m_aSelItems.Find((LPVOID)(INT_PTR)nItem);
		if (aIndex < 0) return -1;
		if (aIndex + 1 > m_aSelItems.GetSize() - 1)
			return -1;
		return (int)(INT_PTR)m_aSelItems.GetAt(aIndex + 1);
	}

	UINT CListUI::GetListType()
	{
		return LT_LIST;
	}

	TListInfoUI* CListUI::GetListInfo()
	{
		return &m_ListInfo;
	}

	bool CListUI::IsDelayedDestroy() const
	{
		return m_pList->IsDelayedDestroy();
	}

	void CListUI::SetDelayedDestroy(bool bDelayed)
	{
		m_pList->SetDelayedDestroy(bDelayed);
	}

	int CListUI::GetGap() const
	{
		return m_pList->GetGap();
	}

	void CListUI::SetGap(int iPadding)
	{
		m_pList->SetGap(iPadding);
	}

	void CListUI::SetItemFont(int index)
	{
		m_ListInfo.nFont = index;
		NeedUpdate();
	}

	void CListUI::SetItemTextStyle(UINT uStyle)
	{
		m_ListInfo.uTextStyle = uStyle;
		NeedUpdate();
	}

	void CListUI::SetItemTextPadding(RECT rc)
	{
		m_ListInfo.rcTextPadding = rc;
		NeedUpdate();
	}

	RECT CListUI::GetItemTextPadding() const
	{
		RECT rect = m_ListInfo.rcTextPadding;
		GetManager()->GetDPIObj()->Scale(&rect);
		return rect;
	}

	void CListUI::SetItemColor(DWORD dwColor)
	{
		m_ListInfo.dwColor = dwColor;
		Invalidate();
	}

	void CListUI::SetItemBackgroundColor(DWORD dwBackgroundColor)
	{
		m_ListInfo.dwBackgroundColor = dwBackgroundColor;
		Invalidate();
	}

	void CListUI::SetItemBkImage(LPCTSTR pStrImage)
	{
		m_ListInfo.sBkImage = pStrImage;
		Invalidate();
	}

	void CListUI::SetAlternateBk(bool bAlternateBk)
	{
		m_ListInfo.bAlternateBk = bAlternateBk;
		Invalidate();
	}

	void CListUI::SetAlternateBkColor(DWORD dwColor)
	{
		m_ListInfo.dwAlternateBackgroundColor = dwColor;
		Invalidate();
	}

	DWORD CListUI::GetItemColor() const
	{
		return m_ListInfo.dwColor;
	}

	DWORD CListUI::GetItemBackgroundColor() const
	{
		return m_ListInfo.dwBackgroundColor;
	}

	LPCTSTR CListUI::GetItemBkImage() const
	{
		return m_ListInfo.sBkImage.GetData();
	}

	bool CListUI::IsAlternateBk() const
	{
		return m_ListInfo.bAlternateBk;
	}

	DWORD CListUI::GetAlternateBkColor() const
	{
		return m_ListInfo.dwAlternateBackgroundColor;
	}

	void CListUI::SetSelectedItemColor(DWORD dwColor)
	{
		m_ListInfo.dwSelectedColor = dwColor;
		Invalidate();
	}

	void CListUI::SetSelectedItemBackgroundColor(DWORD dwBackgroundColor)
	{
		m_ListInfo.dwSelectedBackgroundColor = dwBackgroundColor;
		Invalidate();
	}

	void CListUI::SetSelectedItemImage(LPCTSTR pStrImage)
	{
		m_ListInfo.sSelectedImage = pStrImage;
		Invalidate();
	}

	DWORD CListUI::GetSelectedItemColor() const
	{
		return m_ListInfo.dwSelectedColor;
	}

	DWORD CListUI::GetSelectedItemBackgroundColor() const
	{
		return m_ListInfo.dwSelectedBackgroundColor;
	}

	LPCTSTR CListUI::GetSelectedItemImage() const
	{
		return m_ListInfo.sSelectedImage.GetData();
	}

	void CListUI::SetHoverItemColor(DWORD dwColor)
	{
		m_ListInfo.dwHoverColor = dwColor;
		Invalidate();
	}

	void CListUI::SetHoverItemBackgroundColor(DWORD dwBackgroundColor)
	{
		m_ListInfo.dwHoverBackgroundColor = dwBackgroundColor;
		Invalidate();
	}

	void CListUI::SetHoverItemImage(LPCTSTR pStrImage)
	{
		m_ListInfo.sHoverImage = pStrImage;
		Invalidate();
	}

	DWORD CListUI::GetHoverItemColor() const
	{
		return m_ListInfo.dwHoverColor;
	}
	DWORD CListUI::GetHoverItemBackgroundColor() const
	{
		return m_ListInfo.dwHoverBackgroundColor;
	}

	LPCTSTR CListUI::GetHoverItemImage() const
	{
		return m_ListInfo.sHoverImage.GetData();
	}

	void CListUI::SetDisabledItemColor(DWORD dwColor)
	{
		m_ListInfo.dwDisabledColor = dwColor;
		Invalidate();
	}

	void CListUI::SetDisabledItemBackgroundColor(DWORD dwBackgroundColor)
	{
		m_ListInfo.dwDisabledBackgroundColor = dwBackgroundColor;
		Invalidate();
	}

	void CListUI::SetDisabledItemImage(LPCTSTR pStrImage)
	{
		m_ListInfo.sDisabledImage = pStrImage;
		Invalidate();
	}

	DWORD CListUI::GetDisabledItemColor() const
	{
		return m_ListInfo.dwDisabledColor;
	}

	DWORD CListUI::GetDisabledItemBackgroundColor() const
	{
		return m_ListInfo.dwDisabledBackgroundColor;
	}

	LPCTSTR CListUI::GetDisabledItemImage() const
	{
		return m_ListInfo.sDisabledImage.GetData();
	}

	DWORD CListUI::GetItemLineColor() const
	{
		return m_ListInfo.dwLineColor;
	}

	void CListUI::SetItemLineColor(DWORD dwLineColor)
	{
		m_ListInfo.dwLineColor = dwLineColor;
		Invalidate();
	}
	void CListUI::SetItemShowRowLine(bool bShowLine)
	{
		m_ListInfo.bShowRowLine = bShowLine;
		Invalidate();
	}
	void CListUI::SetItemShowColumnLine(bool bShowLine)
	{
		m_ListInfo.bShowColumnLine = bShowLine;
		Invalidate();
	}
	void CListUI::SetHeaderShowColumnLine(bool bShowLine)
	{
		m_ListInfo.bShowHeaderColumnLine = bShowLine;
		if( m_pHeader != NULL ) m_pHeader->Invalidate();
		Invalidate();
	}
	bool CListUI::IsHeaderShowColumnLine() const
	{
		return m_ListInfo.bShowHeaderColumnLine;
	}
	bool CListUI::IsItemShowHtml()
	{
		return m_ListInfo.bShowHtml;
	}

	void CListUI::SetItemShowHtml(bool bShowHtml)
	{
		if (m_ListInfo.bShowHtml == bShowHtml) return;

		m_ListInfo.bShowHtml = bShowHtml;
		NeedUpdate();
	}

	bool CListUI::IsItemRSelected()
	{
		return m_ListInfo.bRSelected;
	}

	void CListUI::SetItemRSelected(bool bSelected)
	{
		if (m_ListInfo.bRSelected == bSelected) return;

		m_ListInfo.bRSelected = bSelected;
		NeedUpdate();
	}

	void CListUI::SetMultiExpanding(bool bMultiExpandable)
	{
		m_ListInfo.bMultiExpandable = bMultiExpandable;
	}

	bool CListUI::ExpandItem(int iIndex, bool bExpand /*= true*/)
	{
		if (m_iExpandedItem >= 0 && !m_ListInfo.bMultiExpandable) {
			CControlUI* pControl = GetItemAt(m_iExpandedItem);
			if (pControl != NULL) {
				IListItemUI* pItem = static_cast<IListItemUI*>(pControl->GetInterface(_T("ListItem")));
				if (pItem != NULL) pItem->Expand(false);
			}
			m_iExpandedItem = -1;
		}
		if (bExpand) {
			CControlUI* pControl = GetItemAt(iIndex);
			if (pControl == NULL) return false;
			if (!pControl->IsVisible()) return false;
			IListItemUI* pItem = static_cast<IListItemUI*>(pControl->GetInterface(_T("ListItem")));
			if (pItem == NULL) return false;
			m_iExpandedItem = iIndex;
			if (!pItem->Expand(true)) {
				m_iExpandedItem = -1;
				return false;
			}
		}
		NeedUpdate();
		return true;
	}

	int CListUI::GetExpandedItem() const
	{
		return m_iExpandedItem;
	}

	void CListUI::EnsureVisible(int iIndex)
	{
		if (m_iCurSel < 0) return;
		RECT rcItem = m_pList->GetItemAt(iIndex)->GetPos();
		RECT rcList = m_pList->GetPos();
		RECT rcListPadding = m_pList->GetPadding();

		rcList.left += rcListPadding.left;
		rcList.top += rcListPadding.top;
		rcList.right -= rcListPadding.right;
		rcList.bottom -= rcListPadding.bottom;

		CScrollBarUI* pHorizontalScrollBar = m_pList->GetHorizontalScrollBar();
		if (pHorizontalScrollBar && pHorizontalScrollBar->IsVisible()) rcList.bottom -= pHorizontalScrollBar->GetFixedHeight();

		if (rcItem.top >= rcList.top && rcItem.bottom < rcList.bottom) return;
		int dx = 0;
		if (rcItem.top < rcList.top) dx = rcItem.top - rcList.top;
		if (rcItem.bottom > rcList.bottom) dx = rcItem.bottom - rcList.bottom;
		Scroll(0, dx);
	}

	void CListUI::Scroll(int dx, int dy)
	{
		if (dx == 0 && dy == 0) return;
		SIZE sz = m_pList->GetScrollPos();
		m_pList->SetScrollPos(CDuiSize(sz.cx + dx, sz.cy + dy));
	}

	void CListUI::SetAttribute(LPCTSTR pstrName, LPCTSTR pstrValue)
	{
		if (_tcsicmp(pstrName, _T("header")) == 0) GetHeader()->SetVisible(_tcsicmp(pstrValue, _T("hidden")) != 0);
		else if (_tcsicmp(pstrName, _T("header-background-image")) == 0) GetHeader()->SetBackgroundImage(pstrValue);
		else if (_tcsicmp(pstrName, _T("scroll-select")) == 0) SetScrollSelect(_tcsicmp(pstrValue, _T("true")) == 0);
		else if (_tcsicmp(pstrName, _T("multi-expanding")) == 0) SetMultiExpanding(_tcsicmp(pstrValue, _T("true")) == 0);
		else if (_tcsicmp(pstrName, _T("item-font-family")) == 0 || _tcsicmp(pstrName, _T("item-font-size")) == 0 || _tcsicmp(pstrName, _T("item-font-weight")) == 0) {
			if (m_pManager != NULL) {
				CDuiString sFamily;
				int nSize = 0;
				bool bBold = false;
				TFontInfo* pInfo = m_pManager->GetFontInfo(m_ListInfo.nFont);
				if (pInfo == NULL) pInfo = m_pManager->GetDefaultFontInfo();
				if (pInfo != NULL) {
					sFamily = pInfo->sFontName;
					nSize = pInfo->iSize;
					bBold = pInfo->bBold;
				}
				if (_tcsicmp(pstrName, _T("item-font-family")) == 0) sFamily = pstrValue ? pstrValue : _T("");
				else if (_tcsicmp(pstrName, _T("item-font-size")) == 0) {
					LPTSTR pEnd = NULL;
					long v = _tcstol(pstrValue, &pEnd, 10);
					if (pEnd != pstrValue && v > 0) nSize = (int)v;
				}
				else if( !ParseCssFontWeightBold(pstrValue, bBold) ) {
					bBold = false;
				}
				if (sFamily.IsEmpty()) sFamily = _T("Microsoft YaHei UI");
				if (nSize <= 0) nSize = 12;
				int id = m_pManager->EnsureFont(sFamily.GetData(), nSize, bBold, false, false, false);
				if (id >= 0) SetItemFont(id);
			}
		}
		else if (_tcsicmp(pstrName, _T("item-text-align")) == 0) {
			if (_tcsstr(pstrValue, _T("left")) != NULL) {
				m_ListInfo.uTextStyle &= ~(DT_CENTER | DT_RIGHT);
				m_ListInfo.uTextStyle |= DT_LEFT;
			}
			if (_tcsstr(pstrValue, _T("center")) != NULL) {
				m_ListInfo.uTextStyle &= ~(DT_LEFT | DT_RIGHT);
				m_ListInfo.uTextStyle |= DT_CENTER;
			}
			if (_tcsstr(pstrValue, _T("right")) != NULL) {
				m_ListInfo.uTextStyle &= ~(DT_LEFT | DT_CENTER);
				m_ListInfo.uTextStyle |= DT_RIGHT;
			}
		}
		else if (_tcsicmp(pstrName, _T("item-vertical-align")) == 0) {
			if (_tcsstr(pstrValue, _T("top")) != NULL) {
				m_ListInfo.uTextStyle &= ~(DT_VCENTER | DT_BOTTOM);
				m_ListInfo.uTextStyle |= DT_TOP;
			}
			if (_tcsstr(pstrValue, _T("vcenter")) != NULL || _tcsstr(pstrValue, _T("middle")) != NULL) {
				m_ListInfo.uTextStyle &= ~(DT_TOP | DT_BOTTOM | DT_WORDBREAK);
				m_ListInfo.uTextStyle |= DT_VCENTER | DT_SINGLELINE;
			}
			if (_tcsstr(pstrValue, _T("bottom")) != NULL) {
				m_ListInfo.uTextStyle &= ~(DT_TOP | DT_VCENTER);
				m_ListInfo.uTextStyle |= DT_BOTTOM;
			}
		}
		else if (_tcsicmp(pstrName, _T("item-text-overflow")) == 0) {
			if (_tcsicmp(pstrValue, _T("ellipsis")) == 0) m_ListInfo.uTextStyle |= DT_END_ELLIPSIS;
			else m_ListInfo.uTextStyle &= ~DT_END_ELLIPSIS;
		}
		else if (_tcsicmp(pstrName, _T("item-padding")) == 0) {
			RECT rcTextPadding = { 0 };
			if( ParseCssBoxToRect(pstrValue, rcTextPadding) )
				SetItemTextPadding(rcTextPadding);
		}
		else if (_tcsicmp(pstrName, _T("item-color")) == 0) {
			DWORD clrColor = 0;
			if( !ParseColorString(pstrValue, clrColor) ) clrColor = 0;
			SetItemColor(clrColor);
		}
		else if (_tcsicmp(pstrName, _T("item-background-color")) == 0) {
			DWORD clrColor = 0;
			if( !ParseColorString(pstrValue, clrColor) ) clrColor = 0;
			SetItemBackgroundColor(clrColor);
		}
		else if (_tcsicmp(pstrName, _T("item-background-image")) == 0) SetItemBkImage(pstrValue);
		else if (_tcsicmp(pstrName, _T("item-alternate-background")) == 0) {
			AddCustomAttribute(_T("item-alternate-background"), pstrValue);
			SetAlternateBk(_tcsicmp(pstrValue, _T("true")) == 0);
		}
		else if (_tcsicmp(pstrName, _T("item-alternate-background-color")) == 0) {
			DWORD clrColor = 0;
			if( !ParseColorString(pstrValue, clrColor) ) clrColor = 0;
			SetAlternateBkColor(clrColor);
			if( clrColor != 0 ) SetAlternateBk(true);
		}
		else if( _tcsicmp(pstrName, _T("item-foreground-image")) == 0 ) {m_ListInfo.sForegroundImage = pstrValue; Invalidate();}
		else if( _tcsicmp(pstrName, _T("item-foreground-image-hover")) == 0 ) {m_ListInfo.sHoverForegroundImage = pstrValue; Invalidate();}
		else if( _tcsicmp(pstrName, _T("item-foreground-image-selected")) == 0 ) {m_ListInfo.sSelectedForegroundImage = pstrValue; Invalidate();}
		else if (_tcsicmp(pstrName, _T("item-color-selected")) == 0) {
			DWORD clrColor = 0;
			if( !ParseColorString(pstrValue, clrColor) ) clrColor = 0;
			SetSelectedItemColor(clrColor);
		}
		else if (_tcsicmp(pstrName, _T("item-background-color-selected")) == 0) {
			DWORD clrColor = 0;
			if( !ParseColorString(pstrValue, clrColor) ) clrColor = 0;
			SetSelectedItemBackgroundColor(clrColor);
		}
		else if (_tcsicmp(pstrName, _T("item-image-selected")) == 0) SetSelectedItemImage(pstrValue);
		else if (_tcsicmp(pstrName, _T("item-color-hover")) == 0) {
			DWORD clrColor = 0;
			if( !ParseColorString(pstrValue, clrColor) ) clrColor = 0;
			SetHoverItemColor(clrColor);
		}
		else if (_tcsicmp(pstrName, _T("item-background-color-hover")) == 0) {
			DWORD clrColor = 0;
			if( !ParseColorString(pstrValue, clrColor) ) clrColor = 0;
			SetHoverItemBackgroundColor(clrColor);
		}
		else if (_tcsicmp(pstrName, _T("item-image-hover")) == 0) SetHoverItemImage(pstrValue);
		else if (_tcsicmp(pstrName, _T("item-color-disabled")) == 0) {
			DWORD clrColor = 0;
			if( !ParseColorString(pstrValue, clrColor) ) clrColor = 0;
			SetDisabledItemColor(clrColor);
		}
		else if (_tcsicmp(pstrName, _T("item-background-color-disabled")) == 0) {
			DWORD clrColor = 0;
			if( !ParseColorString(pstrValue, clrColor) ) clrColor = 0;
			SetDisabledItemBackgroundColor(clrColor);
		}
		else if (_tcsicmp(pstrName, _T("item-image-disabled")) == 0) SetDisabledItemImage(pstrValue);
		else if (_tcsicmp(pstrName, _T("item-line-color")) == 0) {
			DWORD clrColor = 0;
			if( !ParseColorString(pstrValue, clrColor) ) clrColor = 0;
			SetItemLineColor(clrColor);
		}
		else if (_tcsicmp(pstrName, _T("item-show-row-line")) == 0) {
			AddCustomAttribute(_T("item-show-row-line"), pstrValue);
			SetItemShowRowLine(_tcsicmp(pstrValue, _T("true")) == 0);
		}
		else if (_tcsicmp(pstrName, _T("item-show-column-line")) == 0) {
			AddCustomAttribute(_T("item-show-column-line"), pstrValue);
			SetItemShowColumnLine(_tcsicmp(pstrValue, _T("true")) == 0);
		}
		else if (_tcsicmp(pstrName, _T("header-show-column-line")) == 0) {
			AddCustomAttribute(_T("header-show-column-line"), pstrValue);
			SetHeaderShowColumnLine(_tcsicmp(pstrValue, _T("true")) == 0);
		}
		else if (_tcsicmp(pstrName, _T("item-show-html")) == 0) SetItemShowHtml(_tcsicmp(pstrValue, _T("true")) == 0);
		else if (_tcsicmp(pstrName, _T("multi-select")) == 0) SetMultiSelect(_tcsicmp(pstrValue, _T("true")) == 0);
		else if (_tcsicmp(pstrName, _T("item-right-selected")) == 0) SetItemRSelected(_tcsicmp(pstrValue, _T("true")) == 0);
		else if( _tcsicmp(pstrName, _T("empty-text")) == 0
			|| _tcsicmp(pstrName, _T("empty-description")) == 0 ) {
			if( m_pEmpty == NULL ) {
				CEmptyUI* pEmpty = new CEmptyUI;
				SetEmptyControl(pEmpty);
			}
			if( m_pEmpty != NULL ) m_pEmpty->SetDescription(pstrValue);
		}
		else if( _tcsicmp(pstrName, _T("empty-image")) == 0 ) {
			if( m_pEmpty == NULL ) {
				CEmptyUI* pEmpty = new CEmptyUI;
				SetEmptyControl(pEmpty);
			}
			if( m_pEmpty != NULL ) m_pEmpty->SetImage(pstrValue);
		}

		else CVerticalLayoutUI::SetAttribute(pstrName, pstrValue);
	}

	IListCallbackUI* CListUI::GetTextCallback() const
	{
		return m_pCallback;
	}

	void CListUI::SetTextCallback(IListCallbackUI* pCallback)
	{
		m_pCallback = pCallback;
	}

	SIZE CListUI::GetScrollPos() const
	{
		return m_pList->GetScrollPos();
	}

	SIZE CListUI::GetScrollRange() const
	{
		return m_pList->GetScrollRange();
	}

	void CListUI::SetScrollPos(SIZE szPos, bool bMsg)
	{
		m_pList->SetScrollPos(szPos, bMsg);
	}

	void CListUI::LineUp()
	{
		m_pList->LineUp();
	}

	void CListUI::LineDown()
	{
		m_pList->LineDown();
	}

	void CListUI::PageUp()
	{
		m_pList->PageUp();
	}

	void CListUI::PageDown()
	{
		m_pList->PageDown();
	}

	void CListUI::HomeUp()
	{
		m_pList->HomeUp();
	}

	void CListUI::EndDown()
	{
		m_pList->EndDown();
	}

	void CListUI::LineLeft()
	{
		m_pList->LineLeft();
	}

	void CListUI::LineRight()
	{
		m_pList->LineRight();
	}

	void CListUI::PageLeft()
	{
		m_pList->PageLeft();
	}

	void CListUI::PageRight()
	{
		m_pList->PageRight();
	}

	void CListUI::HomeLeft()
	{
		m_pList->HomeLeft();
	}

	void CListUI::EndRight()
	{
		m_pList->EndRight();
	}

	void CListUI::EnableScrollBar(bool bEnableVertical, bool bEnableHorizontal)
	{
		m_pList->EnableScrollBar(bEnableVertical, bEnableHorizontal);
	}

	CScrollBarUI* CListUI::GetVerticalScrollBar() const
	{
		return m_pList->GetVerticalScrollBar();
	}

	CScrollBarUI* CListUI::GetHorizontalScrollBar() const
	{
		return m_pList->GetHorizontalScrollBar();
	}

	BOOL CListUI::SortItems(PULVCompareFunc pfnCompare, UINT_PTR dwData)
	{
		if (!m_pList)
			return FALSE;
		return m_pList->SortItems(pfnCompare, dwData);
	}

	/////////////////////////////////////////////////////////////////////////////////////
	//
	//

	CListBodyUI::CListBodyUI(CListUI* pOwner) : m_pOwner(pOwner)
	{
		ASSERT(m_pOwner);
	}

	BOOL CListBodyUI::SortItems(PULVCompareFunc pfnCompare, UINT_PTR dwData)
	{
		if (!pfnCompare)
			return FALSE;
		m_pCompareFunc = pfnCompare;
		m_compareData = dwData;
		qsort_s(m_items.GetData(), m_items.GetSize(), sizeof(CControlUI*), CListBodyUI::ItemComareFunc, this);
		IListItemUI *pItem = NULL;
		for (int i = 0; i < m_items.GetSize(); ++i)
		{
			pItem = (IListItemUI*)(static_cast<CControlUI*>(m_items[i])->GetInterface(TEXT("ListItem")));
			if (pItem)
			{
				pItem->SetIndex(i);
				pItem->Select(false);
			}
		}
		m_pOwner->SelectItem(-1);
		if (m_pManager)
		{
			SetPos(GetPos());
			Invalidate();
		}

		return TRUE;
	}

	int __cdecl CListBodyUI::ItemComareFunc(void *pvlocale, const void *item1, const void *item2)
	{
		CListBodyUI *pThis = (CListBodyUI*)pvlocale;
		if (!pThis || !item1 || !item2)
			return 0;
		return pThis->ItemComareFunc(item1, item2);
	}

	int __cdecl CListBodyUI::ItemComareFunc(const void *item1, const void *item2)
	{
		CControlUI *pControl1 = *(CControlUI**)item1;
		CControlUI *pControl2 = *(CControlUI**)item2;
		return m_pCompareFunc((UINT_PTR)pControl1, (UINT_PTR)pControl2, m_compareData);
	}

	int CListBodyUI::GetScrollStepSize() const
	{
		if (m_pOwner != NULL) return m_pOwner->GetScrollStepSize();

		return CVerticalLayoutUI::GetScrollStepSize();
	}

	void CListBodyUI::SetScrollPos(SIZE szPos, bool bMsg)
	{
		int cx = 0;
		int cy = 0;
		if (m_pVerticalScrollBar && m_pVerticalScrollBar->IsVisible()) {
			int iLastScrollPos = m_pVerticalScrollBar->GetScrollPos();
			m_pVerticalScrollBar->SetScrollPos(szPos.cy);
			cy = m_pVerticalScrollBar->GetScrollPos() - iLastScrollPos;
		}

		if (m_pHorizontalScrollBar && m_pHorizontalScrollBar->IsVisible()) {
			int iLastScrollPos = m_pHorizontalScrollBar->GetScrollPos();
			m_pHorizontalScrollBar->SetScrollPos(szPos.cx);
			cx = m_pHorizontalScrollBar->GetScrollPos() - iLastScrollPos;
		}

		RECT rcPos;
		for (int it2 = 0; it2 < m_items.GetSize(); it2++) {
			CControlUI* pControl = static_cast<CControlUI*>(m_items[it2]);
			if (!pControl->IsVisible()) continue;
			if (pControl->IsAbsolute()) continue;

			rcPos = pControl->GetPos();
			rcPos.left -= cx;
			rcPos.right -= cx;
			rcPos.top -= cy;
			rcPos.bottom -= cy;
			pControl->SetPos(rcPos, true);
		}

		Invalidate();
		if (m_pOwner) {
			CListHeaderUI* pHeader = m_pOwner->GetHeader();
			if (pHeader == NULL) return;
			TListInfoUI* pInfo = m_pOwner->GetListInfo();
			pInfo->nColumns = MIN(pHeader->GetCount(), UILIST_MAX_COLUMNS);

			if (!pHeader->IsVisible()) {
				for (int it = 0; it < pHeader->GetCount(); it++) {
					static_cast<CControlUI*>(pHeader->GetItemAt(it))->SetInternVisible(true);
				}
			}
			for (int i = 0; i < pInfo->nColumns; i++) {
				CControlUI* pControl = static_cast<CControlUI*>(pHeader->GetItemAt(i));
				if (!pControl->IsVisible()) continue;
				if (pControl->IsAbsolute()) continue;

				RECT rcPos = pControl->GetPos();
				rcPos.left -= cx;
				rcPos.right -= cx;
				pControl->SetPos(rcPos);
				pInfo->rcColumn[i] = pControl->GetPos();
			}
			if (!pHeader->IsVisible()) {
				for (int it = 0; it < pHeader->GetCount(); it++) {
					static_cast<CControlUI*>(pHeader->GetItemAt(it))->SetInternVisible(false);
				}
			}
		}
	}

	void CListBodyUI::SetPos(RECT rc, bool bNeedInvalidate)
	{
		CControlUI::SetPos(rc, bNeedInvalidate);

		// Adjust for padding
		int iGap = GetGap();
		RECT rcPadding = GetPadding();
		// Adjust for padding
		rc.left += rcPadding.left;
		rc.top += rcPadding.top;
		rc.right -= rcPadding.right;
		rc.bottom -= rcPadding.bottom;

		if (m_pOwner->IsFixedScrollbar() && m_pVerticalScrollBar) rc.right -= m_pVerticalScrollBar->GetFixedWidth();
		else if (m_pVerticalScrollBar && m_pVerticalScrollBar->IsVisible()) rc.right -= m_pVerticalScrollBar->GetFixedWidth();
		if (m_pHorizontalScrollBar && m_pHorizontalScrollBar->IsVisible()) rc.bottom -= m_pHorizontalScrollBar->GetFixedHeight();

		// Determine the minimum size
		SIZE szAvailable = { rc.right - rc.left, rc.bottom - rc.top };
		if (m_pHorizontalScrollBar && m_pHorizontalScrollBar->IsVisible())
			szAvailable.cx += m_pHorizontalScrollBar->GetScrollRange();

		int cxNeeded = 0;
		int nAdjustables = 0;
		int cyFixed = 0;
		int nEstimateNum = 0;
		for (int it1 = 0; it1 < m_items.GetSize(); it1++) {
			CControlUI* pControl = static_cast<CControlUI*>(m_items[it1]);
			if (!pControl->IsVisible()) continue;
			if (pControl->IsAbsolute()) continue;
			SIZE sz = pControl->EstimateSize(szAvailable);
			if (sz.cy == 0) {
				nAdjustables++;
			}
			else {
				if (sz.cy < pControl->GetMinHeight()) sz.cy = pControl->GetMinHeight();
				if (sz.cy > pControl->GetMaxHeight()) sz.cy = pControl->GetMaxHeight();
			}
			cyFixed += sz.cy + pControl->GetMargin().top + pControl->GetMargin().bottom;

			sz.cx = MAX(sz.cx, 0);
			if (sz.cx < pControl->GetMinWidth()) sz.cx = pControl->GetMinWidth();
			if (sz.cx > pControl->GetMaxWidth()) sz.cx = pControl->GetMaxWidth();
			cxNeeded = MAX(cxNeeded, sz.cx);
			nEstimateNum++;
		}
		cyFixed += (nEstimateNum - 1) * iGap;

		if (m_pOwner) {
			CListHeaderUI* pHeader = m_pOwner->GetHeader();
			if (pHeader != NULL && pHeader->GetCount() > 0) {
				cxNeeded = MAX(0, pHeader->EstimateSize(CDuiSize(rc.right - rc.left, rc.bottom - rc.top)).cx);
				if (m_pHorizontalScrollBar && m_pHorizontalScrollBar->IsVisible())
				{
					int nOffset = m_pHorizontalScrollBar->GetScrollPos();
					RECT rcHeader = pHeader->GetPos();
					rcHeader.left = rc.left - nOffset;
					pHeader->SetPos(rcHeader);
				}
			}
		}

		// Place elements
		int cyNeeded = 0;
		int cyExpand = 0;
		if (nAdjustables > 0) cyExpand = MAX(0, (szAvailable.cy - cyFixed) / nAdjustables);
		// Position the elements
		SIZE szRemaining = szAvailable;
		int iPosY = rc.top;
		if (m_pVerticalScrollBar && m_pVerticalScrollBar->IsVisible()) {
			iPosY -= m_pVerticalScrollBar->GetScrollPos();
		}
		int iPosX = rc.left;
		if (m_pHorizontalScrollBar && m_pHorizontalScrollBar->IsVisible()) {
			iPosX -= m_pHorizontalScrollBar->GetScrollPos();
		}
		int iAdjustable = 0;
		int cyFixedRemaining = cyFixed;
		for (int it2 = 0; it2 < m_items.GetSize(); it2++) {
			CControlUI* pControl = static_cast<CControlUI*>(m_items[it2]);
			if (!pControl->IsVisible()) continue;
			if (pControl->IsAbsolute()) {
				SetAbsolutePos(it2);
				continue;
			}

			RECT rcMargin = pControl->GetMargin();
			szRemaining.cy -= rcMargin.top;
			SIZE sz = pControl->EstimateSize(szRemaining);
			if (sz.cy == 0) {
				iAdjustable++;
				sz.cy = cyExpand;
				// Distribute remaining to last element (usually round-off left-overs)
				if (iAdjustable == nAdjustables) {
					sz.cy = MAX(0, szRemaining.cy - rcMargin.bottom - cyFixedRemaining);
				}
				if (sz.cy < pControl->GetMinHeight()) sz.cy = pControl->GetMinHeight();
				if (sz.cy > pControl->GetMaxHeight()) sz.cy = pControl->GetMaxHeight();
			}
			else {
				if (sz.cy < pControl->GetMinHeight()) sz.cy = pControl->GetMinHeight();
				if (sz.cy > pControl->GetMaxHeight()) sz.cy = pControl->GetMaxHeight();
				cyFixedRemaining -= sz.cy;
			}

			sz.cx = MAX(cxNeeded, szAvailable.cx - rcMargin.left - rcMargin.right);

			if (sz.cx < pControl->GetMinWidth()) sz.cx = pControl->GetMinWidth();
			if (sz.cx > pControl->GetMaxWidth()) sz.cx = pControl->GetMaxWidth();

			RECT rcCtrl = { iPosX + rcMargin.left, iPosY + rcMargin.top, iPosX + rcMargin.left + sz.cx, iPosY + sz.cy + rcMargin.top + rcMargin.bottom };
			pControl->SetPos(rcCtrl);

			iPosY += sz.cy + iGap + rcMargin.top + rcMargin.bottom;
			cyNeeded += sz.cy + rcMargin.top + rcMargin.bottom;
			szRemaining.cy -= sz.cy + iGap + rcMargin.bottom;
		}
		cyNeeded += (nEstimateNum - 1) * iGap;

		if (m_pHorizontalScrollBar != NULL) {
			if (cxNeeded > rc.right - rc.left) {
				if (m_pHorizontalScrollBar->IsVisible()) {
					m_pHorizontalScrollBar->SetScrollRange(cxNeeded - (rc.right - rc.left));
				}
				else {
					m_pHorizontalScrollBar->SetVisible(true);
					m_pHorizontalScrollBar->SetScrollRange(cxNeeded - (rc.right - rc.left));
					m_pHorizontalScrollBar->SetScrollPos(0);
					rc.bottom -= m_pHorizontalScrollBar->GetFixedHeight();
				}
			}
			else {
				if (m_pHorizontalScrollBar->IsVisible()) {
					m_pHorizontalScrollBar->SetVisible(false);
					m_pHorizontalScrollBar->SetScrollRange(0);
					m_pHorizontalScrollBar->SetScrollPos(0);
					rc.bottom += m_pHorizontalScrollBar->GetFixedHeight();
				}
			}
		}
		UINT uListType = m_pOwner->GetListType();
		if (uListType == LT_LIST) {
			// 计算横向尺寸
			int nItemCount = m_items.GetSize();
			if (nItemCount > 0)
			{
				CControlUI* pControl = static_cast<CControlUI*>(m_items[0]);
				int nFixedWidth = pControl->GetFixedWidth();
				if (nFixedWidth > 0)
				{
					int nRank = (rc.right - rc.left) / nFixedWidth;
					if (nRank > 0)
					{
						cyNeeded = ((nItemCount - 1) / nRank + 1) * pControl->GetFixedHeight();
					}
				}
			}
		}
		// Process the scrollbar
		ProcessScrollBar(rc, cxNeeded, cyNeeded);
	}

	void CListBodyUI::DoEvent(TEventUI& event)
	{
		if (!IsMouseEnabled() && event.Type > UIEVENT__MOUSEBEGIN && event.Type < UIEVENT__MOUSEEND) {
			if (m_pOwner != NULL) m_pOwner->DoEvent(event);
			else CVerticalLayoutUI::DoEvent(event);
			return;
		}

		CVerticalLayoutUI::DoEvent(event);
	}

	/////////////////////////////////////////////////////////////////////////////////////
	//
	//
	IMPLEMENT_DUICONTROL(CListHeaderUI)

	CListHeaderUI::CListHeaderUI() :
	m_bIsScaleHeader(false)
	{
		// 默认逻辑高度，避免 EstimateSize 为 0 被当成可伸缩后压成一行半高
		SetMinHeight(40);
		SetFixedHeight(40);
		// 与内容区默认分隔线（主题会改色）
		SetBottomBorderWidth(1);
		SetBorderColor(0xDEE2E6FF);
	}

	CListHeaderUI::~CListHeaderUI()
	{
		if( m_pManager != NULL )
			m_pManager->RemovePostPaint(this);
	}

	LPCTSTR CListHeaderUI::GetClass() const
	{
		return _T("ListHeaderUI");
	}

	LPVOID CListHeaderUI::GetInterface(LPCTSTR pstrName)
	{
		if (_tcsicmp(pstrName, DUI_CTR_LISTHEADER) == 0) return this;
		return CHorizontalLayoutUI::GetInterface(pstrName);
	}

	void CListHeaderUI::SetManager(CPaintManagerUI* pManager, CControlUI* pParent, bool bInit)
	{
		if( m_pManager != NULL )
			m_pManager->RemovePostPaint(this);
		CHorizontalLayoutUI::SetManager(pManager, pParent, bInit);
		// PostPaint 仅在 List 空态时由 CListUI 按需挂接，避免常驻同帧重绘
	}

	SIZE CListHeaderUI::EstimateSize(SIZE szAvailable)
	{
		// 须用 GetFixedHeight()（含 DPI Scale）；直接读 m_cxyFixed.cy 会在高 DPI 下偏矮
		SIZE cXY = { 0, GetFixedHeight() };
		if (cXY.cy == 0 && m_pManager != NULL) {
			for (int it = 0; it < m_items.GetSize(); it++) {
				cXY.cy = MAX(cXY.cy, static_cast<CControlUI*>(m_items[it])->EstimateSize(szAvailable).cy);
			}
			int nPad = m_pManager->GetDPIObj()->Scale(16);
			int nMin = m_pManager->GetDefaultFontInfo()->tm.tmHeight + nPad;
			cXY.cy = MAX(cXY.cy, nMin);
		}
		int nMinH = GetMinHeight();
		if( cXY.cy < nMinH ) cXY.cy = nMinH;

		for (int it = 0; it < m_items.GetSize(); it++) {
			cXY.cx += static_cast<CControlUI*>(m_items[it])->EstimateSize(szAvailable).cx;
		}
		if (cXY.cx < szAvailable.cx) cXY.cx = szAvailable.cx;
		return cXY;
	}

	void CListHeaderUI::SetPos(RECT rc, bool bNeedInvalidate)
	{
		CControlUI::SetPos(rc, bNeedInvalidate);
		rc = m_rcItem;

		RECT rcPadding = GetPadding();
		// Adjust for padding
		rc.left += rcPadding.left;
		rc.top += rcPadding.top;
		rc.right -= rcPadding.right;
		rc.bottom -= rcPadding.bottom;
		// 列仍铺满表头高度，保证文字垂直居中；底部分隔线由 PaintSeparator 叠在子项之上

		if (m_items.GetSize() == 0) {
			return;
		}

		int iGap = GetGap();
		// 可用宽先扣纵向滚动条，保证 width=% 与均分剩余同一基准
		SIZE szAvailable = { rc.right - rc.left, rc.bottom - rc.top };
		int nHeaderWidth = szAvailable.cx;
		CListUI *pList = static_cast<CListUI*>(GetParent());
		if (pList != NULL) {
			CScrollBarUI* pVScroll = pList->GetVerticalScrollBar();
			if (pVScroll != NULL) {
				nHeaderWidth -= pVScroll->GetWidth();
				if (nHeaderWidth < 0) nHeaderWidth = 0;
				szAvailable.cx = nHeaderWidth;
			}
		}

		int nAdjustables = 0;
		int cxFixed = 0;
		int nEstimateNum = 0;
		for (int it1 = 0; it1 < m_items.GetSize(); it1++) {
			CControlUI* pControl = static_cast<CControlUI*>(m_items[it1]);
			if (!pControl->IsVisible()) continue;
			if (pControl->IsAbsolute()) continue;
			SIZE sz = { 0, 0 };
			if (m_bIsScaleHeader) {
				CListHeaderItemUI* pHeaderItem = static_cast<CListHeaderItemUI*>(
					pControl->GetInterface(DUI_CTR_LISTHEADERITEM));
				if (pHeaderItem != NULL)
					sz.cx = int(nHeaderWidth * (float)pHeaderItem->GetScale() / 100);
			}
			else {
				sz = pControl->EstimateSize(szAvailable);
			}
			if (sz.cx == 0) {
				nAdjustables++;
			}
			else {
				if (sz.cx < pControl->GetMinWidth()) sz.cx = pControl->GetMinWidth();
				if (sz.cx > pControl->GetMaxWidth()) sz.cx = pControl->GetMaxWidth();
			}
			cxFixed += sz.cx + pControl->GetMargin().left + pControl->GetMargin().right;
			nEstimateNum++;
		}
		cxFixed += (nEstimateNum - 1) * iGap;

		int cxExpand = 0;
		if (nAdjustables > 0) cxExpand = MAX(0, (szAvailable.cx - cxFixed) / nAdjustables);

		// Position the elements（% / 固定宽相对整表头可用宽，勿用 szRemaining）
		SIZE szRemaining = szAvailable;
		int iPosX = rc.left;
		int iAdjustable = 0;
		int cxFixedRemaining = cxFixed;

		for (int it2 = 0; it2 < m_items.GetSize(); it2++) {
			CControlUI* pControl = static_cast<CControlUI*>(m_items[it2]);
			if (!pControl->IsVisible()) continue;
			if (pControl->IsAbsolute()) {
				SetAbsolutePos(it2);
				continue;
			}
			RECT rcMargin = pControl->GetMargin();
			szRemaining.cx -= rcMargin.left;

			SIZE sz = { 0,0 };
			if (m_bIsScaleHeader) {
				CListHeaderItemUI* pHeaderItem = static_cast<CListHeaderItemUI*>(
					pControl->GetInterface(DUI_CTR_LISTHEADERITEM));
				if (pHeaderItem != NULL)
					sz.cx = int(nHeaderWidth * (float)pHeaderItem->GetScale() / 100);
			}
			else {
				sz = pControl->EstimateSize(szAvailable);
			}

			if (sz.cx == 0) {
				iAdjustable++;
				sz.cx = cxExpand;
				// Distribute remaining to last element (usually round-off left-overs)
				if (iAdjustable == nAdjustables) {
					sz.cx = MAX(0, szRemaining.cx - rcMargin.right - cxFixedRemaining);
				}
				if (sz.cx < pControl->GetMinWidth()) sz.cx = pControl->GetMinWidth();
				if (sz.cx > pControl->GetMaxWidth()) sz.cx = pControl->GetMaxWidth();
			}
			else {
				if (sz.cx < pControl->GetMinWidth()) sz.cx = pControl->GetMinWidth();
				if (sz.cx > pControl->GetMaxWidth()) sz.cx = pControl->GetMaxWidth();

				cxFixedRemaining -= sz.cx;
			}

			sz.cy = rc.bottom - rc.top - rcMargin.top - rcMargin.bottom;
			if (sz.cy < 0) sz.cy = 0;
			if (sz.cy < pControl->GetMinHeight()) sz.cy = pControl->GetMinHeight();
			if (sz.cy > pControl->GetMaxHeight()) sz.cy = pControl->GetMaxHeight();

			RECT rcCtrl = { iPosX + rcMargin.left, rc.top + rcMargin.top, iPosX + sz.cx + rcMargin.left + rcMargin.right, rc.top + rcMargin.top + sz.cy };
			pControl->SetPos(rcCtrl);
			iPosX += sz.cx + iGap + rcMargin.left + rcMargin.right;
			szRemaining.cx -= sz.cx + iGap + rcMargin.right;
		}
	}

	void CListHeaderUI::SetAttribute(LPCTSTR pstrName, LPCTSTR pstrValue)
	{
		if (_tcsicmp(pstrName, _T("scale-header")) == 0) SetScaleHeader(_tcsicmp(pstrValue, _T("true")) == 0);
		else CHorizontalLayoutUI::SetAttribute(pstrName, pstrValue);
	}

	void CListHeaderUI::SetScaleHeader(bool bIsScale)
	{
		m_bIsScaleHeader = bIsScale;
	}

	bool CListHeaderUI::IsScaleHeader() const
	{
		return m_bIsScaleHeader;
	}

	void CListHeaderUI::DoInit()
	{

	}

	void CListHeaderUI::PaintBorder(IRenderContext& ctx)
	{
		// 底边留给 PaintSeparator（子项之后），避免与 PaintBorder 叠成 2px
		const DWORD dwBorder = GetPaintBorderColor();
		if( dwBorder == 0 ) return;
		RECT rcBw = GetBorderRectWidth();
		RECT rcBorder;
		if( rcBw.left > 0 ) {
			rcBorder = m_rcItem;
			rcBorder.right = rcBorder.left;
			ctx.DrawLine(rcBorder, rcBw.left, GetAdjustColor(dwBorder), m_nBorderStyle);
		}
		if( rcBw.top > 0 ) {
			rcBorder = m_rcItem;
			rcBorder.bottom = rcBorder.top;
			ctx.DrawLine(rcBorder, rcBw.top, GetAdjustColor(dwBorder), m_nBorderStyle);
		}
		if( rcBw.right > 0 ) {
			rcBorder = m_rcItem;
			rcBorder.right -= 1;
			rcBorder.left = rcBorder.right;
			ctx.DrawLine(rcBorder, rcBw.right, GetAdjustColor(dwBorder), m_nBorderStyle);
		}
	}

	void CListHeaderUI::PaintSeparator(IRenderContext& ctx)
	{
		RECT rcBw = GetBorderRectWidth();
		if( rcBw.bottom <= 0 ) return;
		DWORD clr = GetPaintBorderColor();
		// 优先跟 List 行线同色，视觉权重一致
		CListUI* pList = static_cast<CListUI*>(GetParent());
		if( pList != NULL ) {
			TListInfoUI* pInfo = pList->GetListInfo();
			if( pInfo != NULL && pInfo->dwLineColor != 0 )
				clr = pInfo->dwLineColor;
		}
		if( clr == 0 ) clr = 0xDEE2E6FF;
		RECT rcLine = m_rcItem;
		rcLine.bottom -= 1;
		rcLine.top = rcLine.bottom;
		ctx.DrawLine(rcLine, 1, GetAdjustColor(clr), PS_SOLID);
	}

	bool CListHeaderUI::DoPaint(IRenderContext& ctx, const RECT& rcPaint, CControlUI* pStopControl)
	{
		bool b = CHorizontalLayoutUI::DoPaint(ctx, rcPaint, pStopControl);
		PaintSeparator(ctx);
		return b;
	}

	void CListHeaderUI::DoPostPaint(IRenderContext& ctx, const RECT& rcPaint)
	{
		if( !IsVisible() ) return;
		// 仅空态挂接：Empty 盖住表头后整段重绘（含分隔线）
		Paint(ctx, rcPaint, NULL);
	}

	/////////////////////////////////////////////////////////////////////////////////////
	//
	//
	IMPLEMENT_DUICONTROL(CListHeaderItemUI)

		CListHeaderItemUI::CListHeaderItemUI() : m_bDragable(true), m_uButtonState(0), m_iSepWidth(4),
		m_dwColor(0), m_iFont(-1), m_uTextStyle(DT_VCENTER | DT_CENTER | DT_SINGLELINE), m_bShowHtml(false), m_nScale(0)
	{
		// CDuiBox(top,right,bottom,left)：只要左右留白，垂直交给 PaintText 整格居中
		SetPadding(CDuiBox(0, 8, 0, 8));
		::ZeroMemory(&m_rcTextPadding, sizeof(m_rcTextPadding));
		ptLastMouse.x = ptLastMouse.y = 0;
		SetMinWidth(16);
	}

	LPCTSTR CListHeaderItemUI::GetClass() const
	{
		return _T("ListHeaderItemUI");
	}

	LPVOID CListHeaderItemUI::GetInterface(LPCTSTR pstrName)
	{
		if (_tcsicmp(pstrName, DUI_CTR_LISTHEADERITEM) == 0) return this;
		return CContainerUI::GetInterface(pstrName);
	}

	UINT CListHeaderItemUI::GetControlFlags() const
	{
		if (IsEnabled() && IsColumnResizeEnabled()) return UIFLAG_SETCURSOR;
		else return 0;
	}

	bool CListHeaderItemUI::IsColumnResizeEnabled() const
	{
		if( !m_bDragable || m_iSepWidth == 0 ) return false;
		// 无表头列线时不提供拖拽改宽（无视觉分隔却能拖不合理）
		CControlUI* pHdr = GetParent();
		if( pHdr == NULL || pHdr->GetParent() == NULL ) return true;
		CListUI* pList = static_cast<CListUI*>(pHdr->GetParent()->GetInterface(DUI_CTR_LIST));
		if( pList == NULL ) return true;
		return pList->IsHeaderShowColumnLine();
	}

	void CListHeaderItemUI::SetEnabled(bool bEnable)
	{
		CContainerUI::SetEnabled(bEnable);
		if (!IsEnabled()) {
			m_uButtonState = 0;
		}
	}

	bool CListHeaderItemUI::IsDragable() const
	{
		return m_bDragable;
	}

	void CListHeaderItemUI::SetDragable(bool bDragable)
	{
		m_bDragable = bDragable;
		if (!m_bDragable) m_uButtonState &= ~UISTATE_CAPTURED;
	}

	DWORD CListHeaderItemUI::GetSepWidth() const
	{
		if(m_pManager != NULL) return m_pManager->GetDPIObj()->Scale(m_iSepWidth);
		return m_iSepWidth;
	}

	void CListHeaderItemUI::SetSepWidth(int iWidth)
	{
		m_iSepWidth = iWidth;
	}

	DWORD CListHeaderItemUI::GetTextStyle() const
	{
		return m_uTextStyle;
	}

	void CListHeaderItemUI::SetTextStyle(UINT uStyle)
	{
		m_uTextStyle = uStyle;
		Invalidate();
	}

	DWORD CListHeaderItemUI::GetColor() const
	{
		return m_dwColor;
	}


	void CListHeaderItemUI::SetColor(DWORD dwColor)
	{
		m_dwColor = dwColor;
	}

	RECT CListHeaderItemUI::GetTextPadding() const
	{
		RECT rcTextPadding = m_rcTextPadding;
		if(m_pManager != NULL) m_pManager->GetDPIObj()->Scale(&rcTextPadding);
		return rcTextPadding;
	}

	void CListHeaderItemUI::SetTextPadding(RECT rc)
	{
		m_rcTextPadding = rc;
		Invalidate();
	}

	void CListHeaderItemUI::SetFont(int index)
	{
		m_iFont = index;
	}

	bool CListHeaderItemUI::IsShowHtml()
	{
		return m_bShowHtml;
	}

	void CListHeaderItemUI::SetShowHtml(bool bShowHtml)
	{
		if (m_bShowHtml == bShowHtml) return;

		m_bShowHtml = bShowHtml;
		Invalidate();
	}

	LPCTSTR CListHeaderItemUI::GetImage() const
	{
		return m_sImage.GetData();
	}

	void CListHeaderItemUI::SetImage(LPCTSTR pStrImage)
	{
		m_sImage = pStrImage;
		Invalidate();
	}

	LPCTSTR CListHeaderItemUI::GetHoverImage() const
	{
		return m_sHoverImage.GetData();
	}

	void CListHeaderItemUI::SetHoverImage(LPCTSTR pStrImage)
	{
		m_sHoverImage = pStrImage;
		Invalidate();
	}

	LPCTSTR CListHeaderItemUI::GetActiveImage() const
	{
		return m_sActiveImage.GetData();
	}

	void CListHeaderItemUI::SetActiveImage(LPCTSTR pStrImage)
	{
		m_sActiveImage = pStrImage;
		Invalidate();
	}

	LPCTSTR CListHeaderItemUI::GetFocusImage() const
	{
		return m_sFocusImage.GetData();
	}

	void CListHeaderItemUI::SetFocusImage(LPCTSTR pStrImage)
	{
		m_sFocusImage = pStrImage;
		Invalidate();
	}

	LPCTSTR CListHeaderItemUI::GetSepImage() const
	{
		return m_sSepImage.GetData();
	}

	void CListHeaderItemUI::SetSepImage(LPCTSTR pStrImage)
	{
		m_sSepImage = pStrImage;
		Invalidate();
	}

	void CListHeaderItemUI::SetScale(int nScale)
	{
		m_nScale = nScale;
	}

	int CListHeaderItemUI::GetScale() const
	{
		return m_nScale;
	}

	void CListHeaderItemUI::SetAttribute(LPCTSTR pstrName, LPCTSTR pstrValue)
	{
		if (_tcsicmp(pstrName, _T("draggable")) == 0) SetDragable(_tcsicmp(pstrValue, _T("true")) == 0);
		else if (_tcsicmp(pstrName, _T("sep-width")) == 0) SetSepWidth(_ttoi(pstrValue));
		else if (_tcsicmp(pstrName, _T("text-align")) == 0) {
			if (_tcsstr(pstrValue, _T("left")) != NULL) {
				m_uTextStyle &= ~(DT_CENTER | DT_RIGHT);
				m_uTextStyle |= DT_LEFT;
			}
			if (_tcsstr(pstrValue, _T("center")) != NULL) {
				m_uTextStyle &= ~(DT_LEFT | DT_RIGHT);
				m_uTextStyle |= DT_CENTER;
			}
			if (_tcsstr(pstrValue, _T("right")) != NULL) {
				m_uTextStyle &= ~(DT_LEFT | DT_CENTER);
				m_uTextStyle |= DT_RIGHT;
			}
			Invalidate();
		}
		else if (_tcsicmp(pstrName, _T("vertical-align")) == 0) {
			if (_tcsstr(pstrValue, _T("top")) != NULL) {
				m_uTextStyle &= ~(DT_VCENTER | DT_BOTTOM);
				m_uTextStyle |= DT_TOP;
			}
			else if (_tcsstr(pstrValue, _T("bottom")) != NULL) {
				m_uTextStyle &= ~(DT_TOP | DT_VCENTER);
				m_uTextStyle |= DT_BOTTOM;
			}
			else {
				// middle / center
				m_uTextStyle &= ~(DT_TOP | DT_BOTTOM | DT_WORDBREAK);
				m_uTextStyle |= (DT_VCENTER | DT_SINGLELINE);
			}
			Invalidate();
		}
		else if (_tcsicmp(pstrName, _T("text-overflow")) == 0) {
			if (_tcsicmp(pstrValue, _T("ellipsis")) == 0) m_uTextStyle |= DT_END_ELLIPSIS;
			else m_uTextStyle &= ~DT_END_ELLIPSIS;
		}
		else if (_tcsicmp(pstrName, _T("font-family")) == 0 || _tcsicmp(pstrName, _T("font-size")) == 0) {
			CDuiString sFamily;
			int nSize = 0;
			if (_tcsicmp(pstrName, _T("font-family")) == 0) sFamily = pstrValue ? pstrValue : _T("");
			else {
				LPTSTR pEnd = NULL;
				long v = _tcstol(pstrValue, &pEnd, 10);
				if (pEnd != pstrValue && v > 0) nSize = (int)v;
			}
			if (m_pManager != NULL) {
				TFontInfo* pInfo = m_pManager->GetFontInfo(m_iFont);
				if (pInfo == NULL) pInfo = m_pManager->GetDefaultFontInfo();
				if (pInfo != NULL) {
					if (sFamily.IsEmpty()) sFamily = pInfo->sFontName;
					if (nSize <= 0) nSize = pInfo->iSize;
				}
				if (sFamily.IsEmpty()) sFamily = _T("Microsoft YaHei UI");
				if (nSize <= 0) nSize = 12;
				int id = m_pManager->EnsureFont(sFamily.GetData(), nSize, false, false, false, false);
				if (id >= 0) SetFont(id);
			}
		}
		else if (_tcsicmp(pstrName, _T("color")) == 0) {
			DWORD clrColor = 0;
			if( !ParseColorString(pstrValue, clrColor) ) clrColor = 0;
			SetColor(clrColor);
		}
		else if (_tcsicmp(pstrName, _T("showhtml")) == 0) SetShowHtml(_tcsicmp(pstrValue, _T("true")) == 0);
		else if (_tcsicmp(pstrName, _T("image")) == 0) SetImage(pstrValue);
		else if (_tcsicmp(pstrName, _T("image-hover")) == 0) SetHoverImage(pstrValue);
		else if (_tcsicmp(pstrName, _T("image-active")) == 0) SetActiveImage(pstrValue);
		else if (_tcsicmp(pstrName, _T("image-focus")) == 0) SetFocusImage(pstrValue);
		else if (_tcsicmp(pstrName, _T("sep-image")) == 0) SetSepImage(pstrValue);
		else if (_tcsicmp(pstrName, _T("scale")) == 0) {
			LPTSTR pstr = NULL;
			SetScale(_tcstol(pstrValue, &pstr, 10));

		}
		else CHorizontalLayoutUI::SetAttribute(pstrName, pstrValue);
	}

	void CListHeaderItemUI::DoEvent(TEventUI& event)
	{
		if (!IsMouseEnabled() && event.Type > UIEVENT__MOUSEBEGIN && event.Type < UIEVENT__MOUSEEND) {
			if( !BubbleEvent(event) ) CContainerUI::DoEvent(event);
			return;
		}

		if (event.Type == UIEVENT_SETFOCUS)
		{
			Invalidate();
		}
		if (event.Type == UIEVENT_KILLFOCUS)
		{
			Invalidate();
		}
		if (event.Type == UIEVENT_BUTTONDOWN || event.Type == UIEVENT_DBLCLICK)
		{
			if (!IsEnabled()) return;
			RECT rcSeparator = GetThumbRect();
			if (m_iSepWidth >= 0)
				rcSeparator.left -= 4;
			else
				rcSeparator.right += 4;
			if (::PtInRect(&rcSeparator, event.ptMouse)) {
				if (IsColumnResizeEnabled()) {
					m_uButtonState |= UISTATE_CAPTURED;
					ptLastMouse = event.ptMouse;
				}
			}
			else {
				m_uButtonState |= UISTATE_PUSHED;
				m_pManager->SendNotify(this, DUI_MSGTYPE_HEADERCLICK);
				Invalidate();
			}
			return;
		}
		if (event.Type == UIEVENT_BUTTONUP)
		{
			if ((m_uButtonState & UISTATE_CAPTURED) != 0) {
				m_uButtonState &= ~UISTATE_CAPTURED;
				if (GetParent())
					GetParent()->NeedParentUpdate();
			}
			else if ((m_uButtonState & UISTATE_PUSHED) != 0) {
				m_uButtonState &= ~UISTATE_PUSHED;
				Invalidate();
			}
			return;
		}
		if (event.Type == UIEVENT_MOUSEMOVE)
		{
			if ((m_uButtonState & UISTATE_CAPTURED) != 0) {
				RECT rcMargin = GetMargin();
				RECT rc = m_rcItem;
				if (m_iSepWidth >= 0) {
					rc.right -= ptLastMouse.x - event.ptMouse.x;
				}
				else {
					rc.left -= ptLastMouse.x - event.ptMouse.x;
				}

				if (rc.right - rc.left - rcMargin.right > GetMinWidth()) {
					int cx = rc.right - rc.left - rcMargin.right;
					// 拖拽后改为固定像素；否则 auto/% 在 EstimateSize 里仍回弹
					SetAutoCalcWidth(false);
					m_fWidthPercent = 0.0f;
					m_cxyFixed.cx = cx;
					ptLastMouse = event.ptMouse;
					if (GetParent())
						GetParent()->NeedParentUpdate();
				}
			}
			return;
		}
		if (event.Type == UIEVENT_SETCURSOR)
		{
			RECT rcSeparator = GetThumbRect();
			if (m_iSepWidth >= 0)
				rcSeparator.left -= 4;
			else
				rcSeparator.right += 4;
			if (IsEnabled() && IsColumnResizeEnabled() && ::PtInRect(&rcSeparator, event.ptMouse)) {
				::SetCursor(::LoadCursor(NULL, IDC_SIZEWE));
				return;
			}
		}
		if (event.Type == UIEVENT_MOUSEENTER)
		{
			if (IsEnabled()) {
				m_uButtonState |= UISTATE_HOT;
				Invalidate();
			}
			return;
		}
		if (event.Type == UIEVENT_MOUSELEAVE)
		{
			if (IsEnabled()) {
				m_uButtonState &= ~UISTATE_HOT;
				Invalidate();
			}
			return;
		}
		CHorizontalLayoutUI::DoEvent(event);
	}

	SIZE CListHeaderItemUI::EstimateSize(SIZE szAvailable)
	{
		int cx = 0;
		if( GetWidthPercent() > 0.0f ) {
			if( szAvailable.cx > 0 )
				cx = (int)(szAvailable.cx * (double)GetWidthPercent() + 0.5);
		}
		else if( GetAutoCalcWidth() ) {
			CDuiBox pad = GetPadding();
			RECT rcTextPad = GetTextPadding();
			const int padLR = pad.left + pad.right + rcTextPad.left + rcTextPad.right;
			int sep = (int)GetSepWidth();
			if( sep < 0 ) sep = -sep;
			int slack = 6;
			CDuiString sText = GetText();
			if( m_pManager != NULL ) {
				slack = m_pManager->GetDPIObj()->Scale(6);
				if( !sText.IsEmpty() ) {
					SIZE szText = RenderMeasureTextSize(m_pManager, sText.GetData(), m_iFont, DT_SINGLELINE);
					cx = szText.cx + padLR + sep + slack;
				}
				else {
					cx = padLR + sep;
				}
			}
			else {
				cx = padLR + sep;
			}
			if( cx < GetMinWidth() ) cx = GetMinWidth();
		}
		else {
			cx = GetFixedWidth();
		}

		int cy = GetFixedHeight();
		if( cy > 0 ) return CDuiSize(cx, cy);

		int nPad = 16;
		int nFontH = 14;
		if( m_pManager != NULL ) {
			nPad = m_pManager->GetDPIObj()->Scale(16);
			nFontH = m_pManager->GetDefaultFontInfo()->tm.tmHeight;
			CDuiString sText = GetText();
			if( !sText.IsEmpty() ) {
				SIZE szText = RenderMeasureTextSize(m_pManager, sText.GetData(), m_iFont, DT_SINGLELINE);
				if( szText.cy > nFontH ) nFontH = szText.cy;
			}
		}
		return CDuiSize(cx, nFontH + nPad);
	}

	RECT CListHeaderItemUI::GetThumbRect() const
	{
		if (m_iSepWidth >= 0) return CDuiRect(m_rcItem.right - m_iSepWidth, m_rcItem.top, m_rcItem.right, m_rcItem.bottom);
		else return CDuiRect(m_rcItem.left, m_rcItem.top, m_rcItem.left - m_iSepWidth, m_rcItem.bottom);
	}

	void CListHeaderItemUI::PaintStatusImage(IRenderContext& ctx)
	{
		if (IsFocused()) m_uButtonState |= UISTATE_FOCUSED;
		else m_uButtonState &= ~UISTATE_FOCUSED;

		if ((m_uButtonState & UISTATE_PUSHED) != 0) {
			if (m_sActiveImage.IsEmpty() && !m_sImage.IsEmpty()) DrawImage(ctx, m_sImage.GetData());
			if (!DrawImage(ctx, m_sActiveImage.GetData())) {}
		}
		else if ((m_uButtonState & UISTATE_HOT) != 0) {
			if (m_sHoverImage.IsEmpty() && !m_sImage.IsEmpty()) DrawImage(ctx, m_sImage.GetData());
			if (!DrawImage(ctx, m_sHoverImage.GetData())) {}
		}
		else if ((m_uButtonState & UISTATE_FOCUSED) != 0) {
			if (m_sFocusImage.IsEmpty() && !m_sImage.IsEmpty()) DrawImage(ctx, m_sImage.GetData());
			if (!DrawImage(ctx, m_sFocusImage.GetData())) {}
		}
		else {
			if (!m_sImage.IsEmpty()) {
				if (!DrawImage(ctx, m_sImage.GetData())) {}
			}
		}

		if (!m_sSepImage.IsEmpty()) {
			RECT rcThumb = GetThumbRect();
			rcThumb.left -= m_rcItem.left;
			rcThumb.top -= m_rcItem.top;
			rcThumb.right -= m_rcItem.left;
			rcThumb.bottom -= m_rcItem.top;

			m_sSepImageModify.Empty();
			m_sSepImageModify.SmallFormat(_T("dest='%d,%d,%d,%d'"), rcThumb.left, rcThumb.top, rcThumb.right, rcThumb.bottom);
			if (!DrawImage(ctx, m_sSepImage.GetData(), m_sSepImageModify.GetData())) {}
		}

		PaintColumnLine(ctx);
	}

	void CListHeaderItemUI::PaintColumnLine(IRenderContext& ctx)
	{
		// 跟随 List::header-show-column-line（与内容区 item-show-column-line 独立）
		bool bShow = true;
		DWORD clr = 0xDEE2E6FF;
		CControlUI* pHdr = GetParent();
		CListUI* pList = NULL;
		if( pHdr != NULL && pHdr->GetParent() != NULL )
			pList = static_cast<CListUI*>(pHdr->GetParent()->GetInterface(DUI_CTR_LIST));
		if( pList != NULL ) {
			TListInfoUI* pInfo = pList->GetListInfo();
			if( pInfo != NULL ) {
				bShow = pInfo->bShowHeaderColumnLine;
				if( pInfo->dwLineColor != 0 )
					clr = pInfo->dwLineColor;
			}
		}
		if( !bShow ) return;

		RECT rcLine = { m_rcItem.right - 1, m_rcItem.top, m_rcItem.right - 1, m_rcItem.bottom };
		ctx.DrawLine(rcLine, 1, GetAdjustColor(clr), PS_SOLID);
	}

	bool CListHeaderItemUI::DoPaint(IRenderContext& ctx, const RECT& rcPaint, CControlUI* pStopControl)
	{
		// 表头列通常无子控件：走 Control 绘制，避免 Container 再套一层 clip
		if( GetCount() == 0 )
			return CControlUI::DoPaint(ctx, rcPaint, pStopControl);
		return CHorizontalLayoutUI::DoPaint(ctx, rcPaint, pStopControl);
	}

	void CListHeaderItemUI::PaintText(IRenderContext& ctx)
	{
		CDuiString sText = GetText();
		if (sText.IsEmpty()) return;

		if (m_dwColor == 0 && m_pManager != NULL)
			m_dwColor = m_pManager->GetDefaultFontColor();

		CDuiBox pad = GetPadding();
		RECT rcTextPad = GetTextPadding();
		RECT rc = m_rcItem;
		rc.left += pad.left + rcTextPad.left;
		rc.right -= pad.right + rcTextPad.right;
		rc.top += pad.top + rcTextPad.top;
		rc.bottom -= pad.bottom + rcTextPad.bottom;
		{
			int sep = (int)GetSepWidth();
			if( sep < 0 ) sep = -sep;
			if( sep > 0 && rc.right - rc.left > sep )
				rc.right -= sep;
		}
		if( rc.right <= rc.left || rc.bottom <= rc.top ) return;

		UINT uStyle = m_uTextStyle | DT_SINGLELINE | DT_NOCLIP;
		if( (uStyle & (DT_TOP | DT_VCENTER | DT_BOTTOM)) == 0 )
			uStyle |= DT_VCENTER;
		// DT_LEFT==0：无 CENTER/RIGHT 时保持左对齐，勿误加 DT_CENTER

		int nLinks = 0;
		if (m_bShowHtml)
			ctx.DrawHtmlText(rc, sText.GetData(), GetAdjustColor(m_dwColor),
				NULL, NULL, nLinks, m_iFont, uStyle);
		else
			ctx.DrawText(rc, sText.GetData(), GetAdjustColor(m_dwColor),
				m_iFont, uStyle);
	}

	/////////////////////////////////////////////////////////////////////////////////////
	//
	//
	CListElementUI::CListElementUI() : m_iIndex(-1),
		m_bSelected(false),
		m_uButtonState(0),
		m_pOwner(NULL)
	{
	}

	LPCTSTR CListElementUI::GetClass() const
	{
		return _T("ListElementUI");
	}

	UINT CListElementUI::GetControlFlags() const
	{
		// SETCURSOR → PreferClientHit，避免 html{action:title} 下项被当成 HTCAPTION 丢悬停
		return UIFLAG_WANTRETURN | (IsEnabled() ? UIFLAG_SETCURSOR : 0);
	}

	bool CListElementUI::PreferClientHit() const
	{
		return IsEnabled();
	}

	LPVOID CListElementUI::GetInterface(LPCTSTR pstrName)
	{
		if (_tcsicmp(pstrName, DUI_CTR_LISTITEM) == 0) return static_cast<IListItemUI*>(this);
		if (_tcsicmp(pstrName, DUI_CTR_LISTELEMENT) == 0) return static_cast<CListElementUI*>(this);
		return CControlUI::GetInterface(pstrName);
	}

	IListOwnerUI* CListElementUI::GetOwner()
	{
		return m_pOwner;
	}

	void CListElementUI::SetOwner(CControlUI* pOwner)
	{
		m_pOwner = static_cast<IListOwnerUI*>(pOwner->GetInterface(_T("IListOwner")));
	}

	void CListElementUI::SetVisible(bool bVisible)
	{
		CControlUI::SetVisible(bVisible);
		if (!IsVisible() && m_bSelected)
		{
			m_bSelected = false;
			if (m_pOwner != NULL) m_pOwner->SelectItem(-1);
		}
	}

	void CListElementUI::SetEnabled(bool bEnable)
	{
		CControlUI::SetEnabled(bEnable);
		if (!IsEnabled()) {
			m_uButtonState = 0;
		}
	}

	int CListElementUI::GetIndex() const
	{
		return m_iIndex;
	}

	void CListElementUI::SetIndex(int iIndex)
	{
		m_iIndex = iIndex;
	}

	void CListElementUI::Invalidate()
	{
		if (!IsVisible()) return;

		if (GetParent()) {
			CContainerUI* pParentContainer = static_cast<CContainerUI*>(GetParent()->GetInterface(_T("Container")));
			if (pParentContainer) {
				RECT rc = pParentContainer->GetPos();
				RECT rcPadding = pParentContainer->GetPadding();
				rc.left += rcPadding.left;
				rc.top += rcPadding.top;
				rc.right -= rcPadding.right;
				rc.bottom -= rcPadding.bottom;
				CScrollBarUI* pVerticalScrollBar = pParentContainer->GetVerticalScrollBar();
				if (pVerticalScrollBar && pVerticalScrollBar->IsVisible()) rc.right -= pVerticalScrollBar->GetFixedWidth();
				CScrollBarUI* pHorizontalScrollBar = pParentContainer->GetHorizontalScrollBar();
				if (pHorizontalScrollBar && pHorizontalScrollBar->IsVisible()) rc.bottom -= pHorizontalScrollBar->GetFixedHeight();

				RECT invalidateRc = m_rcItem;
				if (!::IntersectRect(&invalidateRc, &m_rcItem, &rc))
				{
					return;
				}

				CControlUI* pParent = GetParent();
				RECT rcTemp;
				RECT rcParent;
				while ( (pParent = pParent->GetParent()) )
				{
					rcTemp = invalidateRc;
					rcParent = pParent->GetPos();
					if (!::IntersectRect(&invalidateRc, &rcTemp, &rcParent))
					{
						return;
					}
				}

				if (m_pManager != NULL) m_pManager->Invalidate(invalidateRc);
			}
			else {
				CControlUI::Invalidate();
			}
		}
		else {
			CControlUI::Invalidate();
		}
	}

	bool CListElementUI::Activate()
	{
		if (!CControlUI::Activate()) return false;
		if (m_pManager != NULL) m_pManager->SendNotify(this, DUI_MSGTYPE_ITEMACTIVATE);
		return true;
	}

	bool CListElementUI::IsSelected() const
	{
		return m_bSelected;
	}

	bool CListElementUI::Select(bool bSelect)
	{
		if (!IsEnabled()) return false;
		// 取消其它列表项数据
		if (m_pOwner) {
			m_pOwner->UnSelectItem(m_iIndex, true);
		}
		if (bSelect == m_bSelected) return true;
		m_bSelected = bSelect;
		if (m_pOwner) {
			if (bSelect) m_pOwner->SelectItem(m_iIndex);
			else m_pOwner->UnSelectItem(m_iIndex);
		}
		Invalidate();

		return true;
	}

	bool CListElementUI::SelectMulti(bool bSelect)
	{
		if (!IsEnabled()) return false;

		if (bSelect == m_bSelected) return true;
		m_bSelected = bSelect;
		Invalidate();

		return true;
	}

	bool CListElementUI::IsExpanded() const
	{
		return false;
	}

	bool CListElementUI::Expand(bool /*bExpand = true*/)
	{
		return false;
	}

	void CListElementUI::DoEvent(TEventUI& event)
	{
		if (!IsMouseEnabled() && event.Type > UIEVENT__MOUSEBEGIN && event.Type < UIEVENT__MOUSEEND) {
			if (m_pOwner != NULL) m_pOwner->DoEvent(event);
			else CControlUI::DoEvent(event);
			return;
		}

		if (event.Type == UIEVENT_DBLCLICK)
		{
			if (IsEnabled()) {
				Activate();
				Invalidate();
			}
			return;
		}
		if (event.Type == UIEVENT_KEYDOWN && IsEnabled())
		{
			if (event.chKey == VK_RETURN) {
				Activate();
				Invalidate();
				return;
			}
		}
		// An important twist: The list-item will send the event not to its immediate
		// parent but to the "attached" list. A list may actually embed several components
		// in its path to the item, but key-presses etc. needs to go to the actual list.
		if (m_pOwner != NULL) m_pOwner->DoEvent(event); else CControlUI::DoEvent(event);
	}

	void CListElementUI::SetAttribute(LPCTSTR pstrName, LPCTSTR pstrValue)
	{
		if (_tcsicmp(pstrName, _T("selected")) == 0) Select();
		else CControlUI::SetAttribute(pstrName, pstrValue);
	}

	void CListElementUI::DrawItemBk(IRenderContext& ctx, const RECT& rcItem)
	{
		ASSERT(m_pOwner);
		if (m_pOwner == NULL) return;


		TListInfoUI* pInfo = m_pOwner->GetListInfo();
		DWORD iBackColor = 0;
		if( pInfo->bAlternateBk && (m_iIndex % 2) == 1 ) {
			if( pInfo->dwAlternateBackgroundColor != 0 )
				iBackColor = pInfo->dwAlternateBackgroundColor;
		}
		else {
			iBackColor = pInfo->dwBackgroundColor;
		}
		if ((m_uButtonState & UISTATE_HOT) != 0 && pInfo->dwHoverBackgroundColor > 0) {
			iBackColor = pInfo->dwHoverBackgroundColor;
		}
		if (IsSelected() && pInfo->dwSelectedBackgroundColor > 0) {
			iBackColor = pInfo->dwSelectedBackgroundColor;
		}
		if (!IsEnabled() && pInfo->dwDisabledBackgroundColor > 0) {
			iBackColor = pInfo->dwDisabledBackgroundColor;
		}

		if (iBackColor != 0) {
			ctx.DrawColor(m_rcItem, GetAdjustColor(iBackColor));
		}

		if (!IsEnabled()) {
			if (!pInfo->sDisabledImage.IsEmpty()) {
				if (!DrawImage(ctx, pInfo->sDisabledImage.GetData())) {}
				else return;
			}
		}
		if (IsSelected()) {
			if (!pInfo->sSelectedImage.IsEmpty()) {
				if (!DrawImage(ctx, pInfo->sSelectedImage.GetData())) {}
				else return;
			}
		}
		if ((m_uButtonState & UISTATE_HOT) != 0) {
			if (!pInfo->sHoverImage.IsEmpty()) {
				if (!DrawImage(ctx, pInfo->sHoverImage.GetData())) {}
				else return;
			}
		}

		if (!m_sBackgroundImage.IsEmpty()) {
			if (!pInfo->bAlternateBk || m_iIndex % 2 == 0) {
				if (!DrawImage(ctx, m_sBackgroundImage.GetData())) {}
			}
		}

		if (m_sBackgroundImage.IsEmpty()) {
			if (!pInfo->sBkImage.IsEmpty()) {
				if (!DrawImage(ctx, pInfo->sBkImage.GetData())) {}
				else return;
			}
		}

		if (pInfo->dwLineColor != 0) {
			if (pInfo->bShowRowLine) {
				RECT rcLine = { m_rcItem.left, m_rcItem.bottom - 1, m_rcItem.right, m_rcItem.bottom - 1 };
				ctx.DrawLine(rcLine, 1, GetAdjustColor(pInfo->dwLineColor));
			}
			if (pInfo->bShowColumnLine) {
				for (int i = 0; i < pInfo->nColumns; i++) {
					RECT rcLine = { pInfo->rcColumn[i].right - 1, m_rcItem.top, pInfo->rcColumn[i].right - 1, m_rcItem.bottom };
					ctx.DrawLine(rcLine, 1, GetAdjustColor(pInfo->dwLineColor));
				}
			}
		}
	}

	/////////////////////////////////////////////////////////////////////////////////////
	//
	//
	IMPLEMENT_DUICONTROL(CListLabelElementUI)

	CListLabelElementUI::CListLabelElementUI()
		: m_pIcon(NULL)
		, m_pRasterIcon(NULL)
		, m_eIconKind(IconNone)
		, m_hRasterTint(NULL)
		, m_dwRasterTintColor(0)
		, m_nRasterTintW(0)
		, m_nRasterTintH(0)
		, m_nIconSize(16)
		, m_nIconGap(6)
		, m_sIconPos(_T("left"))
		, m_dwIconTint(0)
		, m_dwIconTintHover(0)
		, m_dwIconTintSelected(0)
		, m_dwIconTintDisabled(0)
		, m_bIconTint(false)
		, m_bIconTintAuto(false)
	{
	}

	CListLabelElementUI::~CListLabelElementUI()
	{
		ClearRasterTintCache();
		if( m_pIcon != NULL ) { delete m_pIcon; m_pIcon = NULL; }
		if( m_pRasterIcon != NULL ) { delete m_pRasterIcon; m_pRasterIcon = NULL; }
	}

	LPCTSTR CListLabelElementUI::GetClass() const
	{
		return _T("ListLabelElementUI");
	}

	LPVOID CListLabelElementUI::GetInterface(LPCTSTR pstrName)
	{
		if (_tcsicmp(pstrName, DUI_CTR_LISTLABELELEMENT) == 0) return static_cast<CListLabelElementUI*>(this);
		return CListElementUI::GetInterface(pstrName);
	}

	void CListLabelElementUI::SetManager(CPaintManagerUI* pManager, CControlUI* pParent, bool bInit)
	{
		CControlUI::SetManager(pManager, pParent, bInit);
		if( m_pIcon != NULL ) m_pIcon->SetManager(pManager, this, bInit);
		if( m_pRasterIcon != NULL ) m_pRasterIcon->SetManager(pManager, this, bInit);
	}

	void CListLabelElementUI::DoEvent(TEventUI& event)
	{
		if (!IsMouseEnabled() && event.Type > UIEVENT__MOUSEBEGIN && event.Type < UIEVENT__MOUSEEND) {
			if (m_pOwner != NULL) m_pOwner->DoEvent(event);
			else CListElementUI::DoEvent(event);
			return;
		}

		// 右键选择
		if (m_pOwner != NULL)
		{
			if (m_pOwner->GetListInfo()->bRSelected && event.Type == UIEVENT_RBUTTONDOWN)
			{
				if (IsEnabled()) {
					// 多选
					if ((GetKeyState(VK_CONTROL) & 0x8000) || (GetKeyState(VK_SHIFT) & 0x8000)) {
						if (m_pOwner != NULL) m_pOwner->SelectMultiItem(m_iIndex);
					}
					else {
						Select(true);
					}
				}
				return;
			}
		}

		if (event.Type == UIEVENT_BUTTONDOWN)
		{
			if (IsEnabled()) {
				if(IsRichEvent()) m_pManager->SendNotify(this, DUI_MSGTYPE_BUTTONDOWN);

				// 多选
				if ((GetKeyState(VK_CONTROL) & 0x8000) || (GetKeyState(VK_SHIFT) & 0x8000)) {
					if (m_pOwner != NULL) m_pOwner->SelectMultiItem(m_iIndex);
				}
				else {
					Select(true);
				}
			}
			return;
		}
		if (event.Type == UIEVENT_BUTTONUP)
		{
			if (IsEnabled()) {
				m_pManager->SendNotify(this, DUI_MSGTYPE_ITEMCLICK);
			}
			return;
		}
		if (event.Type == UIEVENT_MOUSEENTER)
		{
			if (IsEnabled()) {
				m_uButtonState |= UISTATE_HOT;
				Invalidate();
				if(IsRichEvent()) m_pManager->SendNotify(this, DUI_MSGTYPE_MOUSEENTER);
			}
			return;
		}
		if (event.Type == UIEVENT_MOUSELEAVE)
		{
			if ((m_uButtonState & UISTATE_HOT) != 0) {
				m_uButtonState &= ~UISTATE_HOT;
				Invalidate();
				if(IsRichEvent()) m_pManager->SendNotify(this, DUI_MSGTYPE_MOUSELEAVE);
			}
			return;
		}
		CListElementUI::DoEvent(event);
	}

	SIZE CListLabelElementUI::EstimateSize(SIZE szAvailable)
	{
		if (m_pOwner == NULL) return CDuiSize(0, 0);
		CDuiString sText = GetText();

		TListInfoUI* pInfo = m_pOwner->GetListInfo();
		SIZE cXY = m_cxyFixed;
		if (cXY.cy == 0 && m_pManager != NULL) {
			cXY.cy = m_pManager->GetFontInfo(pInfo->nFont)->tm.tmHeight + 8;
			cXY.cy += pInfo->rcTextPadding.top + pInfo->rcTextPadding.bottom;
		}

		if( HasIcon() && m_pManager != NULL ) {
			const int nSize = m_pManager->GetDPIObj()->Scale(m_nIconSize);
			const int nGap = m_pManager->GetDPIObj()->Scale(m_nIconGap);
			const bool bVertical = (m_sIconPos.CompareNoCase(_T("top")) == 0
				|| m_sIconPos.CompareNoCase(_T("bottom")) == 0);
			if( bVertical && m_cxyFixed.cy == 0 ) {
				int nTextH = m_pManager->GetFontInfo(pInfo->nFont)->tm.tmHeight;
				cXY.cy = nSize + nGap + nTextH + pInfo->rcTextPadding.top + pInfo->rcTextPadding.bottom;
			}
			else {
				const int nMinH = nSize + pInfo->rcTextPadding.top + pInfo->rcTextPadding.bottom;
				if( cXY.cy < nMinH ) cXY.cy = nMinH;
			}
		}

		if (cXY.cx == 0) {
			cXY.cx = szAvailable.cx;
		}

		return cXY;
	}

	bool CListLabelElementUI::DoPaint(IRenderContext& ctx, const RECT& rcPaint, CControlUI* pStopControl)
	{
		DrawItemBk(ctx, m_rcItem);
		DrawItemText(ctx, m_rcItem);
		return true;
	}

	void CListLabelElementUI::DrawItemText(IRenderContext& ctx, const RECT& rcItem)
	{
		if (m_pOwner == NULL) return;
		TListInfoUI* pInfo = m_pOwner->GetListInfo();
		DWORD iTextColor = pInfo->dwColor;
		if ((m_uButtonState & UISTATE_HOT) != 0) {
			iTextColor = pInfo->dwHoverColor;
		}
		if (IsSelected()) {
			iTextColor = pInfo->dwSelectedColor;
		}
		if (!IsEnabled()) {
			iTextColor = pInfo->dwDisabledColor;
		}

		RECT rcTextPadding = GetManager()->GetDPIObj()->Scale(pInfo->rcTextPadding);
		RECT rcContent = rcItem;
		rcContent.left += rcTextPadding.left;
		rcContent.right -= rcTextPadding.right;
		rcContent.top += rcTextPadding.top;
		rcContent.bottom -= rcTextPadding.bottom;

		UINT uStyle = pInfo->uTextStyle;
		PaintIconAndText(ctx, rcContent, m_rcPaint, iTextColor, pInfo->nFont, uStyle, pInfo->bShowHtml);
	}

	void CListLabelElementUI::PaintIconAndText(IRenderContext& ctx, const RECT& rcContent, const RECT& rcPaint,
		DWORD dwTextColor, int iFont, UINT uTextStyle, bool bShowHtml)
	{
		RECT rcOldPaint = m_rcPaint;
		m_rcPaint = rcPaint;

		RECT rcText = rcContent;
		if( HasIcon() ) {
			SyncIconAppearance();
			RECT rcIcon = { 0 };
			if( LayoutIconAndText(rcContent, rcIcon, rcText) ) {
				if( m_eIconKind == IconRaster && m_pRasterIcon != NULL && m_pRasterIcon->IsVisible() )
					PaintRasterIcon(ctx, rcIcon);
				else if( m_pIcon != NULL && m_pIcon->IsVisible() ) {
					m_pIcon->SetPos(rcIcon, false);
					m_pIcon->PaintIcon(ctx, m_rcPaint);
				}
			}
		}

		CDuiString sText = GetText();
		if( !sText.IsEmpty() ) {
			UINT uStyle = uTextStyle;
			if( HasIcon() ) {
				uStyle &= ~(DT_CENTER | DT_RIGHT | DT_LEFT);
				const bool bVertical = (m_sIconPos.CompareNoCase(_T("top")) == 0
					|| m_sIconPos.CompareNoCase(_T("bottom")) == 0);
				if( bVertical )
					uStyle |= DT_CENTER;
				else
					uStyle |= DT_LEFT;
				if( (uStyle & (DT_VCENTER | DT_BOTTOM | DT_TOP)) == 0 )
					uStyle |= DT_VCENTER;
			}

			int nLinks = 0;
			if( bShowHtml )
				ctx.DrawHtmlText(rcText, sText.GetData(), GetAdjustColor(dwTextColor), NULL, NULL, nLinks, iFont, uStyle);
			else
				ctx.DrawText(rcText, sText.GetData(), GetAdjustColor(dwTextColor), iFont, uStyle);
		}

		m_rcPaint = rcOldPaint;
	}

	void CListLabelElementUI::SetAttribute(LPCTSTR pstrName, LPCTSTR pstrValue)
	{
		if( _tcsicmp(pstrName, _T("icon-size")) == 0 ) {
			SetIconSize(_ttoi(pstrValue));
		}
		else if( _tcsicmp(pstrName, _T("icon-gap")) == 0 ) {
			SetIconGap(_ttoi(pstrValue));
		}
		else if( _tcsicmp(pstrName, _T("icon-position")) == 0
			|| _tcsicmp(pstrName, _T("icon-pos")) == 0 ) {
			SetIconPosition(pstrValue);
		}
		else if( _tcsicmp(pstrName, _T("icon-tint")) == 0
			|| _tcsicmp(pstrName, _T("icon-color")) == 0 ) {
			if( pstrValue == NULL || *pstrValue == _T('\0')
				|| _tcsicmp(pstrValue, _T("none")) == 0
				|| _tcsicmp(pstrValue, _T("false")) == 0
				|| _tcsicmp(pstrValue, _T("original")) == 0 ) {
				SetIconTintAuto(false);
				SetIconTint(0);
			}
			else if( _tcsicmp(pstrValue, _T("auto")) == 0
				|| _tcsicmp(pstrValue, _T("true")) == 0 ) {
				SetIconTintAuto(true);
			}
			else {
				DWORD clr = 0;
				if( ParseColorString(pstrValue, clr) ) SetIconTint(clr);
			}
		}
		else if( _tcsicmp(pstrName, _T("icon-tint-hover")) == 0
			|| _tcsicmp(pstrName, _T("icon-color-hover")) == 0 ) {
			DWORD clr = 0;
			if( ParseColorString(pstrValue, clr) ) SetIconTintHover(clr);
		}
		else if( _tcsicmp(pstrName, _T("icon-tint-selected")) == 0
			|| _tcsicmp(pstrName, _T("icon-color-selected")) == 0
			|| _tcsicmp(pstrName, _T("icon-tint-active")) == 0
			|| _tcsicmp(pstrName, _T("icon-color-active")) == 0 ) {
			DWORD clr = 0;
			if( ParseColorString(pstrValue, clr) ) SetIconTintSelected(clr);
		}
		else if( _tcsicmp(pstrName, _T("icon-tint-disabled")) == 0
			|| _tcsicmp(pstrName, _T("icon-color-disabled")) == 0 ) {
			DWORD clr = 0;
			if( ParseColorString(pstrValue, clr) ) SetIconTintDisabled(clr);
		}
		else if( IsIconAttr(pstrName) ) {
			if( pstrValue == NULL || *pstrValue == _T('\0') ) {
				ClearIcon();
				return;
			}
			if( _tcsicmp(pstrName, _T("icon-src")) == 0 || _tcsicmp(pstrName, _T("icon")) == 0 )
				SetIconSrc(pstrValue);
			else
				SetIconLib(pstrName, pstrValue);
		}
		else
			CListElementUI::SetAttribute(pstrName, pstrValue);
	}

	void CListLabelElementUI::EnsureIcon()
	{
		if( m_pIcon != NULL ) return;
		m_pIcon = new CSvgBoxUI;
		m_pIcon->SetMouseEnabled(false);
		m_pIcon->SetVisible(false);
		if( m_pManager != NULL )
			m_pIcon->SetManager(m_pManager, this, false);
	}

	void CListLabelElementUI::EnsureRasterIcon()
	{
		if( m_pRasterIcon != NULL ) return;
		m_pRasterIcon = new CControlUI;
		m_pRasterIcon->SetMouseEnabled(false);
		m_pRasterIcon->SetVisible(false);
		if( m_pManager != NULL )
			m_pRasterIcon->SetManager(m_pManager, this, false);
	}

	bool CListLabelElementUI::IsIconAttr(LPCTSTR pstrName) const
	{
		return _tcsicmp(pstrName, _T("bsicon")) == 0
			|| _tcsicmp(pstrName, _T("iconpark")) == 0
			|| _tcsicmp(pstrName, _T("lucide")) == 0
			|| _tcsicmp(pstrName, _T("tabler-outline")) == 0
			|| _tcsicmp(pstrName, _T("tabler-filled")) == 0
			|| _tcsicmp(pstrName, _T("remixicon")) == 0
			|| _tcsicmp(pstrName, _T("twicon")) == 0
			|| _tcsicmp(pstrName, _T("icon-src")) == 0
			|| _tcsicmp(pstrName, _T("icon")) == 0;
	}

	bool CListLabelElementUI::IsRasterImagePath(LPCTSTR pstrPath)
	{
		if( pstrPath == NULL || *pstrPath == _T('\0') ) return false;
		CDuiString s(pstrPath);
		s.MakeLower();
		LPCTSTR pExt = NULL;
		for( LPCTSTR p = s.GetData(); *p != _T('\0'); ++p ) {
			if( *p == _T('.') ) pExt = p;
			else if( *p == _T('\'') || *p == _T('"') || *p == _T(' ') || *p == _T('\t') ) {
				if( pExt != NULL ) break;
			}
		}
		if( pExt == NULL ) return false;
		return _tcsncmp(pExt, _T(".bmp"), 4) == 0
			|| _tcsncmp(pExt, _T(".png"), 4) == 0
			|| _tcsncmp(pExt, _T(".jpg"), 4) == 0
			|| _tcsncmp(pExt, _T(".jpeg"), 5) == 0;
	}

	void CListLabelElementUI::RefreshRasterIconImage()
	{
		if( m_pRasterIcon == NULL || m_sRasterPath.IsEmpty() ) return;
		if( !m_pRasterIcon->IsVisible() ) return;
		int nSize = m_nIconSize;
		if( m_pManager != NULL )
			nSize = m_pManager->GetDPIObj()->Scale(m_nIconSize);
		CDuiString sImg = m_sRasterPath;
		if( sImg.Find(_T("file=")) < 0 && sImg.Find(_T("res=")) < 0
			&& sImg.Find(_T("url(")) < 0 ) {
			CDuiString sFmt;
			sFmt.Format(_T("file='%s' dest='0,0,%d,%d'"), m_sRasterPath.GetData(), nSize, nSize);
			sImg = sFmt;
		}
		else if( sImg.Find(_T("dest=")) < 0 ) {
			CDuiString sFmt;
			sFmt.Format(_T("%s dest='0,0,%d,%d'"), m_sRasterPath.GetData(), nSize, nSize);
			sImg = sFmt;
		}
		m_pRasterIcon->SetBackgroundImage(sImg.GetData());
		ClearRasterTintCache();
	}

	void CListLabelElementUI::ClearRasterTintCache()
	{
		if( m_hRasterTint != NULL ) {
			IRenderDevice* pDev = GetRenderDevice();
			if( pDev != NULL ) pDev->InvalidateBitmapGpu(m_hRasterTint);
			::DeleteObject(m_hRasterTint);
			m_hRasterTint = NULL;
		}
		m_dwRasterTintColor = 0;
		m_nRasterTintW = 0;
		m_nRasterTintH = 0;
	}

	bool CListLabelElementUI::EnsureRasterTintCache(DWORD dwColor)
	{
		if( m_pManager == NULL || m_sRasterPath.IsEmpty() || dwColor == 0 )
			return false;

		int nSize = m_pManager->GetDPIObj()->Scale(m_nIconSize);
		if( nSize <= 0 ) return false;

		if( m_hRasterTint != NULL && m_dwRasterTintColor == dwColor
			&& m_nRasterTintW == nSize && m_nRasterTintH == nSize )
			return true;

		ClearRasterTintCache();

		CDuiString sName = m_sRasterPath;
		const int nFile = sName.Find(_T("file='"));
		if( nFile >= 0 ) {
			sName = sName.Mid(nFile + 6);
			const int nEnd = sName.Find(_T('\''));
			if( nEnd >= 0 ) sName = sName.Left(nEnd);
		}
		else {
			const int nUrl = sName.Find(_T("url("));
			if( nUrl >= 0 ) {
				CDuiString sPath;
				if( ParseCssUrlImage(m_sRasterPath.GetData(), sPath) )
					sName = sPath;
			}
		}

		const TImageInfo* pSrc = m_pManager->GetImageEx(sName.GetData());
		if( pSrc == NULL || pSrc->hBitmap == NULL || pSrc->nX <= 0 || pSrc->nY <= 0 )
			return false;

		BITMAP bm = { 0 };
		if( !::GetObject(pSrc->hBitmap, sizeof(bm), &bm) || bm.bmWidth <= 0 || bm.bmHeight <= 0 )
			return false;

		LPBYTE pSrcBits = NULL;
		BYTE* pTempBits = NULL;
		if( bm.bmBits != NULL ) {
			pSrcBits = (LPBYTE)bm.bmBits;
		}
		else if( pSrc->pBits != NULL ) {
			pSrcBits = pSrc->pBits;
		}
		else {
			pTempBits = new BYTE[pSrc->nX * pSrc->nY * 4];
			BITMAPINFO bmi = {};
			bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
			bmi.bmiHeader.biWidth = pSrc->nX;
			bmi.bmiHeader.biHeight = -pSrc->nY;
			bmi.bmiHeader.biPlanes = 1;
			bmi.bmiHeader.biBitCount = 32;
			bmi.bmiHeader.biCompression = BI_RGB;
			HDC hScreen = ::GetDC(NULL);
			int nCopied = ::GetDIBits(hScreen, pSrc->hBitmap, 0, pSrc->nY, pTempBits, &bmi, DIB_RGB_COLORS);
			::ReleaseDC(NULL, hScreen);
			if( nCopied == 0 ) {
				delete[] pTempBits;
				return false;
			}
			pSrcBits = pTempBits;
		}

		BITMAPINFO bmiOut = {};
		bmiOut.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
		bmiOut.bmiHeader.biWidth = nSize;
		bmiOut.bmiHeader.biHeight = -nSize;
		bmiOut.bmiHeader.biPlanes = 1;
		bmiOut.bmiHeader.biBitCount = 32;
		bmiOut.bmiHeader.biCompression = BI_RGB;
		LPBYTE pDest = NULL;
		HBITMAP hTint = ::CreateDIBSection(NULL, &bmiOut, DIB_RGB_COLORS, (void**)&pDest, NULL, 0);
		if( hTint == NULL || pDest == NULL ) {
			delete[] pTempBits;
			return false;
		}

		const BYTE tR = DuiColorR(dwColor);
		const BYTE tG = DuiColorG(dwColor);
		const BYTE tB = DuiColorB(dwColor);
		const int srcW = pSrc->nX;
		const int srcH = pSrc->nY;

		for( int y = 0; y < nSize; ++y ) {
			const int sy = y * srcH / nSize;
			for( int x = 0; x < nSize; ++x ) {
				const int sx = x * srcW / nSize;
				const BYTE* pS = pSrcBits + (sy * srcW + sx) * 4;
				BYTE* pD = pDest + (y * nSize + x) * 4;
				BYTE a = pS[3];
				if( !pSrc->bAlpha ) {
					const int lum = (pS[2] * 30 + pS[1] * 59 + pS[0] * 11) / 100;
					a = (BYTE)(255 - lum);
				}
				pD[0] = (BYTE)((DWORD)tB * a / 255);
				pD[1] = (BYTE)((DWORD)tG * a / 255);
				pD[2] = (BYTE)((DWORD)tR * a / 255);
				pD[3] = a;
			}
		}

		delete[] pTempBits;
		m_hRasterTint = hTint;
		m_dwRasterTintColor = dwColor;
		m_nRasterTintW = nSize;
		m_nRasterTintH = nSize;
		return true;
	}

	void CListLabelElementUI::PaintRasterIcon(IRenderContext& ctx, const RECT& rcIcon)
	{
		if( ShouldTintRasterIcon() ) {
			const DWORD paint = ResolvePaintIconColor();
			if( EnsureRasterTintCache(paint) && m_hRasterTint != NULL ) {
				RECT rcBmp = { 0, 0, m_nRasterTintW, m_nRasterTintH };
				RECT rcCorners = { 0, 0, 0, 0 };
				ctx.DrawImage(m_hRasterTint, rcIcon, m_rcPaint, rcBmp, rcCorners, true, ScaleImageFade());
				return;
			}
		}
		if( m_pRasterIcon != NULL ) {
			m_pRasterIcon->SetPos(rcIcon, false);
			m_pRasterIcon->Paint(ctx, m_rcPaint, NULL);
		}
	}

	bool CListLabelElementUI::ShouldTintRasterIcon() const
	{
		if( m_eIconKind != IconRaster ) return false;
		if( m_bIconTintAuto || m_bIconTint ) return true;
		if( !IsEnabled() ) return m_dwIconTintDisabled != 0;
		if( IsSelected() ) return m_dwIconTintSelected != 0;
		if( (m_uButtonState & UISTATE_HOT) != 0 ) return m_dwIconTintHover != 0;
		return false;
	}

	void CListLabelElementUI::ShowSvgIcon()
	{
		if( m_pRasterIcon != NULL ) {
			m_pRasterIcon->SetVisible(false);
			m_pRasterIcon->SetBackgroundImage(_T(""));
		}
		m_sRasterPath.Empty();
		ClearRasterTintCache();
		m_eIconKind = IconSvg;
		if( m_pIcon != NULL )
			m_pIcon->SetVisible(true);
	}

	void CListLabelElementUI::ShowRasterIcon(LPCTSTR pstrPath)
	{
		EnsureRasterIcon();
		if( m_pIcon != NULL )
			m_pIcon->SetVisible(false);
		m_sRasterPath = pstrPath ? pstrPath : _T("");
		m_eIconKind = IconRaster;
		if( m_pRasterIcon != NULL ) {
			m_pRasterIcon->SetVisible(true);
			RefreshRasterIconImage();
		}
	}

	void CListLabelElementUI::SetIconLib(LPCTSTR pstrLib, LPCTSTR pstrName)
	{
		if( pstrLib == NULL || *pstrLib == _T('\0')
			|| pstrName == NULL || *pstrName == _T('\0')
			|| !IsIconAttr(pstrLib) ) {
			ClearIcon();
			return;
		}
		EnsureIcon();
		m_pIcon->SetAttribute(pstrLib, pstrName);
		ShowSvgIcon();
		Invalidate();
	}

	void CListLabelElementUI::SetIconSrc(LPCTSTR pstrPath)
	{
		if( pstrPath == NULL || *pstrPath == _T('\0') ) {
			ClearIcon();
			return;
		}
		if( IsRasterImagePath(pstrPath) ) {
			ShowRasterIcon(pstrPath);
		}
		else {
			EnsureIcon();
			m_pIcon->SetAttribute(_T("src"), pstrPath);
			ShowSvgIcon();
		}
		Invalidate();
	}

	void CListLabelElementUI::ClearIcon()
	{
		m_eIconKind = IconNone;
		m_sRasterPath.Empty();
		ClearRasterTintCache();
		if( m_pIcon != NULL ) {
			m_pIcon->SetVisible(false);
			m_pIcon->LoadFromUtf8Data("");
		}
		if( m_pRasterIcon != NULL ) {
			m_pRasterIcon->SetVisible(false);
			m_pRasterIcon->SetBackgroundImage(_T(""));
		}
		Invalidate();
	}

	bool CListLabelElementUI::HasIcon() const
	{
		if( m_eIconKind == IconSvg && m_pIcon != NULL && m_pIcon->IsVisible() ) return true;
		if( m_eIconKind == IconRaster && m_pRasterIcon != NULL && m_pRasterIcon->IsVisible() ) return true;
		return false;
	}

	void CListLabelElementUI::SetIconSize(int nSize)
	{
		if( nSize < 8 ) nSize = 8;
		if( nSize > 64 ) nSize = 64;
		if( m_nIconSize == nSize ) return;
		m_nIconSize = nSize;
		if( m_eIconKind == IconRaster ) {
			ClearRasterTintCache();
			RefreshRasterIconImage();
		}
		Invalidate();
	}

	void CListLabelElementUI::SetIconGap(int nGap)
	{
		if( nGap < 0 ) nGap = 0;
		if( m_nIconGap == nGap ) return;
		m_nIconGap = nGap;
		Invalidate();
	}

	void CListLabelElementUI::SetIconPosition(LPCTSTR pstrPos)
	{
		CDuiString s = pstrPos ? pstrPos : _T("left");
		if( s.CompareNoCase(_T("right")) != 0
			&& s.CompareNoCase(_T("top")) != 0
			&& s.CompareNoCase(_T("bottom")) != 0 )
			s = _T("left");
		if( m_sIconPos == s ) return;
		m_sIconPos = s;
		Invalidate();
	}

	void CListLabelElementUI::SetIconTint(DWORD dwColor)
	{
		m_bIconTint = (dwColor != 0);
		m_dwIconTint = dwColor;
		if( m_bIconTint ) m_bIconTintAuto = false;
		ClearRasterTintCache();
		Invalidate();
	}

	void CListLabelElementUI::SetIconTintAuto(bool bAuto)
	{
		const bool bClearExplicit = bAuto && m_bIconTint;
		if( m_bIconTintAuto == bAuto && !bClearExplicit ) return;
		m_bIconTintAuto = bAuto;
		if( bAuto ) {
			m_bIconTint = false;
			m_dwIconTint = 0;
		}
		ClearRasterTintCache();
		Invalidate();
	}

	void CListLabelElementUI::SetIconTintHover(DWORD dwColor)
	{
		m_dwIconTintHover = dwColor;
		ClearRasterTintCache();
		Invalidate();
	}

	void CListLabelElementUI::SetIconTintSelected(DWORD dwColor)
	{
		m_dwIconTintSelected = dwColor;
		ClearRasterTintCache();
		Invalidate();
	}

	void CListLabelElementUI::SetIconTintDisabled(DWORD dwColor)
	{
		m_dwIconTintDisabled = dwColor;
		ClearRasterTintCache();
		Invalidate();
	}

	DWORD CListLabelElementUI::ResolveIconColor() const
	{
		if( m_bIconTint && m_dwIconTint != 0 )
			return m_dwIconTint;
		if( m_pOwner != NULL ) {
			TListInfoUI* pInfo = m_pOwner->GetListInfo();
			if( pInfo != NULL && pInfo->dwColor != 0 )
				return pInfo->dwColor;
		}
		if( m_pManager != NULL ) return m_pManager->GetDefaultFontColor();
		return 0x000000E0;
	}

	DWORD CListLabelElementUI::ResolvePaintIconColor() const
	{
		DWORD clr = ResolveIconColor();
		DWORD clrHover = m_dwIconTintHover;
		DWORD clrSelected = m_dwIconTintSelected;
		DWORD clrDisabled = m_dwIconTintDisabled;
		if( m_pOwner != NULL ) {
			TListInfoUI* pInfo = m_pOwner->GetListInfo();
			if( pInfo != NULL ) {
				if( clrHover == 0 && pInfo->dwHoverColor != 0 ) clrHover = pInfo->dwHoverColor;
				if( clrSelected == 0 && pInfo->dwSelectedColor != 0 ) clrSelected = pInfo->dwSelectedColor;
				if( clrDisabled == 0 && pInfo->dwDisabledColor != 0 ) clrDisabled = pInfo->dwDisabledColor;
			}
		}
		if( clrHover == 0 ) clrHover = clr;
		if( clrSelected == 0 ) clrSelected = clr;
		if( clrDisabled == 0 ) clrDisabled = clr;

		if( !IsEnabled() ) return clrDisabled;
		if( IsSelected() ) return clrSelected;
		if( (m_uButtonState & UISTATE_HOT) != 0 ) return clrHover;
		return clr;
	}

	void CListLabelElementUI::SyncIconAppearance()
	{
		if( m_pIcon == NULL || m_eIconKind != IconSvg ) return;
		m_pIcon->SetEnabled(IsEnabled());
		m_pIcon->SetColor(ResolvePaintIconColor(), false);
		m_pIcon->SetHoverColor(0, false);
		m_pIcon->SetActiveColor(0, false);
		m_pIcon->SetDisabledColor(0, false);
	}

	bool CListLabelElementUI::LayoutIconAndText(const RECT& rcContent, RECT& rcIcon, RECT& rcText) const
	{
		rcText = rcContent;
		::ZeroMemory(&rcIcon, sizeof(rcIcon));
		if( !HasIcon() ) return false;

		int nSize = m_nIconSize;
		int nGap = m_nIconGap;
		if( m_pManager != NULL ) {
			nSize = m_pManager->GetDPIObj()->Scale(m_nIconSize);
			nGap = m_pManager->GetDPIObj()->Scale(m_nIconGap);
		}
		const int cw = rcContent.right - rcContent.left;
		const int ch = rcContent.bottom - rcContent.top;
		if( cw <= 0 || ch <= 0 ) return false;
		if( nSize > cw ) nSize = cw;

		const bool bHasText = !GetText().IsEmpty();
		const bool bTop = (m_sIconPos.CompareNoCase(_T("top")) == 0);
		const bool bBottom = (m_sIconPos.CompareNoCase(_T("bottom")) == 0);
		const bool bRight = (m_sIconPos.CompareNoCase(_T("right")) == 0);

		if( !bHasText ) {
			if( nSize > ch ) nSize = ch;
			rcIcon.left = rcContent.left + (cw - nSize) / 2;
			rcIcon.top = rcContent.top + (ch - nSize) / 2;
			rcIcon.right = rcIcon.left + nSize;
			rcIcon.bottom = rcIcon.top + nSize;
			return true;
		}

		SIZE szText = { 0, 0 };
		if( m_pManager != NULL && m_pOwner != NULL ) {
			TListInfoUI* pInfo = m_pOwner->GetListInfo();
			int iFont = (pInfo != NULL) ? pInfo->nFont : -1;
			CDuiString sText = GetText();
			UINT uMeas = DT_SINGLELINE | DT_LEFT | DT_TOP | DT_CALCRECT;
			szText = RenderMeasureTextSize(const_cast<CPaintManagerUI*>(m_pManager),
				sText.GetData(), iFont, uMeas);
		}
		if( szText.cx < 0 ) szText.cx = 0;
		if( szText.cy < 0 ) szText.cy = 0;

		if( bTop || bBottom ) {
			const int nTextReserve = szText.cy + nGap;
			if( nSize > ch - nTextReserve && ch > nTextReserve )
				nSize = ch - nTextReserve;
			else if( nSize > ch )
				nSize = ch;

			int blockH = nSize + nGap + szText.cy;
			if( blockH > ch ) blockH = ch;
			int y = rcContent.top + (ch - blockH) / 2;
			const int yEnd = (y + blockH > rcContent.bottom) ? rcContent.bottom : (y + blockH);
			rcIcon.left = rcContent.left + (cw - nSize) / 2;
			rcIcon.right = rcIcon.left + nSize;
			rcText.left = rcContent.left;
			rcText.right = rcContent.right;
			if( bBottom ) {
				rcText.top = y;
				rcText.bottom = y + szText.cy;
				if( rcText.bottom > yEnd ) rcText.bottom = yEnd;
				if( rcText.top > rcText.bottom ) rcText.top = rcText.bottom;
				rcIcon.top = rcText.bottom + nGap;
				rcIcon.bottom = rcIcon.top + nSize;
				if( rcIcon.bottom > yEnd ) {
					rcIcon.bottom = yEnd;
					rcIcon.top = rcIcon.bottom - nSize;
					if( rcIcon.top < rcText.bottom ) rcIcon.top = rcText.bottom;
				}
			}
			else {
				rcIcon.top = y;
				rcIcon.bottom = rcIcon.top + nSize;
				rcText.top = rcIcon.bottom + nGap;
				rcText.bottom = yEnd;
				if( rcText.top > rcText.bottom ) rcText.top = rcText.bottom;
			}
			return true;
		}

		if( nSize > ch ) nSize = ch;

		if( bRight ) {
			rcIcon.right = rcContent.right;
			rcIcon.left = rcIcon.right - nSize;
			rcIcon.top = rcContent.top + (ch - nSize) / 2;
			rcIcon.bottom = rcIcon.top + nSize;
			rcText.left = rcContent.left;
			rcText.right = rcIcon.left - nGap;
			if( rcText.right < rcText.left ) rcText.right = rcText.left;
		}
		else {
			rcIcon.left = rcContent.left;
			rcIcon.right = rcIcon.left + nSize;
			rcIcon.top = rcContent.top + (ch - nSize) / 2;
			rcIcon.bottom = rcIcon.top + nSize;
			rcText.left = rcIcon.right + nGap;
			rcText.right = rcContent.right;
			if( rcText.left > rcText.right ) rcText.left = rcText.right;
		}
		rcText.top = rcContent.top;
		rcText.bottom = rcContent.bottom;
		return true;
	}


	/////////////////////////////////////////////////////////////////////////////////////
	//
	//
	IMPLEMENT_DUICONTROL(CListTextElementUI)

	CListTextElementUI::CListTextElementUI() : m_nLinks(0), m_nHoverLink(-1), m_pOwner(NULL)
	{
		::ZeroMemory(&m_rcLinks, sizeof(m_rcLinks));
	}

	CListTextElementUI::~CListTextElementUI()
	{
		CDuiString* pText;
		for (int it = 0; it < m_aTexts.GetSize(); it++) {
			pText = static_cast<CDuiString*>(m_aTexts[it]);
			if (pText) delete pText;
		}
		m_aTexts.Empty();
	}

	LPCTSTR CListTextElementUI::GetClass() const
	{
		return _T("ListTextElementUI");
	}

	LPVOID CListTextElementUI::GetInterface(LPCTSTR pstrName)
	{
		if (_tcsicmp(pstrName, DUI_CTR_LISTTEXTELEMENT) == 0) return static_cast<CListTextElementUI*>(this);
		return CListLabelElementUI::GetInterface(pstrName);
	}

	UINT CListTextElementUI::GetControlFlags() const
	{
		UINT u = UIFLAG_WANTRETURN | (IsEnabled() ? UIFLAG_SETCURSOR : 0);
		if( IsEnabled() && m_nLinks > 0 ) u |= UIFLAG_SETCURSOR;
		return u;
	}

	LPCTSTR CListTextElementUI::GetText(int iIndex) const
	{
		CDuiString* pText = static_cast<CDuiString*>(m_aTexts.GetAt(iIndex));
		if (pText) {
			if (!IsResourceText())
				return pText->GetData();
			return CResourceManager::GetInstance()->GetText(pText->GetData()).GetData();
		}
		return NULL;
	}

	void CListTextElementUI::SetText(int iIndex, LPCTSTR pstrText)
	{
		if (iIndex < 0) return;
		if (m_pOwner != NULL) {
			TListInfoUI* pInfo = m_pOwner->GetListInfo();
			if (pInfo != NULL && iIndex >= pInfo->nColumns) return;
		}
		while (m_aTexts.GetSize() <= iIndex) { m_aTexts.Add(NULL); }

		CDuiString* pText = static_cast<CDuiString*>(m_aTexts[iIndex]);
		if ((pText == NULL && pstrText == NULL) || (pText && *pText == pstrText)) return;

		if (pText) { delete pText; pText = NULL; }
		m_aTexts.SetAt(iIndex, new CDuiString(pstrText));

		Invalidate();
	}

	void CListTextElementUI::SetAttribute(LPCTSTR pstrName, LPCTSTR pstrValue)
	{
		if( _tcsicmp(pstrName, _T("text")) == 0 && pstrValue != NULL ) {
			// 多列：text="名称|类型|大小|时间"（| 分隔）
			int iCol = 0;
			LPCTSTR p = pstrValue;
			while( *p != _T('\0') ) {
				CDuiString sCell;
				while( *p != _T('\0') && *p != _T('|') ) {
					sCell += *p;
					++p;
				}
				SetText(iCol++, sCell.GetData());
				if( *p == _T('|') ) ++p;
			}
		}
		else if( _tcsnicmp(pstrName, _T("text-"), 5) == 0 ) {
			SetText(_ttoi(pstrName + 5), pstrValue);
		}
		else CListLabelElementUI::SetAttribute(pstrName, pstrValue);
	}

	DWORD CListTextElementUI::GetColor(int iIndex) const
	{
		TListInfoUI* pInfo = m_pOwner->GetListInfo();
		if( iIndex < 0 || iIndex >= pInfo->nColumns || m_aTextColors.GetSize() <= 0 ) return pInfo->dwColor;

		DWORD dwColor = (DWORD)(UINT_PTR)m_aTextColors.GetAt(iIndex);
		return dwColor;
	}

	void CListTextElementUI::SetColor(int iIndex, DWORD dwColor)
	{
		if( m_pOwner == NULL ) return;

		TListInfoUI* pInfo = m_pOwner->GetListInfo();
		if( iIndex < 0 || iIndex >= pInfo->nColumns ) return;
		while( m_aTextColors.GetSize() < pInfo->nColumns ) { m_aTextColors.Add((LPVOID)(INT_PTR)pInfo->dwColor); }
		m_aTextColors.SetAt(iIndex, (LPVOID)(UINT_PTR)dwColor);

		Invalidate();
	}

	void CListTextElementUI::SetOwner(CControlUI* pOwner)
	{
		CListElementUI::SetOwner(pOwner);
		m_pOwner = static_cast<IListUI*>(pOwner->GetInterface(_T("IList")));
	}

	CDuiString* CListTextElementUI::GetLinkContent(int iIndex)
	{
		if (iIndex >= 0 && iIndex < m_nLinks) return &m_sLinks[iIndex];
		return NULL;
	}

	void CListTextElementUI::DoEvent(TEventUI& event)
	{
		if (!IsMouseEnabled() && event.Type > UIEVENT__MOUSEBEGIN && event.Type < UIEVENT__MOUSEEND) {
			if (m_pOwner != NULL) m_pOwner->DoEvent(event);
			else CListLabelElementUI::DoEvent(event);
			return;
		}

		// When you hover over a link
		if (event.Type == UIEVENT_SETCURSOR) {
			for (int i = 0; i < m_nLinks; i++) {
				if (::PtInRect(&m_rcLinks[i], event.ptMouse)) {
					::SetCursor(::LoadCursor(NULL, IDC_HAND));
					return;
				}
			}
		}
		if (event.Type == UIEVENT_BUTTONUP && IsEnabled()) {
			for (int i = 0; i < m_nLinks; i++) {
				if (::PtInRect(&m_rcLinks[i], event.ptMouse)) {
					m_pManager->SendNotify(this, DUI_MSGTYPE_LINK, i);
					return;
				}
			}
		}
		if (m_nLinks > 0 && event.Type == UIEVENT_MOUSEMOVE) {
			int nHoverLink = -1;
			for (int i = 0; i < m_nLinks; i++) {
				if (::PtInRect(&m_rcLinks[i], event.ptMouse)) {
					nHoverLink = i;
					break;
				}
			}

			if (m_nHoverLink != nHoverLink) {
				Invalidate();
				m_nHoverLink = nHoverLink;
			}
		}
		if (m_nLinks > 0 && event.Type == UIEVENT_MOUSELEAVE) {
			if (m_nHoverLink != -1) {
				Invalidate();
				m_nHoverLink = -1;
			}
		}
		CListLabelElementUI::DoEvent(event);
	}

	SIZE CListTextElementUI::EstimateSize(SIZE szAvailable)
	{
		TListInfoUI* pInfo = NULL;
		if (m_pOwner) pInfo = m_pOwner->GetListInfo();

		SIZE cXY = m_cxyFixed;
		if (cXY.cy == 0 && m_pManager != NULL) {
			cXY.cy = m_pManager->GetFontInfo(pInfo->nFont)->tm.tmHeight + 8;
			if (pInfo) cXY.cy += pInfo->rcTextPadding.top + pInfo->rcTextPadding.bottom;
		}

		return cXY;
	}

	void CListTextElementUI::DrawItemText(IRenderContext& ctx, const RECT& rcItem)
	{
		if( m_pOwner == NULL ) return;


		TListInfoUI* pInfo = m_pOwner->GetListInfo();
		IListCallbackUI* pCallback = m_pOwner->GetTextCallback();
		m_nLinks = 0;
		int nLinks = lengthof(m_rcLinks);
		for( int i = 0; i < pInfo->nColumns; i++ )
		{
			RECT rcItem = { pInfo->rcColumn[i].left, m_rcItem.top, pInfo->rcColumn[i].right, m_rcItem.bottom };
			ApplyListColumnCellPadding(rcItem, m_pOwner->GetHeader(), i, pInfo);

			DWORD iTextColor = pInfo->dwColor;
			CDuiString strText;
			if( pCallback ) {
				strText = pCallback->GetItemText(this, m_iIndex, i);
				int iState = 0;
				if( (m_uButtonState & UISTATE_HOT) != 0 ) {
					iState = 1;
				}
				if( IsSelected() ) {
					iState = 2;
				}
				if( !IsEnabled() ) {
					iState = 3;
				}
				iTextColor = pCallback->GetItemColor(this, m_iIndex, i, iState);
			}
			else {
				strText.Assign(GetText(i));

				iTextColor = GetColor(i);
				if( (m_uButtonState & UISTATE_HOT) != 0 ) {
					iTextColor = pInfo->dwHoverColor;
				}
				if( IsSelected() ) {
					iTextColor = pInfo->dwSelectedColor;
				}
				if( !IsEnabled() ) {
					iTextColor = pInfo->dwDisabledColor;
				}

			}
			if( pInfo->bShowHtml )
				ctx.DrawHtmlText(rcItem, strText.GetData(), GetAdjustColor(iTextColor), \
				&m_rcLinks[m_nLinks], &m_sLinks[m_nLinks], nLinks, pInfo->nFont,
				ResolveListColumnTextStyle(m_pOwner->GetHeader(), i, pInfo->uTextStyle) | DT_NOCLIP);
			else
				ctx.DrawText(rcItem, strText.GetData(), GetAdjustColor(iTextColor), \
				pInfo->nFont, ResolveListColumnTextStyle(m_pOwner->GetHeader(), i, pInfo->uTextStyle) | DT_NOCLIP);

			m_nLinks += nLinks;
			nLinks = lengthof(m_rcLinks) - m_nLinks; 
		}
		for( int i = m_nLinks; i < lengthof(m_rcLinks); i++ ) {
			::ZeroMemory(m_rcLinks + i, sizeof(RECT));
			((CDuiString*)(m_sLinks + i))->Empty();
		}
	}

	/////////////////////////////////////////////////////////////////////////////////////
	//
	//
	IMPLEMENT_DUICONTROL(CListContainerElementUI)

	CListContainerElementUI::CListContainerElementUI() :
	m_iIndex(-1),
		m_bSelected(false),
		m_uButtonState(0),
		m_pOwner(NULL)
	{
	}

	LPCTSTR CListContainerElementUI::GetClass() const
	{
		return _T("ListContainerElementUI");
	}

	UINT CListContainerElementUI::GetControlFlags() const
	{
		return UIFLAG_WANTRETURN | (IsEnabled() ? UIFLAG_SETCURSOR : 0);
	}

	bool CListContainerElementUI::PreferClientHit() const
	{
		return IsEnabled();
	}

	LPVOID CListContainerElementUI::GetInterface(LPCTSTR pstrName)
	{
		if (_tcsicmp(pstrName, DUI_CTR_LISTITEM) == 0) return static_cast<IListItemUI*>(this);
		if (_tcsicmp(pstrName, DUI_CTR_LISTCONTAINERELEMENT) == 0) return static_cast<CListContainerElementUI*>(this);
		return CContainerUI::GetInterface(pstrName);
	}

	IListOwnerUI* CListContainerElementUI::GetOwner()
	{
		return m_pOwner;
	}

	void CListContainerElementUI::SetOwner(CControlUI* pOwner)
	{
		m_pOwner = static_cast<IListOwnerUI*>(pOwner->GetInterface(_T("IListOwner")));
	}

	void CListContainerElementUI::SetVisible(bool bVisible)
	{
		CContainerUI::SetVisible(bVisible);
		if (!IsVisible() && m_bSelected)
		{
			m_bSelected = false;
			if (m_pOwner != NULL) m_pOwner->SelectItem(-1);
		}
	}

	void CListContainerElementUI::SetEnabled(bool bEnable)
	{
		CControlUI::SetEnabled(bEnable);
		if (!IsEnabled()) {
			m_uButtonState = 0;
		}
	}

	int CListContainerElementUI::GetIndex() const
	{
		return m_iIndex;
	}

	void CListContainerElementUI::SetIndex(int iIndex)
	{
		m_iIndex = iIndex;
	}

	void CListContainerElementUI::Invalidate()
	{
		if (!IsVisible()) return;

		if (GetParent()) {
			CContainerUI* pParentContainer = static_cast<CContainerUI*>(GetParent()->GetInterface(_T("Container")));
			if (pParentContainer) {
				RECT rc = pParentContainer->GetPos();
				RECT rcPadding = pParentContainer->GetPadding();
				rc.left += rcPadding.left;
				rc.top += rcPadding.top;
				rc.right -= rcPadding.right;
				rc.bottom -= rcPadding.bottom;
				CScrollBarUI* pVerticalScrollBar = pParentContainer->GetVerticalScrollBar();
				if (pVerticalScrollBar && pVerticalScrollBar->IsVisible()) rc.right -= pVerticalScrollBar->GetFixedWidth();
				CScrollBarUI* pHorizontalScrollBar = pParentContainer->GetHorizontalScrollBar();
				if (pHorizontalScrollBar && pHorizontalScrollBar->IsVisible()) rc.bottom -= pHorizontalScrollBar->GetFixedHeight();

				RECT invalidateRc = m_rcItem;
				if (!::IntersectRect(&invalidateRc, &m_rcItem, &rc))
				{
					return;
				}

				CControlUI* pParent = GetParent();
				RECT rcTemp;
				RECT rcParent;
				while ( (pParent = pParent->GetParent()) )
				{
					rcTemp = invalidateRc;
					rcParent = pParent->GetPos();
					if (!::IntersectRect(&invalidateRc, &rcTemp, &rcParent))
					{
						return;
					}
				}

				if (m_pManager != NULL) m_pManager->Invalidate(invalidateRc);
			}
			else {
				CContainerUI::Invalidate();
			}
		}
		else {
			CContainerUI::Invalidate();
		}
	}

	bool CListContainerElementUI::Activate()
	{
		if (!CContainerUI::Activate()) return false;
		if (m_pManager != NULL) m_pManager->SendNotify(this, DUI_MSGTYPE_ITEMACTIVATE);
		return true;
	}

	bool CListContainerElementUI::IsSelected() const
	{
		return m_bSelected;
	}

	bool CListContainerElementUI::Select(bool bSelect)
	{
		if (!IsEnabled()) return false;
		// 取消其它列表项数据
		if (m_pOwner) {
			m_pOwner->UnSelectItem(m_iIndex, true);
		}
		if (bSelect == m_bSelected) return true;
		m_bSelected = bSelect;
		if (m_pOwner) {
			if (bSelect) m_pOwner->SelectItem(m_iIndex);
			else m_pOwner->UnSelectItem(m_iIndex);
		}
		Invalidate();

		return true;
	}

	bool CListContainerElementUI::SelectMulti(bool bSelect)
	{
		if (!IsEnabled()) return false;

		if (bSelect == m_bSelected) return true;
		m_bSelected = bSelect;

		Invalidate();
		return true;
	}

	bool CListContainerElementUI::IsExpanded() const
	{
		return false;
	}

	bool CListContainerElementUI::Expand(bool /*bExpand = true*/)
	{
		return false;
	}

	void CListContainerElementUI::DoEvent(TEventUI& event)
	{
		if (!IsMouseEnabled() && event.Type > UIEVENT__MOUSEBEGIN && event.Type < UIEVENT__MOUSEEND) {
			if (m_pOwner != NULL) m_pOwner->DoEvent(event);
			else CContainerUI::DoEvent(event);
			return;
		}

		if (event.Type == UIEVENT_DBLCLICK)
		{
			if (IsEnabled()) {
				Activate();
				Invalidate();
			}
			return;
		}
		if (event.Type == UIEVENT_KEYDOWN && IsEnabled())
		{
			if (event.chKey == VK_RETURN) {
				Activate();
				Invalidate();
				return;
			}
		}
		if (event.Type == UIEVENT_BUTTONDOWN)
		{
			if (IsEnabled()) {
				if(IsRichEvent()) m_pManager->SendNotify(this, DUI_MSGTYPE_BUTTONDOWN);

				// 多选
				if ((GetKeyState(VK_CONTROL) & 0x8000) || (GetKeyState(VK_SHIFT) & 0x8000)) {
					if (m_pOwner != NULL) m_pOwner->SelectMultiItem(m_iIndex);
				}
				else {
					Select(true);
				}
			}
			return;
		}
		// 右键选择
		if (m_pOwner != NULL)
		{
			if (m_pOwner->GetListInfo()->bRSelected && event.Type == UIEVENT_RBUTTONDOWN)
			{
				if (IsEnabled()) {
					// 多选
					if ((GetKeyState(VK_CONTROL) & 0x8000) || (GetKeyState(VK_SHIFT) & 0x8000)) {
						if (m_pOwner != NULL) m_pOwner->SelectMultiItem(m_iIndex);
					}
					else {
						Select(true);
					}
				}
				return;
			}
		}

		if (event.Type == UIEVENT_BUTTONUP)
		{
			if (IsEnabled()) {
				m_pManager->SendNotify(this, DUI_MSGTYPE_ITEMCLICK);
			}
			return;
		}
		if (event.Type == UIEVENT_MOUSEENTER)
		{
			if (IsEnabled()) {
				m_uButtonState |= UISTATE_HOT;
				Invalidate();
				if(IsRichEvent()) m_pManager->SendNotify(this, DUI_MSGTYPE_MOUSEENTER);
			}
			return;
		}
		if (event.Type == UIEVENT_MOUSELEAVE)
		{
			if ((m_uButtonState & UISTATE_HOT) != 0) {
				m_uButtonState &= ~UISTATE_HOT;
				Invalidate();
				if(IsRichEvent()) m_pManager->SendNotify(this, DUI_MSGTYPE_MOUSELEAVE);
			}
			return;
		}
		if (event.Type == UIEVENT_TIMER)
		{
			m_pManager->SendNotify(this, DUI_MSGTYPE_TIMER, event.wParam, event.lParam);
			return;
		}

		if (event.Type == UIEVENT_CONTEXTMENU)
		{
			if (IsContextMenuUsed()) {
				m_pManager->SendNotify(this, DUI_MSGTYPE_MENU, event.wParam, event.lParam);
				return;
			}
		}
		// An important twist: The list-item will send the event not to its immediate
		// parent but to the "attached" list. A list may actually embed several components
		// in its path to the item, but key-presses etc. needs to go to the actual list.
		if (m_pOwner != NULL) m_pOwner->DoEvent(event); else CControlUI::DoEvent(event);
	}


	void CListContainerElementUI::SetAttribute(LPCTSTR pstrName, LPCTSTR pstrValue)
	{
		if (_tcsicmp(pstrName, _T("selected")) == 0) Select();
		//else if( _tcscmp(pstrName, _T("expandable")) == 0 ) SetExpandable(_tcscmp(pstrValue, _T("true")) == 0);
		else CContainerUI::SetAttribute(pstrName, pstrValue);
	}

	bool CListContainerElementUI::DoPaint(IRenderContext& ctx, const RECT& rcPaint, CControlUI* pStopControl)
	{
		DrawItemBk(ctx, m_rcItem);
		return CContainerUI::DoPaint(ctx, rcPaint, pStopControl);
	}

	void CListContainerElementUI::DrawItemText(IRenderContext& ctx, const RECT& rcItem)
	{
		CDuiString sText = GetText();
		if (sText.IsEmpty()) return;

		if (m_pOwner == NULL) return;
		TListInfoUI* pInfo = m_pOwner->GetListInfo();
		DWORD iTextColor = pInfo->dwColor;
		if ((m_uButtonState & UISTATE_HOT) != 0) {
			iTextColor = pInfo->dwHoverColor;
		}
		if (IsSelected()) {
			iTextColor = pInfo->dwSelectedColor;
		}
		if (!IsEnabled()) {
			iTextColor = pInfo->dwDisabledColor;
		}
		int nLinks = 0;
		RECT rcText = rcItem;
		RECT rcTextPadding = GetManager()->GetDPIObj()->Scale(pInfo->rcTextPadding);
		rcText.left += rcTextPadding.left;
		rcText.right -= rcTextPadding.right;
		rcText.top += rcTextPadding.top;
		rcText.bottom -= rcTextPadding.bottom;


		if (pInfo->bShowHtml)
			ctx.DrawHtmlText(rcText, sText.GetData(), GetAdjustColor(iTextColor), \
			NULL, NULL, nLinks, pInfo->nFont, pInfo->uTextStyle);
		else
			ctx.DrawText(rcText, sText.GetData(), GetAdjustColor(iTextColor), \
			pInfo->nFont, pInfo->uTextStyle);
	}

	void CListContainerElementUI::DrawItemBk(IRenderContext& ctx, const RECT& rcItem)
	{
		ASSERT(m_pOwner);
		if (m_pOwner == NULL) return;


		TListInfoUI* pInfo = m_pOwner->GetListInfo();
		DWORD iBackColor = 0;
		if( pInfo->bAlternateBk && (m_iIndex % 2) == 1 ) {
			if( pInfo->dwAlternateBackgroundColor != 0 )
				iBackColor = pInfo->dwAlternateBackgroundColor;
		}
		else {
			iBackColor = pInfo->dwBackgroundColor;
		}

		CDuiString sForegroundImage = pInfo->sForegroundImage;
		if ((m_uButtonState & UISTATE_HOT) != 0 && pInfo->dwHoverBackgroundColor > 0) {
			iBackColor = pInfo->dwHoverBackgroundColor;
			if(!pInfo->sHoverForegroundImage.IsEmpty()) {
				sForegroundImage = pInfo->sHoverForegroundImage;
			}
		}
		if (IsSelected() && pInfo->dwSelectedBackgroundColor > 0) {
			iBackColor = pInfo->dwSelectedBackgroundColor;
			
		}
		if (!IsEnabled() && pInfo->dwDisabledBackgroundColor > 0) {
			iBackColor = pInfo->dwDisabledBackgroundColor;
		}
		if (iBackColor != 0) {
			ctx.DrawColor(m_rcItem, GetAdjustColor(iBackColor));
		}

		if (!IsEnabled()) {
			if (!pInfo->sDisabledImage.IsEmpty()) {
				if (!DrawImage(ctx, pInfo->sDisabledImage.GetData())) {}
				else return;
			}
		}
		if (IsSelected()) {
			if (!pInfo->sSelectedImage.IsEmpty()) {
				bool bDrawOk = DrawImage(ctx, pInfo->sSelectedImage.GetData());
				if(!pInfo->sSelectedForegroundImage.IsEmpty()) {
					DrawImage(ctx, pInfo->sSelectedForegroundImage.GetData());
				}
				if(bDrawOk) return;
			}
		}
		if ((m_uButtonState & UISTATE_HOT) != 0) {
			if (!pInfo->sHoverImage.IsEmpty()) {
				bool bDrawOk = DrawImage(ctx, pInfo->sHoverImage.GetData());
				if(!pInfo->sHoverForegroundImage.IsEmpty()) {
					DrawImage(ctx, pInfo->sHoverForegroundImage.GetData());
				}
				if(bDrawOk) return;
			}
		}

		if (!m_sBackgroundImage.IsEmpty()) {
			if (!pInfo->bAlternateBk || m_iIndex % 2 == 0) {
				if (!DrawImage(ctx, m_sBackgroundImage.GetData())) {}

				if(!pInfo->sForegroundImage.IsEmpty()) {
					DrawImage(ctx, pInfo->sForegroundImage.GetData());
				}
			}
		}

		if (m_sBackgroundImage.IsEmpty()) {
			if (!pInfo->sBkImage.IsEmpty()) {
				DrawImage(ctx, pInfo->sBkImage.GetData());
				if(!pInfo->sForegroundImage.IsEmpty()) {
					DrawImage(ctx, pInfo->sForegroundImage.GetData());
				}
			}
		}

		if (pInfo->dwLineColor != 0) {
			if (pInfo->bShowRowLine) {
				RECT rcLine = { m_rcItem.left, m_rcItem.bottom - 1, m_rcItem.right, m_rcItem.bottom - 1 };
				ctx.DrawLine(rcLine, 1, GetAdjustColor(pInfo->dwLineColor));
			}
			if (pInfo->bShowColumnLine) {
				for (int i = 0; i < pInfo->nColumns; i++) {
					RECT rcLine = { pInfo->rcColumn[i].right - 1, m_rcItem.top, pInfo->rcColumn[i].right - 1, m_rcItem.bottom };
					ctx.DrawLine(rcLine, 1, GetAdjustColor(pInfo->dwLineColor));
				}
			}
		}
	}

	void CListContainerElementUI::SetPos(RECT rc, bool bNeedInvalidate)
	{
		if (m_pOwner == NULL) return;

		UINT uListType = m_pOwner->GetListType();
		if (uListType == LT_LIST) {
			int nFixedWidth = GetFixedWidth();
			if (nFixedWidth > 0)
			{
				int nRank = (rc.right - rc.left) / nFixedWidth;
				if (nRank > 0)
				{
					int nIndex = GetIndex();
					int nfloor = nIndex / nRank;
					int nHeight = rc.bottom - rc.top;

					rc.top = rc.top - nHeight * (nIndex - nfloor);
					rc.left = rc.left + nFixedWidth * (nIndex % nRank);
					rc.right = rc.left + nFixedWidth;
					rc.bottom = nHeight + rc.top;
				}
			}
		}
		CHorizontalLayoutUI::SetPos(rc, bNeedInvalidate);

		if (uListType != LT_LIST && uListType != LT_TREE) return;
		CListUI* pList = static_cast<CListUI*>(m_pOwner);
		if (uListType == LT_TREE)
		{
			pList = (CListUI*)pList->CControlUI::GetInterface(_T("List"));
			if (pList == NULL) return;
		}

		CListHeaderUI *pHeader = pList->GetHeader();
		if (pHeader == NULL || !pHeader->IsVisible()) return;
		int nCount = m_items.GetSize();
		for (int i = 0; i < nCount; i++)
		{
			CControlUI *pListItem = static_cast<CControlUI*>(m_items[i]);
			CControlUI *pHeaderItem = pHeader->GetItemAt(i);
			if (pHeaderItem == NULL) return;
			RECT rcHeaderItem = pHeaderItem->GetPos();
			if (pListItem != NULL && !(rcHeaderItem.left == 0 && rcHeaderItem.right == 0))
			{
				RECT rt = pListItem->GetPos();
				rt.left = rcHeaderItem.left;
				rt.right = rcHeaderItem.right;
				pListItem->SetPos(rt);
			}
		}
	}
} // namespace DuiLib
