#include "StdAfx.h"
#include "IconBrowserWnd.h"
#include "Icons/UIIconEntry.h"
#include "Core/UITheme.h"
#include <algorithm>
#include <map>

namespace {

	DWORD IconBrowserThemeToken(LPCTSTR pstrName, DWORD dwFallback)
	{
		CThemeManager* pTm = CThemeManager::GetInstance();
		if( pTm == NULL ) return dwFallback;
		CTheme* pTh = pTm->GetCurrentTheme();
		if( pTh == NULL ) pTh = pTm->FindTheme(pTm->GetDefaultThemeId());
		if( pTh == NULL ) return dwFallback;
		return pTh->GetToken(pstrName, dwFallback);
	}

	class CIconCellUI : public CVerticalLayoutUI
	{
	public:
		CIconCellUI()
			: m_bPushed(false)
			, m_bHot(false)
			, m_dwHotBk(IconBrowserThemeToken(_T("color-bg-hover"), 0xDCDCE1FF))
		{
			SetCursor(DUI_HAND);
		}

		LPCTSTR GetClass() const { return _T("IconCellUI"); }

		UINT GetControlFlags() const { return UIFLAG_SETCURSOR; }

		bool Activate()
		{
			if( !CControlUI::Activate() ) return false;
			if( m_pManager != NULL )
				m_pManager->SendNotify(this, DUI_MSGTYPE_CLICK);
			return true;
		}

		void DoEvent(TEventUI& event)
		{
			if( !IsMouseEnabled() && event.Type > UIEVENT__MOUSEBEGIN && event.Type < UIEVENT__MOUSEEND ) {
				CVerticalLayoutUI::DoEvent(event);
				return;
			}
			if( event.Type == UIEVENT_SETCURSOR ) {
				::SetCursor(::LoadCursor(NULL, IDC_HAND));
				return;
			}
			if( event.Type == UIEVENT_BUTTONDOWN || event.Type == UIEVENT_DBLCLICK ) {
				if( ::PtInRect(&m_rcItem, event.ptMouse) && IsEnabled() ) {
					m_bPushed = true;
					SetBackgroundColor(m_dwHotBk);
				}
				return;
			}
			if( event.Type == UIEVENT_BUTTONUP ) {
				bool bClick = m_bPushed && ::PtInRect(&m_rcItem, event.ptMouse) && IsEnabled();
				m_bPushed = false;
				SetBackgroundColor(m_bHot ? m_dwHotBk : 0);
				if( bClick ) Activate();
				return;
			}
			if( event.Type == UIEVENT_MOUSEENTER ) {
				m_bHot = true;
				if( !m_bPushed ) SetBackgroundColor(m_dwHotBk);
				return;
			}
			if( event.Type == UIEVENT_MOUSELEAVE ) {
				m_bHot = false;
				m_bPushed = false;
				SetBackgroundColor(0);
				return;
			}
			CVerticalLayoutUI::DoEvent(event);
		}

	private:
		bool m_bPushed;
		bool m_bHot;
		DWORD m_dwHotBk;
	};

} // namespace

DUI_BEGIN_MESSAGE_MAP(CIconBrowserWnd, WindowImplBase)
	DUI_ON_MSGTYPE(DUI_MSGTYPE_CLICK, CIconBrowserWnd::OnClick)
DUI_END_MESSAGE_MAP()

CIconBrowserWnd::CIconBrowserWnd(const IconEntry* pEntries, int nCount, LPCTSTR pstrAttr, LPCTSTR pstrTitle)
	: m_pEntries(pEntries)
	, m_nCount(nCount)
	, m_sAttr(pstrAttr ? pstrAttr : _T(""))
	, m_sTitle(pstrTitle ? pstrTitle : _T("Icons"))
	, m_pIconList(NULL)
	, m_pTitleLabel(NULL)
	, m_pSearchEdit(NULL)
	, m_nTotalHeight(0)
	, m_nMatched(0)
	, m_nVisFirst(-1)
	, m_nVisLast(-1)
	, m_bRefreshing(false)
{
}

CIconBrowserWnd::~CIconBrowserWnd()
{
}

void CIconBrowserWnd::Open(HWND hParent, const IconEntry* pEntries, int nCount, LPCTSTR pstrAttr, LPCTSTR pstrTitle)
{
	if( pEntries == NULL || nCount <= 0 ) return;
	CIconBrowserWnd* pWnd = new CIconBrowserWnd(pEntries, nCount, pstrAttr, pstrTitle);
	pWnd->Create(hParent, pstrTitle, UI_WNDSTYLE_FRAME, WS_EX_WINDOWEDGE, 0, 0, 950, 700);
	pWnd->CenterWindow();
	pWnd->ShowWindow(true);
}

void CIconBrowserWnd::OnFinalMessage(HWND hWnd)
{
	WindowImplBase::OnFinalMessage(hWnd);
	delete this;
}

CDuiString CIconBrowserWnd::GetSkinFile()
{
	return _T("iconbrowser.html");
}

LPCTSTR CIconBrowserWnd::GetWindowClassName() const
{
	return _T("IconBrowserWnd");
}

CDuiString CIconBrowserWnd::GetPrefix(LPCTSTR pstrName)
{
	if( pstrName == NULL || *pstrName == _T('\0') ) return _T("other");
	LPCTSTR pDash = _tcschr(pstrName, _T('-'));
	if( pDash == NULL ) return CDuiString(pstrName);
	return CDuiString(pstrName, (int)(pDash - pstrName));
}

bool CIconBrowserWnd::MatchFilter(LPCTSTR pstrName, LPCTSTR pstrFilter)
{
	if( pstrFilter == NULL || *pstrFilter == _T('\0') ) return true;
	if( pstrName == NULL ) return false;
	CDuiString sName(pstrName);
	CDuiString sFilter(pstrFilter);
	sName.MakeLower();
	sFilter.MakeLower();
	return sName.Find(sFilter) >= 0;
}

void CIconBrowserWnd::UpdateTitle(LPCTSTR pstrExtra)
{
	CDuiString s;
	if( pstrExtra != NULL && *pstrExtra != _T('\0') )
		s.Format(_T("%s (%d) — %s"), m_sTitle.GetData(), m_nCount, pstrExtra);
	else
		s.Format(_T("%s (%d)"), m_sTitle.GetData(), m_nCount);

	if( m_pTitleLabel != NULL )
		m_pTitleLabel->SetText(s);
	CTitleBarUI* pTitleBar = static_cast<CTitleBarUI*>(m_pm.FindControl(_T("titlebar")));
	if( pTitleBar != NULL )
		pTitleBar->SetTitle(s);
	// 只脏标题文字时，与列表脏区并集的外包盒会盖住关闭按钮，但更新区不含该按钮；
	// D2D Flush 丢窗口裁剪后列表可能画进关闭按钮，出现横条/上下半截。整条 titlebar 一起脏掉。
	CControlUI* pBar = m_pm.FindControl(_T("titlebar"));
	if( pBar != NULL )
		pBar->Invalidate();
	if( m_hWnd != NULL )
		::SetWindowText(m_hWnd, s.GetData());
}

bool CIconBrowserWnd::CopyTextToClipboard(LPCTSTR pstrText)
{
	if( pstrText == NULL || *pstrText == _T('\0') ) return false;
	if( !::OpenClipboard(m_hWnd) ) return false;
	::EmptyClipboard();
	size_t nBytes = (_tcslen(pstrText) + 1) * sizeof(TCHAR);
	HGLOBAL hMem = ::GlobalAlloc(GMEM_MOVEABLE, nBytes);
	bool bOk = false;
	if( hMem ) {
		void* p = ::GlobalLock(hMem);
		if( p ) {
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

int CIconBrowserWnd::GetViewportHeight() const
{
	if( m_pIconList == NULL ) return 0;
	// 用控件自身矩形，勿用 GetClientPos（含滚动扩展，会把视口算成整份内容高度）
	RECT rc = m_pIconList->GetPos();
	int h = rc.bottom - rc.top;
	return h > 0 ? h : 0;
}

CControlUI* CIconBrowserWnd::CreateSpacer(int nHeight)
{
	if( nHeight <= 0 ) return NULL;
	CControlUI* p = new CControlUI;
	// 默认 MaxHeight=9999，大 spacer 会被布局钳死，滚动范围错误
	p->SetMaxHeight(nHeight);
	p->SetMinHeight(nHeight);
	p->SetFixedHeight(nHeight);
	p->SetMouseEnabled(false);
	return p;
}

CControlUI* CIconBrowserWnd::CreateHeaderRow(LPCTSTR pstrText)
{
	CHorizontalLayoutUI* pHeader = new CHorizontalLayoutUI;
	pHeader->SetFixedHeight(ROW_HEADER_H);
	pHeader->SetBackgroundColor(IconBrowserThemeToken(_T("color-bg-elevated"), 0xE6E6EBFF));

	CControlUI* pPad = new CControlUI;
	pPad->SetFixedWidth(10);
	pHeader->Add(pPad);

	CLabelUI* pHdr = new CLabelUI;
	pHdr->SetText(pstrText);
	pHdr->SetColor(IconBrowserThemeToken(_T("color-text-secondary"), 0x3C3C46FF));
	pHdr->SetAttribute(_T("text-align"), _T("left"));
	pHeader->Add(pHdr);
	return pHeader;
}

CControlUI* CIconBrowserWnd::CreateIconCell(const IconEntry* pEntry)
{
	if( pEntry == NULL || pEntry->name == NULL ) return NULL;

	CIconCellUI* pCell = new CIconCellUI;
	pCell->SetFixedWidth(120);
	pCell->SetFixedHeight(ROW_ICON_H);
	pCell->SetAlignItems(DT_CENTER);
	pCell->SetJustifyContent(DT_VCENTER);
	pCell->SetAttribute(_T("action"), _T("copy"));

	CDuiString sCopy;
	sCopy.Format(_T("%s=\"%s\""), m_sAttr.GetData(), pEntry->name);
	pCell->AddCustomAttribute(_T("copy-text"), sCopy.GetData());

	DWORD dwIcon = IconBrowserThemeToken(_T("color-text"), 0x333333FF);
	CDuiString sIconColor;
	sIconColor.Format(_T("#%08X"), dwIcon);

	CSvgBoxUI* pSvg = new CSvgBoxUI;
	pSvg->SetFixedWidth(28);
	pSvg->SetFixedHeight(28);
	pSvg->SetMouseEnabled(false);
	pSvg->SetAttribute(m_sAttr.GetData(), pEntry->name);
	pSvg->SetAttribute(_T("color"), sIconColor.GetData());
	pCell->Add(pSvg);

	CLabelUI* pLabel = new CLabelUI;
	pLabel->SetFixedHeight(22);
	pLabel->SetText(pEntry->name);
	pLabel->SetMouseEnabled(false);
	pLabel->SetAttribute(_T("text-align"), _T("center"));
	pLabel->SetAttribute(_T("text-overflow"), _T("ellipsis"));
	pLabel->SetColor(IconBrowserThemeToken(_T("color-text-secondary"), 0x50505AFF));
	pCell->Add(pLabel);
	return pCell;
}

CControlUI* CIconBrowserWnd::CreateIconRow(const std::vector<const IconEntry*>& cells)
{
	CHorizontalLayoutUI* pRow = new CHorizontalLayoutUI;
	pRow->SetFixedHeight(ROW_ICON_H);

	int nInRow = 0;
	for( size_t i = 0; i < cells.size(); ++i ) {
		CControlUI* pCell = CreateIconCell(cells[i]);
		if( pCell != NULL ) pRow->Add(pCell);
		++nInRow;
	}
	while( nInRow < ICONS_PER_ROW ) {
		CControlUI* pPad = new CControlUI;
		pPad->SetFixedWidth(120);
		pRow->Add(pPad);
		++nInRow;
	}
	return pRow;
}

void CIconBrowserWnd::RebuildData()
{
	m_rows.clear();
	m_rowTops.clear();
	m_nTotalHeight = 0;
	m_nMatched = 0;
	m_nVisFirst = -1;
	m_nVisLast = -1;

	if( m_pEntries == NULL || m_nCount <= 0 ) return;

	typedef std::map<std::wstring, std::vector<const IconEntry*> > CatMap;
	CatMap cats;
	for( int i = 0; i < m_nCount; ++i ) {
		const IconEntry& e = m_pEntries[i];
		if( e.name == NULL ) continue;
		if( !MatchFilter(e.name, m_sFilter.GetData()) ) continue;
		++m_nMatched;
		CDuiString sPrefix = GetPrefix(e.name);
		cats[std::wstring(sPrefix.GetData())].push_back(&e);
	}

	struct CatTmp {
		CDuiString prefix;
		std::vector<const IconEntry*> icons;
	};
	std::vector<CatTmp> ordered;
	ordered.reserve(cats.size());
	for( CatMap::iterator it = cats.begin(); it != cats.end(); ++it ) {
		CatTmp c;
		c.prefix = it->first.c_str();
		c.icons.swap(it->second);
		ordered.push_back(c);
	}
	std::sort(ordered.begin(), ordered.end(), [](const CatTmp& a, const CatTmp& b) {
		if( a.icons.size() != b.icons.size() ) return a.icons.size() > b.icons.size();
		return _tcscmp(a.prefix.GetData(), b.prefix.GetData()) < 0;
	});

	m_rows.reserve(ordered.size() * 4);
	for( size_t c = 0; c < ordered.size(); ++c ) {
		VirtRow hdr;
		hdr.bHeader = true;
		hdr.sHeader.Format(_T("%s (%u 个)"), ordered[c].prefix.GetData(), (unsigned)ordered[c].icons.size());
		hdr.nHeight = ROW_HEADER_H;
		m_rows.push_back(hdr);

		const std::vector<const IconEntry*>& icons = ordered[c].icons;
		for( size_t i = 0; i < icons.size(); ) {
			VirtRow row;
			row.bHeader = false;
			row.nHeight = ROW_ICON_H;
			row.cells.reserve(ICONS_PER_ROW);
			for( int n = 0; n < ICONS_PER_ROW && i < icons.size(); ++n, ++i )
				row.cells.push_back(icons[i]);
			m_rows.push_back(row);
		}
	}

	m_rowTops.resize(m_rows.size());
	int y = 0;
	for( size_t i = 0; i < m_rows.size(); ++i ) {
		m_rowTops[i] = y;
		y += m_rows[i].nHeight;
	}
	m_nTotalHeight = y;
}

void CIconBrowserWnd::RefreshViewport(bool bForce)
{
	if( m_pIconList == NULL || m_bRefreshing ) return;
	m_bRefreshing = true;

	if( m_rows.empty() ) {
		m_pIconList->SetDelayedDestroy(false);
		m_pIconList->RemoveAll();
		CLabelUI* pEmpty = new CLabelUI;
		pEmpty->SetFixedHeight(40);
		pEmpty->SetText(_T("没有匹配的图标"));
		pEmpty->SetColor(0x999999FF);
		pEmpty->SetAttribute(_T("text-align"), _T("center"));
		m_pIconList->Add(pEmpty);
		m_nVisFirst = m_nVisLast = -1;
		CDuiString sExtra;
		if( !m_sFilter.IsEmpty() )
			sExtra.Format(_T("匹配 %d"), m_nMatched);
		UpdateTitle(sExtra.IsEmpty() ? NULL : sExtra.GetData());
		m_bRefreshing = false;
		return;
	}

	int scrollY = m_pIconList->GetScrollPos().cy;
	int viewH = GetViewportHeight();
	if( viewH <= 0 ) viewH = 600;

	int y0 = scrollY - VIEW_BUFFER_PX;
	if( y0 < 0 ) y0 = 0;
	int y1 = scrollY + viewH + VIEW_BUFFER_PX;
	if( y1 > m_nTotalHeight ) y1 = m_nTotalHeight;

	int first = 0;
	int last = (int)m_rows.size() - 1;
	for( size_t i = 0; i < m_rows.size(); ++i ) {
		int top = m_rowTops[i];
		int bottom = top + m_rows[i].nHeight;
		if( bottom > y0 ) { first = (int)i; break; }
	}
	for( int i = (int)m_rows.size() - 1; i >= 0; --i ) {
		if( m_rowTops[i] < y1 ) { last = i; break; }
	}
	if( last < first ) last = first;

	if( !bForce && first == m_nVisFirst && last == m_nVisLast ) {
		m_bRefreshing = false;
		return;
	}

	m_nVisFirst = first;
	m_nVisLast = last;

	// 勿 SuspendLayout：UpdateLock>0 时 Add 会把子控件 InternVisible=false，且 Resume 不恢复 → 空白
	m_pIconList->SetDelayedDestroy(false);
	m_pIconList->RemoveAll();

	int topSpacer = m_rowTops[first];
	if( topSpacer > 0 ) {
		CControlUI* p = CreateSpacer(topSpacer);
		if( p ) m_pIconList->Add(p);
	}

	for( int i = first; i <= last; ++i ) {
		const VirtRow& row = m_rows[i];
		CControlUI* pRow = NULL;
		if( row.bHeader )
			pRow = CreateHeaderRow(row.sHeader.GetData());
		else
			pRow = CreateIconRow(row.cells);
		if( pRow ) m_pIconList->Add(pRow);
	}

	int bottomStart = m_rowTops[last] + m_rows[last].nHeight;
	int bottomSpacer = m_nTotalHeight - bottomStart;
	if( bottomSpacer > 0 ) {
		CControlUI* p = CreateSpacer(bottomSpacer);
		if( p ) m_pIconList->Add(p);
	}

	SIZE sz = m_pIconList->GetScrollPos();
	sz.cy = scrollY;
	m_pIconList->SetScrollPos(sz, false);

	// 滚动刷新不要改标题：避免标题局部 Invalidate 与列表脏区叠加导致关闭按钮花屏
	m_bRefreshing = false;
}

void CIconBrowserWnd::InitWindow()
{
	m_pTitleLabel = static_cast<CLabelUI*>(m_pm.FindControl(_T("titleText")));
	m_pSearchEdit = static_cast<CEditUI*>(m_pm.FindControl(_T("searchEdit")));
	m_pIconList = static_cast<CVerticalLayoutUI*>(m_pm.FindControl(_T("iconList")));
	if( m_pIconList != NULL )
		m_pIconList->SetDelayedDestroy(false);

	UpdateTitle();
	RebuildData();
	RefreshViewport(true);
}

LRESULT CIconBrowserWnd::HandleCustomMessage(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	if( uMsg == WM_SIZE ) {
		RefreshViewport(true);
	}
	bHandled = FALSE;
	return 0;
}

void CIconBrowserWnd::Notify(TNotifyUI& msg)
{
	if( msg.sType == DUI_MSGTYPE_SCROLL ) {
		if( msg.pSender == m_pIconList )
			RefreshViewport(false);
		return;
	}
	if( msg.sType == DUI_MSGTYPE_TEXTCHANGED ) {
		if( msg.pSender == m_pSearchEdit && m_pSearchEdit != NULL ) {
			m_sFilter = m_pSearchEdit->GetText();
			RebuildData();
			if( m_pIconList != NULL ) {
				SIZE sz = { 0, 0 };
				m_pIconList->SetScrollPos(sz, false);
			}
			RefreshViewport(true);
			CDuiString sExtra;
			if( !m_sFilter.IsEmpty() )
				sExtra.Format(_T("匹配 %d"), m_nMatched);
			UpdateTitle(sExtra.IsEmpty() ? NULL : sExtra.GetData());
			return;
		}
	}
	WindowImplBase::Notify(msg);
}

void CIconBrowserWnd::OnClick(TNotifyUI& msg)
{
	CControlUI* p = msg.pSender;
	while( p != NULL ) {
		LPCTSTR pCopy = p->GetCustomAttribute(_T("copy-text"));
		if( pCopy != NULL && *pCopy != _T('\0') ) {
			if( CopyTextToClipboard(pCopy) ) {
				CDuiString sExtra;
				sExtra.Format(_T("已复制: %s"), pCopy);
				UpdateTitle(sExtra.GetData());
			}
			else {
				UpdateTitle(_T("复制失败"));
			}
			return;
		}
		if( p->GetAction() == UIACTION_COPY )
			break;
		p = p->GetParent();
	}
	WindowImplBase::OnClick(msg);
}
