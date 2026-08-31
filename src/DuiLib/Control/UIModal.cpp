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
		, m_bSyncOwnerMove(true)
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
	CModalOptions& CModalOptions::SyncOwnerMove(bool sync) { m_bSyncOwnerMove = sync; return *this; }
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

		void Relayout(const RECT& rcWork)
		{
			if( m_hWnd == NULL || !::IsWindow(m_hWnd) ) return;
			int w = rcWork.right - rcWork.left;
			int h = rcWork.bottom - rcWork.top;
			if( w < 1 ) w = 1;
			if( h < 1 ) h = 1;
			::SetWindowPos(m_hWnd, HWND_TOP, rcWork.left, rcWork.top, w, h, SWP_NOACTIVATE);
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
		LRESULT OnWindowPosChanged(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);

		void BuildUI();
		void PlaceWindow(RECT* pOutWork);
		HWND ResolveOwner() const;
		void EnableOwner(bool enable);
		void CaptureOwnerSyncOffset();
		void SyncOwnerPosition();
		void UpdateBackdropMonitor(bool bForce = false);
		void DestroyBackdrop();
		void FireResult(bool ok);

		CModalOptions m_opts;
		CPaintManagerUI m_pm;
		CVerticalLayoutUI* m_pRoot;
		CButtonUI* m_pOkBtn;
		CButtonUI* m_pCancelBtn;
		CModalBackdropWnd* m_pBackdrop;
		HMONITOR m_hBackdropMonitor;
		HWND m_hDisabledOwner;
		POINT m_ptOwnerOffset;
		bool m_bHaveOwnerOffset;
		bool m_bSyncingOwner;
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
		, m_hBackdropMonitor(NULL)
		, m_hDisabledOwner(NULL)
		, m_bHaveOwnerOffset(false)
		, m_bSyncingOwner(false)
		, m_bResult(false)
		, m_bClosed(false)
		, m_bFired(false)
	{
		m_ptOwnerOffset.x = m_ptOwnerOffset.y = 0;
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
		if( enable ) {
			m_hDisabledOwner = NULL;
			m_bHaveOwnerOffset = false;
		}
	}

	void CModalWnd::CaptureOwnerSyncOffset()
	{
		m_bHaveOwnerOffset = false;
		m_ptOwnerOffset.x = m_ptOwnerOffset.y = 0;
		if( !m_opts.m_bSyncOwnerMove ) return;
		if( m_hWnd == NULL || m_hDisabledOwner == NULL ) return;
		if( !::IsWindow(m_hWnd) || !::IsWindow(m_hDisabledOwner) ) return;
		RECT rcModal = { 0 }, rcOwner = { 0 };
		if( !::GetWindowRect(m_hWnd, &rcModal) ) return;
		if( !::GetWindowRect(m_hDisabledOwner, &rcOwner) ) return;
		m_ptOwnerOffset.x = rcOwner.left - rcModal.left;
		m_ptOwnerOffset.y = rcOwner.top - rcModal.top;
		m_bHaveOwnerOffset = true;
	}

	void CModalWnd::SyncOwnerPosition()
	{
		if( !m_opts.m_bSyncOwnerMove || !m_bHaveOwnerOffset || m_bSyncingOwner ) return;
		if( m_hWnd == NULL || m_hDisabledOwner == NULL ) return;
		if( !::IsWindow(m_hWnd) || !::IsWindow(m_hDisabledOwner) ) return;
		if( ::IsZoomed(m_hDisabledOwner) || ::IsIconic(m_hDisabledOwner) ) return;

		// 屏幕坐标（含负坐标副屏）；偏移用打开时物理像素差，跨 DPI 显示器随窗一起平移
		RECT rcModal = { 0 };
		if( !::GetWindowRect(m_hWnd, &rcModal) ) return;
		const int x = rcModal.left + m_ptOwnerOffset.x;
		const int y = rcModal.top + m_ptOwnerOffset.y;

		RECT rcOwner = { 0 };
		if( !::GetWindowRect(m_hDisabledOwner, &rcOwner) ) return;
		if( rcOwner.left == x && rcOwner.top == y ) return;

		m_bSyncingOwner = true;
		::SetWindowPos(m_hDisabledOwner, NULL, x, y, 0, 0,
			SWP_NOSIZE | SWP_NOZORDER | SWP_NOACTIVATE);
		m_bSyncingOwner = false;
	}

	void CModalWnd::UpdateBackdropMonitor(bool bForce)
	{
		if( m_pBackdrop == NULL || m_pBackdrop->GetHWND() == NULL ) return;
		if( m_hWnd == NULL || !::IsWindow(m_hWnd) ) return;

		// 以对话框中心判定所在屏，跨屏拖动时比仅靠窗口角更稳
		RECT rcModal = { 0 };
		if( !::GetWindowRect(m_hWnd, &rcModal) ) return;
		POINT ptCenter = {
			(rcModal.left + rcModal.right) / 2,
			(rcModal.top + rcModal.bottom) / 2
		};
		HMONITOR hm = ::MonitorFromPoint(ptCenter, MONITOR_DEFAULTTONEAREST);
		if( hm == NULL )
			hm = ::MonitorFromWindow(m_hWnd, MONITOR_DEFAULTTONEAREST);
		if( hm == NULL ) return;
		if( !bForce && hm == m_hBackdropMonitor ) return;

		MONITORINFO mi = { sizeof(MONITORINFO) };
		if( !::GetMonitorInfo(hm, &mi) ) return;
		m_hBackdropMonitor = hm;
		m_pBackdrop->Relayout(mi.rcWork);
		RaiseAboveBackdrop();
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
		m_hBackdropMonitor = NULL;
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

		int x = 0, y = 0;
		RECT rcOwner = { 0 };
		if( hOwner && ::IsWindow(hOwner) && ::GetWindowRect(hOwner, &rcOwner) ) {
			// 相对主窗（Owner）居中
			x = rcOwner.left + ((rcOwner.right - rcOwner.left) - w) / 2;
			y = rcOwner.top + ((rcOwner.bottom - rcOwner.top) - h) / 2;
		}
		else {
			int aw = rcWork.right - rcWork.left;
			int ah = rcWork.bottom - rcWork.top;
			x = rcWork.left + (aw - w) / 2;
			y = rcWork.top + (ah - h) / 2;
		}

		// 夹在工作区内，避免主窗靠边时对话框出屏
		if( x + w > rcWork.right ) x = rcWork.right - w;
		if( y + h > rcWork.bottom ) y = rcWork.bottom - h;
		if( x < rcWork.left ) x = rcWork.left;
		if( y < rcWork.top ) y = rcWork.top;

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
		m_hBackdropMonitor = ::MonitorFromWindow(m_hWnd, MONITOR_DEFAULTTONEAREST);

		HWND hOwner = ResolveOwner();
		if( hOwner && ::IsWindow(hOwner) ) {
			m_hDisabledOwner = hOwner;
			::EnableWindow(hOwner, FALSE);
		}
		CaptureOwnerSyncOffset();

		ShowWindow(true, true);
		// 分层窗必须先画出一帧（ULW），再抬到遮罩之上
		::InvalidateRect(m_hWnd, NULL, TRUE);
		::UpdateWindow(m_hWnd);
		RaiseAboveBackdrop();
		// 再整窗提交一帧，确保 D2D AA 圆角已经 ULW 呈现（避免首帧残留锯齿）
		m_pm.Invalidate();
		::UpdateWindow(m_hWnd);
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
		// 根节点勿铺不透明底：标题若带圆角（或裁剪不一致）会在顶角落出白底。
		// 可见底色交给正文/按钮行；角外靠 Clear + RoundClip 保持透明。
		pRoot->SetBackgroundColor(0);
		SIZE szRound = { kModalRound, kModalRound };
		pRoot->SetBorderRadius(szRound);

		// 标题栏：手动画 kind 色，不用 SetKind（会强制 BorderRadius=6，小于根圆角时顶角露白）
		CHorizontalLayoutUI* pTitle = new CHorizontalLayoutUI;
		pTitle->SetName(_T("modalTitleBar"));
		pTitle->SetFixedHeight(kModalTitleH);
		{
			int idx = (int)m_opts.m_kind;
			if( idx < 0 || idx >= 11 ) idx = (int)CONTROLKIND_PRIMARY;
			DWORD titleBg = g_kindColors[idx].Normal.dwBackgroundColor;
			if( titleBg == 0 ) titleBg = 0x0D6EFDFF;
			pTitle->SetBackgroundColor(titleBg);
		}
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
		pBody->SetBackgroundColor(modalBg);
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

		// AttachDialog→ApplyToManager 可能改写底色；分层圆角必须保持透明底
		m_pm.SetWindowBackgroundColor(0);
		pRoot->SetBackgroundColor(0);
		pBody->SetBackgroundColor(modalBg);
		pBtnRow->SetBackgroundColor(modalBg);

		// 再次压平标题圆角，并恢复按钮小圆角
		{
			SIZE flat = { 0, 0 };
			SIZE btnR = { 2, 2 };
			pTitle->SetBorderRadius(flat);
			pTitle->SetBorderWidth(0);
			if( m_pOkBtn ) m_pOkBtn->SetBorderRadius(btnR);
			if( m_pCancelBtn ) m_pCancelBtn->SetBorderRadius(btnR);
		}
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
		// 弹窗禁用 DComp：走 BitmapRT + ULW（与圆角遮罩 Present 一致）
		m_pm.SetLayeredCompositionEnabled(false);
		m_pm.SetWindowBackgroundColor(0);
		m_pm.SetBorderRadius(kModalRound, kModalRound);
		BuildUI();
		bHandled = FALSE;
		return 0;
	}

	LRESULT CModalWnd::OnSize(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& bHandled)
	{
		// 分层圆角靠 D2D RoundClip 抗锯齿；勿 SetWindowRgn（GDI RGN 有锯齿）
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
				return CPaintManagerUI::HitTestCaptionDrag(true);
			UIAction leafAct = pHitCtrl->GetAction();
			if( leafAct == UIACTION_NONE && !pHitCtrl->PreferClientHit() ) {
				for( CControlUI* pWalk = pHitCtrl->GetParent(); pWalk != NULL; pWalk = pWalk->GetParent() ) {
					if( pWalk->IsCaptionDragHit(pt) )
						return CPaintManagerUI::HitTestCaptionDrag(true);
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

	LRESULT CModalWnd::OnWindowPosChanged(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM lParam, BOOL& bHandled)
	{
		const WINDOWPOS* pwp = reinterpret_cast<const WINDOWPOS*>(lParam);
		if( pwp != NULL && (pwp->flags & SWP_NOMOVE) == 0 ) {
			SyncOwnerPosition();
			UpdateBackdropMonitor(false);
		}
		bHandled = FALSE;
		return 0;
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
		case WM_WINDOWPOSCHANGED:
			lRes = OnWindowPosChanged(uMsg, wParam, lParam, bHandled);
			break;
		case WM_DISPLAYCHANGE:
			UpdateBackdropMonitor(true);
			bHandled = FALSE;
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
