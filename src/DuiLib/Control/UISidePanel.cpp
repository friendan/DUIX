#include "StdAfx.h"
#include "UISidePanel.h"

namespace DuiLib
{
	IMPLEMENT_DUICONTROL(CSidePanelUI)

	CSidePanelUI::CSidePanelUI()
		: m_ePlacement(PlacementRight)
		, m_nPanelWidth(320)
		, m_nPanelHeight(280)
		, m_fPanelWidthPercent(0.f)
		, m_fPanelHeightPercent(0.f)
		, m_nDuration(200)
		, m_bMask(true)
		, m_dwMaskColor(0x00000060)
		, m_bClickMaskClose(true)
		, m_bEscClose(false)
		, m_bClosable(true)
		, m_bOpen(false)
		, m_bAnimating(false)
		, m_bChromeReady(false)
		, m_bFillHost(false)
		, m_bHostResize(false)
		, m_eHeaderAction(UIACTION_TITLE)
		, m_pRestoreFocus(NULL)
		, m_pMask(NULL)
		, m_pPanel(NULL)
		, m_pHeader(NULL)
		, m_pTitleLabel(NULL)
		, m_pCloseBtn(NULL)
		, m_pBody(NULL)
	{
		Attach(this);
		SetAbsolute(true);
		TPercentInfo pi = { 0.0, 0.0, 1.0, 1.0 };
		SetAbsolutePercent(pi);
		SetVisible(false);
		SetMouseEnabled(true);
		SetMouseChildEnabled(true);
	}

	CSidePanelUI::~CSidePanelUI()
	{
		StopAnimation();
	}

	LPCTSTR CSidePanelUI::GetClass() const
	{
		return _T("SidePanelUI");
	}

	LPVOID CSidePanelUI::GetInterface(LPCTSTR pstrName)
	{
		if( _tcsicmp(pstrName, DUI_CTR_SIDEPANEL) == 0 )
			return static_cast<CSidePanelUI*>(this);
		return CContainerUI::GetInterface(pstrName);
	}

	UINT CSidePanelUI::GetControlFlags() const
	{
		return UIFLAG_SETCURSOR;
	}

	bool CSidePanelUI::PreferClientHit() const
	{
		return IsVisible() && IsEnabled();
	}

	bool CSidePanelUI::IsOpen() const
	{
		return m_bOpen;
	}

	void CSidePanelUI::SetPlacement(Placement e)
	{
		if( m_ePlacement == e ) return;
		m_ePlacement = e;
		if( IsVisible() ) LayoutChrome();
	}

	CSidePanelUI::Placement CSidePanelUI::GetPlacement() const
	{
		return m_ePlacement;
	}

	void CSidePanelUI::SetPanelWidth(int nWidth)
	{
		if( nWidth < 80 ) nWidth = 80;
		m_fPanelWidthPercent = 0.f;
		if( m_nPanelWidth == nWidth ) return;
		m_nPanelWidth = nWidth;
		if( IsVisible() ) LayoutChrome();
	}

	int CSidePanelUI::GetPanelWidth() const
	{
		return m_nPanelWidth;
	}

	void CSidePanelUI::SetPanelWidthPercent(float fPercent)
	{
		if( fPercent < 0.05f ) fPercent = 0.05f;
		if( fPercent > 1.f ) fPercent = 1.f;
		if( m_fPanelWidthPercent == fPercent ) return;
		m_fPanelWidthPercent = fPercent;
		if( IsVisible() ) LayoutChrome();
	}

	float CSidePanelUI::GetPanelWidthPercent() const
	{
		return m_fPanelWidthPercent;
	}

	void CSidePanelUI::SetPanelHeight(int nHeight)
	{
		if( nHeight < 80 ) nHeight = 80;
		m_fPanelHeightPercent = 0.f;
		if( m_nPanelHeight == nHeight ) return;
		m_nPanelHeight = nHeight;
		if( IsVisible() ) LayoutChrome();
	}

	int CSidePanelUI::GetPanelHeight() const
	{
		return m_nPanelHeight;
	}

	void CSidePanelUI::SetPanelHeightPercent(float fPercent)
	{
		if( fPercent < 0.05f ) fPercent = 0.05f;
		if( fPercent > 1.f ) fPercent = 1.f;
		if( m_fPanelHeightPercent == fPercent ) return;
		m_fPanelHeightPercent = fPercent;
		if( IsVisible() ) LayoutChrome();
	}

	float CSidePanelUI::GetPanelHeightPercent() const
	{
		return m_fPanelHeightPercent;
	}

	void CSidePanelUI::SetDuration(int nMs)
	{
		if( nMs < 0 ) nMs = 0;
		m_nDuration = nMs;
	}

	int CSidePanelUI::GetDuration() const
	{
		return m_nDuration;
	}

	void CSidePanelUI::SetMaskEnabled(bool b)
	{
		m_bMask = b;
		if( m_pMask != NULL ) m_pMask->SetVisible(b && IsVisible());
	}

	bool CSidePanelUI::IsMaskEnabled() const
	{
		return m_bMask;
	}

	void CSidePanelUI::SetMaskColor(DWORD dwColor)
	{
		m_dwMaskColor = dwColor;
		if( m_pMask != NULL && !m_bAnimating )
			m_pMask->SetBackgroundColor(m_bOpen ? m_dwMaskColor : DuiColorSetA(m_dwMaskColor, 0));
	}

	DWORD CSidePanelUI::GetMaskColor() const
	{
		return m_dwMaskColor;
	}

	void CSidePanelUI::SetClickMaskClose(bool b)
	{
		m_bClickMaskClose = b;
	}

	bool CSidePanelUI::IsClickMaskClose() const
	{
		return m_bClickMaskClose;
	}

	void CSidePanelUI::SetEscClose(bool b)
	{
		m_bEscClose = b;
	}

	bool CSidePanelUI::IsEscClose() const
	{
		return m_bEscClose;
	}

	void CSidePanelUI::SetClosable(bool b)
	{
		if( m_bClosable == b ) return;
		m_bClosable = b;
		SyncHeader();
	}

	bool CSidePanelUI::IsClosable() const
	{
		return m_bClosable;
	}

	void CSidePanelUI::SetTitle(LPCTSTR pstrTitle)
	{
		m_sTitle = pstrTitle ? pstrTitle : _T("");
		SyncHeader();
	}

	LPCTSTR CSidePanelUI::GetTitle() const
	{
		return m_sTitle.GetData();
	}

	UIAction CSidePanelUI::ParseHeaderAction(LPCTSTR pstrValue)
	{
		if( pstrValue == NULL || *pstrValue == _T('\0') ) return UIACTION_NONE;
		if( _tcsicmp(pstrValue, _T("title")) == 0 ) return UIACTION_TITLE;
		if( _tcsicmp(pstrValue, _T("move")) == 0
			|| _tcsicmp(pstrValue, _T("movewindow")) == 0 ) return UIACTION_MOVEWINDOW;
		if( _tcsicmp(pstrValue, _T("none")) == 0
			|| _tcsicmp(pstrValue, _T("false")) == 0
			|| _tcscmp(pstrValue, _T("0")) == 0 ) return UIACTION_NONE;
		return UIACTION_NONE;
	}

	void CSidePanelUI::SetHeaderAction(UIAction action)
	{
		if( m_eHeaderAction == action ) return;
		m_eHeaderAction = action;
		ApplyHeaderAction();
	}

	void CSidePanelUI::SetFillHost(bool bFill)
	{
		if( m_bFillHost == bFill ) return;
		m_bFillHost = bFill;
		if( bFill ) {
			m_bHostResize = true;
			m_fPanelWidthPercent = 1.f;
			m_fPanelHeightPercent = 1.f;
			SetMaskEnabled(false);
			if( m_eHeaderAction == UIACTION_NONE )
				SetHeaderAction(UIACTION_TITLE);
		}
		SyncFillHostChrome();
		if( IsVisible() ) LayoutChrome();
		else Invalidate();
	}

	void CSidePanelUI::SyncFillHostChrome()
	{
		EnsureChrome();
		if( m_pPanel == NULL ) return;
		// 铺满时去掉面板描边，避免左/顶 1px 边看起来像露主窗
		if( m_bFillHost ) {
			m_pPanel->SetBorderWidth(0);
		}
		else {
			m_pPanel->SetBorderWidth(1);
			if( m_pPanel->GetBorderColor() == 0 )
				m_pPanel->SetBorderColor(0xDEE2E6FF);
		}
	}

	void CSidePanelUI::SetHostResize(bool bResize)
	{
		if( m_bHostResize == bResize ) return;
		m_bHostResize = bResize;
	}

	LRESULT CSidePanelUI::HitHostResize(POINT ptClient) const
	{
		if( !m_bFillHost || !m_bHostResize || !m_bOpen || m_bAnimating )
			return HTCLIENT;
		if( m_pManager == NULL ) return HTCLIENT;
		HWND hWnd = m_pManager->GetPaintWindow();
		if( hWnd != NULL && ::IsZoomed(hWnd) ) return HTCLIENT;

		RECT rc = m_rcItem;
		if( rc.right <= rc.left || rc.bottom <= rc.top ) return HTCLIENT;
		if( !::PtInRect(&rc, ptClient) ) return HTCLIENT;

		RECT sb = m_pManager->GetSizeBox();
		if( sb.left < 1 && sb.top < 1 && sb.right < 1 && sb.bottom < 1 ) {
			sb.left = 4; sb.top = 4; sb.right = 6; sb.bottom = 6;
		}

		const bool bTop = (ptClient.y < rc.top + sb.top);
		const bool bBottom = (ptClient.y >= rc.bottom - sb.bottom);
		const bool bLeft = (ptClient.x < rc.left + sb.left);
		const bool bRight = (ptClient.x >= rc.right - sb.right);

		if( bTop && bLeft ) return HTTOPLEFT;
		if( bTop && bRight ) return HTTOPRIGHT;
		if( bBottom && bLeft ) return HTBOTTOMLEFT;
		if( bBottom && bRight ) return HTBOTTOMRIGHT;
		if( bTop ) return HTTOP;
		if( bBottom ) return HTBOTTOM;
		if( bLeft ) return HTLEFT;
		if( bRight ) return HTRIGHT;
		return HTCLIENT;
	}

	void CSidePanelUI::ApplyHeaderAction()
	{
		if( m_pHeader != NULL ) {
			m_pHeader->SetAction(m_eHeaderAction);
			// none 时给 header PreferClientHit，避免 html{action:title} 仍把标题栏当成拖窗
			m_pHeader->SetCursor(
				(m_eHeaderAction == UIACTION_TITLE || m_eHeaderAction == UIACTION_MOVEWINDOW)
				? 0 : DUI_ARROW);
		}
		if( m_pTitleLabel != NULL ) {
			m_pTitleLabel->SetAction(m_eHeaderAction);
			// title/move：禁用 Label 鼠标，命中落到 header 便于拖窗
			m_pTitleLabel->SetMouseEnabled(
				!(m_eHeaderAction == UIACTION_TITLE || m_eHeaderAction == UIACTION_MOVEWINDOW));
		}
	}

	void CSidePanelUI::ApplyThemeChrome(DWORD dwPanelBg, DWORD dwBorder, DWORD dwTitleColor)
	{
		EnsureChrome();
		if( m_pPanel != NULL ) {
			m_pPanel->SetBackgroundColor(dwPanelBg);
			m_pPanel->SetBorderColor(dwBorder);
			m_pPanel->SetBorderWidth(m_bFillHost ? 0 : 1);
		}
		if( m_pTitleLabel != NULL && dwTitleColor != 0 )
			m_pTitleLabel->SetColor(dwTitleColor);
		if( m_pHeader != NULL )
			m_pHeader->SetBorderColor(dwBorder);
		ApplyCloseButtonChrome(dwTitleColor != 0 ? dwTitleColor : 0x000000E0, dwPanelBg);
	}

	void CSidePanelUI::ApplyCloseButtonChrome(DWORD dwTitleColor, DWORD dwPanelBg)
	{
		if( m_pCloseBtn == NULL ) return;
		const int r = (int)DuiColorR(dwPanelBg);
		const int g = (int)DuiColorG(dwPanelBg);
		const int b = (int)DuiColorB(dwPanelBg);
		const int lum = (r * 299 + g * 587 + b * 114) / 1000;
		const bool bLight = (lum >= 160);
		DWORD dwIcon = dwTitleColor != 0 ? ((dwTitleColor & 0xFFFFFF00u) | 0xA6u) : (bLight ? 0x000000A6u : 0xFFFFFFB4u);
		DWORD dwIconHot = dwTitleColor != 0 ? dwTitleColor : (bLight ? 0x000000E0u : 0xFFFFFFFFu);
		DWORD dwHoverBk = bLight ? 0x00000014u : 0xFFFFFF22u;
		DWORD dwActiveBk = bLight ? 0x00000026u : 0xFFFFFF33u;
		m_pCloseBtn->SetColor(dwIcon);
		m_pCloseBtn->SetHoverColor(dwIconHot);
		m_pCloseBtn->SetActiveColor(dwIconHot);
		m_pCloseBtn->SetHoverBackgroundColor(dwHoverBk);
		m_pCloseBtn->SetActiveBackgroundColor(dwActiveBk);
	}

	int CSidePanelUI::AnimFrameCount() const
	{
		if( m_nDuration <= 0 ) return 1;
		int n = (m_nDuration + ANIM_ELAPSE - 1) / ANIM_ELAPSE;
		if( n < 1 ) n = 1;
		if( n > 60 ) n = 60;
		return n;
	}

	void CSidePanelUI::EnsureChrome()
	{
		if( m_bChromeReady ) return;
		m_bChromeReady = true;

		// 用 Button 作遮罩：PreferClientHit（SETCURSOR），避免 html{action:title} 下点遮罩变成拖窗
		CButtonUI* pMaskBtn = new CButtonUI;
		pMaskBtn->SetName(_T("__sidepanel_mask"));
		pMaskBtn->SetKind(CONTROLKIND_NONE);
		pMaskBtn->SetMouseEnabled(true);
		pMaskBtn->SetBorderWidth(0);
		pMaskBtn->SetBackgroundColor(DuiColorSetA(m_dwMaskColor, 0));
		pMaskBtn->SetCursor(DUI_ARROW);
		pMaskBtn->OnNotify += MakeDelegate(this, &CSidePanelUI::OnMaskClick);
		m_pMask = pMaskBtn;
		CContainerUI::Add(m_pMask);

		m_pPanel = new CVerticalLayoutUI;
		m_pPanel->SetName(_T("__sidepanel_panel"));
		m_pPanel->SetBackgroundColor(0xFFFFFFFF);
		m_pPanel->SetBorderColor(0xDEE2E6FF);
		m_pPanel->SetBorderWidth(1);
		m_pPanel->SetMouseEnabled(true);
		m_pPanel->SetMouseChildEnabled(true);
		CContainerUI::Add(m_pPanel);

		m_pBody = new CVerticalLayoutUI;
		m_pBody->SetName(_T("__sidepanel_body"));
		m_pBody->SetPadding(CDuiBox(12, 12, 12, 12));
		m_pBody->SetMouseEnabled(true);
		m_pBody->SetMouseChildEnabled(true);
		m_pPanel->Add(m_pBody);

		SyncHeader();
	}

	void CSidePanelUI::SyncHeader()
	{
		EnsureChrome();
		const bool bNeed = m_bClosable || !m_sTitle.IsEmpty();
		if( !bNeed ) {
			if( m_pHeader != NULL ) {
				m_pPanel->Remove(m_pHeader);
				m_pHeader = NULL;
				m_pTitleLabel = NULL;
				m_pCloseBtn = NULL;
			}
			return;
		}

		if( m_pHeader == NULL ) {
			m_pHeader = new CHorizontalLayoutUI;
			m_pHeader->SetName(_T("__sidepanel_header"));
			m_pHeader->SetFixedHeight(44);
			m_pHeader->SetPadding(CDuiBox(0, 8, 0, 12));
			m_pHeader->SetAttribute(_T("align-items"), _T("vcenter"));
			RECT rcBd = { 0, 0, 0, 1 };
			m_pHeader->SetBorderWidth(rcBd);
			m_pHeader->SetBorderColor(0xDEE2E6FF);

			m_pTitleLabel = new CLabelUI;
			m_pTitleLabel->SetName(_T("__sidepanel_title"));
			m_pTitleLabel->SetTextStyle(DT_SINGLELINE | DT_VCENTER | DT_LEFT | DT_END_ELLIPSIS);
			m_pTitleLabel->SetAttribute(_T("vertical-align"), _T("vcenter"));
			m_pTitleLabel->SetFontSize(14);
			m_pHeader->Add(m_pTitleLabel);

			m_pCloseBtn = new CButtonUI;
			m_pCloseBtn->SetName(_T("__sidepanel_close"));
			m_pCloseBtn->SetKind(CONTROLKIND_NONE);
			m_pCloseBtn->SetFixedWidth(32);
			m_pCloseBtn->SetFixedHeight(32);
			m_pCloseBtn->SetText(_T("\xD7"));
			m_pCloseBtn->SetTextStyle(DT_SINGLELINE | DT_VCENTER | DT_CENTER);
			m_pCloseBtn->SetFontSize(16);
			m_pCloseBtn->SetBorderWidth(0);
			SIZE szRound = { 4, 4 };
			m_pCloseBtn->SetBorderRadius(szRound);
			m_pCloseBtn->SetCursor(DUI_HAND);
			ApplyCloseButtonChrome(0x000000E0, 0xFFFFFFFF);
			m_pCloseBtn->OnNotify += MakeDelegate(this, &CSidePanelUI::OnCloseClick);
			m_pHeader->Add(m_pCloseBtn);

			m_pPanel->AddAt(m_pHeader, 0);
			ApplyHeaderAction();
		}

		if( m_pTitleLabel != NULL )
			m_pTitleLabel->SetText(m_sTitle.GetData());
		if( m_pCloseBtn != NULL )
			m_pCloseBtn->SetVisible(m_bClosable);
	}

	bool CSidePanelUI::OnCloseClick(void* param)
	{
		TNotifyUI* pMsg = static_cast<TNotifyUI*>(param);
		if( pMsg == NULL ) return true;
		if( pMsg->sType != DUI_MSGTYPE_CLICK ) return true;
		Hide(true);
		return true;
	}

	bool CSidePanelUI::OnMaskClick(void* param)
	{
		TNotifyUI* pMsg = static_cast<TNotifyUI*>(param);
		if( pMsg == NULL ) return true;
		if( pMsg->sType != DUI_MSGTYPE_CLICK ) return true;
		if( m_bClickMaskClose && m_bMask )
			Hide(true);
		return true;
	}

	bool CSidePanelUI::ParseSizeValue(LPCTSTR pstrValue, int& nPx, float& fPercent)
	{
		nPx = 0;
		fPercent = 0.f;
		if( pstrValue == NULL ) return false;
		LPCTSTR p = pstrValue;
		while( *p == _T(' ') || *p == _T('\t') ) ++p;
		LPTSTR pEnd = NULL;
		double v = _tcstod(p, &pEnd);
		if( pEnd == p ) return false;
		while( *pEnd == _T(' ') || *pEnd == _T('\t') ) ++pEnd;
		if( *pEnd == _T('%') ) {
			fPercent = (float)(v / 100.0);
			if( fPercent < 0.05f ) fPercent = 0.05f;
			if( fPercent > 1.f ) fPercent = 1.f;
			return true;
		}
		nPx = (int)v;
		if( nPx < 80 ) nPx = 80;
		return true;
	}

	int CSidePanelUI::ResolvePanelThickness(const RECT& rcHost) const
	{
		const bool bVert = (m_ePlacement == PlacementTop || m_ePlacement == PlacementBottom);
		if( m_bFillHost ) {
			int nThick = bVert ? (rcHost.bottom - rcHost.top) : (rcHost.right - rcHost.left);
			if( nThick < 1 ) nThick = 1;
			return nThick;
		}
		int nThick = 0;
		if( bVert ) {
			if( m_fPanelHeightPercent > 0.f )
				nThick = (int)((rcHost.bottom - rcHost.top) * m_fPanelHeightPercent + 0.5f);
			else {
				nThick = m_nPanelHeight;
				if( m_pManager != NULL )
					nThick = m_pManager->GetDPIObj()->Scale(nThick);
			}
		}
		else {
			if( m_fPanelWidthPercent > 0.f )
				nThick = (int)((rcHost.right - rcHost.left) * m_fPanelWidthPercent + 0.5f);
			else {
				nThick = m_nPanelWidth;
				if( m_pManager != NULL )
					nThick = m_pManager->GetDPIObj()->Scale(nThick);
			}
		}
		if( nThick < 1 ) nThick = 1;
		return nThick;
	}

	RECT CSidePanelUI::CalcPanelRect(float fProgress) const
	{
		RECT rc = m_rcItem;
		if( fProgress < 0.f ) fProgress = 0.f;
		if( fProgress > 1.f ) fProgress = 1.f;

		const int nThick = ResolvePanelThickness(rc);

		// 打开/关闭端点用整数矩形，避开浮点；铺满打开 = 整个宿主区
		if( fProgress >= 1.f ) {
			if( m_bFillHost ) return rc;
			RECT rcOpen = rc;
			if( m_ePlacement == PlacementLeft ) {
				rcOpen.right = rc.left + nThick;
			}
			else if( m_ePlacement == PlacementRight ) {
				rcOpen.left = rc.right - nThick;
			}
			else if( m_ePlacement == PlacementTop ) {
				rcOpen.bottom = rc.top + nThick;
			}
			else {
				rcOpen.top = rc.bottom - nThick;
			}
			return rcOpen;
		}
		if( fProgress <= 0.f ) {
			RECT rcClosed = rc;
			if( m_ePlacement == PlacementLeft ) {
				rcClosed.left = rc.left - nThick;
				rcClosed.right = rc.left;
			}
			else if( m_ePlacement == PlacementRight ) {
				rcClosed.left = rc.right;
				rcClosed.right = rc.right + nThick;
			}
			else if( m_ePlacement == PlacementTop ) {
				rcClosed.top = rc.top - nThick;
				rcClosed.bottom = rc.top;
			}
			else {
				rcClosed.top = rc.bottom;
				rcClosed.bottom = rc.bottom + nThick;
			}
			return rcClosed;
		}

		// 负方向插值：不能 (LONG)(x+0.5)（向零截断会少 1px）；Left/Top 为正、Right/Bottom 为负
		auto lerp = [](LONG a, LONG b, float t) -> LONG {
			const double v = (double)a + (double)(b - a) * (double)t;
			return (v >= 0.0) ? (LONG)(v + 0.5) : (LONG)(v - 0.5);
		};

		RECT rcPanel = { 0 };
		if( m_ePlacement == PlacementLeft ) {
			const LONG openL = rc.left;
			const LONG closedL = rc.left - nThick;
			const LONG left = lerp(closedL, openL, fProgress);
			rcPanel.left = left;
			rcPanel.right = left + nThick;
			rcPanel.top = rc.top;
			rcPanel.bottom = rc.bottom;
		}
		else if( m_ePlacement == PlacementRight ) {
			const LONG openL = rc.right - nThick;
			const LONG closedL = rc.right;
			const LONG left = lerp(closedL, openL, fProgress);
			rcPanel.left = left;
			rcPanel.right = left + nThick;
			rcPanel.top = rc.top;
			rcPanel.bottom = rc.bottom;
		}
		else if( m_ePlacement == PlacementTop ) {
			const LONG openT = rc.top;
			const LONG closedT = rc.top - nThick;
			const LONG top = lerp(closedT, openT, fProgress);
			rcPanel.top = top;
			rcPanel.bottom = top + nThick;
			rcPanel.left = rc.left;
			rcPanel.right = rc.right;
		}
		else { // PlacementBottom
			const LONG openT = rc.bottom - nThick;
			const LONG closedT = rc.bottom;
			const LONG top = lerp(closedT, openT, fProgress);
			rcPanel.top = top;
			rcPanel.bottom = top + nThick;
			rcPanel.left = rc.left;
			rcPanel.right = rc.right;
		}

		// 铺满过程中钳到宿主区，避免 right/bottom 越界露底
		if( m_bFillHost ) {
			if( rcPanel.left < rc.left ) rcPanel.left = rc.left;
			if( rcPanel.top < rc.top ) rcPanel.top = rc.top;
			if( rcPanel.right > rc.right ) rcPanel.right = rc.right;
			if( rcPanel.bottom > rc.bottom ) rcPanel.bottom = rc.bottom;
		}
		return rcPanel;
	}

	void CSidePanelUI::FocusInside()
	{
		if( m_pManager == NULL || !IsVisible() ) return;
		CControlUI* pFocus = NULL;
		if( m_bClosable && m_pCloseBtn != NULL && m_pCloseBtn->IsVisible() && m_pCloseBtn->IsEnabled() )
			pFocus = m_pCloseBtn;
		if( pFocus == NULL && m_pBody != NULL )
			pFocus = FindFirstTabStop(m_pBody);
		if( pFocus == NULL )
			pFocus = this;
		m_pManager->SetFocus(pFocus);
	}

	CControlUI* CSidePanelUI::FindFirstTabStop(CControlUI* pRoot)
	{
		if( pRoot == NULL || !pRoot->IsVisible() || !pRoot->IsEnabled() ) return NULL;
		if( (pRoot->GetControlFlags() & UIFLAG_TABSTOP) != 0 ) return pRoot;
		IContainerUI* pCont = static_cast<IContainerUI*>(pRoot->GetInterface(_T("IContainer")));
		if( pCont == NULL ) return NULL;
		for( int i = 0; i < pCont->GetCount(); ++i ) {
			CControlUI* p = FindFirstTabStop(pCont->GetItemAt(i));
			if( p != NULL ) return p;
		}
		return NULL;
	}

	void CSidePanelUI::RestoreFocus()
	{
		if( m_pManager == NULL ) {
			m_pRestoreFocus = NULL;
			return;
		}
		CControlUI* pRestore = m_pRestoreFocus;
		m_pRestoreFocus = NULL;
		if( pRestore == NULL ) return;
		if( pRestore->GetManager() != m_pManager ) return;
		if( !pRestore->IsVisible() || !pRestore->IsEnabled() ) return;
		// 勿把焦点还给已隐藏的本抽屉内控件
		for( CControlUI* p = pRestore; p != NULL; p = p->GetParent() ) {
			if( p == this ) return;
		}
		m_pManager->SetFocus(pRestore);
	}

	void CSidePanelUI::ApplyMaskAlpha(float fProgress)
	{
		if( m_pMask == NULL ) return;
		if( !m_bMask ) {
			m_pMask->SetVisible(false);
			return;
		}
		m_pMask->SetVisible(true);
		BYTE aTarget = DuiColorA(m_dwMaskColor);
		BYTE a = (BYTE)((float)aTarget * fProgress + 0.5f);
		m_pMask->SetBackgroundColor(DuiColorSetA(m_dwMaskColor, a));
	}

	void CSidePanelUI::LayoutChrome()
	{
		EnsureChrome();
		if( m_pMask == NULL || m_pPanel == NULL ) return;

		float fProg = m_bOpen ? 1.f : 0.f;
		if( m_bAnimating && IsAnimationRunning(ANIM_OPEN) ) {
			int nTotal = AnimFrameCount();
			int nCur = GetCurrentFrame(ANIM_OPEN);
			fProg = nTotal > 0 ? (float)nCur / (float)nTotal : 1.f;
		}
		else if( m_bAnimating && IsAnimationRunning(ANIM_CLOSE) ) {
			int nTotal = AnimFrameCount();
			int nCur = GetCurrentFrame(ANIM_CLOSE);
			fProg = nTotal > 0 ? 1.f - (float)nCur / (float)nTotal : 0.f;
		}

		m_pMask->SetPos(m_rcItem, false);
		ApplyMaskAlpha(fProg);

		RECT rcPanel = CalcPanelRect(fProg);
		m_pPanel->SetPos(rcPanel, false);
	}

	void CSidePanelUI::Show(bool bAnimate)
	{
		EnsureChrome();
		if( m_bOpen && !m_bAnimating ) {
			LayoutChrome();
			FocusInside();
			Invalidate();
			return;
		}

		StopAnimation();
		m_bAnimating = false;
		if( m_pManager != NULL )
			m_pRestoreFocus = m_pManager->GetFocus();
		m_bOpen = true;
		SetVisible(true);
		SetInternVisible(true);
		NeedParentUpdate();

		if( m_pManager != NULL )
			m_pManager->SendNotify(this, DUI_MSGTYPE_SIDEPANELOPEN);

		if( !bAnimate || m_nDuration <= 0 ) {
			LayoutChrome();
			FocusInside();
			Invalidate();
			return;
		}

		m_bAnimating = true;
		ApplyMaskAlpha(0.f);
		if( m_pPanel != NULL )
			m_pPanel->SetPos(CalcPanelRect(0.f), false);
		StartAnimation(ANIM_ELAPSE, AnimFrameCount(), ANIM_OPEN);
	}

	void CSidePanelUI::Hide(bool bAnimate)
	{
		EnsureChrome();
		if( !m_bOpen && !IsVisible() ) return;

		StopAnimation();
		m_bAnimating = false;

		if( !bAnimate || m_nDuration <= 0 || !IsVisible() ) {
			m_bOpen = false;
			ApplyMaskAlpha(0.f);
			SetVisible(false);
			RestoreFocus();
			if( m_pManager != NULL )
				m_pManager->SendNotify(this, DUI_MSGTYPE_SIDEPANELCLOSE);
			return;
		}

		m_bOpen = false;
		m_bAnimating = true;
		StartAnimation(ANIM_ELAPSE, AnimFrameCount(), ANIM_CLOSE);
	}

	void CSidePanelUI::Toggle(bool bAnimate)
	{
		if( m_bOpen || (m_bAnimating && IsAnimationRunning(ANIM_OPEN)) )
			Hide(bAnimate);
		else
			Show(bAnimate);
	}

	void CSidePanelUI::OnAnimationStep(int nTotalFrame, int nCurFrame, int nAnimationID)
	{
		if( nTotalFrame <= 0 ) return;
		float fProg = (float)nCurFrame / (float)nTotalFrame;
		if( nAnimationID == ANIM_CLOSE )
			fProg = 1.f - fProg;

		ApplyMaskAlpha(fProg);
		if( m_pPanel != NULL )
			m_pPanel->SetPos(CalcPanelRect(fProg), false);
		Invalidate();
	}

	void CSidePanelUI::OnAnimationStop(int nAnimationID)
	{
		m_bAnimating = false;
		if( nAnimationID == ANIM_CLOSE ) {
			m_bOpen = false;
			ApplyMaskAlpha(0.f);
			SetVisible(false);
			RestoreFocus();
			if( m_pManager != NULL )
				m_pManager->SendNotify(this, DUI_MSGTYPE_SIDEPANELCLOSE);
		}
		else if( nAnimationID == ANIM_OPEN ) {
			m_bOpen = true;
			LayoutChrome();
			FocusInside();
		}
	}

	bool CSidePanelUI::Add(CControlUI* pControl)
	{
		EnsureChrome();
		if( pControl == NULL ) return false;
		if( pControl == m_pMask || pControl == m_pPanel )
			return CContainerUI::Add(pControl);
		return m_pBody != NULL ? m_pBody->Add(pControl) : false;
	}

	bool CSidePanelUI::AddAt(CControlUI* pControl, int iIndex)
	{
		EnsureChrome();
		if( pControl == NULL ) return false;
		if( pControl == m_pMask || pControl == m_pPanel )
			return CContainerUI::AddAt(pControl, iIndex);
		return m_pBody != NULL ? m_pBody->AddAt(pControl, iIndex) : false;
	}

	bool CSidePanelUI::Remove(CControlUI* pControl)
	{
		EnsureChrome();
		if( pControl == NULL ) return false;
		if( pControl == m_pMask || pControl == m_pPanel || pControl == m_pHeader
			|| pControl == m_pBody || pControl == m_pTitleLabel || pControl == m_pCloseBtn )
			return false;
		return m_pBody != NULL ? m_pBody->Remove(pControl) : false;
	}

	void CSidePanelUI::RemoveAll()
	{
		EnsureChrome();
		if( m_pBody != NULL ) m_pBody->RemoveAll();
	}

	void CSidePanelUI::SetPos(RECT rc, bool bNeedInvalidate)
	{
		CControlUI::SetPos(rc, bNeedInvalidate);
		LayoutChrome();
	}

	void CSidePanelUI::DoInit()
	{
		EnsureChrome();
		CContainerUI::DoInit();
	}

	void CSidePanelUI::DoCaptureEvent(TEventUI& event)
	{
		if( m_bEscClose && IsVisible() && (m_bOpen || m_bAnimating)
			&& event.Type == UIEVENT_KEYDOWN && event.chKey == VK_ESCAPE ) {
			Hide(true);
			event.StopPropagation();
			return;
		}
		CControlUI::DoCaptureEvent(event);
	}

	void CSidePanelUI::DoEvent(TEventUI& event)
	{
		if( event.Type == UIEVENT_TIMER ) {
			OnAnimationElapse((int)event.wParam);
			return;
		}
		if( m_bEscClose && IsVisible() && (m_bOpen || m_bAnimating)
			&& event.Type == UIEVENT_KEYDOWN && event.chKey == VK_ESCAPE ) {
			Hide(true);
			return;
		}
		if( event.Type == UIEVENT_BUTTONDOWN && m_bClickMaskClose && m_bMask && IsVisible() ) {
			if( m_pPanel != NULL && !::PtInRect(&m_pPanel->GetPos(), event.ptMouse) ) {
				Hide(true);
				return;
			}
		}
		CContainerUI::DoEvent(event);
	}

	void CSidePanelUI::SetAttribute(LPCTSTR pstrName, LPCTSTR pstrValue)
	{
		if( pstrName == NULL ) return;

		if( _tcsicmp(pstrName, _T("placement")) == 0 ) {
			if( _tcsicmp(pstrValue, _T("left")) == 0 ) SetPlacement(PlacementLeft);
			else if( _tcsicmp(pstrValue, _T("top")) == 0 ) SetPlacement(PlacementTop);
			else if( _tcsicmp(pstrValue, _T("bottom")) == 0 ) SetPlacement(PlacementBottom);
			else SetPlacement(PlacementRight);
		}
		else if( _tcsicmp(pstrName, _T("panel-width")) == 0
			|| _tcsicmp(pstrName, _T("width")) == 0 ) {
			int nPx = 0; float fPct = 0.f;
			if( ParseSizeValue(pstrValue, nPx, fPct) ) {
				if( fPct > 0.f ) SetPanelWidthPercent(fPct);
				else SetPanelWidth(nPx);
			}
		}
		else if( _tcsicmp(pstrName, _T("panel-height")) == 0
			|| _tcsicmp(pstrName, _T("height")) == 0 ) {
			int nPx = 0; float fPct = 0.f;
			if( ParseSizeValue(pstrValue, nPx, fPct) ) {
				if( fPct > 0.f ) SetPanelHeightPercent(fPct);
				else SetPanelHeight(nPx);
			}
		}
		else if( _tcsicmp(pstrName, _T("duration")) == 0
			|| _tcsicmp(pstrName, _T("animation-duration")) == 0 ) {
			if( pstrValue != NULL ) SetDuration(_ttoi(pstrValue));
		}
		else if( _tcsicmp(pstrName, _T("mask")) == 0 ) {
			SetMaskEnabled(_tcsicmp(pstrValue, _T("true")) == 0 || _tcscmp(pstrValue, _T("1")) == 0);
		}
		else if( _tcsicmp(pstrName, _T("mask-color")) == 0 ) {
			DWORD c = 0;
			if( ParseColorString(pstrValue, c) ) SetMaskColor(c);
		}
		else if( _tcsicmp(pstrName, _T("click-mask-close")) == 0 ) {
			SetClickMaskClose(_tcsicmp(pstrValue, _T("true")) == 0 || _tcscmp(pstrValue, _T("1")) == 0);
		}
		else if( _tcsicmp(pstrName, _T("esc-close")) == 0 ) {
			SetEscClose(_tcsicmp(pstrValue, _T("true")) == 0 || _tcscmp(pstrValue, _T("1")) == 0);
		}
		else if( _tcsicmp(pstrName, _T("closable")) == 0
			|| _tcsicmp(pstrName, _T("show-close")) == 0 ) {
			SetClosable(_tcsicmp(pstrValue, _T("true")) == 0 || _tcscmp(pstrValue, _T("1")) == 0);
		}
		else if( _tcsicmp(pstrName, _T("title")) == 0 ) {
			SetTitle(pstrValue);
		}
		else if( _tcsicmp(pstrName, _T("header-action")) == 0 ) {
			SetHeaderAction(ParseHeaderAction(pstrValue));
		}
		else if( _tcsicmp(pstrName, _T("header-drag")) == 0 ) {
			const bool bDrag = (_tcsicmp(pstrValue, _T("true")) == 0
				|| _tcscmp(pstrValue, _T("1")) == 0
				|| _tcsicmp(pstrValue, _T("yes")) == 0);
			SetHeaderAction(bDrag ? UIACTION_TITLE : UIACTION_NONE);
		}
		else if( _tcsicmp(pstrName, _T("fill-host")) == 0
			|| _tcsicmp(pstrName, _T("fillhost")) == 0 ) {
			SetFillHost(_tcsicmp(pstrValue, _T("true")) == 0 || _tcscmp(pstrValue, _T("1")) == 0);
		}
		else if( _tcsicmp(pstrName, _T("host-resize")) == 0
			|| _tcsicmp(pstrName, _T("hostresize")) == 0 ) {
			SetHostResize(_tcsicmp(pstrValue, _T("true")) == 0 || _tcscmp(pstrValue, _T("1")) == 0);
		}
		else {
			CContainerUI::SetAttribute(pstrName, pstrValue);
		}
	}
}
