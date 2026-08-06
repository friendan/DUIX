#include "StdAfx.h"
#include "UIModal.h"

namespace DuiLib {

	namespace {

		enum {
			kModalTitleH = 44,
			kModalBtnRowH = 44,
			kModalBtnW = 72,
			kModalBtnH = 30,
			kModalRound = 12, // 窗体圆角（CSS 半径）
		};

		DWORD KindTitleTextColor(ControlKind kind)
		{
			InitKindColors();
			int idx = (int)kind;
			if( idx < 0 || idx >= 11 ) idx = (int)CONTROLKIND_PRIMARY;
			DWORD c = g_kindColors[idx].Normal.dwColor;
			if( c == 0 ) c = 0xFFFFFFFF;
			return c;
		}

	} // namespace

	/////////////////////////////////////////////////////////////////////////////////////
	// CModalOptions

	CModalOptions::CModalOptions()
		: m_kind(CONTROLKIND_PRIMARY)
		, m_bShowCancel(false)
		, m_sOkText(_T("确定"))
		, m_sCancelText(_T("取消"))
		, m_nWidth(420)
		, m_nHeight(200)
		, m_bClickBackdropToClose(true)
		, m_hOwner(NULL)
		, m_fnOnResult(NULL)
		, m_pResultUser(NULL)
	{
	}

	CModalOptions& CModalOptions::Title(LPCTSTR text) { m_sTitle = text ? text : _T(""); return *this; }
	CModalOptions& CModalOptions::Text(LPCTSTR text) { m_sText = text ? text : _T(""); return *this; }
	CModalOptions& CModalOptions::Kind(ControlKind kind) { m_kind = kind; return *this; }
	CModalOptions& CModalOptions::ShowCancel(bool show) { m_bShowCancel = show; return *this; }
	CModalOptions& CModalOptions::OkText(LPCTSTR text) { m_sOkText = text ? text : _T(""); return *this; }
	CModalOptions& CModalOptions::CancelText(LPCTSTR text) { m_sCancelText = text ? text : _T(""); return *this; }
	CModalOptions& CModalOptions::Width(int w) { m_nWidth = w; return *this; }
	CModalOptions& CModalOptions::Height(int h) { m_nHeight = h; return *this; }
	CModalOptions& CModalOptions::ClickBackdropToClose(bool close) { m_bClickBackdropToClose = close; return *this; }
	CModalOptions& CModalOptions::Owner(HWND hOwner) { m_hOwner = hOwner; return *this; }
	CModalOptions& CModalOptions::OnResult(ModalResultCallback fn, void* pUser)
	{
		m_fnOnResult = fn;
		m_pResultUser = pUser;
		return *this;
	}
	CModalOptions& CModalOptions::UserData(LPCTSTR data) { m_sUserData = data ? data : _T(""); return *this; }

	/////////////////////////////////////////////////////////////////////////////////////
	// forward

	class CModalWnd;

	/////////////////////////////////////////////////////////////////////////////////////
	// CModalBackdropWnd — 半透明全屏遮罩

	class CModalBackdropWnd : public CWindowWnd
	{
	public:
		explicit CModalBackdropWnd(CModalWnd* pOwner)
			: m_pOwner(pOwner)
		{
		}

		LPCTSTR GetWindowClassName() const { return _T("DuiModalBackdropWnd"); }
		UINT GetClassStyle() const { return 0; }
		void OnFinalMessage(HWND /*hWnd*/) { delete this; }

		HWND CreateBackdrop(const RECT& rcWork)
		{
			int w = rcWork.right - rcWork.left;
			int h = rcWork.bottom - rcWork.top;
			// 遮罩不要 TOPMOST，避免盖住对话框的分层 Present
			Create(NULL, _T(""), WS_POPUP,
				WS_EX_TOOLWINDOW | WS_EX_LAYERED | WS_EX_NOACTIVATE,
				rcWork.left, rcWork.top, w, h);
			if( m_hWnd == NULL ) return NULL;
			::SetLayeredWindowAttributes(m_hWnd, 0, 96, LWA_ALPHA);
			return m_hWnd;
		}

		void ShowBackdrop()
		{
			if( m_hWnd ) {
				ShowWindow(true, false);
				::SetWindowPos(m_hWnd, HWND_TOP, 0, 0, 0, 0,
					SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE);
			}
		}

		LRESULT HandleMessage(UINT uMsg, WPARAM wParam, LPARAM lParam);

	private:
		friend class CModalWnd;
		CModalWnd* m_pOwner;
	};

	/////////////////////////////////////////////////////////////////////////////////////
	// CModalWnd

	class CModalWnd : public CWindowWnd, public INotifyUI
	{
	public:
		explicit CModalWnd(const CModalOptions& opts);
		~CModalWnd();

		HWND CreateModal();
		void CloseModal(bool ok);
		void RaiseAboveBackdrop();
		bool IsClickBackdropToClose() const { return m_opts.m_bClickBackdropToClose; }

		LPCTSTR GetWindowClassName() const { return _T("DuiModalWnd"); }
		UINT GetClassStyle() const { return CS_DBLCLKS; }
		void OnFinalMessage(HWND hWnd);
		LRESULT HandleMessage(UINT uMsg, WPARAM wParam, LPARAM lParam);
		void Notify(TNotifyUI& msg);

		static CModalWnd* FromHwnd(HWND h);

		friend class CModalBackdropWnd;

	private:
		LRESULT OnCreate(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
		LRESULT OnSize(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
		LRESULT OnClose(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
		LRESULT OnNcHitTest(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
		LRESULT OnKeyDown(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);

		void BuildUI();
		void PlaceWindow(RECT* pOutWork);
		HWND ResolveOwner() const;
		void EnableOwner(bool enable);
		void DestroyBackdrop();
		void FireResult(bool ok);

		CModalOptions m_opts;
		CPaintManagerUI m_pm;
		CVerticalLayoutUI* m_pRoot;
		CButtonUI* m_pOkBtn;
		CButtonUI* m_pCancelBtn;
		CModalBackdropWnd* m_pBackdrop;
		HWND m_hDisabledOwner;
		bool m_bResult;
		bool m_bClosed;
		bool m_bFired;
	};

	CModalWnd::CModalWnd(const CModalOptions& opts)
		: m_opts(opts)
		, m_pRoot(NULL)
		, m_pOkBtn(NULL)
		, m_pCancelBtn(NULL)
		, m_pBackdrop(NULL)
		, m_hDisabledOwner(NULL)
		, m_bResult(false)
		, m_bClosed(false)
		, m_bFired(false)
	{
		if( m_opts.m_sTitle.IsEmpty() )
			m_opts.m_sTitle = _T("提示");
		if( m_opts.m_kind == CONTROLKIND_NONE )
			m_opts.m_kind = CONTROLKIND_PRIMARY;
		if( m_opts.m_nWidth < 200 ) m_opts.m_nWidth = 200;
		if( m_opts.m_nHeight < 120 ) m_opts.m_nHeight = 120;
		if( m_opts.m_sOkText.IsEmpty() ) m_opts.m_sOkText = _T("确定");
		if( m_opts.m_sCancelText.IsEmpty() ) m_opts.m_sCancelText = _T("取消");
	}

	CModalWnd::~CModalWnd()
	{
		DestroyBackdrop();
		EnableOwner(true);
	}

	HWND CModalWnd::ResolveOwner() const
	{
		if( m_opts.m_hOwner && ::IsWindow(m_opts.m_hOwner) )
			return m_opts.m_hOwner;
		HWND h = ::GetForegroundWindow();
		if( h && ::IsWindow(h) ) return h;
		return ::GetActiveWindow();
	}

	void CModalWnd::EnableOwner(bool enable)
	{
		if( m_hDisabledOwner == NULL ) return;
		HWND h = m_hDisabledOwner;
		if( ::IsWindow(h) ) {
			::EnableWindow(h, enable ? TRUE : FALSE);
			if( enable ) {
				// 关闭后若不抢回前台，主窗可能一直无焦点，按钮点不动
				::SetForegroundWindow(h);
				::SetActiveWindow(h);
				::SetFocus(h);
			}
		}
		if( enable ) m_hDisabledOwner = NULL;
	}

	void CModalWnd::RaiseAboveBackdrop()
	{
		if( m_hWnd == NULL || !::IsWindow(m_hWnd) || m_bClosed ) return;
		// 遮罩与对话框同为 TOPMOST：点遮罩会把遮罩抬到对话框之上，需压回去
		::SetWindowPos(m_hWnd, HWND_TOPMOST, 0, 0, 0, 0,
			SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE);
		::BringWindowToTop(m_hWnd);
		::SetForegroundWindow(m_hWnd);
	}

	void CModalWnd::DestroyBackdrop()
	{
		if( m_pBackdrop == NULL ) return;
		HWND h = m_pBackdrop->GetHWND();
		m_pBackdrop->m_pOwner = NULL;
		m_pBackdrop = NULL;
		if( h && ::IsWindow(h) ) {
			::ShowWindow(h, SW_HIDE);
			// 可能从遮罩自身消息里关闭，异步销毁避免重入 DestroyWindow
			::PostMessage(h, WM_CLOSE, 0, 0);
		}
	}

	void CModalWnd::PlaceWindow(RECT* pOutWork)
	{
		int w = m_opts.m_nWidth;
		int h = m_opts.m_nHeight;

		POINT pt = { 0, 0 };
		::GetCursorPos(&pt);
		HWND hOwner = ResolveOwner();
		HMONITOR hm = NULL;
		if( hOwner && ::IsWindow(hOwner) )
			hm = ::MonitorFromWindow(hOwner, MONITOR_DEFAULTTONEAREST);
		else
			hm = ::MonitorFromPoint(pt, MONITOR_DEFAULTTONEAREST);

		MONITORINFO mi = { sizeof(MONITORINFO) };
		::GetMonitorInfo(hm, &mi);
		RECT rcWork = mi.rcWork;
		if( pOutWork ) *pOutWork = rcWork;

		int aw = rcWork.right - rcWork.left;
		int ah = rcWork.bottom - rcWork.top;
		int x = rcWork.left + (aw - w) / 2;
		int y = rcWork.top + (ah - h) / 3;
		::SetWindowPos(m_hWnd, HWND_TOPMOST, x, y, w, h, SWP_NOACTIVATE);
	}

	HWND CModalWnd::CreateModal()
	{
		int w = m_opts.m_nWidth;
		int h = m_opts.m_nHeight;

		Create(NULL, _T("Modal"), WS_POPUP,
			WS_EX_TOOLWINDOW | WS_EX_TOPMOST,
			0, 0, w, h);
		if( m_hWnd == NULL ) return NULL;

		RECT rcWork = { 0 };
		PlaceWindow(&rcWork);

		// 遮罩：全工作区半透明；点击关闭见 ClickBackdropToClose
		m_pBackdrop = new CModalBackdropWnd(this);
		if( m_pBackdrop->CreateBackdrop(rcWork) == NULL ) {
			delete m_pBackdrop;
			m_pBackdrop = NULL;
		}
		else {
			m_pBackdrop->ShowBackdrop();
		}

		HWND hOwner = ResolveOwner();
		if( hOwner && ::IsWindow(hOwner) ) {
			m_hDisabledOwner = hOwner;
			::EnableWindow(hOwner, FALSE);
		}

		ShowWindow(true, true);
		// 分层窗必须先画出一帧（ULW），再抬到遮罩之上
		::InvalidateRect(m_hWnd, NULL, TRUE);
		::UpdateWindow(m_hWnd);
		RaiseAboveBackdrop();
		return m_hWnd;
	}

	void CModalWnd::FireResult(bool ok)
	{
		if( m_bFired ) return;
		m_bFired = true;
		if( m_opts.m_fnOnResult )
			m_opts.m_fnOnResult(ok, m_opts.m_sUserData.GetData(), m_opts.m_pResultUser);
	}

	void CModalWnd::CloseModal(bool ok)
	{
		if( m_bClosed ) return;
		m_bClosed = true;
		m_bResult = ok;
		FireResult(ok);

		DestroyBackdrop();
		EnableOwner(true);

		if( m_hWnd && ::IsWindow(m_hWnd) ) {
			::ShowWindow(m_hWnd, SW_HIDE);
			::PostMessage(m_hWnd, WM_CLOSE, 0, 0);
		}
	}

	void CModalWnd::BuildUI()
	{
		InitKindColors();

		int w = m_opts.m_nWidth;
		int h = m_opts.m_nHeight;
		DWORD dwTitleFg = KindTitleTextColor(m_opts.m_kind);

		DWORD modalBg = 0xFFFFFFFF;
		DWORD modalTx = 0x3C3C3CFF;
		DWORD modalBd = 0xE6E6E6FF;
		CThemeManager* tm = CThemeManager::GetInstance();
		if( tm != NULL ) {
			CTheme* th = tm->GetCurrentTheme();
			if( th == NULL ) th = tm->FindTheme(tm->GetDefaultThemeId());
			if( th != NULL ) {
				modalBg = th->GetToken(_T("color-modal-bg"), th->GetToken(_T("color-bg"), modalBg));
				modalTx = th->GetToken(_T("color-modal-text"), th->GetToken(_T("color-text"), modalTx));
				modalBd = th->GetToken(_T("color-modal-border"), th->GetToken(_T("color-border"), modalBd));
			}
		}

		m_pm.AddFont(0, _T("Microsoft YaHei UI"), 14, true, false, false, false);
		m_pm.AddFont(1, _T("Microsoft YaHei UI"), 13, false, false, false, false);

		CVerticalLayoutUI* pRoot = new CVerticalLayoutUI;
		m_pRoot = pRoot;
		pRoot->SetName(_T("modalRoot"));
		pRoot->SetFixedWidth(w);
		pRoot->SetFixedHeight(h);
		pRoot->SetBackgroundColor(modalBg);
		// 分层 + BorderRadius：D2D 抗锯齿圆角；角外透明（勿 SetWindowRgn）
		SIZE szRound = { kModalRound, kModalRound };
		pRoot->SetBorderRadius(szRound);

		// 标题栏
		CHorizontalLayoutUI* pTitle = new CHorizontalLayoutUI;
		pTitle->SetName(_T("modalTitleBar"));
		pTitle->SetFixedHeight(kModalTitleH);
		pTitle->SetKind(m_opts.m_kind);
		// SetKind 会带上按钮级 BorderRadius(12)，标题四角被裁掉会露出底下灰/白块；
		// 直角填充，由根节点圆角裁剪即可。
		SIZE szTitleFlat = { 0, 0 };
		pTitle->SetBorderRadius(szTitleFlat);
		pTitle->SetBorderWidth(0);
		pTitle->SetAlignItems(DT_VCENTER);
		pTitle->SetPadding(CDuiBox(0, 8, 0, 16));
		pTitle->SetAttribute(_T("action"), _T("title"));

		CLabelUI* pTitleLabel = new CLabelUI;
		pTitleLabel->SetName(_T("modalTitle"));
		pTitleLabel->SetText(m_opts.m_sTitle.GetData());
		pTitleLabel->SetFont(0);
		pTitleLabel->SetColor(dwTitleFg);
		pTitleLabel->SetAttribute(_T("text-align"), _T("left"));
		pTitleLabel->SetAttribute(_T("vertical-align"), _T("vcenter"));
		pTitleLabel->SetAttribute(_T("action"), _T("title"));
		pTitleLabel->SetMouseEnabled(false);
		pTitle->Add(pTitleLabel);
		pRoot->Add(pTitle);

		// 正文
		CVerticalLayoutUI* pBody = new CVerticalLayoutUI;
		pBody->SetName(_T("modalBody"));
		pBody->SetPadding(CDuiBox(20, 16, 12, 16));
		pBody->SetAttribute(_T("action"), _T("title"));

		CLabelUI* pText = new CLabelUI;
		pText->SetName(_T("modalText"));
		pText->SetText(m_opts.m_sText.GetData());
		pText->SetFont(1);
		pText->SetColor(modalTx);
		pText->SetAttribute(_T("text-align"), _T("left"));
		pText->SetAttribute(_T("vertical-align"), _T("top"));
		pText->SetAttribute(_T("text-overflow"), _T("clip"));
		pText->SetAttribute(_T("word-break"), _T("break-word"));
		pText->SetMouseEnabled(false);
		pBody->Add(pText);
		pRoot->Add(pBody);

		// 按钮行
		CHorizontalLayoutUI* pBtnRow = new CHorizontalLayoutUI;
		pBtnRow->SetName(_T("modalBtnRow"));
		pBtnRow->SetFixedHeight(kModalBtnRowH);
		pBtnRow->SetAlignItems(DT_VCENTER);
		pBtnRow->SetGap(8);
		pBtnRow->SetPadding(CDuiBox(0, 12, 0, 0));
		pBtnRow->SetBackgroundColor(modalBg);

		CControlUI* pSep = new CControlUI;
		pSep->SetFixedHeight(1);
		pSep->SetBackgroundColor(modalBd);
		pRoot->Add(pSep);

		CControlUI* pSpacer = new CControlUI;
		pSpacer->SetMouseEnabled(false);
		pBtnRow->Add(pSpacer);

		if( m_opts.m_bShowCancel ) {
			CButtonUI* pCancel = new CButtonUI;
			m_pCancelBtn = pCancel;
			pCancel->SetName(_T("modalCancelBtn"));
			pCancel->SetText(m_opts.m_sCancelText.GetData());
			pCancel->SetFixedWidth(kModalBtnW);
			pCancel->SetFixedHeight(kModalBtnH);
			pCancel->SetKind(CONTROLKIND_SECONDARY);
			SIZE szBtnRound = { 2, 2 };
			pCancel->SetBorderRadius(szBtnRound);
			pBtnRow->Add(pCancel);
		}

		CButtonUI* pOk = new CButtonUI;
		m_pOkBtn = pOk;
		pOk->SetName(_T("modalOkBtn"));
		pOk->SetText(m_opts.m_sOkText.GetData());
		pOk->SetFixedWidth(kModalBtnW);
		pOk->SetFixedHeight(kModalBtnH);
		pOk->SetKind(m_opts.m_kind);
		SIZE szOkRound = { 2, 2 };
		pOk->SetBorderRadius(szOkRound);
		pBtnRow->Add(pOk);

		pRoot->Add(pBtnRow);

		m_pm.AttachDialog(pRoot);
		m_pm.AddNotifier(this);
	}

	CModalWnd* CModalWnd::FromHwnd(HWND h)
	{
		if( h == NULL || !::IsWindow(h) ) return NULL;
		TCHAR szClass[64] = { 0 };
		::GetClassName(h, szClass, 63);
		if( _tcscmp(szClass, _T("DuiModalWnd")) != 0 ) return NULL;
		CWindowWnd* pWnd = reinterpret_cast<CWindowWnd*>(::GetWindowLongPtr(h, GWLP_USERDATA));
		return static_cast<CModalWnd*>(pWnd);
	}

	void CModalWnd::OnFinalMessage(HWND /*hWnd*/)
	{
		DestroyBackdrop();
		EnableOwner(true);
		delete this;
	}

	void CModalWnd::Notify(TNotifyUI& msg)
	{
		if( msg.sType != DUI_MSGTYPE_CLICK ) return;
		if( msg.pSender == m_pOkBtn ) {
			CloseModal(true);
			return;
		}
		if( msg.pSender == m_pCancelBtn ) {
			CloseModal(false);
			return;
		}
	}

	LRESULT CModalWnd::OnCreate(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& bHandled)
	{
		LONG styleValue = ::GetWindowLong(*this, GWL_STYLE);
		styleValue &= ~WS_CAPTION;
		::SetWindowLong(*this, GWL_STYLE, styleValue | WS_CLIPSIBLINGS | WS_CLIPCHILDREN);

		m_pm.Init(m_hWnd);
		m_pm.SetLayered(true);
		DWORD winBg = 0xFFFFFFFF;
		CThemeManager* tm = CThemeManager::GetInstance();
		if( tm != NULL ) {
			CTheme* th = tm->GetCurrentTheme();
			if( th == NULL ) th = tm->FindTheme(tm->GetDefaultThemeId());
			if( th != NULL )
				winBg = th->GetToken(_T("color-modal-bg"), th->GetToken(_T("color-bg"), winBg));
		}
		m_pm.SetWindowBackgroundColor(winBg);
		// 圆角跟阴影：不设 SetWindowRgn（分层 AA），靠 RoundCorner 让 CShadowUI 画圆角阴影
		m_pm.SetBorderRadius(kModalRound, kModalRound);
		BuildUI();
		bHandled = FALSE;
		return 0;
	}

	LRESULT CModalWnd::OnSize(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& bHandled)
	{
		bHandled = FALSE;
		return 0;
	}

	LRESULT CModalWnd::OnClose(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& bHandled)
	{
		if( !m_bClosed )
			CloseModal(false);
		bHandled = FALSE;
		return 0;
	}

	LRESULT CModalWnd::OnNcHitTest(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM lParam, BOOL& bHandled)
	{
		POINT pt;
		pt.x = GET_X_LPARAM(lParam);
		pt.y = GET_Y_LPARAM(lParam);
		::ScreenToClient(m_hWnd, &pt);

		CControlUI* pHitCtrl = m_pm.FindControl(pt);
		if( pHitCtrl != NULL ) {
			if( pHitCtrl->IsCaptionDragHit(pt) )
				return HTCAPTION;
			UIAction leafAct = pHitCtrl->GetAction();
			if( leafAct == UIACTION_NONE && !pHitCtrl->PreferClientHit() ) {
				for( CControlUI* pWalk = pHitCtrl->GetParent(); pWalk != NULL; pWalk = pWalk->GetParent() ) {
					if( pWalk->IsCaptionDragHit(pt) )
						return HTCAPTION;
					UIAction parentAct = pWalk->GetAction();
					if( parentAct == UIACTION_TITLE || parentAct == UIACTION_MOVEWINDOW )
						break;
					if( parentAct != UIACTION_NONE ) break;
				}
			}
		}
		bHandled = FALSE;
		return HTCLIENT;
	}

	LRESULT CModalWnd::OnKeyDown(UINT /*uMsg*/, WPARAM wParam, LPARAM /*lParam*/, BOOL& bHandled)
	{
		if( wParam == VK_ESCAPE ) {
			CloseModal(false);
			return 0;
		}
		if( wParam == VK_RETURN ) {
			CloseModal(true);
			return 0;
		}
		bHandled = FALSE;
		return 0;
	}

	LRESULT CModalWnd::HandleMessage(UINT uMsg, WPARAM wParam, LPARAM lParam)
	{
		LRESULT lRes = 0;
		BOOL bHandled = TRUE;
		switch( uMsg ) {
		case WM_CREATE:
			lRes = OnCreate(uMsg, wParam, lParam, bHandled);
			break;
		case WM_SIZE:
			lRes = OnSize(uMsg, wParam, lParam, bHandled);
			break;
		case WM_CLOSE:
			lRes = OnClose(uMsg, wParam, lParam, bHandled);
			break;
		case WM_NCHITTEST:
			lRes = OnNcHitTest(uMsg, wParam, lParam, bHandled);
			break;
		case WM_KEYDOWN:
			lRes = OnKeyDown(uMsg, wParam, lParam, bHandled);
			break;
		default:
			bHandled = FALSE;
			break;
		}

		if( m_pm.MessageHandler(uMsg, wParam, lParam, lRes) ) return lRes;
		if( bHandled ) return lRes;
		return CWindowWnd::HandleMessage(uMsg, wParam, lParam);
	}

	/////////////////////////////////////////////////////////////////////////////////////
	// CModalBackdropWnd

	LRESULT CModalBackdropWnd::HandleMessage(UINT uMsg, WPARAM wParam, LPARAM lParam)
	{
		// 勿激活遮罩，否则会抢到 TOPMOST 栈顶盖住对话框
		if( uMsg == WM_MOUSEACTIVATE )
			return MA_NOACTIVATE;

		if( uMsg == WM_LBUTTONDOWN || uMsg == WM_RBUTTONDOWN ) {
			if( m_pOwner == NULL ) return 0;
			if( m_pOwner->IsClickBackdropToClose() )
				m_pOwner->CloseModal(false);
			else
				m_pOwner->RaiseAboveBackdrop();
			return 0;
		}
		if( uMsg == WM_CLOSE ) {
			::DestroyWindow(m_hWnd);
			return 0;
		}
		if( uMsg == WM_SETCURSOR ) {
			::SetCursor(::LoadCursor(NULL, IDC_ARROW));
			return TRUE;
		}
		if( uMsg == WM_PAINT ) {
			PAINTSTRUCT ps;
			HDC hdc = ::BeginPaint(m_hWnd, &ps);
			RECT rc;
			::GetClientRect(m_hWnd, &rc);
			HBRUSH br = ::CreateSolidBrush(RGB(0, 0, 0));
			::FillRect(hdc, &rc, br);
			::DeleteObject(br);
			::EndPaint(m_hWnd, &ps);
			return 0;
		}
		if( uMsg == WM_ERASEBKGND )
			return 1;
		return CWindowWnd::HandleMessage(uMsg, wParam, lParam);
	}

	/////////////////////////////////////////////////////////////////////////////////////
	// CModal

	HWND CModal::Show(LPCTSTR text, const CModalOptions& opts)
	{
		CModalOptions o = opts;
		if( text && *text ) o.Text(text);
		CModalWnd* pWnd = new CModalWnd(o);
		HWND h = pWnd->CreateModal();
		if( h == NULL ) {
			delete pWnd;
			return NULL;
		}
		return h;
	}

	HWND CModal::Show(LPCTSTR title, LPCTSTR text, const CModalOptions& opts)
	{
		CModalOptions o = opts;
		if( title && *title ) o.Title(title);
		if( text && *text ) o.Text(text);
		CModalWnd* pWnd = new CModalWnd(o);
		HWND h = pWnd->CreateModal();
		if( h == NULL ) {
			delete pWnd;
			return NULL;
		}
		return h;
	}

	HWND CModal::ShowSuccess(LPCTSTR text, ModalResultCallback fn, void* pUser)
	{
		return Show(_T("提示"), text,
			CModalOptions().Kind(CONTROLKIND_SUCCESS).OnResult(fn, pUser));
	}

	HWND CModal::ShowDanger(LPCTSTR text, ModalResultCallback fn, void* pUser)
	{
		return Show(_T("提示"), text,
			CModalOptions().Kind(CONTROLKIND_DANGER).OnResult(fn, pUser));
	}

	HWND CModal::ShowWarning(LPCTSTR text, ModalResultCallback fn, void* pUser)
	{
		return Show(_T("提示"), text,
			CModalOptions().Kind(CONTROLKIND_WARNING).OnResult(fn, pUser));
	}

	HWND CModal::ShowInfo(LPCTSTR text, ModalResultCallback fn, void* pUser)
	{
		return Show(_T("提示"), text,
			CModalOptions().Kind(CONTROLKIND_INFO).OnResult(fn, pUser));
	}

	HWND CModal::Confirm(LPCTSTR title, LPCTSTR text, ModalResultCallback fn, void* pUser)
	{
		return Show(title, text,
			CModalOptions()
				.Kind(CONTROLKIND_PRIMARY)
				.ShowCancel(true)
				.OnResult(fn, pUser));
	}

	void CModal::Dismiss(HWND hModal)
	{
		CModalWnd* p = CModalWnd::FromHwnd(hModal);
		if( p ) p->CloseModal(false);
		else if( hModal && ::IsWindow(hModal) )
			::PostMessage(hModal, WM_CLOSE, 0, 0);
	}

} // namespace DuiLib
