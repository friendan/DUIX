#include "StdAfx.h"
#include "UITitleBar.h"

namespace DuiLib
{
	IMPLEMENT_DUICONTROL(CTitleBarLeftUI)
	IMPLEMENT_DUICONTROL(CTitleBarSysUI)
	IMPLEMENT_DUICONTROL(CTitleBarUI)

	//////////////////////////////////////////////////////////////////////////
	// CTitleBarLeftUI
	CTitleBarLeftUI::CTitleBarLeftUI()
	{
		SetFixedWidth(0); // 吃剩余宽度
		SetAttribute(_T("align-items"), _T("vcenter"));
		SetGap(8);
	}

	LPCTSTR CTitleBarLeftUI::GetClass() const
	{
		return _T("TitleBarLeftUI");
	}

	LPVOID CTitleBarLeftUI::GetInterface(LPCTSTR pstrName)
	{
		if( _tcsicmp(pstrName, DUI_CTR_TITLEBARLEFT) == 0 ) return static_cast<CTitleBarLeftUI*>(this);
		return CHorizontalLayoutUI::GetInterface(pstrName);
	}

	//////////////////////////////////////////////////////////////////////////
	// CTitleBarSysUI
	CTitleBarSysUI::CTitleBarSysUI()
	{
		SetAttribute(_T("align-items"), _T("vcenter"));
		SetGap(0);
	}

	LPCTSTR CTitleBarSysUI::GetClass() const
	{
		return _T("TitleBarSysUI");
	}

	LPVOID CTitleBarSysUI::GetInterface(LPCTSTR pstrName)
	{
		if( _tcsicmp(pstrName, DUI_CTR_TITLEBARSYS) == 0 ) return static_cast<CTitleBarSysUI*>(this);
		return CHorizontalLayoutUI::GetInterface(pstrName);
	}

	SIZE CTitleBarSysUI::EstimateSize(SIZE szAvailable)
	{
		SIZE sz = MeasureContent(szAvailable);
		if( GetFixedHeight() > 0 ) sz.cy = GetFixedHeight();
		else sz.cy = 0;
		if( sz.cx <= 0 ) sz.cx = 0;
		return sz;
	}

	//////////////////////////////////////////////////////////////////////////
	// CTitleBarUI
	CTitleBarUI::CTitleBarUI()
		: m_pLeft(NULL)
		, m_pSys(NULL)
		, m_pTitleLabel(NULL)
		, m_pMinBtn(NULL)
		, m_pMaxBtn(NULL)
		, m_pRestoreBtn(NULL)
		, m_pCloseBtn(NULL)
		, m_bShowMin(true)
		, m_bShowMax(true)
		, m_bShowClose(true)
		, m_bMinimizeToTray(false)
		, m_bCloseToTray(false)
		, m_nBtnWidth(46)
		, m_bNotifyCancel(false)
		, m_bChromeReady(false)
	{
		SetFixedHeight(40);
		SetBackgroundColor(0x333333FF);
		SetAction(UIACTION_TITLE);
		SetAttribute(_T("align-items"), _T("vcenter"));
		SetGap(0);
		// padding: CSS top,right,bottom,left → 左侧缩进 12
		SetPadding(CDuiBox(0, 0, 0, 12));
		EnsureChrome();
	}

	CTitleBarUI::~CTitleBarUI()
	{
	}

	LPCTSTR CTitleBarUI::GetClass() const
	{
		return _T("TitleBarUI");
	}

	LPVOID CTitleBarUI::GetInterface(LPCTSTR pstrName)
	{
		if( _tcsicmp(pstrName, DUI_CTR_TITLEBAR) == 0 ) return static_cast<CTitleBarUI*>(this);
		return CHorizontalLayoutUI::GetInterface(pstrName);
	}

	bool CTitleBarUI::ParseBoolValue(LPCTSTR pstrValue)
	{
		if( pstrValue == NULL ) return false;
		return ( _tcsicmp(pstrValue, _T("true")) == 0
			|| _tcsicmp(pstrValue, _T("1")) == 0
			|| _tcsicmp(pstrValue, _T("yes")) == 0 );
	}

	static int CALLBACK TitleBarEnumFontProc(const LOGFONT* /*lf*/, const TEXTMETRIC* /*tm*/, DWORD /*type*/, LPARAM lParam)
	{
		*reinterpret_cast<bool*>(lParam) = true;
		return 0; // 找到即停
	}

	bool CTitleBarUI::FontFamilyExists(LPCTSTR pstrFace)
	{
		if( pstrFace == NULL || *pstrFace == _T('\0') ) return false;
		HDC hDC = ::GetDC(NULL);
		if( hDC == NULL ) return false;
		LOGFONT lf;
		::ZeroMemory(&lf, sizeof(lf));
		lf.lfCharSet = DEFAULT_CHARSET;
		_tcsncpy_s(lf.lfFaceName, LF_FACESIZE, pstrFace, _TRUNCATE);
		bool bFound = false;
		::EnumFontFamiliesEx(hDC, &lf, TitleBarEnumFontProc, reinterpret_cast<LPARAM>(&bFound), 0);
		::ReleaseDC(NULL, hDC);
		return bFound;
	}

	bool CTitleBarUI::ResolveSegoeIconFont(CDuiString& sFace)
	{
		static bool s_bResolved = false;
		static bool s_bOk = false;
		static CDuiString s_sFace;
		if( s_bResolved ) {
			sFace = s_sFace;
			return s_bOk;
		}
		s_bResolved = true;
		static const LPCTSTR kFaces[] = {
			_T("Segoe Fluent Icons"), // Win11
			_T("Segoe MDL2 Assets"),  // Win10+
		};
		for( int i = 0; i < (int)(sizeof(kFaces) / sizeof(kFaces[0])); ++i ) {
			if( FontFamilyExists(kFaces[i]) ) {
				s_bOk = true;
				s_sFace = kFaces[i];
				sFace = s_sFace;
				return true;
			}
		}
		sFace.Empty();
		return false;
	}

	void CTitleBarUI::ApplySysButtonStyle(CButtonUI* pBtn, bool bClose)
	{
		if( pBtn == NULL ) return;
		pBtn->SetKind(CONTROLKIND_NONE);
		// 常态透明：整条标题栏只画 TitleBar 底色，避免按钮不透明底与栏色有缝
		pBtn->SetBackgroundColor(0);
		pBtn->SetBorderColor(0);
		pBtn->SetBorderWidth(0);
		pBtn->SetColor(0xB4B4BEFF);
		pBtn->SetHoverColor(0xFFFFFFFF);
		DWORD closeHover = 0xE81123FF;
		CThemeManager* tm = CThemeManager::GetInstance();
		if( tm != NULL ) {
			CTheme* th = tm->GetCurrentTheme();
			if( th == NULL ) th = tm->FindTheme(tm->GetDefaultThemeId());
			if( th != NULL )
				closeHover = th->GetToken(_T("color-titlebar-close-hover"),
					th->GetToken(_T("color-danger"), closeHover));
		}
		pBtn->SetHoverBackgroundColor(bClose ? closeHover : 0x505050FF);
		pBtn->SetActiveBackgroundColor(bClose ? closeHover : 0x505050FF);
		pBtn->SetTextStyle(DT_SINGLELINE | DT_VCENTER | DT_CENTER);
	}

	void CTitleBarUI::SyncSysButtonChrome()
	{
		if( !m_bChromeReady ) return;
		DWORD dwBk = GetBackgroundColor();
		if( dwBk == 0 ) dwBk = 0x333333FF;

		int r = (int)((dwBk >> 24) & 0xFF);
		int g = (int)((dwBk >> 16) & 0xFF);
		int b = (int)((dwBk >> 8) & 0xFF);
		int lum = (r * 299 + g * 587 + b * 114) / 1000;
		const bool bLightBar = (lum >= 160);

		DWORD dwIcon = bLightBar ? 0x000000A6 : 0xFFFFFFB4;
		DWORD dwIconHover = bLightBar ? 0x000000E0 : 0xFFFFFFFF;
		DWORD dwHoverBk = bLightBar ? 0x0000001A : 0xFFFFFF33;
		DWORD closeHover = 0xE81123FF;
		CThemeManager* tm = CThemeManager::GetInstance();
		if( tm != NULL ) {
			CTheme* th = tm->GetCurrentTheme();
			if( th == NULL ) th = tm->FindTheme(tm->GetDefaultThemeId());
			if( th != NULL )
				closeHover = th->GetToken(_T("color-titlebar-close-hover"),
					th->GetToken(_T("color-danger"), closeHover));
		}
		if( m_pTitleLabel != NULL ) {
			DWORD dwTitle = m_pTitleLabel->GetColor();
			if( dwTitle != 0 ) {
				dwIconHover = dwTitle;
				dwIcon = (dwTitle & 0xFFFFFF00) | 0xB4;
			}
		}

		CButtonUI* btns[] = { m_pMinBtn, m_pMaxBtn, m_pRestoreBtn, m_pCloseBtn };
		for( int i = 0; i < 4; ++i ) {
			if( btns[i] == NULL ) continue;
			btns[i]->SetKind(CONTROLKIND_NONE);
			btns[i]->SetBackgroundColor(0);
			btns[i]->SetBorderColor(0);
			btns[i]->SetBorderWidth(0);
			btns[i]->SetColor(dwIcon);
			btns[i]->SetHoverColor(dwIconHover);
			const bool bClose = (btns[i] == m_pCloseBtn);
			const DWORD dwHov = bClose ? closeHover : dwHoverBk;
			btns[i]->SetHoverBackgroundColor(dwHov);
			btns[i]->SetActiveBackgroundColor(dwHov);
		}

		// 系统钮槽必须与 TitleBar 同色；透明时若槽上另有底/脏区，会看成「按钮区」分界
		if( m_pSys != NULL ) {
			m_pSys->SetBackgroundColor(dwBk);
			m_pSys->SetBorderColor(0);
			m_pSys->SetBorderWidth(0);
		}
		if( m_pLeft != NULL ) {
			m_pLeft->SetBackgroundColor(0);
			m_pLeft->SetBorderColor(0);
			m_pLeft->SetBorderWidth(0);
			// 左侧自定义图标钮（如 wallpaper）：跟系统钮同色，避免深色栏上发黑看不见
			for( int i = 0; i < m_pLeft->GetCount(); ++i ) {
				CControlUI* pChild = m_pLeft->GetItemAt(i);
				if( pChild == NULL ) continue;
				if( pChild->GetInterface(DUI_CTR_THEMESWITCHER) != NULL ) continue;
				CButtonUI* pBtn = static_cast<CButtonUI*>(pChild->GetInterface(DUI_CTR_BUTTON));
				if( pBtn == NULL ) continue;
				if( pBtn->GetKind() != CONTROLKIND_NONE ) continue;
				if( !pBtn->HasIcon() ) continue;
				pBtn->SetIconTint(dwIcon);
				pBtn->SetIconTintHover(dwIconHover);
				pBtn->SetBackgroundColor(0);
				pBtn->SetHoverBackgroundColor(dwHoverBk);
				pBtn->SetActiveBackgroundColor(dwHoverBk);
				pBtn->SetBorderColor(0);
				pBtn->SetBorderWidth(0);
			}
		}
	}

	void CTitleBarUI::ApplySysButtonIcons()
	{
		if( m_pMinBtn == NULL || m_pMaxBtn == NULL || m_pRestoreBtn == NULL || m_pCloseBtn == NULL )
			return;

		CDuiString sFace;
		if( ResolveSegoeIconFont(sFace) ) {
			// ChromeMinimize / Maximize / Restore / Close（MDL2 与 Fluent 码点一致）
			m_pMinBtn->SetText(_T("\xE921"));
			m_pMaxBtn->SetText(_T("\xE922"));
			m_pRestoreBtn->SetText(_T("\xE923"));
			m_pCloseBtn->SetText(_T("\xE8BB"));
			CButtonUI* btns[] = { m_pMinBtn, m_pMaxBtn, m_pRestoreBtn, m_pCloseBtn };
			for( int i = 0; i < 4; ++i ) {
				btns[i]->SetFontFamily(sFace.GetData());
				btns[i]->SetFontSize(10);
			}
			return;
		}

		// 无 Segoe 图标字体：普通 Unicode 回退
		m_pMinBtn->SetText(_T("─"));
		m_pMaxBtn->SetText(_T("□"));
		m_pRestoreBtn->SetText(_T("❐"));
		m_pCloseBtn->SetText(_T("✕"));
		m_pMinBtn->SetFontSize(14);
		m_pMaxBtn->SetFontSize(17); // □ 字形留白大
		m_pRestoreBtn->SetFontSize(15);
		m_pCloseBtn->SetFontSize(14);
	}

	void CTitleBarUI::EnsureChrome()
	{
		if( m_bChromeReady ) return;
		m_bChromeReady = true;

		m_pLeft = new CTitleBarLeftUI;
		CHorizontalLayoutUI::Add(m_pLeft);

		m_pTitleLabel = new CLabelUI;
		m_pTitleLabel->SetName(_T("titlebar_title"));
		m_pTitleLabel->SetColor(0xFFFFFFFF);
		m_pTitleLabel->SetFontSize(13);
		m_pTitleLabel->SetAutoCalcWidth(true);
		m_pTitleLabel->SetMouseEnabled(false);
		m_pTitleLabel->SetVisible(false);
		m_pLeft->Add(m_pTitleLabel);

		m_pSys = new CTitleBarSysUI;
		// 初始即与 TitleBar 同色，避免主题套色前短暂露出默认浅底
		m_pSys->SetBackgroundColor(GetBackgroundColor() != 0 ? GetBackgroundColor() : 0x333333FF);
		CHorizontalLayoutUI::Add(m_pSys);

		m_pMinBtn = new CButtonUI;
		m_pMinBtn->SetName(_T("minbtn"));
		m_pMinBtn->SetToolTip(m_bMinimizeToTray ? _T("最小化到托盘") : _T("最小化"));
		ApplySysButtonStyle(m_pMinBtn, false);
		m_pMinBtn->OnNotify += MakeDelegate(this, &CTitleBarUI::OnSysButtonNotify);
		m_pSys->Add(m_pMinBtn);

		m_pMaxBtn = new CButtonUI;
		m_pMaxBtn->SetName(_T("maxbtn"));
		m_pMaxBtn->SetToolTip(_T("最大化"));
		ApplySysButtonStyle(m_pMaxBtn, false);
		m_pMaxBtn->OnNotify += MakeDelegate(this, &CTitleBarUI::OnSysButtonNotify);
		m_pSys->Add(m_pMaxBtn);

		m_pRestoreBtn = new CButtonUI;
		m_pRestoreBtn->SetName(_T("restorebtn"));
		m_pRestoreBtn->SetToolTip(_T("还原"));
		m_pRestoreBtn->SetVisible(false);
		ApplySysButtonStyle(m_pRestoreBtn, false);
		m_pRestoreBtn->OnNotify += MakeDelegate(this, &CTitleBarUI::OnSysButtonNotify);
		m_pSys->Add(m_pRestoreBtn);

		m_pCloseBtn = new CButtonUI;
		m_pCloseBtn->SetName(_T("closebtn"));
		m_pCloseBtn->SetToolTip(m_bCloseToTray ? _T("关闭到托盘") : _T("关闭"));
		ApplySysButtonStyle(m_pCloseBtn, true);
		m_pCloseBtn->OnNotify += MakeDelegate(this, &CTitleBarUI::OnSysButtonNotify);
		m_pSys->Add(m_pCloseBtn);

		ApplySysButtonIcons();
		SyncSysButtonMetrics();
		SyncSysButtonVisibility();
	}

	void CTitleBarUI::SyncSysButtonMetrics()
	{
		// SetFixed* 存逻辑像素；GetFixedHeight() 会再 Scale，此处勿用以免高 DPI 二次放大裁切图标
		const int h = m_cxyFixed.cy > 0 ? m_cxyFixed.cy : 40;
		const int w = m_nBtnWidth > 0 ? m_nBtnWidth : 46;
		CButtonUI* btns[] = { m_pMinBtn, m_pMaxBtn, m_pRestoreBtn, m_pCloseBtn };
		for( int i = 0; i < 4; ++i ) {
			if( btns[i] == NULL ) continue;
			btns[i]->SetFixedWidth(w);
			btns[i]->SetFixedHeight(h);
		}
		if( m_pLeft != NULL ) m_pLeft->SetFixedHeight(h);
		if( m_pSys != NULL ) m_pSys->SetFixedHeight(h);
	}

	void CTitleBarUI::SyncSysButtonVisibility()
	{
		if( m_pMinBtn != NULL ) m_pMinBtn->SetVisible(m_bShowMin);
		if( m_pCloseBtn != NULL ) m_pCloseBtn->SetVisible(m_bShowClose);
		if( !m_bShowMax ) {
			if( m_pMaxBtn != NULL ) m_pMaxBtn->SetVisible(false);
			if( m_pRestoreBtn != NULL ) m_pRestoreBtn->SetVisible(false);
		}
		else {
			// 最大化/还原互斥由 WinImplBase OnSysCommand 按 name 切换；此处仅保证 max 可见起点
			if( m_pMaxBtn != NULL && m_pRestoreBtn != NULL ) {
				if( !m_pMaxBtn->IsVisible() && !m_pRestoreBtn->IsVisible() )
					m_pMaxBtn->SetVisible(true);
			}
			else if( m_pMaxBtn != NULL ) {
				m_pMaxBtn->SetVisible(true);
			}
		}
		NeedUpdate();
	}

	void CTitleBarUI::SetTitle(LPCTSTR pstrText)
	{
		EnsureChrome();
		if( m_pTitleLabel == NULL ) return;
		m_pTitleLabel->SetText(pstrText ? pstrText : _T(""));
		const bool bHas = (pstrText != NULL && *pstrText != _T('\0'));
		m_pTitleLabel->SetVisible(bHas);
		NeedUpdate();
	}

	CDuiString CTitleBarUI::GetTitle() const
	{
		if( m_pTitleLabel == NULL ) return CDuiString();
		return m_pTitleLabel->GetText();
	}

	void CTitleBarUI::SetShowMin(bool bShow)
	{
		if( m_bShowMin == bShow ) return;
		m_bShowMin = bShow;
		EnsureChrome();
		SyncSysButtonVisibility();
	}

	void CTitleBarUI::SetMinimizeToTray(bool bTray)
	{
		if( m_bMinimizeToTray == bTray ) return;
		m_bMinimizeToTray = bTray;
		EnsureChrome();
		if( m_pMinBtn != NULL )
			m_pMinBtn->SetToolTip(bTray ? _T("最小化到托盘") : _T("最小化"));
	}

	void CTitleBarUI::SetCloseToTray(bool bTray)
	{
		if( m_bCloseToTray == bTray ) return;
		m_bCloseToTray = bTray;
		EnsureChrome();
		if( m_pCloseBtn != NULL )
			m_pCloseBtn->SetToolTip(bTray ? _T("关闭到托盘") : _T("关闭"));
	}

	void CTitleBarUI::SetShowMax(bool bShow)
	{
		if( m_bShowMax == bShow ) return;
		m_bShowMax = bShow;
		EnsureChrome();
		SyncSysButtonVisibility();
	}

	void CTitleBarUI::SetShowClose(bool bShow)
	{
		if( m_bShowClose == bShow ) return;
		m_bShowClose = bShow;
		EnsureChrome();
		SyncSysButtonVisibility();
	}

	void CTitleBarUI::SetBtnWidth(int nWidth)
	{
		if( nWidth <= 0 ) nWidth = 46;
		if( m_nBtnWidth == nWidth ) return;
		m_nBtnWidth = nWidth;
		EnsureChrome();
		SyncSysButtonMetrics();
		NeedUpdate();
	}

	void CTitleBarUI::SetFixedHeight(int cy)
	{
		CHorizontalLayoutUI::SetFixedHeight(cy);
		if( m_bChromeReady ) SyncSysButtonMetrics();
	}

	void CTitleBarUI::CancelNotify()
	{
		m_bNotifyCancel = true;
	}

	void CTitleBarUI::ResetNotifyCancel()
	{
		m_bNotifyCancel = false;
	}

	bool CTitleBarUI::QueryAllowNotify(LPCTSTR pstrMsg, WPARAM wParam, LPARAM lParam)
	{
		m_bNotifyCancel = false;
		if( m_pManager != NULL )
			m_pManager->SendNotify(this, pstrMsg, wParam, lParam, false);
		return !m_bNotifyCancel;
	}

	HWND CTitleBarUI::GetOwnerHWND() const
	{
		if( m_pManager == NULL ) return NULL;
		return m_pManager->GetPaintWindow();
	}

	void CTitleBarUI::DoSysMin()
	{
		HWND hWnd = GetOwnerHWND();
		if( hWnd == NULL ) return;
		if( m_bMinimizeToTray ) {
			// 不能只 SW_HIDE：Win10/11 常残留任务栏按钮；走 HideWindowFromTaskbar。
			CTrayIcon::HideWindowFromTaskbar(hWnd);
		}
		else {
			::SendMessage(hWnd, WM_SYSCOMMAND, SC_MINIMIZE, 0);
		}
	}

	void CTitleBarUI::DoSysMax()
	{
		HWND hWnd = GetOwnerHWND();
		if( hWnd == NULL ) return;
		::SendMessage(hWnd, WM_SYSCOMMAND, ::IsZoomed(hWnd) ? SC_RESTORE : SC_MAXIMIZE, 0);
	}

	void CTitleBarUI::DoSysClose()
	{
		HWND hWnd = GetOwnerHWND();
		if( hWnd == NULL ) return;
		if( m_bCloseToTray )
			CTrayIcon::HideWindowFromTaskbar(hWnd);
		else
			::PostMessage(hWnd, WM_CLOSE, 0, 0);
	}

	bool CTitleBarUI::OnSysButtonNotify(void* param)
	{
		TNotifyUI* pMsg = static_cast<TNotifyUI*>(param);
		if( pMsg == NULL || pMsg->sType != DUI_MSGTYPE_CLICK ) return true;
		CControlUI* pSender = pMsg->pSender;
		if( pSender == NULL ) return true;

		if( pSender == m_pMinBtn ) {
			if( !QueryAllowNotify(DUI_MSGTYPE_TITLEBARMINING, 0, 0) ) return true;
			DoSysMin();
			if( m_pManager != NULL )
				m_pManager->SendNotify(this, DUI_MSGTYPE_TITLEBARMIN, 0, 0, false);
			return true;
		}
		if( pSender == m_pMaxBtn || pSender == m_pRestoreBtn ) {
			if( !QueryAllowNotify(DUI_MSGTYPE_TITLEBARMAXING, 0, 0) ) return true;
			DoSysMax();
			if( m_pManager != NULL )
				m_pManager->SendNotify(this, DUI_MSGTYPE_TITLEBARMAX, 0, 0, false);
			return true;
		}
		if( pSender == m_pCloseBtn ) {
			if( !QueryAllowNotify(DUI_MSGTYPE_TITLEBARCLOSING, 0, 0) ) return true;
			DoSysClose();
			if( m_pManager != NULL )
				m_pManager->SendNotify(this, DUI_MSGTYPE_TITLEBARCLOSE, 0, 0, false);
			return true;
		}
		return true;
	}

	bool CTitleBarUI::Add(CControlUI* pControl)
	{
		EnsureChrome();
		if( pControl == NULL ) return false;
		if( pControl == m_pLeft || pControl == m_pSys )
			return CHorizontalLayoutUI::Add(pControl);

		// 全部用户子控件进入左侧
		if( m_pLeft == NULL ) return false;
		bool bOk = m_pLeft->Add(pControl);
		// 子控件（lucide 图标钮）加入后再同步着色
		if( bOk ) SyncSysButtonChrome();
		return bOk;
	}

	bool CTitleBarUI::AddAt(CControlUI* pControl, int iIndex)
	{
		EnsureChrome();
		if( pControl == NULL ) return false;
		if( pControl == m_pLeft || pControl == m_pSys )
			return CHorizontalLayoutUI::AddAt(pControl, iIndex);

		if( m_pLeft == NULL ) return false;
		bool bOk = m_pLeft->AddAt(pControl, iIndex);
		if( bOk ) SyncSysButtonChrome();
		return bOk;
	}

	void CTitleBarUI::SetAttribute(LPCTSTR pstrName, LPCTSTR pstrValue)
	{
		if( _tcsicmp(pstrName, _T("title")) == 0 ) {
			SetTitle(pstrValue);
		}
		else if( _tcsicmp(pstrName, _T("show-min")) == 0 ) {
			SetShowMin(ParseBoolValue(pstrValue));
		}
		else if( _tcsicmp(pstrName, _T("min-to-tray")) == 0
			|| _tcsicmp(pstrName, _T("minimize-to-tray")) == 0 ) {
			SetMinimizeToTray(ParseBoolValue(pstrValue));
		}
		else if( _tcsicmp(pstrName, _T("close-to-tray")) == 0 ) {
			SetCloseToTray(ParseBoolValue(pstrValue));
		}
		else if( _tcsicmp(pstrName, _T("show-max")) == 0 ) {
			SetShowMax(ParseBoolValue(pstrValue));
		}
		else if( _tcsicmp(pstrName, _T("show-close")) == 0 ) {
			SetShowClose(ParseBoolValue(pstrValue));
		}
		else if( _tcsicmp(pstrName, _T("btn-width")) == 0 ) {
			if( pstrValue != NULL ) SetBtnWidth(_ttoi(pstrValue));
		}
		else if( _tcsicmp(pstrName, _T("color")) == 0 || _tcsicmp(pstrName, _T("textcolor")) == 0 ) {
			EnsureChrome();
			DWORD dwColor = 0;
			if( pstrValue != NULL && ParseColorString(pstrValue, dwColor) && m_pTitleLabel != NULL )
				m_pTitleLabel->SetColor(dwColor);
			SyncSysButtonChrome();
		}
		else {
			CHorizontalLayoutUI::SetAttribute(pstrName, pstrValue);
			if( _tcsicmp(pstrName, _T("height")) == 0 && m_bChromeReady )
				SyncSysButtonMetrics();
			if( _tcsicmp(pstrName, _T("background-color")) == 0
				|| _tcsicmp(pstrName, _T("bkcolor")) == 0 ) {
				SyncSysButtonChrome();
			}
		}
	}

	bool CTitleBarUI::IsCaptionDragHit(POINT pt) const
	{
		UIAction a = GetAction();
		if( a != UIACTION_TITLE && a != UIACTION_MOVEWINDOW ) return false;
		if( !::PtInRect(&m_rcItem, pt) ) return false;

		if( m_pSys != NULL && m_pSys->IsVisible() ) {
			RECT rc = m_pSys->GetPos();
			if( ::PtInRect(&rc, pt) ) return false;
		}

		// 左侧交互子控件（ThemeSwitcher / 图标钮 / Edit 等）不参与拖窗。
		// 若仍返回 true → HTCAPTION，无客户区 WM_MOUSE*，tooltip / 悬停会失效。
		if( m_pManager != NULL && m_pLeft != NULL ) {
			CControlUI* pHit = m_pManager->FindControl(pt);
			if( pHit != NULL && pHit->PreferClientHit() ) {
				for( CControlUI* q = pHit; q != NULL; q = q->GetParent() ) {
					if( q == m_pLeft ) {
						return false;
					}
					if( q == this ) break;
				}
			}
		}
		return true;
	}
}
