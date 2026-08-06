#include "StdAfx.h"
#include "UITransfer.h"

namespace DuiLib
{
	//////////////////////////////////////////////////////////////////////////
	IMPLEMENT_DUICONTROL(CTransferItemUI)

	CTransferItemUI::CTransferItemUI()
		: m_bTarget(false)
	{
		SetMouseEnabled(false);
	}

	LPCTSTR CTransferItemUI::GetClass() const { return _T("TransferItemUI"); }

	LPVOID CTransferItemUI::GetInterface(LPCTSTR pstrName)
	{
		if( _tcsicmp(pstrName, DUI_CTR_TRANSFERITEM) == 0 ) return static_cast<CTransferItemUI*>(this);
		if( _tcsicmp(pstrName, _T("TransferItem")) == 0 ) return static_cast<CTransferItemUI*>(this);
		return CControlUI::GetInterface(pstrName);
	}

	void CTransferItemUI::SetValue(LPCTSTR pstr)
	{
		m_sValue = pstr ? pstr : _T("");
	}

	LPCTSTR CTransferItemUI::GetValue() const
	{
		if( !m_sValue.IsEmpty() ) return m_sValue.GetData();
		return m_sText.GetData();
	}

	void CTransferItemUI::SetTarget(bool b) { m_bTarget = b; }
	bool CTransferItemUI::IsTarget() const { return m_bTarget; }

	void CTransferItemUI::SetAttribute(LPCTSTR pstrName, LPCTSTR pstrValue)
	{
		if( _tcsicmp(pstrName, _T("text")) == 0 || _tcsicmp(pstrName, _T("title")) == 0 || _tcsicmp(pstrName, _T("label")) == 0 ) {
			SetText(pstrValue);
		}
		else if( _tcsicmp(pstrName, _T("value")) == 0 || _tcsicmp(pstrName, _T("key")) == 0 ) {
			SetValue(pstrValue);
		}
		else if( _tcsicmp(pstrName, _T("target")) == 0 || _tcsicmp(pstrName, _T("selected")) == 0 ) {
			SetTarget(_tcsicmp(pstrValue, _T("true")) == 0);
		}
		else {
			CControlUI::SetAttribute(pstrName, pstrValue);
		}
	}

	bool CTransferItemUI::DoPaint(IRenderContext& /*ctx*/, const RECT& /*rcPaint*/, CControlUI* /*pStopControl*/)
	{
		return true;
	}

	SIZE CTransferItemUI::EstimateSize(SIZE /*szAvailable*/)
	{
		SIZE sz = { 0, 0 };
		return sz;
	}

	//////////////////////////////////////////////////////////////////////////
	IMPLEMENT_DUICONTROL(CTransferUI)

	CTransferUI::CTransferUI()
		: m_bBuilt(false)
		, m_bShowSelectAll(true)
		, m_nItemHeight(32)
		, m_sSourceTitle(_T("可选"))
		, m_sTargetTitle(_T("已选"))
		, m_pLeftPanel(NULL)
		, m_pRightPanel(NULL)
		, m_pLeftHeader(NULL)
		, m_pRightHeader(NULL)
		, m_pLeftAll(NULL)
		, m_pRightAll(NULL)
		, m_pLeftTitle(NULL)
		, m_pRightTitle(NULL)
		, m_pLeftCount(NULL)
		, m_pRightCount(NULL)
		, m_pLeftList(NULL)
		, m_pRightList(NULL)
		, m_pBtnToTarget(NULL)
		, m_pBtnToSource(NULL)
		, m_bRefreshing(false)
	{
		SetKind(CONTROLKIND_NONE);
		SetGap(8);
	}

	CTransferUI::~CTransferUI()
	{
	}

	LPCTSTR CTransferUI::GetClass() const { return _T("TransferUI"); }

	LPVOID CTransferUI::GetInterface(LPCTSTR pstrName)
	{
		if( _tcsicmp(pstrName, DUI_CTR_TRANSFER) == 0 ) return static_cast<CTransferUI*>(this);
		return CHorizontalLayoutUI::GetInterface(pstrName);
	}

	int CTransferUI::ScaleValue(int v) const
	{
		return m_pManager ? m_pManager->GetDPIObj()->Scale(v) : v;
	}

	void CTransferUI::SetTitles(LPCTSTR pstrTitles)
	{
		m_sSourceTitle = _T("可选");
		m_sTargetTitle = _T("已选");
		if( pstrTitles == NULL || *pstrTitles == _T('\0') ) {
			UpdateHeaders();
			return;
		}
		CDuiString s = pstrTitles;
		int n = s.Find(_T('|'));
		if( n < 0 ) {
			m_sSourceTitle = s;
		}
		else {
			m_sSourceTitle = s.Left(n);
			m_sTargetTitle = s.Mid(n + 1);
		}
		UpdateHeaders();
	}

	LPCTSTR CTransferUI::GetSourceTitle() const { return m_sSourceTitle.GetData(); }
	LPCTSTR CTransferUI::GetTargetTitle() const { return m_sTargetTitle.GetData(); }

	void CTransferUI::ParseItemsAttr(LPCTSTR pstr)
	{
		if( pstr == NULL || *pstr == _T('\0') ) return;
		LPCTSTR p = pstr;
		while( *p ) {
			while( *p == _T('|') ) ++p;
			if( !*p ) break;
			LPCTSTR start = p;
			while( *p && *p != _T('|') ) ++p;
			CDuiString token(start, (int)(p - start));
			token.Trim();
			if( token.IsEmpty() ) continue;
			Item it;
			int colon = token.Find(_T(':'));
			if( colon >= 0 ) {
				it.sText = token.Left(colon);
				it.sValue = token.Mid(colon + 1);
				it.sText.Trim();
				it.sValue.Trim();
			}
			else {
				it.sText = token;
				it.sValue = token;
			}
			if( it.sValue.IsEmpty() ) it.sValue = it.sText;
			m_items.push_back(it);
		}
	}

	void CTransferUI::ApplyTargetKeysAttr(LPCTSTR pstr)
	{
		if( pstr == NULL || *pstr == _T('\0') ) return;
		LPCTSTR p = pstr;
		while( *p ) {
			while( *p == _T('|') ) ++p;
			if( !*p ) break;
			LPCTSTR start = p;
			while( *p && *p != _T('|') ) ++p;
			CDuiString key(start, (int)(p - start));
			key.Trim();
			if( key.IsEmpty() ) continue;
			for( size_t i = 0; i < m_items.size(); ++i ) {
				if( m_items[i].sValue == key || m_items[i].sText == key )
					m_items[i].bTarget = true;
			}
		}
	}

	void CTransferUI::SetItems(LPCTSTR pstrItems)
	{
		m_sItemsAttr = pstrItems ? pstrItems : _T("");
		m_items.clear();
		ParseItemsAttr(m_sItemsAttr);
		if( !m_sTargetAttr.IsEmpty() ) ApplyTargetKeysAttr(m_sTargetAttr);
		if( m_bBuilt ) RefreshLists();
	}

	void CTransferUI::SetTargetKeys(LPCTSTR pstrKeys)
	{
		m_sTargetAttr = pstrKeys ? pstrKeys : _T("");
		for( size_t i = 0; i < m_items.size(); ++i )
			m_items[i].bTarget = false;
		ApplyTargetKeysAttr(m_sTargetAttr);
		if( m_bBuilt ) RefreshLists();
	}

	CDuiString CTransferUI::GetTargetKeys() const
	{
		CDuiString s;
		for( size_t i = 0; i < m_items.size(); ++i ) {
			if( !m_items[i].bTarget ) continue;
			if( !s.IsEmpty() ) s += _T("|");
			s += m_items[i].sValue;
		}
		return s;
	}

	CDuiString CTransferUI::GetSourceKeys() const
	{
		CDuiString s;
		for( size_t i = 0; i < m_items.size(); ++i ) {
			if( m_items[i].bTarget ) continue;
			if( !s.IsEmpty() ) s += _T("|");
			s += m_items[i].sValue;
		}
		return s;
	}

	int CTransferUI::GetItemCount() const { return (int)m_items.size(); }

	const CTransferUI::Item* CTransferUI::GetItem(int i) const
	{
		if( i < 0 || i >= (int)m_items.size() ) return NULL;
		return &m_items[i];
	}

	void CTransferUI::ClearItems()
	{
		m_items.clear();
		if( m_bBuilt ) RefreshLists();
	}

	void CTransferUI::AddItem(LPCTSTR pstrText, LPCTSTR pstrValue, bool bTarget)
	{
		Item it;
		it.sText = pstrText ? pstrText : _T("");
		if( pstrValue && *pstrValue ) it.sValue = pstrValue;
		else it.sValue = it.sText;
		it.bTarget = bTarget;
		m_items.push_back(it);
		if( m_bBuilt ) RefreshLists();
	}

	void CTransferUI::SetItemHeight(int n)
	{
		if( n < 20 ) n = 20;
		m_nItemHeight = n;
		if( m_bBuilt ) RefreshLists();
	}

	int CTransferUI::GetItemHeight() const { return m_nItemHeight; }

	void CTransferUI::SetShowSelectAll(bool b)
	{
		m_bShowSelectAll = b;
		if( m_pLeftAll ) m_pLeftAll->SetVisible(b);
		if( m_pRightAll ) m_pRightAll->SetVisible(b);
	}

	bool CTransferUI::IsShowSelectAll() const { return m_bShowSelectAll; }

	int CTransferUI::CountSide(bool bTarget) const
	{
		int n = 0;
		for( size_t i = 0; i < m_items.size(); ++i )
			if( m_items[i].bTarget == bTarget ) ++n;
		return n;
	}

	int CTransferUI::CountChecked(bool bTarget) const
	{
		int n = 0;
		for( size_t i = 0; i < m_items.size(); ++i )
			if( m_items[i].bTarget == bTarget && m_items[i].bChecked ) ++n;
		return n;
	}

	void CTransferUI::SetAllChecked(bool bTarget, bool bChecked)
	{
		for( size_t i = 0; i < m_items.size(); ++i ) {
			if( m_items[i].bTarget == bTarget )
				m_items[i].bChecked = bChecked;
		}
		RefreshLists();
	}

	void CTransferUI::CollectDataNodes()
	{
		if( !m_items.empty() ) return;
		for( int i = 0; i < GetCount(); ++i ) {
			CControlUI* p = GetItemAt(i);
			CTransferItemUI* pItem = p ? static_cast<CTransferItemUI*>(p->GetInterface(DUI_CTR_TRANSFERITEM)) : NULL;
			if( pItem == NULL ) continue;
			Item it;
			it.sText = pItem->GetText();
			it.sValue = pItem->GetValue();
			it.bTarget = pItem->IsTarget();
			if( it.sText.IsEmpty() ) it.sText = it.sValue;
			if( it.sValue.IsEmpty() ) it.sValue = it.sText;
			m_items.push_back(it);
		}
		if( m_items.empty() && !m_sItemsAttr.IsEmpty() )
			ParseItemsAttr(m_sItemsAttr);
		if( !m_sTargetAttr.IsEmpty() )
			ApplyTargetKeysAttr(m_sTargetAttr);
	}

	void CTransferUI::BuildShell()
	{
		while( GetCount() > 0 )
			RemoveAt(0);

		auto MakePanel = [&](bool bLeft) -> CVerticalLayoutUI* {
			CVerticalLayoutUI* panel = new CVerticalLayoutUI();
			panel->SetBorderWidth(1);
			panel->SetBorderColor(0xD9D9D9FF);
			SIZE br = { 6, 6 };
			panel->SetBorderRadius(br);
			panel->SetBackgroundColor(0xFFFFFFFF);

			CHorizontalLayoutUI* header = new CHorizontalLayoutUI();
			header->SetFixedHeight(ScaleValue(40));
			header->SetPadding(CDuiBox(0, 10, 0, 10));
			header->SetGap(ScaleValue(6));
			header->SetBackgroundColor(0xFAFAFAFF);

			CCheckBoxUI* all = new CCheckBoxUI();
			all->SetText(_T(""));
			all->SetFixedWidth(ScaleValue(22));
			all->SetVisible(m_bShowSelectAll);
			all->Selected(false, false);
			all->OnNotify += MakeDelegate(this, &CTransferUI::OnHeaderCheckNotify);
			header->Add(all);

			CLabelUI* title = new CLabelUI();
			title->SetText(bLeft ? m_sSourceTitle : m_sTargetTitle);
			title->SetTextStyle(DT_SINGLELINE | DT_VCENTER | DT_LEFT);
			title->SetColor(0x000000E0);
			title->SetMouseEnabled(false);
			header->Add(title);

			CControlUI* spacer = new CControlUI();
			header->Add(spacer);

			CLabelUI* count = new CLabelUI();
			count->SetFixedWidth(ScaleValue(56));
			count->SetTextStyle(DT_SINGLELINE | DT_VCENTER | DT_RIGHT);
			count->SetColor(0x00000073);
			count->SetText(_T("0/0"));
			count->SetMouseEnabled(false);
			header->Add(count);

			CListUI* list = new CListUI();
			list->SetAttribute(_T("overflow"), _T("auto"));

			panel->Add(header);
			panel->Add(list);

			if( bLeft ) {
				m_pLeftPanel = panel;
				m_pLeftHeader = header;
				m_pLeftAll = all;
				m_pLeftTitle = title;
				m_pLeftCount = count;
				m_pLeftList = list;
			}
			else {
				m_pRightPanel = panel;
				m_pRightHeader = header;
				m_pRightAll = all;
				m_pRightTitle = title;
				m_pRightCount = count;
				m_pRightList = list;
			}
			return panel;
		};

		Add(MakePanel(true));

		CVerticalLayoutUI* mid = new CVerticalLayoutUI();
		mid->SetFixedWidth(ScaleValue(48));
		mid->SetGap(ScaleValue(8));
		CControlUI* sp1 = new CControlUI();
		mid->Add(sp1);

		m_pBtnToTarget = new CButtonUI();
		m_pBtnToTarget->SetText(_T(">"));
		m_pBtnToTarget->SetFixedHeight(ScaleValue(32));
		m_pBtnToTarget->SetKind(CONTROLKIND_PRIMARY);
		m_pBtnToTarget->SetEnabled(false);
		m_pBtnToTarget->OnNotify += MakeDelegate(this, &CTransferUI::OnBtnNotify);
		mid->Add(m_pBtnToTarget);

		m_pBtnToSource = new CButtonUI();
		m_pBtnToSource->SetText(_T("<"));
		m_pBtnToSource->SetFixedHeight(ScaleValue(32));
		m_pBtnToSource->SetKind(CONTROLKIND_DEFAULT);
		m_pBtnToSource->SetEnabled(false);
		m_pBtnToSource->OnNotify += MakeDelegate(this, &CTransferUI::OnBtnNotify);
		mid->Add(m_pBtnToSource);

		CControlUI* sp2 = new CControlUI();
		mid->Add(sp2);
		Add(mid);

		Add(MakePanel(false));

		// 建壳后立刻套当前主题，避免先闪浅色
		CThemeManager* tm = CThemeManager::GetInstance();
		CTheme* th = NULL;
		if( tm != NULL ) {
			th = tm->GetCurrentTheme();
			if( th == NULL ) th = tm->FindTheme(tm->GetDefaultThemeId());
		}
		if( th != NULL ) {
			DWORD ctrlBg = th->GetToken(_T("color-control-bg"), th->GetToken(_T("color-bg"), 0xFFFFFFFF));
			DWORD bgElev = th->GetToken(_T("color-bg-elevated"), 0xF8F9FAFF);
			DWORD border = th->GetToken(_T("color-border"), 0xDEE2E6FF);
			DWORD text = th->GetToken(_T("color-text"), 0x000000E0);
			DWORD textSec = th->GetToken(_T("color-text-secondary"), 0x000000A6);
			DWORD bgHover = th->GetToken(_T("color-bg-hover"), bgElev);
			DWORD selection = th->GetToken(_T("color-selection"), bgElev);
			DWORD primaryOn = th->GetToken(_T("color-primary-text"), 0xFFFFFFFF);
			ApplyThemeChrome(ctrlBg, bgElev, border, text, textSec,
				ctrlBg, text, bgHover, selection, primaryOn, border);
		}
	}

	void CTransferUI::ApplyThemeChrome(DWORD dwPanelBg, DWORD dwHeaderBg, DWORD dwBorder,
		DWORD dwTitleColor, DWORD dwCountColor,
		DWORD dwListBg, DWORD dwItemColor, DWORD dwItemHoverBg,
		DWORD dwItemSelBg, DWORD dwItemSelColor, DWORD dwItemLine)
	{
		auto paintSide = [&](CVerticalLayoutUI* panel, CHorizontalLayoutUI* header,
			CLabelUI* title, CLabelUI* count, CListUI* list) {
			if( panel != NULL ) {
				panel->SetBackgroundColor(dwPanelBg);
				panel->SetBorderColor(dwBorder);
			}
			if( header != NULL )
				header->SetBackgroundColor(dwHeaderBg);
			if( title != NULL )
				title->SetColor(dwTitleColor);
			if( count != NULL )
				count->SetColor(dwCountColor);
			if( list != NULL ) {
				list->SetBackgroundColor(dwListBg);
				list->SetBorderColor(dwBorder);
				CDuiString s;
				s.Format(_T("#%08X"), dwItemColor);
				list->SetAttribute(_T("item-color"), s.GetData());
				s.Format(_T("#%08X"), dwListBg);
				list->SetAttribute(_T("item-background-color"), s.GetData());
				s.Format(_T("#%08X"), dwItemColor);
				list->SetAttribute(_T("item-color-hover"), s.GetData());
				s.Format(_T("#%08X"), dwItemHoverBg);
				list->SetAttribute(_T("item-background-color-hover"), s.GetData());
				s.Format(_T("#%08X"), dwItemSelColor);
				list->SetAttribute(_T("item-color-selected"), s.GetData());
				s.Format(_T("#%08X"), dwItemSelBg);
				list->SetAttribute(_T("item-background-color-selected"), s.GetData());
				s.Format(_T("#%08X"), dwItemLine);
				list->SetAttribute(_T("item-line-color"), s.GetData());
			}
		};
		paintSide(m_pLeftPanel, m_pLeftHeader, m_pLeftTitle, m_pLeftCount, m_pLeftList);
		paintSide(m_pRightPanel, m_pRightHeader, m_pRightTitle, m_pRightCount, m_pRightList);
		Invalidate();
	}

	void CTransferUI::FillList(CListUI* pList, bool bTarget)
	{
		if( pList == NULL ) return;
		pList->RemoveAll();
		for( size_t i = 0; i < m_items.size(); ++i ) {
			if( m_items[i].bTarget != bTarget ) continue;
			CListContainerElementUI* row = new CListContainerElementUI();
			row->SetFixedHeight(ScaleValue(m_nItemHeight));
			row->SetPadding(CDuiBox(0, 10, 0, 10));
			row->SetGap(ScaleValue(8));

			CCheckBoxUI* cb = new CCheckBoxUI();
			cb->SetText(m_items[i].sText);
			cb->Selected(m_items[i].bChecked, false);
			CDuiString ud;
			ud.Format(_T("%d"), (int)i);
			cb->SetUserData(ud);
			cb->OnNotify += MakeDelegate(this, &CTransferUI::OnCheckNotify);
			row->Add(cb);
			pList->Add(row);
		}
	}

	void CTransferUI::RefreshLists()
	{
		if( !m_bBuilt || m_pLeftList == NULL || m_pRightList == NULL ) return;
		m_bRefreshing = true;
		FillList(m_pLeftList, false);
		FillList(m_pRightList, true);
		m_bRefreshing = false;
		UpdateHeaders();
		UpdateButtons();
		NeedUpdate();
	}

	void CTransferUI::UpdateHeaders()
	{
		if( m_pLeftTitle ) m_pLeftTitle->SetText(m_sSourceTitle);
		if( m_pRightTitle ) m_pRightTitle->SetText(m_sTargetTitle);

		int nSrc = CountSide(false);
		int nSrcChk = CountChecked(false);
		int nTgt = CountSide(true);
		int nTgtChk = CountChecked(true);

		if( m_pLeftCount ) {
			CDuiString s;
			s.Format(_T("%d/%d"), nSrcChk, nSrc);
			m_pLeftCount->SetText(s);
		}
		if( m_pRightCount ) {
			CDuiString s;
			s.Format(_T("%d/%d"), nTgtChk, nTgt);
			m_pRightCount->SetText(s);
		}

		m_bRefreshing = true;
		if( m_pLeftAll ) {
			bool all = (nSrc > 0 && nSrcChk == nSrc);
			m_pLeftAll->Selected(all, false);
		}
		if( m_pRightAll ) {
			bool all = (nTgt > 0 && nTgtChk == nTgt);
			m_pRightAll->Selected(all, false);
		}
		m_bRefreshing = false;
	}

	void CTransferUI::UpdateButtons()
	{
		if( m_pBtnToTarget ) m_pBtnToTarget->SetEnabled(CountChecked(false) > 0);
		if( m_pBtnToSource ) m_pBtnToSource->SetEnabled(CountChecked(true) > 0);
	}

	void CTransferUI::MoveCheckedToTarget(bool bNotify)
	{
		bool moved = false;
		for( size_t i = 0; i < m_items.size(); ++i ) {
			if( !m_items[i].bTarget && m_items[i].bChecked ) {
				m_items[i].bTarget = true;
				m_items[i].bChecked = false;
				moved = true;
			}
		}
		if( !moved ) return;
		RefreshLists();
		if( bNotify && m_pManager )
			m_pManager->SendNotify(this, _T("transferchange"));
	}

	void CTransferUI::MoveCheckedToSource(bool bNotify)
	{
		bool moved = false;
		for( size_t i = 0; i < m_items.size(); ++i ) {
			if( m_items[i].bTarget && m_items[i].bChecked ) {
				m_items[i].bTarget = false;
				m_items[i].bChecked = false;
				moved = true;
			}
		}
		if( !moved ) return;
		RefreshLists();
		if( bNotify && m_pManager )
			m_pManager->SendNotify(this, _T("transferchange"));
	}

	bool CTransferUI::OnBtnNotify(void* param)
	{
		TNotifyUI* pMsg = (TNotifyUI*)param;
		if( pMsg == NULL || pMsg->sType != DUI_MSGTYPE_CLICK ) return true;
		if( pMsg->pSender == m_pBtnToTarget ) MoveCheckedToTarget(true);
		else if( pMsg->pSender == m_pBtnToSource ) MoveCheckedToSource(true);
		return true;
	}

	bool CTransferUI::OnCheckNotify(void* param)
	{
		if( m_bRefreshing ) return true;
		TNotifyUI* pMsg = (TNotifyUI*)param;
		if( pMsg == NULL || pMsg->sType != DUI_MSGTYPE_SELECTCHANGED ) return true;
		CCheckBoxUI* cb = static_cast<CCheckBoxUI*>(pMsg->pSender->GetInterface(DUI_CTR_CHECKBOX));
		if( cb == NULL ) return true;
		int idx = _ttoi(cb->GetUserData());
		if( idx < 0 || idx >= (int)m_items.size() ) return true;
		m_items[idx].bChecked = cb->IsSelected();
		UpdateHeaders();
		UpdateButtons();
		return true;
	}

	bool CTransferUI::OnHeaderCheckNotify(void* param)
	{
		if( m_bRefreshing ) return true;
		TNotifyUI* pMsg = (TNotifyUI*)param;
		if( pMsg == NULL || pMsg->sType != DUI_MSGTYPE_SELECTCHANGED ) return true;
		if( pMsg->pSender == m_pLeftAll )
			SetAllChecked(false, m_pLeftAll->IsSelected());
		else if( pMsg->pSender == m_pRightAll )
			SetAllChecked(true, m_pRightAll->IsSelected());
		return true;
	}

	void CTransferUI::EnsureBuilt()
	{
		if( m_bBuilt ) return;
		CollectDataNodes();
		BuildShell();
		m_bBuilt = true;
		RefreshLists();
	}

	void CTransferUI::DoInit()
	{
		CHorizontalLayoutUI::DoInit();
		EnsureBuilt();
	}

	void CTransferUI::SetAttribute(LPCTSTR pstrName, LPCTSTR pstrValue)
	{
		if( _tcsicmp(pstrName, _T("titles")) == 0 || _tcsicmp(pstrName, _T("title")) == 0 ) {
			SetTitles(pstrValue);
		}
		else if( _tcsicmp(pstrName, _T("items")) == 0 || _tcsicmp(pstrName, _T("options")) == 0 || _tcsicmp(pstrName, _T("data")) == 0 ) {
			m_sItemsAttr = pstrValue ? pstrValue : _T("");
			if( m_bBuilt ) SetItems(m_sItemsAttr);
		}
		else if( _tcsicmp(pstrName, _T("target")) == 0 || _tcsicmp(pstrName, _T("selected")) == 0
			|| _tcsicmp(pstrName, _T("target-keys")) == 0 ) {
			m_sTargetAttr = pstrValue ? pstrValue : _T("");
			if( m_bBuilt ) SetTargetKeys(m_sTargetAttr);
		}
		else if( _tcsicmp(pstrName, _T("item-height")) == 0 ) {
			SetItemHeight(_ttoi(pstrValue));
		}
		else if( _tcsicmp(pstrName, _T("show-select-all")) == 0 || _tcsicmp(pstrName, _T("showselectall")) == 0 ) {
			SetShowSelectAll(_tcsicmp(pstrValue, _T("true")) == 0);
		}
		else {
			CHorizontalLayoutUI::SetAttribute(pstrName, pstrValue);
		}
	}
}
