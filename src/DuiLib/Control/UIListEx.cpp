#include "stdafx.h"
#include "UIListEx.h"

namespace DuiLib {

	/////////////////////////////////////////////////////////////////////////////////////
	//
	//
	IMPLEMENT_DUICONTROL(CListExUI)

	CListExUI::CListExUI() : m_pEditUI(NULL), m_pComboBoxUI(NULL), m_bAddMessageFilter(FALSE),m_nRow(-1),m_nColum(-1),m_pXCallback(NULL)
	{
	}

	LPCTSTR CListExUI::GetClass() const
	{
		return _T("XListUI");
	}

	UINT CListExUI::GetControlFlags() const
	{
		return UIFLAG_TABSTOP;
	}

	LPVOID CListExUI::GetInterface(LPCTSTR pstrName)
	{
		if( _tcsicmp(pstrName, _T("ListEx")) == 0 ) return static_cast<IListOwnerUI*>(this);
		return CListUI::GetInterface(pstrName);
	}
	BOOL CListExUI::CheckColumEditable(int nColum)
	{
		CListContainerHeaderItemUI* pHItem = static_cast<CListContainerHeaderItemUI*>(m_pHeader->GetItemAt(nColum));
		return pHItem != NULL? pHItem->GetColumeEditable() : FALSE;
	}
	void CListExUI::InitListCtrl()
	{
		if (!m_bAddMessageFilter)
		{
			GetManager()->AddNotifier(this);
			m_bAddMessageFilter = TRUE;
		}
	}
	CRichEditUI* CListExUI::GetEditUI()
	{
		if (m_pEditUI == NULL)
		{
			m_pEditUI = new CRichEditUI;
			m_pEditUI->SetName(_T("ListEx_Edit"));
			LPCTSTR pDefaultAttributes = GetManager()->GetDefaultAttributeList(_T("RichEdit"));
			if( pDefaultAttributes ) {
				m_pEditUI->ApplyAttributeList(pDefaultAttributes);
			}
			m_pEditUI->SetBackgroundColor(0xFFFFFFFF);
			m_pEditUI->SetRich(false);
			m_pEditUI->SetMultiLine(false);
			m_pEditUI->SetWantReturn(true);
			m_pEditUI->SetAbsolute(true);
			m_pEditUI->SetAttribute(_T("auto-hscroll"), _T("true"));
			Add(m_pEditUI);
		}
		if (m_pComboBoxUI)
		{
			RECT rc = {0,0,0,0};
			m_pComboBoxUI->SetPos(rc);
		}

		return m_pEditUI;
	}

	BOOL CListExUI::CheckColumComboBoxable(int nColum)
	{
		CListContainerHeaderItemUI* pHItem = static_cast<CListContainerHeaderItemUI*>(m_pHeader->GetItemAt(nColum));
		return pHItem != NULL? pHItem->GetColumeComboable() : FALSE;
	}

	CComboBoxUI* CListExUI::GetComboBoxUI()
	{
		if (m_pComboBoxUI == NULL)
		{
			m_pComboBoxUI = new CComboBoxUI;
			m_pComboBoxUI->SetName(_T("ListEx_Combo"));
			LPCTSTR pDefaultAttributes = GetManager()->GetDefaultAttributeList(_T("Combo"));
			if( pDefaultAttributes ) {
				m_pComboBoxUI->ApplyAttributeList(pDefaultAttributes);
			}

			Add(m_pComboBoxUI);
		}
		if (m_pEditUI)
		{
			RECT rc = {0,0,0,0};
			m_pEditUI->SetPos(rc);
		}

		return m_pComboBoxUI;
	}

	BOOL CListExUI::CheckColumCheckBoxable(int nColum)
	{
		CControlUI* p = m_pHeader->GetItemAt(nColum);
		CListContainerHeaderItemUI* pHItem = static_cast<CListContainerHeaderItemUI*>(p->GetInterface(_T("ListContainerHeaderItem")));
		return pHItem != NULL? pHItem->GetColumeCheckable() : FALSE;
	}

	void CListExUI::Notify(TNotifyUI& msg)
	{	
		CDuiString strName = msg.pSender->GetName();

		//复选框
		if(_tcsicmp(msg.sType.GetData(), _T("listheaditemchecked")) == 0)
		{
			BOOL bCheck = (BOOL)msg.lParam;
			//判断是否是本LIST发送的notify
			CListHeaderUI* pHeader = GetHeader();
			for (int i = 0; i < pHeader->GetCount(); i++)
			{
				if (pHeader->GetItemAt(i) == msg.pSender)
				{
					for (int i = 0; i < GetCount(); ++i) {
						CControlUI* p = GetItemAt(i);
						CListTextExtElementUI* pLItem = static_cast<CListTextExtElementUI*>(p->GetInterface(_T("ListTextExElement")));
						if (pLItem != NULL) {
							pLItem->SetCheck(bCheck);
						}
					}
					break;
				}
			}
		}
		else if (_tcsicmp(msg.sType.GetData(), DUI_MSGTYPE_LISTITEMCHECKED) == 0)
		{
			for (int i = 0; i < GetCount(); ++i) {
				CControlUI* p = GetItemAt(i);
				CListTextExtElementUI* pLItem = static_cast<CListTextExtElementUI*>(p->GetInterface(_T("ListTextExElement")));
				if (pLItem != NULL && pLItem == msg.pSender)
				{
					OnListItemChecked(LOWORD(msg.wParam), HIWORD(msg.wParam), msg.lParam);
					break;
				}
			}
		}

		//编辑框、组合框
		if (_tcsicmp(strName.GetData(), _T("ListEx_Edit")) == 0 && m_pEditUI && m_nRow >= 0 && m_nColum >= 0)
		{
			if(_tcsicmp(msg.sType.GetData(), DUI_MSGTYPE_SETFOCUS) == 0)
			{

			}
			else if(_tcsicmp(msg.sType.GetData(), DUI_MSGTYPE_KILLFOCUS) == 0)
			{
				CDuiString sText = m_pEditUI->GetText();
				CListTextExtElementUI* pRowCtrl = (CListTextExtElementUI*)GetItemAt(m_nRow);
				if (pRowCtrl)
				{
					pRowCtrl->SetText(m_nColum, sText.GetData());
				}

				//重置当前行列
				SetEditRowAndColum(-1, -1);

				//隐藏编辑框
				RECT rc = {0,0,0,0};
				m_pEditUI->SetPos(rc);
				m_pEditUI->SetVisible(false);
			}
		}
		else if (_tcsicmp(strName.GetData(), _T("ListEx_Combo")) == 0 && m_pComboBoxUI && m_nRow >= 0 && m_nColum >= 0)
		{
			int  iCurSel, iOldSel;
			iCurSel = msg.wParam;
			iOldSel = msg.lParam;

			if(_tcsicmp(msg.sType.GetData(), DUI_MSGTYPE_SETFOCUS) == 0)
			{

			}
			else if(_tcsicmp(msg.sType.GetData(), DUI_MSGTYPE_KILLFOCUS) == 0)
			{
			}
			else if(_tcsicmp(msg.sType.GetData(), DUI_MSGTYPE_LISTITEMSELECT) == 0 && iOldSel >= 0)
			{
				CListTextExtElementUI* pRowCtrl = (CListTextExtElementUI*)GetItemAt(m_nRow);
				if (pRowCtrl)
				{
					pRowCtrl->SetText(m_nColum, m_pComboBoxUI->GetText().GetData());
				}

				//隐藏组合框
				RECT rc = {0,0,0,0};
				m_pComboBoxUI->SetPos(rc);
			}
		}
		else if(_tcsicmp(msg.sType.GetData(), _T("scroll")) == 0 && (m_pComboBoxUI || m_pEditUI) && m_nRow >= 0 && m_nColum >= 0)
		{
			HideEditAndComboCtrl();
		}
	}
	void CListExUI::HideEditAndComboCtrl()
	{
		//隐藏编辑框
		RECT rc = {0,0,0,0};
		if(m_pEditUI)
		{	
			m_pEditUI->SetPos(rc);

			m_pEditUI->SetVisible(false);
		}

		if(m_pComboBoxUI)
		{	
			m_pComboBoxUI->SetPos(rc);
		}
	}
	IListComboCallbackUI* CListExUI::GetTextArrayCallback() const
	{
		return m_pXCallback;
	}

	void CListExUI::SetTextArrayCallback(IListComboCallbackUI* pCallback)
	{
		m_pXCallback = pCallback;
	}
	void CListExUI::OnListItemClicked(int nIndex, int nColum, RECT* lpRCColum, LPCTSTR lpstrText)
	{
		RECT rc = {0,0,0,0};
		if (nColum < 0)
		{
			if (m_pEditUI)
			{
				m_pEditUI->SetPos(rc);

				m_pEditUI->SetVisible(false);
			}
			if (m_pComboBoxUI)
			{
				m_pComboBoxUI->SetPos(rc);
			}
		}
		else
		{
			if (CheckColumEditable(nColum) && GetEditUI())
			{
				//保存当前行列
				SetEditRowAndColum(nIndex, nColum);
				
				m_pEditUI->SetVisible(true);
				//移动位置
				m_pEditUI->SetFixedWidth(lpRCColum->right - lpRCColum->left);
				m_pEditUI->SetFixedHeight(lpRCColum->bottom - lpRCColum->top);
				m_pEditUI->SetFixedXY(CDuiSize(lpRCColum->left,lpRCColum->top));
				SIZE szTextSize = RenderMeasureTextSize(m_pManager, _T("TTT"), m_ListInfo.nFont, DT_CALCRECT | DT_SINGLELINE);
				m_pEditUI->SetTextPadding(CDuiRect(2, (lpRCColum->bottom - lpRCColum->top - szTextSize.cy) / 2, 2, 0));
				//设置文字
				m_pEditUI->SetText(lpstrText);

				m_pEditUI->SetFocus();
			}
			else if(CheckColumComboBoxable(nColum) && GetComboBoxUI())
			{
				//重置组合框
				m_pComboBoxUI->RemoveAll();

				//保存当前行列
				SetEditRowAndColum(nIndex, nColum);

				//设置文字
				m_pComboBoxUI->SetText(lpstrText);

				//获取
				if (m_pXCallback)
				{
					m_pXCallback->GetItemComboTextArray(m_pComboBoxUI, nIndex, nColum);
				}

				//移动位置
				m_pComboBoxUI->SetPos(*lpRCColum);
				m_pComboBoxUI->SetVisible(TRUE);
			}
			else
			{
				if (m_pEditUI)
				{
					m_pEditUI->SetPos(rc);

					m_pEditUI->SetVisible(false);
				}
				if (m_pComboBoxUI)
				{
					m_pComboBoxUI->SetPos(rc);
				}
			}
		}
	}
	void CListExUI::OnListItemChecked(int nIndex, int nColum, BOOL bChecked)
	{
		CControlUI* p = m_pHeader->GetItemAt(nColum);
		CListContainerHeaderItemUI* pHItem = static_cast<CListContainerHeaderItemUI*>(p->GetInterface(_T("ListContainerHeaderItem")));
		if (pHItem == NULL)
		{
			return;
		}

		//如果选中，那么检查是否全部都处于选中状态
		if (bChecked)
		{
			BOOL bCheckAll = TRUE;
			for(int i = 0; i < GetCount(); i++) 
			{
				CControlUI* p = GetItemAt(i);
				CListTextExtElementUI* pLItem = static_cast<CListTextExtElementUI*>(p->GetInterface(_T("ListTextExElement")));
				if( pLItem != NULL && !pLItem->GetCheck()) 
				{
					bCheckAll = FALSE;
					break;
				}
			}
			if (bCheckAll)
			{
				pHItem->SetCheck(TRUE);
			}
			else
			{
				pHItem->SetCheck(FALSE);
			}
		}
		else
		{
			pHItem->SetCheck(FALSE);
		}
	}
	void CListExUI::DoEvent(TEventUI& event)
	{
		if (event.Type == UIEVENT_BUTTONDOWN || event.Type == UIEVENT_SCROLLWHEEL)
		{
			HideEditAndComboCtrl();
		}

		CListUI::DoEvent(event);
	}
	void CListExUI::SetColumItemColor(int nIndex, int nColum, DWORD iBKColor)
	{
		CControlUI* p = GetItemAt(nIndex);
		CListTextExtElementUI* pLItem = static_cast<CListTextExtElementUI*>(p->GetInterface(_T("ListTextExElement")));
		if( pLItem != NULL) 
		{
			DWORD iTextBkColor = iBKColor;
			pLItem->SetColumItemColor(nColum, iTextBkColor);
		}
	}

	BOOL CListExUI::GetColumItemColor(int nIndex, int nColum, DWORD& iBKColor)
	{
		CControlUI* p = GetItemAt(nIndex);
		CListTextExtElementUI* pLItem = static_cast<CListTextExtElementUI*>(p->GetInterface(_T("ListTextExElement")));
		if( pLItem == NULL) 
		{
			return FALSE;
		}
		pLItem->GetColumItemColor(nColum, iBKColor);
		return TRUE;
	}

	/////////////////////////////////////////////////////////////////////////////////////
	//
	//
	IMPLEMENT_DUICONTROL(CListContainerHeaderItemUI)

	CListContainerHeaderItemUI::CListContainerHeaderItemUI() : m_bDragable(TRUE), m_uButtonState(0), m_iSepWidth(4),
		m_uTextStyle(DT_VCENTER | DT_CENTER | DT_SINGLELINE), m_dwColor(0), m_iFont(-1), m_bShowHtml(FALSE),
		m_bEditable(FALSE),m_bComboable(FALSE),m_bCheckBoxable(FALSE),m_uCheckBoxState(0),m_bChecked(FALSE),m_pOwner(NULL)
	{
		SetPadding(CDuiBox(0, 2, 0, 2));
		::ZeroMemory(&m_rcTextPadding, sizeof(m_rcTextPadding));
		m_cxyCheckBox.cx = m_cxyCheckBox.cy = 0;
		ptLastMouse.x = ptLastMouse.y = 0;
		SetMinWidth(16);
	}

	LPCTSTR CListContainerHeaderItemUI::GetClass() const
	{
		return _T("ListContainerHeaderItemUI");
	}

	LPVOID CListContainerHeaderItemUI::GetInterface(LPCTSTR pstrName)
	{
		if( _tcsicmp(pstrName, _T("ListContainerHeaderItem")) == 0 ) return this;
		return CContainerUI::GetInterface(pstrName);
	}

	UINT CListContainerHeaderItemUI::GetControlFlags() const
	{
		if( IsEnabled() && IsColumnResizeEnabled() ) return UIFLAG_SETCURSOR;
		else return 0;
	}

	BOOL CListContainerHeaderItemUI::IsColumnResizeEnabled() const
	{
		if( !m_bDragable || m_iSepWidth == 0 ) return FALSE;
		CControlUI* pHdr = GetParent();
		if( pHdr == NULL || pHdr->GetParent() == NULL ) return TRUE;
		CListUI* pList = static_cast<CListUI*>(pHdr->GetParent()->GetInterface(DUI_CTR_LIST));
		if( pList == NULL ) return TRUE;
		return pList->IsHeaderShowColumnLine() ? TRUE : FALSE;
	}

	void CListContainerHeaderItemUI::SetEnabled(BOOL bEnable)
	{
		CContainerUI::SetEnabled(bEnable);
		if( !IsEnabled() ) {
			m_uButtonState = 0;
		}
	}

	BOOL CListContainerHeaderItemUI::IsDragable() const
	{
		return m_bDragable;
	}

	void CListContainerHeaderItemUI::SetDragable(BOOL bDragable)
	{
		m_bDragable = bDragable;
		if ( !m_bDragable ) m_uButtonState &= ~UISTATE_CAPTURED;
	}

	DWORD CListContainerHeaderItemUI::GetSepWidth() const
	{
		if(m_pManager != NULL) m_pManager->GetDPIObj()->Scale(m_iSepWidth);
		return m_iSepWidth;
	}

	void CListContainerHeaderItemUI::SetSepWidth(int iWidth)
	{
		m_iSepWidth = iWidth;
	}

	DWORD CListContainerHeaderItemUI::GetTextStyle() const
	{
		return m_uTextStyle;
	}

	void CListContainerHeaderItemUI::SetTextStyle(UINT uStyle)
	{
		m_uTextStyle = uStyle;
		Invalidate();
	}

	DWORD CListContainerHeaderItemUI::GetColor() const
	{
		return m_dwColor;
	}


	void CListContainerHeaderItemUI::SetColor(DWORD dwColor)
	{
		m_dwColor = dwColor;
	}

	RECT CListContainerHeaderItemUI::GetTextPadding() const
	{
		RECT rcTextPadding = m_rcTextPadding;
		if(m_pManager != NULL) m_pManager->GetDPIObj()->Scale(&rcTextPadding);
		return rcTextPadding;
	}

	void CListContainerHeaderItemUI::SetTextPadding(RECT rc)
	{
		m_rcTextPadding = rc;
		Invalidate();
	}

	void CListContainerHeaderItemUI::SetFont(int index)
	{
		m_iFont = index;
	}

	BOOL CListContainerHeaderItemUI::IsShowHtml()
	{
		return m_bShowHtml;
	}

	void CListContainerHeaderItemUI::SetShowHtml(BOOL bShowHtml)
	{
		if( m_bShowHtml == bShowHtml ) return;

		m_bShowHtml = bShowHtml;
		Invalidate();
	}

	LPCTSTR CListContainerHeaderItemUI::GetImage() const
	{
		return m_sImage.GetData();
	}

	void CListContainerHeaderItemUI::SetImage(LPCTSTR pStrImage)
	{
		m_sImage = pStrImage;
		Invalidate();
	}

	LPCTSTR CListContainerHeaderItemUI::GetHoverImage() const
	{
		return m_sHoverImage.GetData();
	}

	void CListContainerHeaderItemUI::SetHoverImage(LPCTSTR pStrImage)
	{
		m_sHoverImage = pStrImage;
		Invalidate();
	}

	LPCTSTR CListContainerHeaderItemUI::GetActiveImage() const
	{
		return m_sActiveImage.GetData();
	}

	void CListContainerHeaderItemUI::SetActiveImage(LPCTSTR pStrImage)
	{
		m_sActiveImage = pStrImage;
		Invalidate();
	}

	LPCTSTR CListContainerHeaderItemUI::GetFocusImage() const
	{
		return m_sFocusImage.GetData();
	}

	void CListContainerHeaderItemUI::SetFocusImage(LPCTSTR pStrImage)
	{
		m_sFocusImage = pStrImage;
		Invalidate();
	}

	LPCTSTR CListContainerHeaderItemUI::GetSepImage() const
	{
		return m_sSepImage.GetData();
	}

	void CListContainerHeaderItemUI::SetSepImage(LPCTSTR pStrImage)
	{
		m_sSepImage = pStrImage;
		Invalidate();
	}

	void CListContainerHeaderItemUI::SetAttribute(LPCTSTR pstrName, LPCTSTR pstrValue)
	{
		if( _tcsicmp(pstrName, _T("draggable")) == 0 ) SetDragable(_tcsicmp(pstrValue, _T("true")) == 0);
		else if( _tcsicmp(pstrName, _T("sep-width")) == 0 ) SetSepWidth(_ttoi(pstrValue));
		else if( _tcsicmp(pstrName, _T("text-align")) == 0 ) 
		{
			if( _tcsstr(pstrValue, _T("left")) != NULL ) {
				m_uTextStyle &= ~(DT_CENTER | DT_RIGHT);
				m_uTextStyle |= DT_LEFT;
			}
			if( _tcsstr(pstrValue, _T("center")) != NULL ) {
				m_uTextStyle &= ~(DT_LEFT | DT_RIGHT);
				m_uTextStyle |= DT_CENTER;
			}
			if( _tcsstr(pstrValue, _T("right")) != NULL ) {
				m_uTextStyle &= ~(DT_LEFT | DT_CENTER);
				m_uTextStyle |= DT_RIGHT;
			}
			Invalidate();
		}
		else if( _tcsicmp(pstrName, _T("vertical-align")) == 0 ) {
			if( _tcsstr(pstrValue, _T("top")) != NULL ) {
				m_uTextStyle &= ~(DT_VCENTER | DT_BOTTOM);
				m_uTextStyle |= DT_TOP;
			}
			else if( _tcsstr(pstrValue, _T("bottom")) != NULL ) {
				m_uTextStyle &= ~(DT_TOP | DT_VCENTER);
				m_uTextStyle |= DT_BOTTOM;
			}
			else {
				m_uTextStyle &= ~(DT_TOP | DT_BOTTOM | DT_WORDBREAK);
				m_uTextStyle |= (DT_VCENTER | DT_SINGLELINE);
			}
			Invalidate();
		}
		else if( _tcsicmp(pstrName, _T("text-overflow")) == 0 ) 
		{
			if( _tcsicmp(pstrValue, _T("ellipsis")) == 0 ) m_uTextStyle |= DT_END_ELLIPSIS;
			else m_uTextStyle &= ~DT_END_ELLIPSIS;
		}    
		else if( _tcsicmp(pstrName, _T("font-family")) == 0 || _tcsicmp(pstrName, _T("font-size")) == 0 ) {
			CDuiString sFamily;
			int nSize = 0;
			if( _tcsicmp(pstrName, _T("font-family")) == 0 ) sFamily = pstrValue ? pstrValue : _T("");
			else {
				LPTSTR pEnd = NULL;
				long v = _tcstol(pstrValue, &pEnd, 10);
				if( pEnd != pstrValue && v > 0 ) nSize = (int)v;
			}
			if( m_pManager != NULL ) {
				TFontInfo* pInfo = m_pManager->GetFontInfo(m_iFont);
				if( pInfo == NULL ) pInfo = m_pManager->GetDefaultFontInfo();
				if( pInfo != NULL ) {
					if( sFamily.IsEmpty() ) sFamily = pInfo->sFontName;
					if( nSize <= 0 ) nSize = pInfo->iSize;
				}
				if( sFamily.IsEmpty() ) sFamily = _T("Microsoft YaHei UI");
				if( nSize <= 0 ) nSize = 12;
				int id = m_pManager->EnsureFont(sFamily.GetData(), nSize, false, false, false, false);
				if( id >= 0 ) SetFont(id);
			}
		}
		else if( _tcsicmp(pstrName, _T("color")) == 0 ) 
		{
			DWORD clrColor = 0;
			if( !ParseColorString(pstrValue, clrColor) ) clrColor = 0;
			SetColor(clrColor);
		}
		else if( _tcsicmp(pstrName, _T("showhtml")) == 0 ) SetShowHtml(_tcsicmp(pstrValue, _T("true")) == 0);
		else if( _tcsicmp(pstrName, _T("image")) == 0 ) SetImage(pstrValue);
		else if( _tcsicmp(pstrName, _T("image-hover")) == 0 ) SetHoverImage(pstrValue);
		else if( _tcsicmp(pstrName, _T("image-active")) == 0 ) SetActiveImage(pstrValue);
		else if( _tcsicmp(pstrName, _T("image-focus")) == 0 ) SetFocusImage(pstrValue);
		else if( _tcsicmp(pstrName, _T("sep-image")) == 0 ) SetSepImage(pstrValue);

		else if( _tcsicmp(pstrName, _T("editable")) == 0 ) SetColumeEditable(_tcsicmp(pstrValue, _T("true")) == 0);
		else if( _tcsicmp(pstrName, _T("comboable")) == 0 ) SetColumeComboable(_tcsicmp(pstrValue, _T("true")) == 0);
		else if( _tcsicmp(pstrName, _T("checkable")) == 0 ) SetColumeCheckable(_tcsicmp(pstrValue, _T("true")) == 0);
		else if( _tcsicmp(pstrName, _T("checkbox-width")) == 0 ) SetCheckBoxWidth(_ttoi(pstrValue));
		else if( _tcsicmp(pstrName, _T("checkbox-height")) == 0 ) SetCheckBoxHeight(_ttoi(pstrValue));
		else if( _tcsicmp(pstrName, _T("checkbox-image")) == 0 ) SetCheckBoxNormalImage(pstrValue);
		else if( _tcsicmp(pstrName, _T("checkbox-image-hover")) == 0 ) SetCheckBoxHoverImage(pstrValue);
		else if( _tcsicmp(pstrName, _T("checkbox-image-active")) == 0 ) SetCheckBoxActiveImage(pstrValue);
		else if( _tcsicmp(pstrName, _T("checkbox-image-focus")) == 0 ) SetCheckBoxFocusedImage(pstrValue);
		else if( _tcsicmp(pstrName, _T("checkbox-image-disabled")) == 0 ) SetCheckBoxDisabledImage(pstrValue);
		else if( _tcsicmp(pstrName, _T("checkbox-image-selected")) == 0 ) SetCheckBoxSelectedImage(pstrValue);
		else if( _tcsicmp(pstrName, _T("checkbox-foreground-image")) == 0 ) SetCheckBoxForegroundImage(pstrValue);

		else CContainerUI::SetAttribute(pstrName, pstrValue);
	}

	void CListContainerHeaderItemUI::DoEvent(TEventUI& event)
	{
		if( !IsMouseEnabled() && event.Type > UIEVENT__MOUSEBEGIN && event.Type < UIEVENT__MOUSEEND ) {
			if( !BubbleEvent(event) ) CContainerUI::DoEvent(event);
			return;
		}

		//CheckBoxAble
		if (m_bCheckBoxable)
		{
			RECT rcCheckBox;
			GetCheckBoxRect(rcCheckBox);

			if( event.Type == UIEVENT_BUTTONDOWN || event.Type == UIEVENT_DBLCLICK )
			{
				if( ::PtInRect(&rcCheckBox, event.ptMouse)) 
				{
					m_uCheckBoxState |= UISTATE_PUSHED | UISTATE_CAPTURED;
					Invalidate();
				}
			}
			else if( event.Type == UIEVENT_MOUSEMOVE )
			{
				if( (m_uCheckBoxState & UISTATE_CAPTURED) != 0 ) 
				{
					if( ::PtInRect(&rcCheckBox, event.ptMouse) ) 
						m_uCheckBoxState |= UISTATE_PUSHED;
					else 
						m_uCheckBoxState &= ~UISTATE_PUSHED;
					Invalidate();
				}
				else if (::PtInRect(&rcCheckBox, event.ptMouse))
				{
					m_uCheckBoxState |= UISTATE_HOT;
					Invalidate();
				}
				else
				{
					m_uCheckBoxState &= ~UISTATE_HOT;
					Invalidate();
				}
			}
			else if( event.Type == UIEVENT_BUTTONUP )
			{
				if( (m_uCheckBoxState & UISTATE_CAPTURED) != 0 )
				{
					if( ::PtInRect(&rcCheckBox, event.ptMouse) ) 
					{
						SetCheck(!GetCheck());
						CContainerUI* pOwner = (CContainerUI*)m_pParent;
						if (pOwner)
						{
							m_pManager->SendNotify(this, DUI_MSGTYPE_LISTHEADITEMCHECKED, pOwner->GetItemIndex(this), m_bChecked);
						}

					}
					m_uCheckBoxState &= ~(UISTATE_PUSHED | UISTATE_CAPTURED);
					Invalidate();
				}
				else if (::PtInRect(&rcCheckBox, event.ptMouse))
				{

				}
			}
			else if( event.Type == UIEVENT_MOUSEENTER )
			{
				if( ::PtInRect(&rcCheckBox, event.ptMouse) ) 
				{
					m_uCheckBoxState |= UISTATE_HOT;
					Invalidate();
				}
			}
			else if( event.Type == UIEVENT_MOUSELEAVE )
			{
				m_uCheckBoxState &= ~UISTATE_HOT;
				Invalidate();
			}
		}

		if( event.Type == UIEVENT_SETFOCUS ) 
		{
			Invalidate();
		}
		if( event.Type == UIEVENT_KILLFOCUS ) 
		{
			Invalidate();
		}
		if( event.Type == UIEVENT_BUTTONDOWN || event.Type == UIEVENT_DBLCLICK )
		{
			if( !IsEnabled() ) return;
			RECT rcSeparator = GetThumbRect();
			if (m_iSepWidth>=0)
				rcSeparator.left-=4;
			else
				rcSeparator.right+=4;
			if( ::PtInRect(&rcSeparator, event.ptMouse) ) {
				if( IsColumnResizeEnabled() ) {
					m_uButtonState |= UISTATE_CAPTURED;
					ptLastMouse = event.ptMouse;
				}
			}
			else {
				m_uButtonState |= UISTATE_PUSHED;
				m_pManager->SendNotify(this, DUI_MSGTYPE_LISTHEADERCLICK);
				Invalidate();
			}
			return;
		}
		if( event.Type == UIEVENT_BUTTONUP )
		{
			if( (m_uButtonState & UISTATE_CAPTURED) != 0 ) {
				m_uButtonState &= ~UISTATE_CAPTURED;
				if( GetParent() ) 
					GetParent()->NeedParentUpdate();
			}
			else if( (m_uButtonState & UISTATE_PUSHED) != 0 ) {
				m_uButtonState &= ~UISTATE_PUSHED;
				Invalidate();
			}
			return;
		}
		if( event.Type == UIEVENT_MOUSEMOVE )
		{
			if( (m_uButtonState & UISTATE_CAPTURED) != 0 ) {
				RECT rc = m_rcItem;
				if( m_iSepWidth >= 0 ) {
					rc.right -= ptLastMouse.x - event.ptMouse.x;
				}
				else {
					rc.left -= ptLastMouse.x - event.ptMouse.x;
				}

				if( rc.right - rc.left > GetMinWidth() ) {
					int cx = rc.right - rc.left;
					SetAutoCalcWidth(false);
					m_fWidthPercent = 0.0f;
					m_cxyFixed.cx = cx;
					ptLastMouse = event.ptMouse;
					if( GetParent() ) 
						GetParent()->NeedParentUpdate();
				}
			}
			return;
		}
		if( event.Type == UIEVENT_SETCURSOR )
		{
			RECT rcSeparator = GetThumbRect();
			if (m_iSepWidth>=0)
				rcSeparator.left-=4;
			else
				rcSeparator.right+=4;
			if( IsEnabled() && IsColumnResizeEnabled() && ::PtInRect(&rcSeparator, event.ptMouse) ) {
				::SetCursor(::LoadCursor(NULL, MAKEINTRESOURCE(IDC_SIZEWE)));
				return;
			}
		}
		if( event.Type == UIEVENT_MOUSEENTER )
		{
			if( IsEnabled() ) {
				m_uButtonState |= UISTATE_HOT;
				Invalidate();
			}
			return;
		}
		if( event.Type == UIEVENT_MOUSELEAVE )
		{
			if( IsEnabled() ) {
				m_uButtonState &= ~UISTATE_HOT;
				Invalidate();
			}
			return;
		}
		CContainerUI::DoEvent(event);
	}

	SIZE CListContainerHeaderItemUI::EstimateSize(SIZE szAvailable)
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
					SIZE szContent = MeasureContent(szAvailable);
					cx = (szContent.cx > 0 ? szContent.cx : 0) + padLR + sep;
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
		if( cy <= 0 ) {
			if( m_pManager != NULL )
				cy = m_pManager->GetDefaultFontInfo()->tm.tmHeight + 14;
			else
				cy = 28;
		}
		return CDuiSize(cx, cy);
	}

	RECT CListContainerHeaderItemUI::GetThumbRect() const
	{
		if( m_iSepWidth >= 0 ) return CDuiRect(m_rcItem.right - m_iSepWidth, m_rcItem.top, m_rcItem.right, m_rcItem.bottom);
		else return CDuiRect(m_rcItem.left, m_rcItem.top, m_rcItem.left - m_iSepWidth, m_rcItem.bottom);
	}

	void CListContainerHeaderItemUI::PaintStatusImage(IRenderContext& ctx)
	{
		//HeadItem Bkgnd
		if( IsFocused() ) m_uButtonState |= UISTATE_FOCUSED;
		else m_uButtonState &= ~ UISTATE_FOCUSED;

		if( (m_uButtonState & UISTATE_PUSHED) != 0 ) {
			if( m_sActiveImage.IsEmpty() && !m_sImage.IsEmpty() ) DrawImage(ctx, m_sImage.GetData());
			if( !DrawImage(ctx, m_sActiveImage.GetData()) ) {}
		}
		else if( (m_uButtonState & UISTATE_HOT) != 0 ) {
			if( m_sHoverImage.IsEmpty() && !m_sImage.IsEmpty() ) DrawImage(ctx, m_sImage.GetData());
			if( !DrawImage(ctx, m_sHoverImage.GetData()) ) {}
		}
		else if( (m_uButtonState & UISTATE_FOCUSED) != 0 ) {
			if( m_sFocusImage.IsEmpty() && !m_sImage.IsEmpty() ) DrawImage(ctx, m_sImage.GetData());
			if( !DrawImage(ctx, m_sFocusImage.GetData()) ) {}
		}
		else {
			if( !m_sImage.IsEmpty() ) {
				if( !DrawImage(ctx, m_sImage.GetData()) ) {}
			}
		}

		if( !m_sSepImage.IsEmpty() ) {
			RECT rcThumb = GetThumbRect();
			rcThumb.left -= m_rcItem.left;
			rcThumb.top -= m_rcItem.top;
			rcThumb.right -= m_rcItem.left;
			rcThumb.bottom -= m_rcItem.top;

			m_sSepImageModify.Empty();
			m_sSepImageModify.SmallFormat(_T("dest='%d,%d,%d,%d'"), rcThumb.left, rcThumb.top, rcThumb.right, rcThumb.bottom);
			if( !DrawImage(ctx, m_sSepImage.GetData(), m_sSepImageModify.GetData()) ) {}
		}

		// 表头列线：跟随 List header-show-column-line
		{
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
					if( pInfo->dwLineColor != 0 ) clr = pInfo->dwLineColor;
				}
			}
			if( bShow ) {
				RECT rcLine = { m_rcItem.right - 1, m_rcItem.top, m_rcItem.right - 1, m_rcItem.bottom };
				ctx.DrawLine(rcLine, 1, GetAdjustColor(clr), PS_SOLID);
			}
		}

		if(m_bCheckBoxable)
		{
			m_uCheckBoxState &= ~UISTATE_PUSHED;

			if( (m_uCheckBoxState & UISTATE_SELECTED) != 0 ) {
				if( !m_sCheckBoxSelectedImage.IsEmpty() ) {
					if( !DrawCheckBoxImage(ctx, m_sCheckBoxSelectedImage.GetData()) ) {}
					else goto Label_ForegroundImage;
				}
			}

			if( IsFocused() ) m_uCheckBoxState |= UISTATE_FOCUSED;
			else m_uCheckBoxState &= ~ UISTATE_FOCUSED;
			if( !IsEnabled() ) m_uCheckBoxState |= UISTATE_DISABLED;
			else m_uCheckBoxState &= ~ UISTATE_DISABLED;

			if( (m_uCheckBoxState & UISTATE_DISABLED) != 0 ) {
				if( !m_sCheckBoxDisabledImage.IsEmpty() ) {
					if( !DrawCheckBoxImage(ctx, m_sCheckBoxDisabledImage.GetData()) ) {}
					else return;
				}
			}
			else if( (m_uCheckBoxState & UISTATE_PUSHED) != 0 ) {
				if( !m_sCheckBoxActiveImage.IsEmpty() ) {
					if( !DrawCheckBoxImage(ctx, m_sCheckBoxActiveImage.GetData()) ) {}
					else return;
				}
			}
			else if( (m_uCheckBoxState & UISTATE_HOT) != 0 ) {
				if( !m_sCheckBoxHoverImage.IsEmpty() ) {
					if( !DrawCheckBoxImage(ctx, m_sCheckBoxHoverImage.GetData()) ) {}
					else return;
				}
			}
			else if( (m_uCheckBoxState & UISTATE_FOCUSED) != 0 ) {
				if( !m_sCheckBoxFocusedImage.IsEmpty() ) {
					if( !DrawCheckBoxImage(ctx, m_sCheckBoxFocusedImage.GetData()) ) {}
					else return;
				}
			}

			if( !m_sCheckBoxNormalImage.IsEmpty() ) {
				if( !DrawCheckBoxImage(ctx, m_sCheckBoxNormalImage.GetData()) ) {}
				else return;
			}

Label_ForegroundImage:
			if( !m_sCheckBoxForegroundImage.IsEmpty() ) {
				if( !DrawCheckBoxImage(ctx, m_sCheckBoxForegroundImage.GetData()) ) {}
			}
		}
	}

	void CListContainerHeaderItemUI::PaintText(IRenderContext& ctx)
	{
		if( m_dwColor == 0 ) m_dwColor = m_pManager->GetDefaultFontColor();

		RECT rcText = m_rcItem;
		RECT rcPad = GetPadding();
		RECT rcTextPad = GetTextPadding();
		rcText.left += rcPad.left + rcTextPad.left;
		rcText.top += rcPad.top + rcTextPad.top;
		rcText.right -= rcPad.right + rcTextPad.right;
		rcText.bottom -= rcPad.bottom + rcTextPad.bottom;
		if (m_bCheckBoxable) {
			RECT rcCheck;
			GetCheckBoxRect(rcCheck);
			rcText.left += (rcCheck.right - rcCheck.left);
		}
		{
			int sep = (int)GetSepWidth();
			if( sep < 0 ) sep = -sep;
			if( sep > 0 && rcText.right - rcText.left > sep )
				rcText.right -= sep;
		}
		if( rcText.right <= rcText.left || rcText.bottom <= rcText.top ) return;

		CDuiString sText = GetText();
		if( sText.IsEmpty() ) return;

		UINT uStyle = m_uTextStyle | DT_SINGLELINE | DT_NOCLIP;
		if( (uStyle & (DT_TOP | DT_VCENTER | DT_BOTTOM)) == 0 )
			uStyle |= DT_VCENTER;

		int nLinks = 0;
		if( m_bShowHtml )
			ctx.DrawHtmlText(rcText, sText.GetData(), GetAdjustColor(m_dwColor),
				NULL, NULL, nLinks, m_iFont, uStyle);
		else
			ctx.DrawText(rcText, sText.GetData(), GetAdjustColor(m_dwColor),
				m_iFont, uStyle);
	}

	BOOL CListContainerHeaderItemUI::GetColumeEditable()
	{
		return m_bEditable;
	}

	void CListContainerHeaderItemUI::SetColumeEditable(BOOL bEnable)
	{
		m_bEditable = bEnable;
	}

	BOOL CListContainerHeaderItemUI::GetColumeComboable()
	{
		return m_bComboable;
	}

	void CListContainerHeaderItemUI::SetColumeComboable(BOOL bEnable)
	{
		m_bComboable = bEnable;
	}

	BOOL CListContainerHeaderItemUI::GetColumeCheckable()
	{
		return m_bCheckBoxable;
	}
	void CListContainerHeaderItemUI::SetColumeCheckable(BOOL bEnable)
	{
		m_bCheckBoxable = bEnable;
	}
	void CListContainerHeaderItemUI::SetCheck(BOOL bCheck)
	{
		if( m_bChecked == bCheck ) return;
		m_bChecked = bCheck;
		if( m_bChecked ) m_uCheckBoxState |= UISTATE_SELECTED;
		else m_uCheckBoxState &= ~UISTATE_SELECTED;
		Invalidate();
	}

	BOOL CListContainerHeaderItemUI::GetCheck()
	{
		return m_bChecked;
	}
	BOOL CListContainerHeaderItemUI::DrawCheckBoxImage(IRenderContext& ctx, LPCTSTR pStrImage, LPCTSTR pStrModify)
	{
		RECT rcCheckBox;
		GetCheckBoxRect(rcCheckBox);
		return ctx.DrawImageString(rcCheckBox, m_rcPaint, pStrImage, pStrModify);
	}
	LPCTSTR CListContainerHeaderItemUI::GetCheckBoxNormalImage()
	{
		return m_sCheckBoxNormalImage.GetData();
	}

	void CListContainerHeaderItemUI::SetCheckBoxNormalImage(LPCTSTR pStrImage)
	{
		m_sCheckBoxNormalImage = pStrImage;
	}

	LPCTSTR CListContainerHeaderItemUI::GetCheckBoxHoverImage()
	{
		return m_sCheckBoxHoverImage.GetData();
	}

	void CListContainerHeaderItemUI::SetCheckBoxHoverImage(LPCTSTR pStrImage)
	{
		m_sCheckBoxHoverImage = pStrImage;
	}

	LPCTSTR CListContainerHeaderItemUI::GetCheckBoxActiveImage()
	{
		return m_sCheckBoxActiveImage.GetData();
	}

	void CListContainerHeaderItemUI::SetCheckBoxActiveImage(LPCTSTR pStrImage)
	{
		m_sCheckBoxActiveImage = pStrImage;
	}

	LPCTSTR CListContainerHeaderItemUI::GetCheckBoxFocusedImage()
	{
		return m_sCheckBoxFocusedImage.GetData();
	}

	void CListContainerHeaderItemUI::SetCheckBoxFocusedImage(LPCTSTR pStrImage)
	{
		m_sCheckBoxFocusedImage = pStrImage;
	}

	LPCTSTR CListContainerHeaderItemUI::GetCheckBoxDisabledImage()
	{
		return m_sCheckBoxDisabledImage.GetData();
	}

	void CListContainerHeaderItemUI::SetCheckBoxDisabledImage(LPCTSTR pStrImage)
	{
		m_sCheckBoxDisabledImage = pStrImage;
	}
	LPCTSTR CListContainerHeaderItemUI::GetCheckBoxSelectedImage()
	{
		return m_sCheckBoxSelectedImage.GetData();
	}

	void CListContainerHeaderItemUI::SetCheckBoxSelectedImage(LPCTSTR pStrImage)
	{
		m_sCheckBoxSelectedImage = pStrImage;
	}
	LPCTSTR CListContainerHeaderItemUI::GetCheckBoxForegroundImage()
	{
		return m_sCheckBoxForegroundImage.GetData();
	}

	void CListContainerHeaderItemUI::SetCheckBoxForegroundImage(LPCTSTR pStrImage)
	{
		m_sCheckBoxForegroundImage = pStrImage;
	}
	int CListContainerHeaderItemUI::GetCheckBoxWidth() const
	{
		if(m_pManager) m_pManager->GetDPIObj()->Scale(m_cxyCheckBox.cx);
		return m_cxyCheckBox.cx;
	}

	void CListContainerHeaderItemUI::SetCheckBoxWidth(int cx)
	{
		if( cx < 0 ) return; 
		m_cxyCheckBox.cx = cx;
	}

	int CListContainerHeaderItemUI::GetCheckBoxHeight()  const 
	{
		if(m_pManager) m_pManager->GetDPIObj()->Scale(m_cxyCheckBox.cy);
		return m_cxyCheckBox.cy;
	}

	void CListContainerHeaderItemUI::SetCheckBoxHeight(int cy)
	{
		if( cy < 0 ) return; 
		m_cxyCheckBox.cy = cy;
	}
	void CListContainerHeaderItemUI::GetCheckBoxRect(RECT &rc)
	{
		memset(&rc, 0x00, sizeof(rc)); 
		int nItemHeight = m_rcItem.bottom - m_rcItem.top;
		rc.left = m_rcItem.left + 6;
		rc.top = m_rcItem.top + (nItemHeight - GetCheckBoxHeight()) / 2;
		rc.right = rc.left + GetCheckBoxWidth();
		rc.bottom = rc.top + GetCheckBoxHeight();
	}

	void CListContainerHeaderItemUI::SetOwner(CContainerUI* pOwner)
	{
		m_pOwner = pOwner;
	}
	CContainerUI* CListContainerHeaderItemUI::GetOwner()
	{
		return m_pOwner;
	}
	/////////////////////////////////////////////////////////////////////////////////////
	//
	//
	IMPLEMENT_DUICONTROL(CListTextExtElementUI)

	CListTextExtElementUI::CListTextExtElementUI() : 
	m_nLinks(0), m_nHoverLink(-1), m_pOwner(NULL),m_uCheckBoxState(0),m_bChecked(FALSE)
	{
		::ZeroMemory(&m_rcLinks, sizeof(m_rcLinks));
		m_cxyCheckBox.cx = m_cxyCheckBox.cy = 0;

		::ZeroMemory(&ColumCorlorArray, sizeof(ColumCorlorArray));
	}

	CListTextExtElementUI::~CListTextExtElementUI()
	{
		CDuiString* pText;
		for( int it = 0; it < m_aTexts.GetSize(); it++ ) {
			pText = static_cast<CDuiString*>(m_aTexts[it]);
			if( pText ) delete pText;
		}
		m_aTexts.Empty();
	}

	LPCTSTR CListTextExtElementUI::GetClass() const
	{
		return _T("ListTextExElementUI");
	}

	LPVOID CListTextExtElementUI::GetInterface(LPCTSTR pstrName)
	{
		if( _tcsicmp(pstrName, _T("ListTextExElement")) == 0 ) return static_cast<CListTextExtElementUI*>(this);
		return CListLabelElementUI::GetInterface(pstrName);
	}

	UINT CListTextExtElementUI::GetControlFlags() const
	{
		return UIFLAG_WANTRETURN | ( (IsEnabled() && m_nLinks > 0) ? UIFLAG_SETCURSOR : 0);
	}

	LPCTSTR CListTextExtElementUI::GetText(int iIndex) const
	{
		CDuiString* pText = static_cast<CDuiString*>(m_aTexts.GetAt(iIndex));
		if( pText ) return pText->GetData();
		return NULL;
	}

	void CListTextExtElementUI::SetText(int iIndex, LPCTSTR pstrText)
	{
		if( m_pOwner == NULL ) return;
		TListInfoUI* pInfo = m_pOwner->GetListInfo();
		if( iIndex < 0 || iIndex >= pInfo->nColumns ) return;
		while( m_aTexts.GetSize() < pInfo->nColumns ) { m_aTexts.Add(NULL); }

		CDuiString* pText = static_cast<CDuiString*>(m_aTexts[iIndex]);
		if( (pText == NULL && pstrText == NULL) || (pText && *pText == pstrText) ) return;

		if ( pText )
			pText->Assign(pstrText);
		else
			m_aTexts.SetAt(iIndex, new CDuiString(pstrText));
		Invalidate();
	}

	void CListTextExtElementUI::SetOwner(CControlUI* pOwner)
	{
		CListElementUI::SetOwner(pOwner);
		m_pOwner = static_cast<CListUI*>(pOwner->GetInterface(_T("List")));
	}

	CDuiString* CListTextExtElementUI::GetLinkContent(int iIndex)
	{
		if( iIndex >= 0 && iIndex < m_nLinks ) return &m_sLinks[iIndex];
		return NULL;
	}

	void CListTextExtElementUI::DoEvent(TEventUI& event)
	{
		if( !IsMouseEnabled() && event.Type > UIEVENT__MOUSEBEGIN && event.Type < UIEVENT__MOUSEEND ) {
			if( m_pOwner != NULL ) m_pOwner->DoEvent(event);
			else CListLabelElementUI::DoEvent(event);
			return;
		}

		// When you hover over a link
		if( event.Type == UIEVENT_SETCURSOR ) {
			for( int i = 0; i < m_nLinks; i++ ) {
				if( ::PtInRect(&m_rcLinks[i], event.ptMouse) ) {
					::SetCursor(::LoadCursor(NULL, MAKEINTRESOURCE(IDC_HAND)));
					return;
				}
			}      
		}
		if( event.Type == UIEVENT_BUTTONUP && IsEnabled() ) {
			for( int i = 0; i < m_nLinks; i++ ) {
				if( ::PtInRect(&m_rcLinks[i], event.ptMouse) ) {
					m_pManager->SendNotify(this, DUI_MSGTYPE_LINK, i);
					return;
				}
			}
		}
		if( m_nLinks > 0 && event.Type == UIEVENT_MOUSEMOVE ) {
			int nHoverLink = -1;
			for( int i = 0; i < m_nLinks; i++ ) {
				if( ::PtInRect(&m_rcLinks[i], event.ptMouse) ) {
					nHoverLink = i;
					break;
				}
			}

			if(m_nHoverLink != nHoverLink) {
				Invalidate();
				m_nHoverLink = nHoverLink;
			}
		}
		if( m_nLinks > 0 && event.Type == UIEVENT_MOUSELEAVE ) {
			if(m_nHoverLink != -1) {
				Invalidate();
				m_nHoverLink = -1;
			}
		}

		//检查是否需要显示编辑框或者组合框	
		CListExUI * pListCtrl = (CListExUI *)m_pOwner;
		int nColum = HitTestColum(event.ptMouse);
		if(event.Type == UIEVENT_BUTTONUP && m_pOwner->IsFocused())
		{
			RECT rc = {0,0,0,0};
			if (nColum >= 0)
			{
				GetColumRect(nColum, rc);
			}

			pListCtrl->OnListItemClicked(GetIndex(), nColum, &rc, GetText(nColum));
		}

		//检查是否需要显示CheckBox
		TListInfoUI* pInfo = m_pOwner->GetListInfo();
		for( int i = 0; i < pInfo->nColumns; i++ )
		{
			if (pListCtrl->CheckColumCheckBoxable(i))
			{
				RECT rcCheckBox;
				GetCheckBoxRect(i, rcCheckBox);

				if( event.Type == UIEVENT_BUTTONDOWN || event.Type == UIEVENT_DBLCLICK )
				{
					if( ::PtInRect(&rcCheckBox, event.ptMouse)) 
					{
						m_uCheckBoxState |= UISTATE_PUSHED | UISTATE_CAPTURED;
						Invalidate();
					}
				}
				else if( event.Type == UIEVENT_MOUSEMOVE )
				{
					if( (m_uCheckBoxState & UISTATE_CAPTURED) != 0 ) 
					{
						if( ::PtInRect(&rcCheckBox, event.ptMouse) ) 
							m_uCheckBoxState |= UISTATE_PUSHED;
						else 
							m_uCheckBoxState &= ~UISTATE_PUSHED;
						Invalidate();
					}
				}
				else if( event.Type == UIEVENT_BUTTONUP )
				{
					if( (m_uCheckBoxState & UISTATE_CAPTURED) != 0 )
					{
						if( ::PtInRect(&rcCheckBox, event.ptMouse) ) 
						{
							SetCheck(!GetCheck());
							if (m_pManager)
							{
								m_pManager->SendNotify(this, DUI_MSGTYPE_LISTITEMCHECKED, MAKEWPARAM(GetIndex(), 0), m_bChecked);
							}
						}
						m_uCheckBoxState &= ~(UISTATE_PUSHED | UISTATE_CAPTURED);
						Invalidate();
					}
				}
				else if( event.Type == UIEVENT_MOUSEENTER )
				{
					if( ::PtInRect(&rcCheckBox, event.ptMouse) ) 
					{
						m_uCheckBoxState |= UISTATE_HOT;
						Invalidate();
					}
				}
				else if( event.Type == UIEVENT_MOUSELEAVE )
				{
					m_uCheckBoxState &= ~UISTATE_HOT;
					Invalidate();
				}
			}
		}

		CListLabelElementUI::DoEvent(event);
	}

	SIZE CListTextExtElementUI::EstimateSize(SIZE szAvailable)
	{
		TListInfoUI* pInfo = NULL;
		if( m_pOwner ) pInfo = m_pOwner->GetListInfo();

		SIZE cXY = m_cxyFixed;
		if( cXY.cy == 0 && m_pManager != NULL && pInfo != NULL) {
			cXY.cy = m_pManager->GetFontInfo(pInfo->nFont)->tm.tmHeight + 8;
			cXY.cy += pInfo->rcTextPadding.top + pInfo->rcTextPadding.bottom;
		}

		return cXY;
	}

	void CListTextExtElementUI::DrawItemText(IRenderContext& ctx, const RECT& rcItem)
	{
		if( m_pOwner == NULL ) return;
		TListInfoUI* pInfo = m_pOwner->GetListInfo();
		DWORD iTextColor = pInfo->dwColor;

		if( (m_uButtonState & UISTATE_HOT) != 0 ) {
			iTextColor = pInfo->dwHoverColor;
		}
		if( IsSelected() ) {
			iTextColor = pInfo->dwSelectedColor;
		}
		if( !IsEnabled() ) {
			iTextColor = pInfo->dwDisabledColor;
		}
		IListCallbackUI* pCallback = m_pOwner->GetTextCallback();
		//DUIASSERT(pCallback);
		//if( pCallback == NULL ) return;


		CListExUI * pListCtrl = (CListExUI *)m_pOwner;
		m_nLinks = 0;
		int nLinks = lengthof(m_rcLinks);
		for( int i = 0; i < pInfo->nColumns; i++ )
		{
			RECT rcItem = { pInfo->rcColumn[i].left, m_rcItem.top, pInfo->rcColumn[i].right, m_rcItem.bottom };

			DWORD iTextBkColor = 0;
			if (GetColumItemColor(i, iTextBkColor))
			{	
				ctx.DrawColor(rcItem, iTextBkColor);
			}

			rcItem.left += pInfo->rcTextPadding.left;
			rcItem.right -= pInfo->rcTextPadding.right;
			rcItem.top += pInfo->rcTextPadding.top;
			rcItem.bottom -= pInfo->rcTextPadding.bottom;

			//检查是否需要显示CheckBox
			if (pListCtrl->CheckColumCheckBoxable(i))
			{
				RECT rcCheckBox;
				GetCheckBoxRect(i, rcCheckBox);
				rcItem.left += (rcCheckBox.right - rcCheckBox.left);
			}

			CDuiString strText;//不使用LPCTSTR，否则限制太多 by cddjr 2011/10/20
			if( pCallback ) strText = pCallback->GetItemText(this, m_iIndex, i);
			else strText.Assign(GetText(i));
			UINT uColStyle = DT_SINGLELINE | pInfo->uTextStyle;
			CListHeaderUI* pHeader = pListCtrl->GetHeader();
			if( pHeader != NULL && i < pHeader->GetCount() ) {
				CControlUI* pCol = pHeader->GetItemAt(i);
				CListContainerHeaderItemUI* pHItem = pCol == NULL ? NULL :
					static_cast<CListContainerHeaderItemUI*>(pCol->GetInterface(_T("ListContainerHeaderItem")));
				if( pHItem == NULL && pCol != NULL )
					pHItem = static_cast<CListContainerHeaderItemUI*>(pCol->GetInterface(DUI_CTR_LISTHEADERITEM));
				if( pHItem != NULL ) {
					UINT uHdr = pHItem->GetTextStyle();
					uColStyle &= ~(DT_LEFT | DT_CENTER | DT_RIGHT | DT_TOP | DT_VCENTER | DT_BOTTOM);
					uColStyle |= (uHdr & (DT_LEFT | DT_CENTER | DT_RIGHT | DT_TOP | DT_VCENTER | DT_BOTTOM));
					if( (uColStyle & (DT_TOP | DT_VCENTER | DT_BOTTOM)) == 0 ) uColStyle |= DT_VCENTER;
					if( (uColStyle & (DT_LEFT | DT_CENTER | DT_RIGHT)) == 0 ) uColStyle |= DT_CENTER;
					uColStyle |= DT_SINGLELINE;
				}
				else {
					CListHeaderItemUI* pHI = pCol == NULL ? NULL :
						static_cast<CListHeaderItemUI*>(pCol->GetInterface(DUI_CTR_LISTHEADERITEM));
					if( pHI != NULL ) {
						UINT uHdr = pHI->GetTextStyle();
						uColStyle &= ~(DT_LEFT | DT_CENTER | DT_RIGHT | DT_TOP | DT_VCENTER | DT_BOTTOM);
						uColStyle |= (uHdr & (DT_LEFT | DT_CENTER | DT_RIGHT | DT_TOP | DT_VCENTER | DT_BOTTOM));
						if( (uColStyle & (DT_TOP | DT_VCENTER | DT_BOTTOM)) == 0 ) uColStyle |= DT_VCENTER;
						if( (uColStyle & (DT_LEFT | DT_CENTER | DT_RIGHT)) == 0 ) uColStyle |= DT_CENTER;
						uColStyle |= DT_SINGLELINE;
					}
				}
			}
			if( pInfo->bShowHtml )
				ctx.DrawHtmlText(rcItem, strText.GetData(), GetAdjustColor(iTextColor), \
				&m_rcLinks[m_nLinks], &m_sLinks[m_nLinks], nLinks, pInfo->nFont, uColStyle);
			else
				ctx.DrawText(rcItem, strText.GetData(), GetAdjustColor(iTextColor), \
				pInfo->nFont, uColStyle);

			m_nLinks += nLinks;
			nLinks = lengthof(m_rcLinks) - m_nLinks; 
		}
		for( int i = m_nLinks; i < lengthof(m_rcLinks); i++ ) {
			::ZeroMemory(m_rcLinks + i, sizeof(RECT));
			((CDuiString*)(m_sLinks + i))->Empty();
		}
	}
	void CListTextExtElementUI::PaintStatusImage(IRenderContext& ctx)
	{
		CListExUI * pListCtrl = (CListExUI *)m_pOwner;
		TListInfoUI* pInfo = m_pOwner->GetListInfo();
		for( int i = 0; i < pInfo->nColumns; i++ )
		{
			if (pListCtrl->CheckColumCheckBoxable(i))
			{
				RECT rcCheckBox;
				GetCheckBoxRect(i, rcCheckBox);

				m_uCheckBoxState &= ~UISTATE_PUSHED;

				if( (m_uCheckBoxState & UISTATE_SELECTED) != 0 ) {
					if( !m_sCheckBoxSelectedImage.IsEmpty() ) {
						if( !DrawCheckBoxImage(ctx, m_sCheckBoxSelectedImage.GetData(), NULL, rcCheckBox) ) {}
						else goto Label_ForegroundImage;
					}
				}

				if( IsFocused() ) m_uCheckBoxState |= UISTATE_FOCUSED;
				else m_uCheckBoxState &= ~ UISTATE_FOCUSED;
				if( !IsEnabled() ) m_uCheckBoxState |= UISTATE_DISABLED;
				else m_uCheckBoxState &= ~ UISTATE_DISABLED;

				if( (m_uCheckBoxState & UISTATE_DISABLED) != 0 ) {
					if( !m_sCheckBoxDisabledImage.IsEmpty() ) {
						if( !DrawCheckBoxImage(ctx, m_sCheckBoxDisabledImage.GetData(), NULL, rcCheckBox) ) {}
						else return;
					}
				}
				else if( (m_uCheckBoxState & UISTATE_PUSHED) != 0 ) {
					if( !m_sCheckBoxActiveImage.IsEmpty() ) {
						if( !DrawCheckBoxImage(ctx, m_sCheckBoxActiveImage.GetData(), NULL, rcCheckBox) ) {}
						else return;
					}
				}
				else if( (m_uCheckBoxState & UISTATE_HOT) != 0 ) {
					if( !m_sCheckBoxHoverImage.IsEmpty() ) {
						if( !DrawCheckBoxImage(ctx, m_sCheckBoxHoverImage.GetData(), NULL, rcCheckBox) ) {}
						else return;
					}
				}
				else if( (m_uCheckBoxState & UISTATE_FOCUSED) != 0 ) {
					if( !m_sCheckBoxFocusedImage.IsEmpty() ) {
						if( !DrawCheckBoxImage(ctx, m_sCheckBoxFocusedImage.GetData(), NULL, rcCheckBox) ) {}
						else return;
					}
				}

				if( !m_sCheckBoxNormalImage.IsEmpty() ) {
					if( !DrawCheckBoxImage(ctx, m_sCheckBoxNormalImage.GetData(), NULL, rcCheckBox) ) {}
					else return;
				}

Label_ForegroundImage:
				if( !m_sCheckBoxForegroundImage.IsEmpty() ) {
					if( !DrawCheckBoxImage(ctx, m_sCheckBoxForegroundImage.GetData(), NULL, rcCheckBox) ) {}
				}
			}
		}
	}
	BOOL CListTextExtElementUI::DrawCheckBoxImage(IRenderContext& ctx, LPCTSTR pStrImage, LPCTSTR pStrModify, RECT& rcCheckBox)
	{
		return ctx.DrawImageString(rcCheckBox, m_rcPaint, pStrImage, pStrModify);
	}
	void CListTextExtElementUI::SetAttribute(LPCTSTR pstrName, LPCTSTR pstrValue)
	{
		if( _tcsicmp(pstrName, _T("checkbox-width")) == 0 ) SetCheckBoxWidth(_ttoi(pstrValue));
		else if( _tcsicmp(pstrName, _T("checkbox-height")) == 0 ) SetCheckBoxHeight(_ttoi(pstrValue));
		else if( _tcsicmp(pstrName, _T("checkbox-image")) == 0 ) SetCheckBoxNormalImage(pstrValue);
		else if( _tcsicmp(pstrName, _T("checkbox-image-hover")) == 0 ) SetCheckBoxHoverImage(pstrValue);
		else if( _tcsicmp(pstrName, _T("checkbox-image-active")) == 0 ) SetCheckBoxActiveImage(pstrValue);
		else if( _tcsicmp(pstrName, _T("checkbox-image-focus")) == 0 ) SetCheckBoxFocusedImage(pstrValue);
		else if( _tcsicmp(pstrName, _T("checkbox-image-disabled")) == 0 ) SetCheckBoxDisabledImage(pstrValue);
		else if( _tcsicmp(pstrName, _T("checkbox-image-selected")) == 0 ) SetCheckBoxSelectedImage(pstrValue);
		else if( _tcsicmp(pstrName, _T("checkbox-foreground-image")) == 0 ) SetCheckBoxForegroundImage(pstrValue);
		else CListLabelElementUI::SetAttribute(pstrName, pstrValue);
	}
	LPCTSTR CListTextExtElementUI::GetCheckBoxNormalImage()
	{
		return m_sCheckBoxNormalImage.GetData();
	}

	void CListTextExtElementUI::SetCheckBoxNormalImage(LPCTSTR pStrImage)
	{
		m_sCheckBoxNormalImage = pStrImage;
	}

	LPCTSTR CListTextExtElementUI::GetCheckBoxHoverImage()
	{
		return m_sCheckBoxHoverImage.GetData();
	}

	void CListTextExtElementUI::SetCheckBoxHoverImage(LPCTSTR pStrImage)
	{
		m_sCheckBoxHoverImage = pStrImage;
	}

	LPCTSTR CListTextExtElementUI::GetCheckBoxActiveImage()
	{
		return m_sCheckBoxActiveImage.GetData();
	}

	void CListTextExtElementUI::SetCheckBoxActiveImage(LPCTSTR pStrImage)
	{
		m_sCheckBoxActiveImage = pStrImage;
	}

	LPCTSTR CListTextExtElementUI::GetCheckBoxFocusedImage()
	{
		return m_sCheckBoxFocusedImage.GetData();
	}

	void CListTextExtElementUI::SetCheckBoxFocusedImage(LPCTSTR pStrImage)
	{
		m_sCheckBoxFocusedImage = pStrImage;
	}

	LPCTSTR CListTextExtElementUI::GetCheckBoxDisabledImage()
	{
		return m_sCheckBoxDisabledImage.GetData();
	}

	void CListTextExtElementUI::SetCheckBoxDisabledImage(LPCTSTR pStrImage)
	{
		m_sCheckBoxDisabledImage = pStrImage;
	}
	LPCTSTR CListTextExtElementUI::GetCheckBoxSelectedImage()
	{
		return m_sCheckBoxSelectedImage.GetData();
	}

	void CListTextExtElementUI::SetCheckBoxSelectedImage(LPCTSTR pStrImage)
	{
		m_sCheckBoxSelectedImage = pStrImage;
	}
	LPCTSTR CListTextExtElementUI::GetCheckBoxForegroundImage()
	{
		return m_sCheckBoxForegroundImage.GetData();
	}

	void CListTextExtElementUI::SetCheckBoxForegroundImage(LPCTSTR pStrImage)
	{
		m_sCheckBoxForegroundImage = pStrImage;
	}

	bool CListTextExtElementUI::DoPaint(IRenderContext& ctx, const RECT& rcPaint, CControlUI* pStopControl)
	{
		if( !::IntersectRect(&m_rcPaint, &rcPaint, &m_rcItem) ) return true;
		DrawItemBk(ctx, m_rcItem);
		PaintStatusImage(ctx);
		DrawItemText(ctx, m_rcItem);
		return true;
	}
	void CListTextExtElementUI::GetCheckBoxRect(int nIndex, RECT &rc)
	{
		memset(&rc, 0x00, sizeof(rc));
		int nItemHeight = m_rcItem.bottom - m_rcItem.top;
		rc.left = m_rcItem.left + 6;
		rc.top = m_rcItem.top + (nItemHeight - GetCheckBoxHeight()) / 2;
		rc.right = rc.left + GetCheckBoxWidth();
		rc.bottom = rc.top + GetCheckBoxHeight();
	}
	int CListTextExtElementUI::GetCheckBoxWidth() const
	{
		return m_cxyCheckBox.cx;
	}

	void CListTextExtElementUI::SetCheckBoxWidth(int cx)
	{
		if( cx < 0 ) return; 
		m_cxyCheckBox.cx = cx;
	}

	int CListTextExtElementUI::GetCheckBoxHeight()  const 
	{
		return m_cxyCheckBox.cy;
	}

	void CListTextExtElementUI::SetCheckBoxHeight(int cy)
	{
		if( cy < 0 ) return; 
		m_cxyCheckBox.cy = cy;
	}

	void CListTextExtElementUI::SetCheck(BOOL bCheck)
	{
		if( m_bChecked == bCheck ) return;
		m_bChecked = bCheck;
		if( m_bChecked ) m_uCheckBoxState |= UISTATE_SELECTED;
		else m_uCheckBoxState &= ~UISTATE_SELECTED;
		Invalidate();
	}

	BOOL  CListTextExtElementUI::GetCheck() const
	{
		return m_bChecked;
	}

	int CListTextExtElementUI::HitTestColum(POINT ptMouse)
	{
		TListInfoUI* pInfo = m_pOwner->GetListInfo();
		for( int i = 0; i < pInfo->nColumns; i++ )
		{
			RECT rcItem = { pInfo->rcColumn[i].left, m_rcItem.top, pInfo->rcColumn[i].right, m_rcItem.bottom };
			rcItem.left += pInfo->rcTextPadding.left;
			rcItem.right -= pInfo->rcTextPadding.right;
			rcItem.top += pInfo->rcTextPadding.top;
			rcItem.bottom -= pInfo->rcTextPadding.bottom;

			if( ::PtInRect(&rcItem, ptMouse)) 
			{
				return i;
			}
		}
		return -1;
	}

	BOOL CListTextExtElementUI::CheckColumEditable(int nColum)
	{
		return m_pOwner->CheckColumEditable(nColum);
	}
	void CListTextExtElementUI::GetColumRect(int nColum, RECT &rc)
	{
		TListInfoUI* pInfo = m_pOwner->GetListInfo();
		RECT rcOwnerPos = m_pOwner->GetPos();

		rc.left = pInfo->rcColumn[nColum].left + 1;
		rc.top  = 1;
		rc.right = pInfo->rcColumn[nColum].right - 1;
		rc.bottom = m_rcItem.bottom - m_rcItem.top - 1;
		OffsetRect(&rc, -rcOwnerPos.left, m_rcItem.top - rcOwnerPos.top);
	}

	void CListTextExtElementUI::SetColumItemColor(int nColum, DWORD iBKColor)
	{
		ColumCorlorArray[nColum].bEnable = TRUE;
		ColumCorlorArray[nColum].iBKColor = iBKColor;
		Invalidate();
	}
	BOOL CListTextExtElementUI::GetColumItemColor(int nColum, DWORD& iBKColor)
	{
		if (!ColumCorlorArray[nColum].bEnable)
		{
			return FALSE;
		}
		iBKColor = ColumCorlorArray[nColum].iBKColor;
		return TRUE;
	}

} // namespace DuiLib

