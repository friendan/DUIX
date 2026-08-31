#include "StdAfx.h"
#include "UIToolCard.h"

namespace DuiLib
{
	IMPLEMENT_DUICONTROL(CToolCardHeaderUI)
	IMPLEMENT_DUICONTROL(CToolCardBodyUI)
	IMPLEMENT_DUICONTROL(CToolCardUI)

	namespace
	{
		/// 整行标题栏：点击 / 悬停；右侧自定义槽挂在此行末尾
		class CToolCardChromeUI : public CHorizontalLayoutUI
		{
		public:
			CToolCardChromeUI() : m_pOwner(NULL) {}
			void SetOwner(CToolCardUI* pOwner) { m_pOwner = pOwner; }

			bool PreferClientHit() const override { return true; }

			void DoEvent(TEventUI& event) override
			{
				if( !IsMouseEnabled() && event.Type > UIEVENT__MOUSEBEGIN && event.Type < UIEVENT__MOUSEEND ) {
					BubbleEvent(event);
					return;
				}
				if( event.Type == UIEVENT_BUTTONUP ) {
					if( m_pOwner != NULL ) m_pOwner->OnHeaderClick();
					return;
				}
				if( event.Type == UIEVENT_MOUSEENTER ) {
					if( m_pOwner != NULL ) m_pOwner->OnHeaderHoverChanged(true);
				}
				else if( event.Type == UIEVENT_MOUSELEAVE ) {
					if( m_pOwner != NULL ) m_pOwner->OnHeaderHoverChanged(false);
				}
				CHorizontalLayoutUI::DoEvent(event);
			}

		protected:
			CToolCardUI* m_pOwner;
		};

		/// file 种：点文件名打开；其它 kind 鼠标穿透到 chrome 做折叠
		class CToolCardTitleUI : public CLabelUI
		{
		public:
			CToolCardTitleUI() : m_pOwner(NULL) {}
			void SetOwner(CToolCardUI* pOwner) { m_pOwner = pOwner; }

			void DoEvent(TEventUI& event) override
			{
				if( event.Type == UIEVENT_BUTTONUP && IsMouseEnabled() && m_pOwner != NULL ) {
					m_pOwner->OnTitleClick();
					return;
				}
				CLabelUI::DoEvent(event);
			}

		protected:
			CToolCardUI* m_pOwner;
		};
	}

	//////////////////////////////////////////////////////////////////////////
	// CToolCardHeaderUI — 标题栏右侧自定义槽
	CToolCardHeaderUI::CToolCardHeaderUI()
	{
		SetAttribute(_T("align-items"), _T("vcenter"));
		SetGap(6);
	}

	LPCTSTR CToolCardHeaderUI::GetClass() const
	{
		return _T("ToolCardHeaderUI");
	}

	LPVOID CToolCardHeaderUI::GetInterface(LPCTSTR pstrName)
	{
		if( _tcsicmp(pstrName, DUI_CTR_TOOLCARDHEADER) == 0 )
			return static_cast<CToolCardHeaderUI*>(this);
		return CHorizontalLayoutUI::GetInterface(pstrName);
	}

	SIZE CToolCardHeaderUI::EstimateSize(SIZE szAvailable)
	{
		SIZE sz = MeasureContent(szAvailable);
		if( GetFixedHeight() > 0 ) sz.cy = GetFixedHeight();
		else sz.cy = 0;
		if( sz.cx < 0 ) sz.cx = 0;
		return sz;
	}

	//////////////////////////////////////////////////////////////////////////
	// CToolCardBodyUI — 内容区 + 默认右键菜单
	namespace
	{
		enum {
			ID_TOOLCARD_SELECTALL = 42101,
			ID_TOOLCARD_COPY = 42102
		};
	}

	CToolCardBodyUI::CToolCardBodyUI()
		: m_bTextSelected(false)
		, m_dwNormalBk(0xFAFBFCFF)
		, m_dwSelectedBk(0xCFE2FFFF)
	{
		SetPadding(CDuiBox(8, 10, 8, 10));
		SetGap(4);
		SetBackgroundColor(m_dwNormalBk);
		EnableScrollBar(true, false);
		SetShowScrollbar(true);
		SetContextMenuUsed(true);
	}

	LPCTSTR CToolCardBodyUI::GetClass() const
	{
		return _T("ToolCardBodyUI");
	}

	LPVOID CToolCardBodyUI::GetInterface(LPCTSTR pstrName)
	{
		if( _tcsicmp(pstrName, DUI_CTR_TOOLCARDBODY) == 0 )
			return static_cast<CToolCardBodyUI*>(this);
		return CVerticalLayoutUI::GetInterface(pstrName);
	}

	void CToolCardBodyUI::CollectTextRecursive(CControlUI* pControl, CDuiString& sOut)
	{
		if( pControl == NULL || !pControl->IsVisible() ) return;
		CContainerUI* pCont = static_cast<CContainerUI*>(pControl->GetInterface(DUI_CTR_CONTAINER));
		if( pCont != NULL ) {
			for( int i = 0; i < pCont->GetCount(); ++i )
				CollectTextRecursive(pCont->GetItemAt(i), sOut);
			return;
		}
		CDuiString s = pControl->GetText();
		if( s.IsEmpty() ) return;
		if( !sOut.IsEmpty() ) sOut += _T("\r\n");
		sOut += s;
	}

	CDuiString CToolCardBodyUI::CollectBodyText() const
	{
		CDuiString s;
		for( int i = 0; i < GetCount(); ++i )
			CollectTextRecursive(GetItemAt(i), s);
		return s;
	}

	bool CToolCardBodyUI::CopyTextToClipboard(HWND hWnd, LPCTSTR pstrText)
	{
		if( pstrText == NULL || *pstrText == _T('\0') ) return false;
		if( !::OpenClipboard(hWnd) ) return false;
		::EmptyClipboard();
		bool bOk = false;
		size_t nBytes = (_tcslen(pstrText) + 1) * sizeof(TCHAR);
		HGLOBAL hMem = ::GlobalAlloc(GMEM_MOVEABLE, nBytes);
		if( hMem != NULL ) {
			void* p = ::GlobalLock(hMem);
			if( p != NULL ) {
				memcpy(p, pstrText, nBytes);
				::GlobalUnlock(hMem);
#ifdef _UNICODE
				bOk = (::SetClipboardData(CF_UNICODETEXT, hMem) != NULL);
#else
				bOk = (::SetClipboardData(CF_TEXT, hMem) != NULL);
#endif
				if( !bOk ) ::GlobalFree(hMem);
			}
			else {
				::GlobalFree(hMem);
			}
		}
		::CloseClipboard();
		return bOk;
	}

	void CToolCardBodyUI::ApplySelectionChrome()
	{
		SetBackgroundColor(m_bTextSelected ? m_dwSelectedBk : m_dwNormalBk);
		Invalidate();
	}

	void CToolCardBodyUI::SelectAllText()
	{
		m_bTextSelected = true;
		ApplySelectionChrome();
	}

	void CToolCardBodyUI::ClearTextSelection()
	{
		if( !m_bTextSelected ) return;
		m_bTextSelected = false;
		ApplySelectionChrome();
	}

	bool CToolCardBodyUI::CopyBodyText()
	{
		CDuiString s = CollectBodyText();
		if( s.IsEmpty() ) return false;
		HWND hWnd = (m_pManager != NULL) ? m_pManager->GetPaintWindow() : NULL;
		return CopyTextToClipboard(hWnd, s.GetData());
	}

	void CToolCardBodyUI::ShowBuiltinContextMenu(POINT ptClient)
	{
		if( m_pManager == NULL ) return;
		HWND hPaint = m_pManager->GetPaintWindow();
		POINT pt = ptClient;
		::ClientToScreen(hPaint, &pt);

		CDuiString sText = CollectBodyText();
		const bool bHasText = !sText.IsEmpty();

		HMENU hPop = ::CreatePopupMenu();
		if( hPop == NULL ) return;
		::AppendMenu(hPop, MF_STRING, ID_TOOLCARD_SELECTALL, _T("全选(&A)"));
		::AppendMenu(hPop, MF_STRING | (bHasText ? 0 : MF_GRAYED),
			ID_TOOLCARD_COPY, _T("复制(&C)"));
		if( !bHasText )
			::EnableMenuItem(hPop, ID_TOOLCARD_SELECTALL, MF_BYCOMMAND | MF_GRAYED);

		UINT uCmd = (UINT)::TrackPopupMenu(hPop,
			TPM_LEFTALIGN | TPM_RIGHTBUTTON | TPM_RETURNCMD,
			pt.x, pt.y, 0, hPaint, NULL);
		::DestroyMenu(hPop);

		if( uCmd == ID_TOOLCARD_SELECTALL )
			SelectAllText();
		else if( uCmd == ID_TOOLCARD_COPY )
			CopyBodyText();
	}

	void CToolCardBodyUI::DoEvent(TEventUI& event)
	{
		if( event.Type == UIEVENT_BUTTONDOWN || event.Type == UIEVENT_DBLCLICK ) {
			ClearTextSelection();
			if( m_pManager != NULL ) m_pManager->SetFocus(this);
		}
		if( event.Type == UIEVENT_CONTEXTMENU ) {
			if( IsContextMenuUsed() ) {
				POINT pt = event.ptMouse;
				ShowBuiltinContextMenu(pt);
				return;
			}
		}
		if( event.Type == UIEVENT_KEYDOWN && IsEnabled() ) {
			const bool bCtrl = (::GetKeyState(VK_CONTROL) & 0x8000) != 0;
			if( bCtrl && (event.chKey == _T('A') || event.chKey == _T('a')) ) {
				SelectAllText();
				return;
			}
			if( bCtrl && (event.chKey == _T('C') || event.chKey == _T('c')) ) {
				CopyBodyText();
				return;
			}
		}
		CVerticalLayoutUI::DoEvent(event);
	}

	void CToolCardBodyUI::SetAttribute(LPCTSTR pstrName, LPCTSTR pstrValue)
	{
		if( _tcsicmp(pstrName, _T("menu")) == 0 || _tcsicmp(pstrName, _T("contextmenu")) == 0 ) {
			SetContextMenuUsed(_tcsicmp(pstrValue, _T("true")) == 0 || _tcscmp(pstrValue, _T("1")) == 0);
		}
		else if( _tcsicmp(pstrName, _T("background-color")) == 0
			|| _tcsicmp(pstrName, _T("bkcolor")) == 0
			|| _tcsicmp(pstrName, _T("bk-color")) == 0 ) {
			CVerticalLayoutUI::SetAttribute(pstrName, pstrValue);
			m_dwNormalBk = GetBackgroundColor();
			if( !m_bTextSelected ) SetBackgroundColor(m_dwNormalBk);
		}
		else {
			CVerticalLayoutUI::SetAttribute(pstrName, pstrValue);
		}
	}

	//////////////////////////////////////////////////////////////////////////
	CToolCardUI::CToolCardUI()
		: m_kind(TOOLCARD_GENERIC)
		, m_bExpanded(true)
		, m_bBuilt(false)
		, m_bHeaderHover(false)
		, m_bShowChevron(true)
		, m_bShowKindBadge(true)
		, m_bShowTitle(true)
		, m_bBadgeBkCustom(false)
		, m_nHeaderHeight(32)
		, m_nBodyHeight(0)
		, m_nBodyMinHeight(100)
		, m_nBodyMaxHeight(500)
		, m_nBadgeWidth(44)
		, m_pHeader(NULL)
		, m_pHeaderSlot(NULL)
		, m_pChevron(NULL)
		, m_pKindBadge(NULL)
		, m_pTitle(NULL)
		, m_pBody(NULL)
		, m_pBodyLabel(NULL)
		, m_dwHeaderBk(0xF1F3F5FF)
		, m_dwHeaderHoverBk(0xE9ECEFFF)
		, m_dwChevronColor(0x6C757DFF)
		, m_dwTitleColor(0x212529FF)
		, m_dwTitleLinkColor(0x0D6EFDFF)
		, m_dwBadgeTextColor(0xFFFFFFFF)
		, m_dwBadgeBkCustom(0)
	{
		SetGap(0);
		SetBorderWidth(1);
		SetBorderColor(0xDEE2E6FF);
		SIZE szRound = { 6, 6 };
		SetBorderRadius(szRound);
		SetBackgroundColor(0xFFFFFFFF);
	}

	CToolCardUI::~CToolCardUI()
	{
	}

	LPCTSTR CToolCardUI::GetClass() const
	{
		return _T("ToolCardUI");
	}

	LPVOID CToolCardUI::GetInterface(LPCTSTR pstrName)
	{
		if( _tcsicmp(pstrName, DUI_CTR_TOOLCARD) == 0 )
			return static_cast<CToolCardUI*>(this);
		return CVerticalLayoutUI::GetInterface(pstrName);
	}

	void CToolCardUI::DoInit()
	{
		CVerticalLayoutUI::DoInit();
		EnsureBuilt();
		SyncKindBadge();
		SyncTitleFromMeta();
		SyncHeaderChrome();
		SyncBodyVisibility();
		UpdateFixedHeight();
	}

	void CToolCardUI::EnsureBuilt()
	{
		if( m_bBuilt ) return;
		m_bBuilt = true;

		CToolCardChromeUI* pChrome = new CToolCardChromeUI;
		pChrome->SetOwner(this);
		pChrome->SetFixedHeight(m_nHeaderHeight);
		pChrome->SetBackgroundColor(m_dwHeaderBk);
		pChrome->SetMouseEnabled(true);
		pChrome->SetGap(8);
		pChrome->SetAlignItems(DT_VCENTER);
		pChrome->SetPadding(CDuiBox(0, 10, 0, 10));
		CVerticalLayoutUI::Add(pChrome);
		m_pHeader = pChrome;

		m_pChevron = new CLabelUI;
		m_pChevron->SetName(_T("toolcard_chevron"));
		m_pChevron->SetText(_T("\u25BC"));
		m_pChevron->SetTextStyle(DT_CENTER | DT_VCENTER | DT_SINGLELINE);
		m_pChevron->SetColor(m_dwChevronColor);
		m_pChevron->SetFixedWidth(16);
		m_pChevron->SetMouseEnabled(false);
		m_pHeader->Add(m_pChevron);

		m_pKindBadge = new CLabelUI;
		m_pKindBadge->SetName(_T("toolcard_badge"));
		m_pKindBadge->SetTextStyle(DT_CENTER | DT_VCENTER | DT_SINGLELINE);
		m_pKindBadge->SetColor(m_dwBadgeTextColor);
		m_pKindBadge->SetBackgroundColor(0x0D6EFDFF);
		SIZE szBadgeRound = { 4, 4 };
		m_pKindBadge->SetBorderRadius(szBadgeRound);
		m_pKindBadge->SetFixedWidth(m_nBadgeWidth);
		m_pKindBadge->SetFixedHeight(20);
		m_pKindBadge->SetMouseEnabled(false);
		m_pKindBadge->SetFont(0);
		m_pHeader->Add(m_pKindBadge);

		m_pTitle = new CToolCardTitleUI;
		static_cast<CToolCardTitleUI*>(m_pTitle)->SetOwner(this);
		m_pTitle->SetName(_T("toolcard_title"));
		m_pTitle->SetTextStyle(DT_LEFT | DT_VCENTER | DT_SINGLELINE | DT_END_ELLIPSIS);
		m_pTitle->SetColor(m_dwTitleColor);
		m_pTitle->SetMouseEnabled(false);
		m_pTitle->SetFixedWidth(0);
		m_pHeader->Add(m_pTitle);

		m_pHeaderSlot = new CToolCardHeaderUI;
		m_pHeaderSlot->SetName(_T("toolcard_header"));
		m_pHeader->Add(m_pHeaderSlot);
		SyncHeaderSlotMetrics();

		m_pBody = new CToolCardBodyUI;
		m_pBody->SetName(_T("toolcard_body"));
		SyncBodySlotDefaults(m_pBody);
		CVerticalLayoutUI::Add(m_pBody);
	}

	void CToolCardUI::SyncHeaderSlotMetrics()
	{
		if( m_pHeaderSlot == NULL ) return;
		const int h = m_nHeaderHeight > 0 ? m_nHeaderHeight : 32;
		m_pHeaderSlot->SetFixedHeight(h);
		if( m_pHeaderSlot->GetName().IsEmpty() )
			m_pHeaderSlot->SetName(_T("toolcard_header"));
	}

	void CToolCardUI::SyncBodySlotDefaults(CToolCardBodyUI* pBody)
	{
		if( pBody == NULL ) return;
		if( pBody->GetName().IsEmpty() )
			pBody->SetName(_T("toolcard_body"));
		pBody->EnableScrollBar(true, false);
		pBody->SetShowScrollbar(true);
	}

	void CToolCardUI::DetachFromParent(CContainerUI* pParent, CControlUI* pChild)
	{
		if( pParent == NULL || pChild == NULL ) return;
		pParent->SetAutoDestroy(false);
		pParent->Remove(pChild);
		pParent->SetAutoDestroy(true);
	}

	bool CToolCardUI::IsHeaderSlotControl(CControlUI* pControl) const
	{
		return pControl != NULL && pControl->GetInterface(DUI_CTR_TOOLCARDHEADER) != NULL;
	}

	bool CToolCardUI::IsBodySlotControl(CControlUI* pControl) const
	{
		return pControl != NULL && pControl->GetInterface(DUI_CTR_TOOLCARDBODY) != NULL;
	}

	bool CToolCardUI::AdoptHeaderSlot(CToolCardHeaderUI* pNew)
	{
		if( pNew == NULL ) return false;
		EnsureBuilt();
		if( pNew == m_pHeaderSlot ) return true;

		if( m_pHeaderSlot != NULL ) {
			while( m_pHeaderSlot->GetCount() > 0 ) {
				CControlUI* p = m_pHeaderSlot->GetItemAt(0);
				DetachFromParent(m_pHeaderSlot, p);
				pNew->Add(p);
			}
			DetachFromParent(m_pHeader, m_pHeaderSlot);
			delete m_pHeaderSlot;
			m_pHeaderSlot = NULL;
		}

		m_pHeaderSlot = pNew;
		SyncHeaderSlotMetrics();
		if( m_pHeader == NULL ) return false;
		return m_pHeader->Add(m_pHeaderSlot);
	}

	bool CToolCardUI::AdoptBodySlot(CToolCardBodyUI* pNew)
	{
		if( pNew == NULL ) return false;
		EnsureBuilt();
		if( pNew == m_pBody ) return true;

		if( m_pBody != NULL ) {
			while( m_pBody->GetCount() > 0 ) {
				CControlUI* p = m_pBody->GetItemAt(0);
				DetachFromParent(m_pBody, p);
				pNew->Add(p);
			}
			if( m_pBodyLabel != NULL && m_pBodyLabel->GetParent() != pNew )
				m_pBodyLabel = NULL;
			DetachFromParent(this, m_pBody);
			delete m_pBody;
			m_pBody = NULL;
		}

		m_pBody = pNew;
		SyncBodySlotDefaults(m_pBody);
		const bool ok = CVerticalLayoutUI::Add(m_pBody);
		SyncBodyVisibility();
		UpdateFixedHeight();
		return ok;
	}

	void CToolCardUI::SetCardKind(Kind kind)
	{
		if( m_kind == kind ) return;
		m_kind = kind;
		EnsureBuilt();
		SyncKindBadge();
		SyncTitleFromMeta();
		SyncHeaderChrome();
		Invalidate();
	}

	void CToolCardUI::SetCardKindString(LPCTSTR pstr)
	{
		if( pstr == NULL ) return;
		if( _tcsicmp(pstr, _T("file")) == 0 ) SetCardKind(TOOLCARD_FILE);
		else if( _tcsicmp(pstr, _T("cmd")) == 0 || _tcsicmp(pstr, _T("command")) == 0
			|| _tcsicmp(pstr, _T("shell")) == 0 )
			SetCardKind(TOOLCARD_CMD);
		else
			SetCardKind(TOOLCARD_GENERIC);
	}

	CDuiString CToolCardUI::GetCardKindString() const
	{
		switch( m_kind ) {
		case TOOLCARD_FILE: return _T("file");
		case TOOLCARD_CMD: return _T("cmd");
		default: return _T("generic");
		}
	}

	void CToolCardUI::SyncKindBadge()
	{
		EnsureBuilt();
		if( m_pKindBadge == NULL ) return;

		if( !m_sBadgeTextOverride.IsEmpty() )
			m_pKindBadge->SetText(m_sBadgeTextOverride.GetData());
		else {
			switch( m_kind ) {
			case TOOLCARD_FILE: m_pKindBadge->SetText(_T("File")); break;
			case TOOLCARD_CMD: m_pKindBadge->SetText(_T("Cmd")); break;
			default: m_pKindBadge->SetText(_T("Tool")); break;
			}
		}

		if( m_bBadgeBkCustom )
			m_pKindBadge->SetBackgroundColor(m_dwBadgeBkCustom);
		else {
			switch( m_kind ) {
			case TOOLCARD_FILE: m_pKindBadge->SetBackgroundColor(0x198754FF); break;
			case TOOLCARD_CMD: m_pKindBadge->SetBackgroundColor(0x0D6EFDFF); break;
			default: m_pKindBadge->SetBackgroundColor(0x6C757DFF); break;
			}
		}
		m_pKindBadge->SetColor(m_dwBadgeTextColor);
		if( m_nBadgeWidth > 0 ) m_pKindBadge->SetFixedWidth(m_nBadgeWidth);
	}

	bool CToolCardUI::ParseBoolValue(LPCTSTR pstrValue)
	{
		if( pstrValue == NULL ) return false;
		return ( _tcsicmp(pstrValue, _T("true")) == 0
			|| _tcscmp(pstrValue, _T("1")) == 0
			|| _tcsicmp(pstrValue, _T("yes")) == 0 );
	}

	DWORD CToolCardUI::ParseColorValue(LPCTSTR pstrValue)
	{
		DWORD clr = 0;
		if( pstrValue != NULL && ParseColorString(pstrValue, clr) ) return clr;
		return 0;
	}

	void CToolCardUI::SetShowChevron(bool b)
	{
		if( m_bShowChevron == b ) return;
		m_bShowChevron = b;
		SyncHeaderChrome();
	}

	void CToolCardUI::SetShowKindBadge(bool b)
	{
		if( m_bShowKindBadge == b ) return;
		m_bShowKindBadge = b;
		SyncHeaderChrome();
	}

	void CToolCardUI::SetShowTitle(bool b)
	{
		if( m_bShowTitle == b ) return;
		m_bShowTitle = b;
		SyncHeaderChrome();
	}

	void CToolCardUI::SetHeaderBkColor(DWORD dwColor)
	{
		if( m_dwHeaderBk == dwColor ) return;
		m_dwHeaderBk = dwColor;
		SyncHeaderChrome();
	}

	void CToolCardUI::SetHeaderHoverBkColor(DWORD dwColor)
	{
		if( m_dwHeaderHoverBk == dwColor ) return;
		m_dwHeaderHoverBk = dwColor;
		SyncHeaderChrome();
	}

	void CToolCardUI::SetChevronColor(DWORD dwColor)
	{
		m_dwChevronColor = dwColor;
		EnsureBuilt();
		if( m_pChevron != NULL ) m_pChevron->SetColor(m_dwChevronColor);
		Invalidate();
	}

	void CToolCardUI::SetTitleColor(DWORD dwColor)
	{
		m_dwTitleColor = dwColor;
		SyncHeaderChrome();
	}

	void CToolCardUI::SetTitleLinkColor(DWORD dwColor)
	{
		m_dwTitleLinkColor = dwColor;
		SyncHeaderChrome();
	}

	void CToolCardUI::SetKindBadgeText(LPCTSTR pstr)
	{
		m_sBadgeTextOverride = pstr ? pstr : _T("");
		SyncKindBadge();
		Invalidate();
	}

	CDuiString CToolCardUI::GetKindBadgeText() const
	{
		if( !m_sBadgeTextOverride.IsEmpty() ) return m_sBadgeTextOverride;
		if( m_pKindBadge != NULL ) return m_pKindBadge->GetText();
		return CDuiString();
	}

	void CToolCardUI::SetKindBadgeTextColor(DWORD dwColor)
	{
		m_dwBadgeTextColor = dwColor;
		SyncKindBadge();
		Invalidate();
	}

	void CToolCardUI::SetKindBadgeBkColor(DWORD dwColor)
	{
		if( dwColor == 0 ) {
			m_bBadgeBkCustom = false;
			m_dwBadgeBkCustom = 0;
		}
		else {
			m_bBadgeBkCustom = true;
			m_dwBadgeBkCustom = dwColor;
		}
		SyncKindBadge();
		Invalidate();
	}

	DWORD CToolCardUI::GetKindBadgeBkColor() const
	{
		if( m_bBadgeBkCustom ) return m_dwBadgeBkCustom;
		if( m_pKindBadge != NULL ) return m_pKindBadge->GetBackgroundColor();
		return 0;
	}

	void CToolCardUI::SetKindBadgeWidth(int n)
	{
		if( n < 0 ) n = 0;
		if( m_nBadgeWidth == n ) return;
		m_nBadgeWidth = n;
		EnsureBuilt();
		if( m_pKindBadge != NULL && m_nBadgeWidth > 0 )
			m_pKindBadge->SetFixedWidth(m_nBadgeWidth);
		RequestAncestorLayout();
	}

	void CToolCardUI::SyncTitleFromMeta()
	{
		EnsureBuilt();
		if( m_pTitle == NULL ) return;
		if( !m_sTitleOverride.IsEmpty() ) {
			m_pTitle->SetText(m_sTitleOverride.GetData());
			return;
		}
		if( m_kind == TOOLCARD_FILE && !m_sPath.IsEmpty() )
			m_pTitle->SetText(m_sPath.GetData());
		else if( m_kind == TOOLCARD_CMD && !m_sCommand.IsEmpty() )
			m_pTitle->SetText(m_sCommand.GetData());
	}

	void CToolCardUI::SetTitle(LPCTSTR pstrText)
	{
		m_sTitleOverride = pstrText ? pstrText : _T("");
		EnsureBuilt();
		SyncTitleFromMeta();
	}

	CDuiString CToolCardUI::GetTitle() const
	{
		if( m_pTitle != NULL ) return m_pTitle->GetText();
		return m_sTitleOverride;
	}

	void CToolCardUI::SetPath(LPCTSTR pstrPath)
	{
		m_sPath = pstrPath ? pstrPath : _T("");
		if( m_kind == TOOLCARD_GENERIC && !m_sPath.IsEmpty() )
			m_kind = TOOLCARD_FILE;
		EnsureBuilt();
		SyncKindBadge();
		SyncTitleFromMeta();
	}

	void CToolCardUI::SetCommand(LPCTSTR pstrCmd)
	{
		m_sCommand = pstrCmd ? pstrCmd : _T("");
		if( m_kind == TOOLCARD_GENERIC && !m_sCommand.IsEmpty() )
			m_kind = TOOLCARD_CMD;
		EnsureBuilt();
		SyncKindBadge();
		SyncTitleFromMeta();
	}

	void CToolCardUI::SetHeaderHeight(int n)
	{
		if( n < 20 ) n = 20;
		if( m_nHeaderHeight == n ) return;
		m_nHeaderHeight = n;
		EnsureBuilt();
		if( m_pHeader != NULL ) m_pHeader->SetFixedHeight(m_nHeaderHeight);
		SyncHeaderSlotMetrics();
		UpdateFixedHeight();
		RequestAncestorLayout();
	}

	void CToolCardUI::SetBodyHeight(int n)
	{
		if( n < 0 ) n = 0;
		if( m_nBodyHeight == n ) return;
		m_nBodyHeight = n;
		UpdateFixedHeight();
		RequestAncestorLayout();
	}

	void CToolCardUI::SetBodyMinHeight(int n)
	{
		if( n < 0 ) n = 0;
		if( m_nBodyMinHeight == n ) return;
		m_nBodyMinHeight = n;
		UpdateFixedHeight();
		RequestAncestorLayout();
	}

	void CToolCardUI::SetBodyMaxHeight(int n)
	{
		if( n < 0 ) n = 0;
		if( m_nBodyMaxHeight == n ) return;
		m_nBodyMaxHeight = n;
		UpdateFixedHeight();
		RequestAncestorLayout();
	}

	int CToolCardUI::ScaleValue(int v) const
	{
		return m_pManager ? m_pManager->GetDPIObj()->Scale(v) : v;
	}

	void CToolCardUI::SetBodyText(LPCTSTR pstr)
	{
		EnsureBuilt();
		if( m_pBody == NULL ) return;
		if( m_pBodyLabel == NULL ) {
			m_pBodyLabel = new CLabelUI;
			m_pBodyLabel->SetTextStyle(DT_LEFT | DT_TOP | DT_WORDBREAK);
			m_pBodyLabel->SetColor(0x495057FF);
			m_pBodyLabel->SetMouseEnabled(false);
			m_pBody->Add(m_pBodyLabel);
		}
		m_pBodyLabel->SetText(pstr ? pstr : _T(""));
		UpdateFixedHeight();
		RequestAncestorLayout();
	}

	LPCTSTR CToolCardUI::GetBodyText() const
	{
		if( m_pBodyLabel != NULL ) return m_pBodyLabel->GetText().GetData();
		return _T("");
	}

	CLabelUI* CToolCardUI::AppendBodyLine(LPCTSTR pstrText, DWORD dwColor, int nHeight)
	{
		EnsureBuilt();
		if( m_pBody == NULL ) return NULL;
		CLabelUI* pLab = new CLabelUI;
		pLab->SetText(pstrText ? pstrText : _T(""));
		pLab->SetTextStyle(DT_LEFT | DT_VCENTER | DT_SINGLELINE | DT_END_ELLIPSIS);
		pLab->SetColor(dwColor);
		if( nHeight > 0 ) pLab->SetFixedHeight(nHeight);
		pLab->SetMouseEnabled(true);
		m_pBody->Add(pLab);
		m_pBody->ClearTextSelection();
		UpdateFixedHeight();
		RequestAncestorLayout();
		return pLab;
	}

	void CToolCardUI::ClearBody()
	{
		EnsureBuilt();
		if( m_pBody == NULL ) return;
		m_pBody->RemoveAll();
		m_pBodyLabel = NULL;
		m_pBody->ClearTextSelection();
		UpdateFixedHeight();
		RequestAncestorLayout();
	}

	CDuiString CToolCardUI::CollectBodyText() const
	{
		if( m_pBody == NULL ) return CDuiString();
		return m_pBody->CollectBodyText();
	}

	void CToolCardUI::SelectAllBody()
	{
		EnsureBuilt();
		if( m_pBody != NULL ) m_pBody->SelectAllText();
	}

	bool CToolCardUI::CopyBodyText()
	{
		EnsureBuilt();
		return m_pBody != NULL && m_pBody->CopyBodyText();
	}

	void CToolCardUI::SetExpanded(bool bExpanded, bool bNotify)
	{
		EnsureBuilt();
		if( m_bExpanded == bExpanded ) {
			SyncBodyVisibility();
			SyncHeaderChrome();
			UpdateFixedHeight();
			return;
		}
		m_bExpanded = bExpanded;
		SyncBodyVisibility();
		SyncHeaderChrome();
		UpdateFixedHeight();
		RequestAncestorLayout();
		if( bNotify && m_pManager != NULL ) {
			m_pManager->SendNotify(this,
				m_bExpanded ? DUI_MSGTYPE_ITEMEXPAND : DUI_MSGTYPE_ITEMCOLLAPSE);
		}
	}

	void CToolCardUI::ToggleExpanded()
	{
		SetExpanded(!m_bExpanded, true);
	}

	void CToolCardUI::SyncHeaderChrome()
	{
		EnsureBuilt();
		if( m_pHeader == NULL ) return;
		m_pHeader->SetBackgroundColor(m_bHeaderHover ? m_dwHeaderHoverBk : m_dwHeaderBk);
		if( m_pChevron != NULL ) {
			m_pChevron->SetVisible(m_bShowChevron);
			m_pChevron->SetColor(m_dwChevronColor);
			m_pChevron->SetText(m_bExpanded ? _T("\u25B2") : _T("\u25BC"));
		}
		if( m_pKindBadge != NULL )
			m_pKindBadge->SetVisible(m_bShowKindBadge);
		if( m_pTitle != NULL ) {
			m_pTitle->SetVisible(m_bShowTitle);
			if( m_kind == TOOLCARD_FILE ) {
				m_pTitle->SetColor(m_dwTitleLinkColor);
				m_pTitle->SetMouseEnabled(true);
			}
			else {
				m_pTitle->SetColor(m_dwTitleColor);
				m_pTitle->SetMouseEnabled(false);
			}
		}
	}

	void CToolCardUI::SyncBodyVisibility()
	{
		EnsureBuilt();
		if( m_pBody == NULL ) return;
		m_pBody->SetVisible(m_bExpanded);
		if( m_bExpanded ) m_pBody->SetInternVisible(true);
	}

	int CToolCardUI::CalcBodyViewportHeight(int cxAvail) const
	{
		if( !m_bExpanded || m_pBody == NULL ) return 0;

		int minH = m_nBodyMinHeight > 0 ? ScaleValue(m_nBodyMinHeight) : 0;
		int maxH = m_nBodyMaxHeight > 0 ? ScaleValue(m_nBodyMaxHeight) : 0;
		int h = 0;
		if( m_nBodyHeight > 0 ) {
			h = ScaleValue(m_nBodyHeight);
		}
		else {
			SIZE szAvail = { cxAvail > 0 ? cxAvail : 480, 9999 };
			SIZE szContent = m_pBody->MeasureContent(szAvail);
			h = szContent.cy;
			if( h < 0 ) h = 0;
		}
		if( minH > 0 && h < minH ) h = minH;
		if( maxH > 0 && h > maxH ) h = maxH;
		return h;
	}

	void CToolCardUI::UpdateFixedHeight()
	{
		EnsureBuilt();
		int headerH = ScaleValue(m_nHeaderHeight);
		if( m_pHeader != NULL ) m_pHeader->SetFixedHeight(headerH);

		int cy = headerH;
		if( m_bExpanded && m_pBody != NULL ) {
			int cx = 0;
			RECT rc = GetPos();
			cx = rc.right - rc.left;
			if( cx <= 0 && GetFixedWidth() > 0 ) cx = GetFixedWidth();
			if( cx <= 0 ) cx = 480;

			int bodyH = CalcBodyViewportHeight(cx);
			m_pBody->SetFixedHeight(bodyH);
			cy += bodyH;
		}
		else if( m_pBody != NULL ) {
			m_pBody->SetFixedHeight(0);
		}
		SetFixedHeight(cy);
	}

	void CToolCardUI::RequestAncestorLayout()
	{
		for( CControlUI* p = this; p != NULL; p = p->GetParent() )
			p->NeedUpdate();
		if( m_pManager != NULL ) m_pManager->NeedUpdate();
	}

	void CToolCardUI::OnHeaderClick()
	{
		EnsureBuilt();
		ToggleExpanded();
	}

	void CToolCardUI::OnTitleClick()
	{
		OpenFile();
	}

	void CToolCardUI::OpenFile()
	{
		EnsureBuilt();
		if( m_kind != TOOLCARD_FILE ) return;
		if( m_pManager != NULL )
			m_pManager->SendNotify(this, DUI_MSGTYPE_TOOLCARDOPEN, 0, (LPARAM)this);
	}

	void CToolCardUI::OnHeaderHoverChanged(bool bHot)
	{
		if( m_bHeaderHover == bHot ) return;
		m_bHeaderHover = bHot;
		SyncHeaderChrome();
	}

	bool CToolCardUI::Add(CControlUI* pControl)
	{
		EnsureBuilt();
		if( pControl == NULL ) return false;
		if( pControl == m_pHeader || pControl == m_pBody )
			return CVerticalLayoutUI::Add(pControl);
		if( IsHeaderSlotControl(pControl) )
			return AdoptHeaderSlot(static_cast<CToolCardHeaderUI*>(pControl->GetInterface(DUI_CTR_TOOLCARDHEADER)));
		if( IsBodySlotControl(pControl) )
			return AdoptBodySlot(static_cast<CToolCardBodyUI*>(pControl->GetInterface(DUI_CTR_TOOLCARDBODY)));
		if( m_pBody != NULL )
			return m_pBody->Add(pControl);
		return CVerticalLayoutUI::Add(pControl);
	}

	bool CToolCardUI::AddAt(CControlUI* pControl, int iIndex)
	{
		EnsureBuilt();
		if( pControl == NULL ) return false;
		if( pControl == m_pHeader || pControl == m_pBody )
			return CVerticalLayoutUI::AddAt(pControl, iIndex);
		if( IsHeaderSlotControl(pControl) )
			return AdoptHeaderSlot(static_cast<CToolCardHeaderUI*>(pControl->GetInterface(DUI_CTR_TOOLCARDHEADER)));
		if( IsBodySlotControl(pControl) )
			return AdoptBodySlot(static_cast<CToolCardBodyUI*>(pControl->GetInterface(DUI_CTR_TOOLCARDBODY)));
		if( m_pBody != NULL )
			return m_pBody->AddAt(pControl, iIndex);
		return CVerticalLayoutUI::AddAt(pControl, iIndex);
	}

	SIZE CToolCardUI::EstimateSize(SIZE szAvailable)
	{
		EnsureBuilt();
		SIZE sz = CControlUI::EstimateSize(szAvailable);
		int headerH = ScaleValue(m_nHeaderHeight);
		if( !m_bExpanded ) {
			sz.cy = headerH;
			return sz;
		}
		int cx = szAvailable.cx > 0 ? szAvailable.cx : (GetFixedWidth() > 0 ? GetFixedWidth() : 480);
		sz.cy = headerH + CalcBodyViewportHeight(cx);
		return sz;
	}

	void CToolCardUI::SetAttribute(LPCTSTR pstrName, LPCTSTR pstrValue)
	{
		if( _tcsicmp(pstrName, _T("kind")) == 0 ) {
			SetCardKindString(pstrValue);
		}
		else if( _tcsicmp(pstrName, _T("title")) == 0 || _tcsicmp(pstrName, _T("text")) == 0 ) {
			SetTitle(pstrValue);
		}
		else if( _tcsicmp(pstrName, _T("path")) == 0 || _tcsicmp(pstrName, _T("file")) == 0 ) {
			SetPath(pstrValue);
		}
		else if( _tcsicmp(pstrName, _T("command")) == 0 || _tcsicmp(pstrName, _T("cmd")) == 0 ) {
			SetCommand(pstrValue);
		}
		else if( _tcsicmp(pstrName, _T("expanded")) == 0 ) {
			SetExpanded(_tcsicmp(pstrValue, _T("true")) == 0, false);
		}
		else if( _tcsicmp(pstrName, _T("header-height")) == 0 ) {
			SetHeaderHeight(_ttoi(pstrValue));
		}
		else if( _tcsicmp(pstrName, _T("show-chevron")) == 0 ) {
			SetShowChevron(ParseBoolValue(pstrValue));
		}
		else if( _tcsicmp(pstrName, _T("show-badge")) == 0
			|| _tcsicmp(pstrName, _T("show-kind-badge")) == 0 ) {
			SetShowKindBadge(ParseBoolValue(pstrValue));
		}
		else if( _tcsicmp(pstrName, _T("show-title")) == 0 ) {
			SetShowTitle(ParseBoolValue(pstrValue));
		}
		else if( _tcsicmp(pstrName, _T("header-bk")) == 0
			|| _tcsicmp(pstrName, _T("header-bkcolor")) == 0
			|| _tcsicmp(pstrName, _T("header-background-color")) == 0 ) {
			SetHeaderBkColor(ParseColorValue(pstrValue));
		}
		else if( _tcsicmp(pstrName, _T("header-hover-bk")) == 0
			|| _tcsicmp(pstrName, _T("header-hover-bkcolor")) == 0
			|| _tcsicmp(pstrName, _T("header-hover-background-color")) == 0 ) {
			SetHeaderHoverBkColor(ParseColorValue(pstrValue));
		}
		else if( _tcsicmp(pstrName, _T("chevron-color")) == 0 ) {
			SetChevronColor(ParseColorValue(pstrValue));
		}
		else if( _tcsicmp(pstrName, _T("title-color")) == 0 ) {
			SetTitleColor(ParseColorValue(pstrValue));
		}
		else if( _tcsicmp(pstrName, _T("title-link-color")) == 0
			|| _tcsicmp(pstrName, _T("title-color-file")) == 0 ) {
			SetTitleLinkColor(ParseColorValue(pstrValue));
		}
		else if( _tcsicmp(pstrName, _T("badge-text")) == 0
			|| _tcsicmp(pstrName, _T("kind-badge-text")) == 0 ) {
			SetKindBadgeText(pstrValue);
		}
		else if( _tcsicmp(pstrName, _T("badge-color")) == 0
			|| _tcsicmp(pstrName, _T("badge-text-color")) == 0 ) {
			SetKindBadgeTextColor(ParseColorValue(pstrValue));
		}
		else if( _tcsicmp(pstrName, _T("badge-bk")) == 0
			|| _tcsicmp(pstrName, _T("badge-bkcolor")) == 0
			|| _tcsicmp(pstrName, _T("badge-background-color")) == 0 ) {
			if( pstrValue != NULL && (_tcsicmp(pstrValue, _T("auto")) == 0
				|| _tcsicmp(pstrValue, _T("default")) == 0 || _tcscmp(pstrValue, _T("0")) == 0) )
				SetKindBadgeBkColor(0);
			else
				SetKindBadgeBkColor(ParseColorValue(pstrValue));
		}
		else if( _tcsicmp(pstrName, _T("badge-width")) == 0 ) {
			SetKindBadgeWidth(_ttoi(pstrValue));
		}
		else if( _tcsicmp(pstrName, _T("body-height")) == 0 ) {
			SetBodyHeight(_ttoi(pstrValue));
		}
		else if( _tcsicmp(pstrName, _T("body-min-height")) == 0
			|| _tcsicmp(pstrName, _T("min-body-height")) == 0 ) {
			SetBodyMinHeight(_ttoi(pstrValue));
		}
		else if( _tcsicmp(pstrName, _T("body-max-height")) == 0
			|| _tcsicmp(pstrName, _T("max-body-height")) == 0 ) {
			SetBodyMaxHeight(_ttoi(pstrValue));
		}
		else if( _tcsicmp(pstrName, _T("body")) == 0 || _tcsicmp(pstrName, _T("body-text")) == 0 ) {
			SetBodyText(pstrValue);
		}
		else {
			CVerticalLayoutUI::SetAttribute(pstrName, pstrValue);
		}
	}
}
