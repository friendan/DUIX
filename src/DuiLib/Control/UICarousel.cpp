#include "StdAfx.h"
#include "UICarousel.h"

namespace DuiLib
{
	IMPLEMENT_DUICONTROL(CCarouselUI)
	IMPLEMENT_DUICONTROL(CCarouselItemUI)

	// ====== CCarouselUI ======

	CCarouselUI::CCarouselUI()
		: m_nCurrentIndex(-1)
		, m_nInterval(5000)
		, m_bWrap(true)
		, m_bPauseOnHover(true)
		, m_bShowControls(true)
		, m_bRidePending(false)
		, m_bTimerActive(false)
		, m_nControlsGap(10)
		, m_nPageGap(15)
		, m_nPageWidth(50)
		, m_pControlBar(NULL)
		, m_pPageLabel(NULL)
		, m_pGapBeforePage(NULL)
		, m_pGapAfterPage(NULL)
		, m_pGapAfterFirst(NULL)
		, m_pGapBeforeLast(NULL)
	{
		SetBackgroundColor(0xC8C8C8FF);
		EnsureControlBar();
	}

	CCarouselUI::~CCarouselUI()
	{
		StopTimer();
	}

	LPCTSTR CCarouselUI::GetClass() const
	{
		return _T("CarouselUI");
	}

	LPVOID CCarouselUI::GetInterface(LPCTSTR pstrName)
	{
		if( _tcsicmp(pstrName, DUI_CTR_CAROUSEL) == 0 ) return static_cast<CCarouselUI*>(this);
		return CVerticalLayoutUI::GetInterface(pstrName);
	}

	void CCarouselUI::EnsureControlBar()
	{
		if( m_pControlBar != NULL ) return;

		m_pControlBar = new CHorizontalLayoutUI;
		m_pControlBar->SetFixedHeight(30);
		m_pControlBar->SetBackgroundColor(0x32323CFF);
		m_pControlBar->SetAttribute(_T("align-items"), _T("vcenter"));
		m_pControlBar->SetAttribute(_T("padding"), _T("4,2,4,2"));

		CSpacerUI* pLeft = new CSpacerUI;
		m_pControlBar->Add(pLeft);

		AddControlButton(_T("|<<"), _T("carousel_btn_first"));
		m_pGapAfterFirst = AddGap(m_nControlsGap);
		AddControlButton(_T("<"), _T("carousel_btn_prev"));
		m_pGapBeforePage = AddGap(m_nPageGap);

		m_pPageLabel = new CLabelUI;
		m_pPageLabel->SetFixedWidth(m_nPageWidth);
		m_pPageLabel->SetFixedHeight(26);
		m_pPageLabel->SetAttribute(_T("text-align"), _T("center"));
		m_pPageLabel->SetAttribute(_T("vertical-align"), _T("vcenter"));
		m_pPageLabel->SetAttribute(_T("color"), _T("#C8C8C8FF"));
		m_pPageLabel->SetAttribute(_T("font-size"), _T("12"));
		m_pPageLabel->SetText(_T(""));
		m_pControlBar->Add(m_pPageLabel);

		m_pGapAfterPage = AddGap(m_nPageGap);
		AddControlButton(_T(">"), _T("carousel_btn_next"));
		m_pGapBeforeLast = AddGap(m_nControlsGap);
		AddControlButton(_T(">>|"), _T("carousel_btn_last"));

		CSpacerUI* pRight = new CSpacerUI;
		m_pControlBar->Add(pRight);

		CVerticalLayoutUI::Add(m_pControlBar);
		m_pControlBar->SetVisible(m_bShowControls);
	}

	CButtonUI* CCarouselUI::AddControlButton(LPCTSTR pstrText, LPCTSTR pstrName)
	{
		CButtonUI* pBtn = new CButtonUI;
		pBtn->SetName(pstrName);
		pBtn->SetText(pstrText);
		pBtn->SetFixedWidth(50);
		pBtn->SetFixedHeight(26);
		pBtn->SetKind(CONTROLKIND_PRIMARY);
		pBtn->OnNotify += MakeDelegate(this, &CCarouselUI::OnControlClick);
		m_pControlBar->Add(pBtn);
		return pBtn;
	}

	CControlUI* CCarouselUI::AddGap(int nWidth)
	{
		CControlUI* pGap = new CControlUI;
		pGap->SetFixedWidth(nWidth);
		pGap->SetFixedHeight(1);
		pGap->SetMouseEnabled(false);
		m_pControlBar->Add(pGap);
		return pGap;
	}

	void CCarouselUI::ApplyControlsKind(LPCTSTR pstrKind)
	{
		if( m_pControlBar == NULL || pstrKind == NULL ) return;
		m_sControlsKind = pstrKind;
		for( int i = 0; i < m_pControlBar->GetCount(); ++i ) {
			CControlUI* p = m_pControlBar->GetItemAt(i);
			if( p == NULL || p == m_pPageLabel ) continue;
			if( p->GetInterface(DUI_CTR_BUTTON) == NULL ) continue;
			p->SetAttribute(_T("kind"), pstrKind);
		}
	}

	bool CCarouselUI::OnControlClick(void* param)
	{
		TNotifyUI* pNotify = static_cast<TNotifyUI*>(param);
		if( pNotify == NULL || pNotify->sType != DUI_MSGTYPE_CLICK || pNotify->pSender == NULL )
			return true;
		CDuiString sName = pNotify->pSender->GetName();
		if( sName == _T("carousel_btn_first") ) GoTo(0);
		else if( sName == _T("carousel_btn_prev") ) Prev();
		else if( sName == _T("carousel_btn_next") ) Next();
		else if( sName == _T("carousel_btn_last") ) GoTo(GetItemCount() - 1);
		return true;
	}

	int CCarouselUI::FindControlBarIndex() const
	{
		if( m_pControlBar == NULL ) return GetCount();
		for( int i = 0; i < GetCount(); ++i ) {
			if( GetItemAt(i) == m_pControlBar ) return i;
		}
		return GetCount();
	}

	int CCarouselUI::GetItemCount() const
	{
		int n = 0;
		for( int i = 0; i < GetCount(); ++i ) {
			CControlUI* p = GetItemAt(i);
			if( p != NULL && p->GetInterface(DUI_CTR_CAROUSELITEM) != NULL ) ++n;
		}
		return n;
	}

	bool CCarouselUI::Add(CControlUI* pControl)
	{
		EnsureControlBar();
		int iBar = FindControlBarIndex();
		return AddAt(pControl, iBar);
	}

	bool CCarouselUI::AddAt(CControlUI* pControl, int iIndex)
	{
		if( pControl == NULL ) return false;
		EnsureControlBar();

		if( pControl == m_pControlBar )
			return CVerticalLayoutUI::AddAt(pControl, iIndex);

		int iBar = FindControlBarIndex();
		if( iIndex < 0 || iIndex > iBar ) iIndex = iBar;

		bool bOk = CVerticalLayoutUI::AddAt(pControl, iIndex);
		if( !bOk ) return false;

		CCarouselItemUI* pItem = static_cast<CCarouselItemUI*>(pControl->GetInterface(DUI_CTR_CAROUSELITEM));
		if( pItem != NULL ) {
			// 占满控制栏以外的剩余高度
			pItem->SetFixedHeight(0);
			if( m_nCurrentIndex < 0 ) {
				m_nCurrentIndex = 0;
				ShowItem(0);
			}
			else {
				pItem->SetVisible(false);
			}
			UpdatePageLabel();
			UpdateIndicators();
		}
		return true;
	}

	void CCarouselUI::ShowItem(int index)
	{
		int i = 0;
		for( int n = 0; n < GetCount(); ++n ) {
			CControlUI* p = GetItemAt(n);
			CCarouselItemUI* pItem = p ? static_cast<CCarouselItemUI*>(p->GetInterface(DUI_CTR_CAROUSELITEM)) : NULL;
			if( pItem == NULL ) continue;
			pItem->SetVisible(i == index);
			++i;
		}
	}

	void CCarouselUI::UpdatePageLabel()
	{
		if( m_pPageLabel == NULL ) return;
		int total = GetItemCount();
		if( total <= 0 ) {
			m_pPageLabel->SetText(_T(""));
			return;
		}
		int cur = m_nCurrentIndex < 0 ? 0 : m_nCurrentIndex;
		CDuiString s;
		s.Format(_T("%d/%d"), cur + 1, total);
		m_pPageLabel->SetText(s);
	}

	void CCarouselUI::UpdateIndicators()
	{
		for( int n = 0; n < GetCount(); ++n ) {
			CControlUI* p = GetItemAt(n);
			if( p == NULL || p == m_pControlBar ) continue;
			if( p->GetInterface(DUI_CTR_CAROUSELITEM) != NULL ) continue;
			CDuiString sName = p->GetName();
			if( sName.Find(_T("carouselIndicators")) < 0 ) continue;

			CContainerUI* pBox = static_cast<CContainerUI*>(p->GetInterface(DUI_CTR_CONTAINER));
			if( pBox == NULL ) continue;
			for( int i = 0; i < pBox->GetCount(); ++i ) {
				CControlUI* pDot = pBox->GetItemAt(i);
				if( pDot == NULL ) continue;
				pDot->SetBackgroundColor(i == m_nCurrentIndex ? 0xFFFFFFFF : 0x808080FF);
			}
			break;
		}
	}

	void CCarouselUI::GoTo(int index)
	{
		int count = GetItemCount();
		if( count == 0 || index < 0 || index >= count ) return;
		if( index == m_nCurrentIndex ) return;

		int from = m_nCurrentIndex;
		ShowItem(index);
		m_nCurrentIndex = index;
		UpdatePageLabel();
		UpdateIndicators();
		Invalidate();

		if( m_pManager != NULL )
			m_pManager->SendNotify(this, DUI_MSGTYPE_SLIDECHANGED, (WPARAM)index, (LPARAM)from);

		if( m_bTimerActive ) {
			StopTimer();
			StartTimer();
		}
	}

	void CCarouselUI::Next()
	{
		int count = GetItemCount();
		if( count == 0 ) return;
		int nextIdx = m_nCurrentIndex + 1;
		if( nextIdx >= count ) {
			if( !m_bWrap ) return;
			nextIdx = 0;
		}
		GoTo(nextIdx);
	}

	void CCarouselUI::Prev()
	{
		int count = GetItemCount();
		if( count == 0 ) return;
		int prevIdx = m_nCurrentIndex - 1;
		if( prevIdx < 0 ) {
			if( !m_bWrap ) return;
			prevIdx = count - 1;
		}
		GoTo(prevIdx);
	}

	void CCarouselUI::Play()
	{
		if( m_nInterval <= 0 ) return;
		StartTimer();
	}

	void CCarouselUI::Pause()
	{
		StopTimer();
	}

	void CCarouselUI::StartTimer()
	{
		if( m_nInterval <= 0 || m_pManager == NULL ) return;
		StopTimer();
		if( SetTimer(TIMER_ID, (UINT)m_nInterval) )
			m_bTimerActive = true;
	}

	void CCarouselUI::StopTimer()
	{
		if( m_pManager != NULL )
			KillTimer(TIMER_ID);
		m_bTimerActive = false;
	}

	bool CCarouselUI::IsCursorInside() const
	{
		if( m_pManager == NULL ) return false;
		POINT pt;
		if( !::GetCursorPos(&pt) ) return false;
		::ScreenToClient(m_pManager->GetPaintWindow(), &pt);
		return ::PtInRect(&m_rcItem, pt) != FALSE;
	}

	void CCarouselUI::DoInit()
	{
		CVerticalLayoutUI::DoInit();
		if( m_bRidePending )
			Play();
		UpdatePageLabel();
	}

	void CCarouselUI::DoEvent(TEventUI& event)
	{
		if( event.Type == UIEVENT_TIMER && event.wParam == TIMER_ID ) {
			if( m_bPauseOnHover && IsCursorInside() )
				return;
			Next();
			return;
		}
		CVerticalLayoutUI::DoEvent(event);
	}

	void CCarouselUI::SetAttribute(LPCTSTR pstrName, LPCTSTR pstrValue)
	{
		if( _tcsicmp(pstrName, _T("interval")) == 0 ) {
			m_nInterval = _ttoi(pstrValue);
			if( m_bTimerActive ) {
				StopTimer();
				if( m_nInterval > 0 ) StartTimer();
			}
		}
		else if( _tcsicmp(pstrName, _T("ride")) == 0 ) {
			bool bRide = (_tcsicmp(pstrValue, _T("true")) == 0 || _tcscmp(pstrValue, _T("1")) == 0);
			m_bRidePending = bRide;
			if( bRide && m_pManager != NULL ) Play();
		}
		else if( _tcsicmp(pstrName, _T("wrap")) == 0 ) {
			m_bWrap = (_tcsicmp(pstrValue, _T("true")) == 0 || _tcscmp(pstrValue, _T("1")) == 0);
		}
		else if( _tcsicmp(pstrName, _T("pause")) == 0 ) {
			m_bPauseOnHover = (_tcsicmp(pstrValue, _T("true")) == 0 || _tcscmp(pstrValue, _T("1")) == 0);
		}
		else if( _tcsicmp(pstrName, _T("controls")) == 0 ) {
			m_bShowControls = (_tcsicmp(pstrValue, _T("true")) == 0 || _tcscmp(pstrValue, _T("1")) == 0);
			EnsureControlBar();
			if( m_pControlBar ) m_pControlBar->SetVisible(m_bShowControls);
		}
		else if( _tcsicmp(pstrName, _T("controls-kind")) == 0 ) {
			EnsureControlBar();
			ApplyControlsKind(pstrValue);
		}
		else if( _tcsicmp(pstrName, _T("controls-gap")) == 0 ) {
			m_nControlsGap = _ttoi(pstrValue);
			if( m_pGapAfterFirst ) m_pGapAfterFirst->SetFixedWidth(m_nControlsGap);
			if( m_pGapBeforeLast ) m_pGapBeforeLast->SetFixedWidth(m_nControlsGap);
		}
		else if( _tcsicmp(pstrName, _T("page-width")) == 0 ) {
			m_nPageWidth = _ttoi(pstrValue);
			if( m_pPageLabel ) m_pPageLabel->SetFixedWidth(m_nPageWidth);
		}
		else if( _tcsicmp(pstrName, _T("page-gap")) == 0 ) {
			m_nPageGap = _ttoi(pstrValue);
			if( m_pGapBeforePage ) m_pGapBeforePage->SetFixedWidth(m_nPageGap);
			if( m_pGapAfterPage ) m_pGapAfterPage->SetFixedWidth(m_nPageGap);
		}
		else {
			CVerticalLayoutUI::SetAttribute(pstrName, pstrValue);
		}
	}

	// ====== CCarouselItemUI ======

	CCarouselItemUI::CCarouselItemUI()
		: m_pCaptionBar(NULL)
		, m_pCaptionTitle(NULL)
		, m_pCaptionText(NULL)
	{
		SetVisible(false);
		SetFixedHeight(0);
	}

	CCarouselItemUI::~CCarouselItemUI()
	{
	}

	LPCTSTR CCarouselItemUI::GetClass() const
	{
		return _T("CarouselItemUI");
	}

	LPVOID CCarouselItemUI::GetInterface(LPCTSTR pstrName)
	{
		if( _tcsicmp(pstrName, DUI_CTR_CAROUSELITEM) == 0 ) return static_cast<CCarouselItemUI*>(this);
		return CVerticalLayoutUI::GetInterface(pstrName);
	}

	int CCarouselItemUI::FindCaptionBarIndex() const
	{
		if( m_pCaptionBar == NULL ) return GetCount();
		for( int i = 0; i < GetCount(); ++i ) {
			if( GetItemAt(i) == m_pCaptionBar ) return i;
		}
		return GetCount();
	}

	bool CCarouselItemUI::Add(CControlUI* pControl)
	{
		if( m_pCaptionBar != NULL && pControl != m_pCaptionBar )
			return AddAt(pControl, FindCaptionBarIndex());
		return CVerticalLayoutUI::Add(pControl);
	}

	bool CCarouselItemUI::AddAt(CControlUI* pControl, int iIndex)
	{
		if( pControl == NULL ) return false;
		if( pControl != m_pCaptionBar && m_pCaptionBar != NULL ) {
			int iCap = FindCaptionBarIndex();
			if( iIndex < 0 || iIndex > iCap ) iIndex = iCap;
		}
		return CVerticalLayoutUI::AddAt(pControl, iIndex);
	}

	void CCarouselItemUI::EnsureCaptionBar()
	{
		if( m_pCaptionBar != NULL ) return;

		m_pCaptionBar = new CHorizontalLayoutUI;
		m_pCaptionBar->SetFixedHeight(60);
		m_pCaptionBar->SetBackgroundColor(0x32323CFF);
		m_pCaptionBar->SetAttribute(_T("padding"), _T("16,6,16,6"));

		CVerticalLayoutUI* pCaptionVBox = new CVerticalLayoutUI;
		pCaptionVBox->SetFixedHeight(0);

		m_pCaptionTitle = new CLabelUI;
		m_pCaptionTitle->SetAttribute(_T("color"), _T("#FFFFFFFF"));
		m_pCaptionTitle->SetAttribute(_T("font-size"), _T("16"));
		m_pCaptionTitle->SetAttribute(_T("text-align"), _T("left"));
		m_pCaptionTitle->SetAttribute(_T("vertical-align"), _T("vcenter"));
		m_pCaptionTitle->SetFixedHeight(28);
		pCaptionVBox->Add(m_pCaptionTitle);

		m_pCaptionText = new CLabelUI;
		m_pCaptionText->SetAttribute(_T("color"), _T("#C8C8C8FF"));
		m_pCaptionText->SetAttribute(_T("font-size"), _T("12"));
		m_pCaptionText->SetAttribute(_T("text-align"), _T("left"));
		m_pCaptionText->SetAttribute(_T("vertical-align"), _T("vcenter"));
		m_pCaptionText->SetFixedHeight(22);
		pCaptionVBox->Add(m_pCaptionText);

		m_pCaptionBar->Add(pCaptionVBox);
		CVerticalLayoutUI::Add(m_pCaptionBar);
	}

	void CCarouselItemUI::ApplyCaptionAlign(LPCTSTR pstrAlign)
	{
		if( pstrAlign == NULL ) return;
		LPCTSTR pAlign = _T("left");
		if( _tcsicmp(pstrAlign, _T("center")) == 0 ) pAlign = _T("center");
		else if( _tcsicmp(pstrAlign, _T("right")) == 0 ) pAlign = _T("right");
		if( m_pCaptionTitle ) m_pCaptionTitle->SetAttribute(_T("text-align"), pAlign);
		if( m_pCaptionText ) m_pCaptionText->SetAttribute(_T("text-align"), pAlign);
	}

	void CCarouselItemUI::ApplyCaptionKind(LPCTSTR pstrKind)
	{
		if( m_pCaptionBar == NULL || pstrKind == NULL ) return;
		InitKindColors();
		ControlKind kind = CONTROLKIND_DEFAULT;
		if( _tcsicmp(pstrKind, _T("primary")) == 0 ) kind = CONTROLKIND_PRIMARY;
		else if( _tcsicmp(pstrKind, _T("secondary")) == 0 ) kind = CONTROLKIND_SECONDARY;
		else if( _tcsicmp(pstrKind, _T("success")) == 0 ) kind = CONTROLKIND_SUCCESS;
		else if( _tcsicmp(pstrKind, _T("danger")) == 0 ) kind = CONTROLKIND_DANGER;
		else if( _tcsicmp(pstrKind, _T("warning")) == 0 ) kind = CONTROLKIND_WARNING;
		else if( _tcsicmp(pstrKind, _T("info")) == 0 ) kind = CONTROLKIND_INFO;
		else if( _tcsicmp(pstrKind, _T("light")) == 0 ) kind = CONTROLKIND_LIGHT;
		else if( _tcsicmp(pstrKind, _T("dark")) == 0 ) kind = CONTROLKIND_DARK;
		else if( _tcsicmp(pstrKind, _T("default")) == 0 ) kind = CONTROLKIND_DEFAULT;
		else return;

		const KindStateColors& normal = g_kindColors[(int)kind].Normal;
		m_pCaptionBar->SetBackgroundColor(normal.dwBackgroundColor);
		if( m_pCaptionTitle ) m_pCaptionTitle->SetColor(normal.dwColor != 0 ? normal.dwColor : 0xFFFFFFFF);
		if( m_pCaptionText ) {
			DWORD c = normal.dwColor != 0 ? normal.dwColor : 0xC8C8C8FF;
			m_pCaptionText->SetColor(c);
		}
		m_pCaptionBar->Invalidate();
	}

	void CCarouselItemUI::SetAttribute(LPCTSTR pstrName, LPCTSTR pstrValue)
	{
		if( _tcsicmp(pstrName, _T("caption-title")) == 0 || _tcsicmp(pstrName, _T("caption-text")) == 0 ) {
			EnsureCaptionBar();
			if( _tcsicmp(pstrName, _T("caption-title")) == 0 && m_pCaptionTitle )
				m_pCaptionTitle->SetText(pstrValue);
			else if( m_pCaptionText )
				m_pCaptionText->SetText(pstrValue);
		}
		else if( _tcsicmp(pstrName, _T("caption-align")) == 0 ) {
			EnsureCaptionBar();
			ApplyCaptionAlign(pstrValue);
		}
		else if( _tcsicmp(pstrName, _T("caption-kind")) == 0 ) {
			EnsureCaptionBar();
			ApplyCaptionKind(pstrValue);
		}
		else if( _tcsicmp(pstrName, _T("caption-background")) == 0 ) {
			EnsureCaptionBar();
			DWORD clr = 0;
			if( ParseColorString(pstrValue, clr) && m_pCaptionBar )
				m_pCaptionBar->SetBackgroundColor(clr);
		}
		else {
			CVerticalLayoutUI::SetAttribute(pstrName, pstrValue);
		}
	}
}
