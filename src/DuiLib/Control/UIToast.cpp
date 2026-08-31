#include "StdAfx.h"
#include "UIToast.h"
#include <vector>
#include <algorithm>
#include <commctrl.h>

namespace DuiLib {

	namespace {

		enum {
			WM_TOAST_TICK = WM_APP + 0x54A0,
			kToastTickMs = 200,
			kToastSingleHeight = 44,
			kToastDualHeight = 68,
			kToastIconSize = 20,
			kToastOwnerSubclassId = 0x544F4153, // 'TOAS'
		};

		// 线程池定时器回调；只 PostMessage 回 UI 线程（不依赖 HWND WM_TIMER）
		VOID CALLBACK ToastQueueTimerProc(PVOID lpParameter, BOOLEAN /*TimerOrWaitFired*/)
		{
			HWND hWnd = reinterpret_cast<HWND>(lpParameter);
			if( hWnd != NULL && ::IsWindow(hWnd) )
				::PostMessage(hWnd, WM_TOAST_TICK, 0, 0);
		}

		std::vector<HWND> g_activeToasts;
		int g_nMaxCount = 0;

		struct OwnerTrackEntry {
			HWND hwnd;
			int refs;
		};
		std::vector<OwnerTrackEntry> g_ownerTracks;

		void RelayoutActiveToasts();
		void AddOwnerTrack(HWND hOwner);
		void ReleaseOwnerTrack(HWND hOwner);

		void CleanupClosedToasts()
		{
			for( auto it = g_activeToasts.begin(); it != g_activeToasts.end(); ) {
				if( !::IsWindow(*it) ) it = g_activeToasts.erase(it);
				else ++it;
			}
		}

		void UnregisterToast(HWND hWnd)
		{
			auto it = std::find(g_activeToasts.begin(), g_activeToasts.end(), hWnd);
			if( it == g_activeToasts.end() ) return;
			g_activeToasts.erase(it);
			RelayoutActiveToasts();
		}

		DWORD KindFgColor(ControlKind kind)
		{
			InitKindColors();
			int idx = (int)kind;
			if( idx < 0 || idx >= 11 ) idx = (int)CONTROLKIND_INFO;
			DWORD c = g_kindColors[idx].Normal.dwColor;
			if( c == 0 ) c = 0x212529FF;
			return c;
		}

		LPCTSTR KindFilledIcon(ControlKind kind)
		{
			switch( kind ) {
			case CONTROLKIND_SUCCESS: return _T("circle-check");
			case CONTROLKIND_DANGER:  return _T("circle-x");
			case CONTROLKIND_WARNING: return _T("alert-triangle");
			case CONTROLKIND_PRIMARY: return _T("circle-check");
			case CONTROLKIND_INFO:
			default:                  return _T("info-circle");
			}
		}

	} // namespace

	/////////////////////////////////////////////////////////////////////////////////////
	// CToastOptions

	CToastOptions::CToastOptions()
		: m_kind(CONTROLKIND_INFO)
		, m_nDuration(4000)
		, m_bShowClose(true)
		, m_bShowIcon(true)
		, m_align(ToastAlign_ScreenBottomRight)
		, m_hOwner(NULL)
		, m_nMinWidth(350)
		, m_nMaxWidth(600)
		, m_nGap(16)
		, m_nHeight(0)
		, m_bPauseOnHover(true)
		, m_fnOnClick(NULL)
		, m_pClickUser(NULL)
		, m_fnOnDismiss(NULL)
		, m_pDismissUser(NULL)
		, m_bClickDismiss(true)
	{
	}

	CToastOptions& CToastOptions::Title(LPCTSTR text) { m_sTitle = text ? text : _T(""); return *this; }
	CToastOptions& CToastOptions::Text(LPCTSTR text) { m_sText = text ? text : _T(""); return *this; }
	CToastOptions& CToastOptions::Kind(ControlKind kind) { m_kind = kind; return *this; }
	CToastOptions& CToastOptions::Duration(int ms) { m_nDuration = ms; return *this; }
	CToastOptions& CToastOptions::ShowClose(bool show) { m_bShowClose = show; return *this; }
	CToastOptions& CToastOptions::ShowIcon(bool show) { m_bShowIcon = show; return *this; }
	CToastOptions& CToastOptions::Icon(LPCTSTR lib, LPCTSTR name)
	{
		m_sIconLib = lib ? lib : _T("");
		m_sIconName = name ? name : _T("");
		return *this;
	}
	CToastOptions& CToastOptions::Align(ToastAlign align) { m_align = align; return *this; }
	CToastOptions& CToastOptions::Owner(HWND hOwner) { m_hOwner = hOwner; return *this; }
	CToastOptions& CToastOptions::MinWidth(int w) { m_nMinWidth = w; return *this; }
	CToastOptions& CToastOptions::MaxWidth(int w) { m_nMaxWidth = w; return *this; }
	CToastOptions& CToastOptions::Gap(int gap) { m_nGap = gap; return *this; }
	CToastOptions& CToastOptions::Height(int h) { m_nHeight = h; return *this; }
	CToastOptions& CToastOptions::PauseOnHover(bool pause) { m_bPauseOnHover = pause; return *this; }
	CToastOptions& CToastOptions::OnClick(ToastClickCallback fn, void* pUser)
	{
		m_fnOnClick = fn;
		m_pClickUser = pUser;
		return *this;
	}
	CToastOptions& CToastOptions::OnDismiss(ToastDismissCallback fn, void* pUser)
	{
		m_fnOnDismiss = fn;
		m_pDismissUser = pUser;
		return *this;
	}
	CToastOptions& CToastOptions::UserData(LPCTSTR data) { m_sUserData = data ? data : _T(""); return *this; }
	CToastOptions& CToastOptions::ClickDismiss(bool dismiss) { m_bClickDismiss = dismiss; return *this; }

	/////////////////////////////////////////////////////////////////////////////////////
	// CToastWnd

	class CToastWnd : public CWindowWnd, public INotifyUI
	{
	public:
		CToastWnd(const CToastOptions& opts);
		~CToastWnd();

		HWND CreateToast();
		void Dismiss(ToastDismissReason reason = ToastDismiss_Manual, bool postClose = true);
		static void RelayoutAll();

		LPCTSTR GetWindowClassName() const { return _T("DuiToastWnd"); }
		UINT GetClassStyle() const { return CS_DBLCLKS; }
		void OnFinalMessage(HWND hWnd);
		LRESULT HandleMessage(UINT uMsg, WPARAM wParam, LPARAM lParam);
		void Notify(TNotifyUI& msg);

	private:
		LRESULT OnCreate(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
		LRESULT OnSize(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
		LRESULT OnClose(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
		LRESULT OnNcHitTest(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);

		void BuildUI();
		void PlaceWindow();
		void EnforceGroupMaxCount();
		void ApplyStackOffset(int stackOffset);
		void NoteUserMoved();
		bool IsSameStackGroup(const CToastWnd* other) const;
		bool IsWindowAlign() const;
		void AttachOwnerTrack();
		void DetachOwnerTrack();
		static CToastWnd* FromHwnd(HWND h);
		void StartTimer();
		void StopTimer();
		void SetPaused(bool paused);
		void UpdateTimerLabel();
		void OnTimerTick();
		bool IsCursorOverWindow() const;
		void FireClick();
		void FireDismiss(ToastDismissReason reason);
		void SetupClickableLabel(CLabelUI* pLabel);
		static ToastDismissReason NormalizeReason(WPARAM wParam);

		int MeasureTextWidth(LPCTSTR text, int iFont) const;
		int MeasureTextHeight(LPCTSTR text, int iFont, int nWidth) const;
		int ChromeWidth() const;
		void ResolveLayoutSize();
		int ResolveWidth() const;
		int ResolveHeight() const;
		bool HasDualLine() const;
		HWND ResolveOwner() const;

		CToastOptions m_opts;
		CPaintManagerUI m_pm;
		CHorizontalLayoutUI* m_pRoot;
		CLabelUI* m_pTimerLabel;
		CButtonUI* m_pCloseBtn;
		HWND m_hTrackedOwner;
		RECT m_rcStackedPos;
		int m_nLayoutW;
		int m_nLayoutH;
		int m_nRemaining;
		DWORD m_dwExpireTick;
		DWORD m_dwPauseStart;
		HANDLE m_hQueueTimer;
		bool m_bDismissed;
		bool m_bTimerActive;
		bool m_bPaused;
		bool m_bMouseTracking;
		bool m_bClickFired;
		bool m_bDismissFired;
		bool m_bDetachedFromStack;
		bool m_bApplyingStack;
	};

	CToastWnd::CToastWnd(const CToastOptions& opts)
		: m_opts(opts)
		, m_pRoot(NULL)
		, m_pTimerLabel(NULL)
		, m_pCloseBtn(NULL)
		, m_hTrackedOwner(NULL)
		, m_nLayoutW(0)
		, m_nLayoutH(0)
		, m_nRemaining(0)
		, m_dwExpireTick(0)
		, m_dwPauseStart(0)
		, m_hQueueTimer(NULL)
		, m_bDismissed(false)
		, m_bTimerActive(false)
		, m_bPaused(false)
		, m_bMouseTracking(false)
		, m_bClickFired(false)
		, m_bDismissFired(false)
		, m_bDetachedFromStack(false)
		, m_bApplyingStack(false)
	{
		::ZeroMemory(&m_rcStackedPos, sizeof(m_rcStackedPos));
		// 仅标题：当作单行正文
		if( m_opts.m_sText.IsEmpty() && !m_opts.m_sTitle.IsEmpty() ) {
			m_opts.m_sText = m_opts.m_sTitle;
			m_opts.m_sTitle.Empty();
		}
		if( m_opts.m_kind == CONTROLKIND_NONE )
			m_opts.m_kind = CONTROLKIND_INFO;
	}

	CToastWnd::~CToastWnd()
	{
		DetachOwnerTrack();
		StopTimer();
	}

	HWND CToastWnd::ResolveOwner() const
	{
		if( m_opts.m_hOwner && ::IsWindow(m_opts.m_hOwner) )
			return m_opts.m_hOwner;
		HWND h = ::GetForegroundWindow();
		if( h && ::IsWindow(h) ) return h;
		return ::GetActiveWindow();
	}

	int CToastWnd::MeasureTextWidth(LPCTSTR text, int iFont) const
	{
		if( text == NULL || *text == _T('\0') ) return 0;
		SIZE sz = RenderMeasureTextSize(const_cast<CPaintManagerUI*>(&m_pm), text, iFont,
			DT_SINGLELINE | DT_CALCRECT | DT_LEFT | DT_TOP);
		return sz.cx;
	}

	int CToastWnd::MeasureTextHeight(LPCTSTR text, int iFont, int nWidth) const
	{
		if( text == NULL || *text == _T('\0') || nWidth <= 0 ) return 0;
		RECT rc = { 0, 0, nWidth, 9999 };
		RenderMeasureText(const_cast<CPaintManagerUI*>(&m_pm), rc, text, 0, iFont,
			DT_CALCRECT | DT_WORDBREAK | DT_EDITCONTROL | DT_LEFT | DT_TOP);
		int h = rc.bottom - rc.top;
		return h > 0 ? h : 0;
	}

	int CToastWnd::ChromeWidth() const
	{
		const int insetL = 14;
		const int insetR = 8;
		const int pad = 10;
		int chrome = insetL + insetR;
		int nChildren = 1; // textCol
		if( m_opts.m_bShowIcon ) {
			chrome += kToastIconSize;
			++nChildren;
		}
		if( m_opts.m_nDuration > 0 ) {
			chrome += 30;
			++nChildren;
		}
		if( m_opts.m_bShowClose ) {
			chrome += 22;
			++nChildren;
		}
		if( nChildren > 1 )
			chrome += pad * (nChildren - 1);
		return chrome;
	}

	void CToastWnd::ResolveLayoutSize()
	{
		int minW = m_opts.m_nMinWidth > 0 ? m_opts.m_nMinWidth : 350;
		if( minW < 200 ) minW = 200;
		int maxW = m_opts.m_nMaxWidth > 0 ? m_opts.m_nMaxWidth : 600;
		if( maxW < minW ) maxW = minW;

		int chrome = ChromeWidth();
		bool bDual = HasDualLine();
		int needTextW = 0;
		if( bDual ) {
			int tw = MeasureTextWidth(m_opts.m_sTitle.GetData(), 0);
			int bw = MeasureTextWidth(m_opts.m_sText.GetData(), 1);
			needTextW = tw > bw ? tw : bw;
		}
		else {
			needTextW = MeasureTextWidth(m_opts.m_sText.GetData(), 0);
		}

		int w = chrome + needTextW;
		if( w < minW ) w = minW;
		if( w > maxW ) w = maxW;

		int textAreaW = w - chrome;
		if( textAreaW < 40 ) textAreaW = 40;

		int h = 0;
		if( m_opts.m_nHeight > 0 ) {
			h = m_opts.m_nHeight;
		}
		else if( needTextW <= textAreaW ) {
			h = bDual ? kToastDualHeight : kToastSingleHeight;
		}
		else if( bDual ) {
			const int titleH = 20;
			int bodyH = MeasureTextHeight(m_opts.m_sText.GetData(), 1, textAreaW);
			if( bodyH < 18 ) bodyH = 18;
			int textColH = titleH + 2 + bodyH;
			h = textColH + 16;
			if( h < kToastDualHeight ) h = kToastDualHeight;
		}
		else {
			int bodyH = MeasureTextHeight(m_opts.m_sText.GetData(), 0, textAreaW);
			if( bodyH < kToastIconSize ) bodyH = kToastIconSize;
			h = bodyH + 16;
			if( h < kToastSingleHeight ) h = kToastSingleHeight;
		}

		m_nLayoutW = w;
		m_nLayoutH = h;
	}

	int CToastWnd::ResolveWidth() const
	{
		if( m_nLayoutW > 0 ) return m_nLayoutW;
		int w = m_opts.m_nMinWidth;
		if( w < 200 ) w = 200;
		if( m_opts.m_nMaxWidth > 0 && w > m_opts.m_nMaxWidth )
			w = m_opts.m_nMaxWidth;
		return w;
	}

	bool CToastWnd::HasDualLine() const
	{
		return !m_opts.m_sTitle.IsEmpty() && !m_opts.m_sText.IsEmpty();
	}

	int CToastWnd::ResolveHeight() const
	{
		if( m_nLayoutH > 0 ) return m_nLayoutH;
		if( m_opts.m_nHeight > 0 ) return m_opts.m_nHeight;
		return HasDualLine() ? kToastDualHeight : kToastSingleHeight;
	}

	HWND CToastWnd::CreateToast()
	{
		int w = ResolveWidth();
		int h = ResolveHeight();
		HWND hRef = ResolveOwner();

		// 不要把主窗口设为 Owner：owned popup 会触发父窗 WM_NCACTIVATE /
		// 局部脏区重绘，D2D 下圆角按钮容易出现“裂开”，悬停才恢复。
		Create(NULL, _T("Toast"), WS_POPUP,
			WS_EX_TOOLWINDOW | WS_EX_TOPMOST | WS_EX_NOACTIVATE,
			0, 0, w, h);

		if( m_hWnd == NULL ) return NULL;

		PlaceWindow();
		ShowWindow(true, false);
		StartTimer();

		if( hRef && ::IsWindow(hRef) ) {
			// 勿同步 RedrawWindow：itemmoved 等 notify 路径中会重入绘制（AppGrid 拖拽还原竞态）
			::InvalidateRect(hRef, NULL, FALSE);
		}
		return m_hWnd;
	}

	void CToastWnd::BuildUI()
	{
		InitKindColors();

		DWORD dwFg = KindFgColor(m_opts.m_kind);
		DWORD dwMuted = DuiColorSetA(dwFg, 0xB3);
		bool bDual = HasDualLine();

		m_pm.AddFont(0, _T("Microsoft YaHei UI"), 13, true, false, false, false);
		m_pm.AddFont(1, _T("Microsoft YaHei UI"), 12, false, false, false, false);

		ResolveLayoutSize();
		int w = ResolveWidth();
		int h = ResolveHeight();
		int chrome = ChromeWidth();
		int textAreaW = w - chrome;
		if( textAreaW < 40 ) textAreaW = 40;

		bool bWrap = false;
		if( bDual )
			bWrap = MeasureTextWidth(m_opts.m_sText.GetData(), 1) > textAreaW;
		else
			bWrap = MeasureTextWidth(m_opts.m_sText.GetData(), 0) > textAreaW;

		CHorizontalLayoutUI* pRoot = new CHorizontalLayoutUI;
		m_pRoot = pRoot;
		pRoot->SetName(_T("toastRoot"));
		pRoot->SetFixedWidth(w);
		pRoot->SetFixedHeight(h);
		pRoot->SetKind(m_opts.m_kind);
		pRoot->SetPadding(CDuiBox(0, 8, 0, 14));
		pRoot->SetGap(10);
		pRoot->SetAlignItems(DT_VCENTER);
		SIZE szRound = { 8, 8 };
		pRoot->SetBorderRadius(szRound);
		pRoot->SetAttribute(_T("action"), _T("title"));

		if( m_opts.m_bShowIcon ) {
			CSvgBoxUI* pIcon = new CSvgBoxUI;
			pIcon->SetName(_T("toastIcon"));
			pIcon->SetFixedWidth(kToastIconSize);
			pIcon->SetFixedHeight(kToastIconSize);
			if( !m_opts.m_sIconLib.IsEmpty() && !m_opts.m_sIconName.IsEmpty() ) {
				pIcon->SetAttribute(m_opts.m_sIconLib.GetData(), m_opts.m_sIconName.GetData());
				// twemoji 多色，不套 kind 前景色
				if( m_opts.m_sIconLib.CompareNoCase(_T("twicon")) != 0 )
					pIcon->SetColor(dwFg);
			}
			else {
				pIcon->SetAttribute(_T("tabler-filled"), KindFilledIcon(m_opts.m_kind));
				pIcon->SetColor(dwFg);
			}
			pIcon->SetMouseEnabled(false);
			pRoot->Add(pIcon);
		}

		CVerticalLayoutUI* pTextCol = new CVerticalLayoutUI;
		pTextCol->SetName(_T("toastTextCol"));
		pTextCol->SetGap(bDual ? 2 : 0);
		pTextCol->SetMouseEnabled(false);

		if( bDual ) {
			const int titleH = 20;
			int bodyH = bWrap
				? MeasureTextHeight(m_opts.m_sText.GetData(), 1, textAreaW)
				: 18;
			if( bodyH < 18 ) bodyH = 18;
			pTextCol->SetFixedHeight(titleH + 2 + bodyH);

			CLabelUI* pTitle = new CLabelUI;
			pTitle->SetName(_T("toastTitle"));
			pTitle->SetText(m_opts.m_sTitle.GetData());
			pTitle->SetColor(dwFg);
			pTitle->SetFont(0);
			pTitle->SetFixedHeight(titleH);
			pTitle->SetAttribute(_T("text-align"), _T("left"));
			pTitle->SetAttribute(_T("vertical-align"), _T("vcenter"));
			SetupClickableLabel(pTitle);
			pTextCol->Add(pTitle);

			CLabelUI* pBody = new CLabelUI;
			pBody->SetName(_T("toastBody"));
			pBody->SetText(m_opts.m_sText.GetData());
			pBody->SetColor(dwMuted);
			pBody->SetFont(1);
			pBody->SetFixedHeight(bodyH);
			pBody->SetAttribute(_T("text-align"), _T("left"));
			if( bWrap ) {
				pBody->SetAttribute(_T("word-break"), _T("break-word"));
			}
			else {
				pBody->SetAttribute(_T("vertical-align"), _T("vcenter"));
			}
			SetupClickableLabel(pBody);
			pTextCol->Add(pBody);
		}
		else {
			int bodyH = bWrap
				? MeasureTextHeight(m_opts.m_sText.GetData(), 0, textAreaW)
				: kToastIconSize;
			if( bodyH < kToastIconSize ) bodyH = kToastIconSize;
			pTextCol->SetFixedHeight(bodyH);

			CLabelUI* pTitle = new CLabelUI;
			pTitle->SetName(_T("toastTitle"));
			pTitle->SetText(m_opts.m_sText.GetData());
			pTitle->SetColor(dwFg);
			pTitle->SetFont(0);
			pTitle->SetFixedHeight(bodyH);
			pTitle->SetAttribute(_T("text-align"), _T("left"));
			if( bWrap ) {
				pTitle->SetAttribute(_T("word-break"), _T("break-word"));
			}
			else {
				pTitle->SetAttribute(_T("vertical-align"), _T("vcenter"));
			}
			SetupClickableLabel(pTitle);
			pTextCol->Add(pTitle);
		}
		pRoot->Add(pTextCol);

		m_pTimerLabel = new CLabelUI;
		m_pTimerLabel->SetName(_T("toastTimer"));
		m_pTimerLabel->SetFixedWidth(30);
		m_pTimerLabel->SetColor(dwMuted);
		m_pTimerLabel->SetFont(1);
		m_pTimerLabel->SetAttribute(_T("text-align"), _T("center"));
		m_pTimerLabel->SetAttribute(_T("vertical-align"), _T("vcenter"));
		m_pTimerLabel->SetMouseEnabled(false);
		m_pTimerLabel->SetVisible(m_opts.m_nDuration > 0);
		pRoot->Add(m_pTimerLabel);

		if( m_opts.m_bShowClose ) {
			m_pCloseBtn = new CButtonUI;
			m_pCloseBtn->SetName(_T("toastCloseBtn"));
			m_pCloseBtn->SetKind(CONTROLKIND_NONE);
			m_pCloseBtn->SetText(_T("\x00D7"));
			m_pCloseBtn->SetFixedWidth(22);
			m_pCloseBtn->SetFixedHeight(22);
			m_pCloseBtn->SetBackgroundColor(0);
			m_pCloseBtn->SetBorderWidth(0);
			m_pCloseBtn->SetColor(dwFg);
			m_pCloseBtn->SetHoverBackgroundColor(DuiColorSetA(dwFg, 0x40));
			m_pCloseBtn->SetHoverColor(dwFg);
			m_pCloseBtn->SetActiveBackgroundColor(DuiColorSetA(dwFg, 0x60));
			m_pCloseBtn->SetActiveColor(dwFg);
			m_pCloseBtn->SetAttribute(_T("text-align"), _T("center"));
			m_pCloseBtn->SetAttribute(_T("vertical-align"), _T("vcenter"));
			SIZE szBtnRound = { 4, 4 };
			m_pCloseBtn->SetBorderRadius(szBtnRound);
			pRoot->Add(m_pCloseBtn);
		}

		// 默认关阴影：AttachDialog→Shadow::Create 会 SetWindowLongPtr 子类化宿主，
		// 在部分环境下会干扰本窗定时器投递；Toast 用圆角 RGN 即可。
		m_pm.GetShadow()->ShowShadow(false);
		m_pm.AttachDialog(pRoot);
		// AttachDialog→ApplyToManager 会写窗口底；kind 根已在 SetWindowBackgroundColor 中跳过，
		// 这里再 SetKind 一次，避免以后路径改动再次盖掉底色。
		pRoot->SetKind(m_opts.m_kind);
		pRoot->SetBorderRadius(szRound);
		m_pm.AddNotifier(this);
		m_pm.SetBorderRadius(szRound.cx, szRound.cy);
		ResizeClient(w, h);
	}

	void CToastWnd::PlaceWindow()
	{
		CleanupClosedToasts();
		EnforceGroupMaxCount();
		g_activeToasts.push_back(m_hWnd);
		RelayoutAll();
		AttachOwnerTrack();
	}

	void CToastWnd::EnforceGroupMaxCount()
	{
		if( g_nMaxCount <= 0 ) return;

		for( ;; ) {
			int nGroup = 0;
			HWND hOldest = NULL;
			for( size_t i = 0; i < g_activeToasts.size(); ++i ) {
				CToastWnd* q = FromHwnd(g_activeToasts[i]);
				if( q == NULL || q->m_bDismissed ) continue;
				if( !IsSameStackGroup(q) ) continue;
				if( hOldest == NULL )
					hOldest = g_activeToasts[i];
				++nGroup;
			}
			// 即将再入队 1 条：组内已有 >= Max 则顶掉该组最旧
			if( nGroup < g_nMaxCount )
				break;
			if( hOldest == NULL )
				break;

			auto it = std::find(g_activeToasts.begin(), g_activeToasts.end(), hOldest);
			if( it != g_activeToasts.end() )
				g_activeToasts.erase(it);
			if( ::IsWindow(hOldest) ) {
				::ShowWindow(hOldest, SW_HIDE);
				::PostMessage(hOldest, WM_CLOSE, (WPARAM)ToastDismiss_Evicted, 0);
			}
		}
	}

	bool CToastWnd::IsWindowAlign() const
	{
		return (int)m_opts.m_align > (int)ToastAlign_ScreenCenter;
	}

	void CToastWnd::AttachOwnerTrack()
	{
		DetachOwnerTrack();
		if( !IsWindowAlign() ) return;
		HWND h = ResolveOwner();
		if( h == NULL || !::IsWindow(h) ) return;
		m_hTrackedOwner = h;
		AddOwnerTrack(h);
	}

	void CToastWnd::DetachOwnerTrack()
	{
		if( m_hTrackedOwner == NULL ) return;
		ReleaseOwnerTrack(m_hTrackedOwner);
		m_hTrackedOwner = NULL;
	}

	CToastWnd* CToastWnd::FromHwnd(HWND h)
	{
		if( h == NULL || !::IsWindow(h) ) return NULL;
		TCHAR szClass[64] = { 0 };
		::GetClassName(h, szClass, 63);
		if( _tcscmp(szClass, _T("DuiToastWnd")) != 0 ) return NULL;
		CWindowWnd* pWnd = reinterpret_cast<CWindowWnd*>(::GetWindowLongPtr(h, GWLP_USERDATA));
		return static_cast<CToastWnd*>(pWnd);
	}

	bool CToastWnd::IsSameStackGroup(const CToastWnd* other) const
	{
		if( other == NULL ) return false;
		if( m_opts.m_align != other->m_opts.m_align ) return false;
		if( (int)m_opts.m_align > (int)ToastAlign_ScreenCenter )
			return ResolveOwner() == other->ResolveOwner();
		return true;
	}

	void CToastWnd::ApplyStackOffset(int stackOffset)
	{
		if( m_hWnd == NULL || !::IsWindow(m_hWnd) || m_bDismissed ) return;
		if( m_bDetachedFromStack ) return;

		int w = ResolveWidth();
		int h = ResolveHeight();
		int gap = m_opts.m_nGap > 0 ? m_opts.m_nGap : 16;
		ToastAlign align = m_opts.m_align;
		bool isScreen = ((int)align <= (int)ToastAlign_ScreenCenter);
		int x = 0, y = 0;

		if( isScreen ) {
			POINT pt = { 0, 0 };
			::GetCursorPos(&pt);
			HMONITOR hm = ::MonitorFromPoint(pt, MONITOR_DEFAULTTONEAREST);
			MONITORINFO mi = { sizeof(MONITORINFO) };
			::GetMonitorInfo(hm, &mi);
			int aw = mi.rcWork.right - mi.rcWork.left;
			int ah = mi.rcWork.bottom - mi.rcWork.top;
			int ox = mi.rcWork.left;
			int oy = mi.rcWork.top;

			switch( align ) {
			case ToastAlign_ScreenTopLeft:
				x = ox + gap; y = oy + gap + stackOffset; break;
			case ToastAlign_ScreenTopCenter:
				x = ox + (aw - w) / 2; y = oy + gap + stackOffset; break;
			case ToastAlign_ScreenTopRight:
				x = ox + aw - w - gap; y = oy + gap + stackOffset; break;
			case ToastAlign_ScreenBottomLeft:
				x = ox + gap; y = oy + ah - gap - stackOffset - h; break;
			case ToastAlign_ScreenBottomCenter:
				x = ox + (aw - w) / 2; y = oy + ah - gap - stackOffset - h; break;
			case ToastAlign_ScreenBottomRight:
				x = ox + aw - w - gap; y = oy + ah - gap - stackOffset - h; break;
			case ToastAlign_ScreenCenter:
			default:
				x = ox + (aw - w) / 2; y = oy + (ah - h) / 2; break;
			}
		}
		else {
			HWND owner = ResolveOwner();
			RECT r = { 0, 0, 800, 600 };
			if( owner && ::IsWindow(owner) )
				::GetWindowRect(owner, &r);
			int aw = r.right - r.left;
			int ah = r.bottom - r.top;

			switch( align ) {
			case ToastAlign_WindowTopLeft:
				x = r.left + gap; y = r.top + gap + stackOffset; break;
			case ToastAlign_WindowTopCenter:
				x = r.left + (aw - w) / 2; y = r.top + gap + stackOffset; break;
			case ToastAlign_WindowTopRight:
				x = r.right - w - gap; y = r.top + gap + stackOffset; break;
			case ToastAlign_WindowBottomLeft:
				x = r.left + gap; y = r.bottom - gap - stackOffset - h; break;
			case ToastAlign_WindowBottomCenter:
				x = r.left + (aw - w) / 2; y = r.bottom - gap - stackOffset - h; break;
			case ToastAlign_WindowBottomRight:
				x = r.right - w - gap; y = r.bottom - gap - stackOffset - h; break;
			case ToastAlign_WindowCenter:
			default:
				x = r.left + (aw - w) / 2; y = r.top + (ah - h) / 2; break;
			}
		}

		m_bApplyingStack = true;
		::SetWindowPos(m_hWnd, HWND_TOPMOST, x, y, w, h, SWP_NOACTIVATE);
		::GetWindowRect(m_hWnd, &m_rcStackedPos);
		m_bApplyingStack = false;
	}

	void CToastWnd::NoteUserMoved()
	{
		if( m_bDismissed || m_bDetachedFromStack || m_bApplyingStack ) return;
		if( m_hWnd == NULL || !::IsWindow(m_hWnd) ) return;

		RECT rc = { 0 };
		::GetWindowRect(m_hWnd, &rc);
		const int kSlop = 2;
		if( abs(rc.left - m_rcStackedPos.left) > kSlop ||
			abs(rc.top - m_rcStackedPos.top) > kSlop )
			m_bDetachedFromStack = true;
	}

	void CToastWnd::RelayoutAll()
	{
		CleanupClosedToasts();
		const int stackGap = 8;
		for( size_t i = 0; i < g_activeToasts.size(); ++i ) {
			CToastWnd* p = FromHwnd(g_activeToasts[i]);
			if( p == NULL || p->m_bDismissed || p->m_bDetachedFromStack ) continue;

			int offset = 0;
			for( size_t j = 0; j < i; ++j ) {
				CToastWnd* q = FromHwnd(g_activeToasts[j]);
				if( q == NULL || q->m_bDismissed || q->m_bDetachedFromStack ) continue;
				if( !p->IsSameStackGroup(q) ) continue;
				offset += q->ResolveHeight() + stackGap;
			}
			p->ApplyStackOffset(offset);
		}
	}

	namespace {
		void RelayoutActiveToasts()
		{
			CToastWnd::RelayoutAll();
		}

		LRESULT CALLBACK ToastOwnerSubclassProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam,
			UINT_PTR /*uIdSubclass*/, DWORD_PTR /*dwRefData*/)
		{
			if( uMsg == WM_WINDOWPOSCHANGED ) {
				WINDOWPOS* wp = reinterpret_cast<WINDOWPOS*>(lParam);
				if( wp == NULL || (wp->flags & SWP_NOMOVE) == 0 || (wp->flags & SWP_NOSIZE) == 0 )
					RelayoutActiveToasts();
			}
			else if( uMsg == WM_NCDESTROY ) {
				for( auto it = g_ownerTracks.begin(); it != g_ownerTracks.end(); ++it ) {
					if( it->hwnd == hWnd ) {
						g_ownerTracks.erase(it);
						break;
					}
				}
				::RemoveWindowSubclass(hWnd, ToastOwnerSubclassProc, kToastOwnerSubclassId);
			}
			return ::DefSubclassProc(hWnd, uMsg, wParam, lParam);
		}

		void AddOwnerTrack(HWND hOwner)
		{
			if( hOwner == NULL || !::IsWindow(hOwner) ) return;
			for( size_t i = 0; i < g_ownerTracks.size(); ++i ) {
				if( g_ownerTracks[i].hwnd == hOwner ) {
					g_ownerTracks[i].refs++;
					return;
				}
			}
			if( ::SetWindowSubclass(hOwner, ToastOwnerSubclassProc, kToastOwnerSubclassId, 0) ) {
				OwnerTrackEntry e = { hOwner, 1 };
				g_ownerTracks.push_back(e);
			}
		}

		void ReleaseOwnerTrack(HWND hOwner)
		{
			if( hOwner == NULL ) return;
			for( auto it = g_ownerTracks.begin(); it != g_ownerTracks.end(); ++it ) {
				if( it->hwnd != hOwner ) continue;
				it->refs--;
				if( it->refs <= 0 ) {
					if( ::IsWindow(hOwner) )
						::RemoveWindowSubclass(hOwner, ToastOwnerSubclassProc, kToastOwnerSubclassId);
					g_ownerTracks.erase(it);
				}
				return;
			}
		}
	}

	void CToastWnd::StartTimer()
	{
		StopTimer();
		m_nRemaining = m_opts.m_nDuration;
		m_bPaused = false;
		m_bMouseTracking = false;
		m_dwPauseStart = 0;
		if( m_nRemaining <= 0 ) {
			if( m_pTimerLabel ) m_pTimerLabel->SetVisible(false);
			return;
		}
		m_dwExpireTick = ::GetTickCount() + (DWORD)m_nRemaining;
		UpdateTimerLabel();
		// CreateTimerQueueTimer → PostMessage：与宿主 WM_TIMER / Shadow 子类化解耦
		if( m_hWnd != NULL && ::IsWindow(m_hWnd) ) {
			HANDLE hTimer = NULL;
			if( ::CreateTimerQueueTimer(&hTimer, NULL, ToastQueueTimerProc,
				reinterpret_cast<PVOID>(m_hWnd),
				kToastTickMs, kToastTickMs, WT_EXECUTEDEFAULT) ) {
				m_hQueueTimer = hTimer;
				m_bTimerActive = true;
			}
		}
	}

	void CToastWnd::StopTimer()
	{
		if( m_hQueueTimer != NULL ) {
			// INVALID_HANDLE_VALUE：等回调退出后再删，避免 PostMessage 踩已毁窗
			::DeleteTimerQueueTimer(NULL, m_hQueueTimer, INVALID_HANDLE_VALUE);
			m_hQueueTimer = NULL;
		}
		m_bTimerActive = false;
		m_bPaused = false;
		m_bMouseTracking = false;
	}

	void CToastWnd::SetPaused(bool paused)
	{
		if( !m_opts.m_bPauseOnHover || m_opts.m_nDuration <= 0 ) return;
		if( m_bPaused == paused ) return;
		DWORD now = ::GetTickCount();
		if( paused ) {
			m_bPaused = true;
			m_dwPauseStart = now;
		}
		else {
			if( m_bPaused && m_dwPauseStart != 0 )
				m_dwExpireTick += (now - m_dwPauseStart);
			m_bPaused = false;
			m_dwPauseStart = 0;
			OnTimerTick();
			return;
		}
		UpdateTimerLabel();
		m_pm.Invalidate();
	}

	void CToastWnd::UpdateTimerLabel()
	{
		if( !m_pTimerLabel ) return;
		int sec = (m_nRemaining + 999) / 1000; // 向上取整：2.0s→2，1.01s→2，1.0s→1
		if( sec <= 0 ) {
			m_pTimerLabel->SetVisible(false);
			return;
		}
		m_pTimerLabel->SetVisible(true);
		TCHAR buf[16];
		_stprintf_s(buf, 16, _T("%ds"), sec);
		m_pTimerLabel->SetText(buf);
		m_pTimerLabel->Invalidate();
	}

	void CToastWnd::OnTimerTick()
	{
		if( m_bDismissed || !m_bTimerActive ) return;

		DWORD now = ::GetTickCount();
		if( m_opts.m_bPauseOnHover ) {
			const bool over = IsCursorOverWindow();
			if( over ) {
				if( !m_bPaused ) {
					m_bPaused = true;
					m_dwPauseStart = now;
				}
				return;
			}
			if( m_bPaused ) {
				if( m_dwPauseStart != 0 )
					m_dwExpireTick += (now - m_dwPauseStart);
				m_bPaused = false;
				m_dwPauseStart = 0;
				m_bMouseTracking = false;
			}
		}

		int left = (int)(m_dwExpireTick - now);
		if( left <= 0 ) {
			m_nRemaining = 0;
			Dismiss(ToastDismiss_Timeout);
			return;
		}
		m_nRemaining = left;
		UpdateTimerLabel();
		m_pm.Invalidate();
	}

	bool CToastWnd::IsCursorOverWindow() const
	{
		if( m_hWnd == NULL || !::IsWindow(m_hWnd) ) return false;
		POINT pt = { 0, 0 };
		if( !::GetCursorPos(&pt) ) return false;
		RECT rc = { 0 };
		::GetWindowRect(m_hWnd, &rc);
		return ::PtInRect(&rc, pt) != FALSE;
	}

	void CToastWnd::SetupClickableLabel(CLabelUI* pLabel)
	{
		if( pLabel == NULL ) return;
		if( m_opts.m_fnOnClick != NULL ) {
			// clickable + SETCURSOR → HTCLIENT，不继承根上 action=title，点文字跳详情
			pLabel->SetClickable(true);
			pLabel->SetMouseEnabled(true);
			pLabel->SetCursor(DUI_HAND);
		}
		else {
			pLabel->SetMouseEnabled(false);
		}
	}

	void CToastWnd::FireClick()
	{
		if( m_bDismissed || m_bClickFired ) return;
		if( m_opts.m_fnOnClick == NULL ) return;
		m_bClickFired = true;

		ToastClickCallback fn = m_opts.m_fnOnClick;
		void* pUser = m_opts.m_pClickUser;
		CDuiString sData = m_opts.m_sUserData;
		HWND hToast = m_hWnd;
		bool bDismiss = m_opts.m_bClickDismiss;

		fn(hToast, sData.GetData(), pUser);

		if( bDismiss && ::IsWindow(hToast) )
			Dismiss(ToastDismiss_Manual);
	}

	ToastDismissReason CToastWnd::NormalizeReason(WPARAM wParam)
	{
		int r = (int)wParam;
		if( r == (int)ToastDismiss_Timeout ||
			r == (int)ToastDismiss_Manual ||
			r == (int)ToastDismiss_Evicted )
			return (ToastDismissReason)r;
		return ToastDismiss_Manual;
	}

	void CToastWnd::FireDismiss(ToastDismissReason reason)
	{
		if( m_bDismissFired ) return;
		m_bDismissFired = true;
		if( m_opts.m_fnOnDismiss == NULL ) return;

		ToastDismissCallback fn = m_opts.m_fnOnDismiss;
		void* pUser = m_opts.m_pDismissUser;
		CDuiString sData = m_opts.m_sUserData;
		HWND hToast = m_hWnd;
		fn(hToast, reason, sData.GetData(), pUser);
	}

	void CToastWnd::Dismiss(ToastDismissReason reason, bool postClose)
	{
		if( m_bDismissed ) return;
		m_bDismissed = true;
		FireDismiss(reason);
		StopTimer();
		DetachOwnerTrack();
		UnregisterToast(m_hWnd);
		if( m_hWnd && ::IsWindow(m_hWnd) ) {
			::ShowWindow(m_hWnd, SW_HIDE);
			if( postClose )
				::PostMessage(m_hWnd, WM_CLOSE, 0, 0);
		}
	}

	void CToastWnd::OnFinalMessage(HWND /*hWnd*/)
	{
		DetachOwnerTrack();
		UnregisterToast(m_hWnd);
		delete this;
	}

	void CToastWnd::Notify(TNotifyUI& msg)
	{
		if( msg.sType != DUI_MSGTYPE_CLICK ) return;
		if( msg.pSender == m_pCloseBtn ) {
			Dismiss(ToastDismiss_Manual);
			return;
		}
		if( m_opts.m_fnOnClick != NULL )
			FireClick();
	}

	LRESULT CToastWnd::OnCreate(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& bHandled)
	{
		LONG styleValue = ::GetWindowLong(*this, GWL_STYLE);
		styleValue &= ~WS_CAPTION;
		::SetWindowLong(*this, GWL_STYLE, styleValue | WS_CLIPSIBLINGS | WS_CLIPCHILDREN);

		m_pm.Init(m_hWnd);
		BuildUI();
		bHandled = FALSE;
		return 0;
	}

	LRESULT CToastWnd::OnSize(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& bHandled)
	{
		SIZE szBorderRadius = m_pm.GetBorderRadius();
		if( !::IsIconic(*this) && (szBorderRadius.cx > 0 || szBorderRadius.cy > 0) ) {
			CDuiRect rcWnd;
			::GetWindowRect(*this, &rcWnd);
			rcWnd.Offset(-rcWnd.left, -rcWnd.top);
			rcWnd.right++; rcWnd.bottom++;
			SIZE szEllipse = CssRadiusToEllipse(szBorderRadius);
			HRGN hRgn = ::CreateRoundRectRgn(rcWnd.left, rcWnd.top, rcWnd.right, rcWnd.bottom,
				szEllipse.cx, szEllipse.cy);
			::SetWindowRgn(*this, hRgn, TRUE);
			::DeleteObject(hRgn);
		}
		bHandled = FALSE;
		return 0;
	}

	LRESULT CToastWnd::OnClose(UINT /*uMsg*/, WPARAM wParam, LPARAM /*lParam*/, BOOL& bHandled)
	{
		if( !m_bDismissed ) {
			// CToast::Dismiss / MaxCount 顶掉：经 WM_CLOSE wParam 带原因
			Dismiss(NormalizeReason(wParam), false);
		}
		bHandled = FALSE;
		return 0;
	}

	LRESULT CToastWnd::OnNcHitTest(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM lParam, BOOL& bHandled)
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
			// 叶子无 action 时向上找；PreferClientHit（热态/SETCURSOR 等）不继承父级拖拽
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

	LRESULT CToastWnd::HandleMessage(UINT uMsg, WPARAM wParam, LPARAM lParam)
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
		case WM_TOAST_TICK:
			OnTimerTick();
			return 0;
		case WM_CLOSE:
			lRes = OnClose(uMsg, wParam, lParam, bHandled);
			break;
		case WM_NCHITTEST:
			lRes = OnNcHitTest(uMsg, wParam, lParam, bHandled);
			break;
		case WM_WINDOWPOSCHANGED:
			if( !m_bApplyingStack ) {
				WINDOWPOS* wp = reinterpret_cast<WINDOWPOS*>(lParam);
				if( wp == NULL || (wp->flags & SWP_NOMOVE) == 0 )
					NoteUserMoved();
			}
			bHandled = FALSE;
			break;
		case WM_EXITSIZEMOVE:
			NoteUserMoved();
			bHandled = FALSE;
			break;
		case WM_KILLFOCUS:
			return 0;
		case WM_MOUSEMOVE:
		case WM_NCMOUSEMOVE:
			if( m_opts.m_bPauseOnHover && m_opts.m_nDuration > 0 && !m_bMouseTracking ) {
				TRACKMOUSEEVENT tme;
				::ZeroMemory(&tme, sizeof(tme));
				tme.cbSize = sizeof(tme);
				tme.dwFlags = TME_LEAVE | TME_NONCLIENT;
				tme.hwndTrack = m_hWnd;
				::TrackMouseEvent(&tme);
				m_bMouseTracking = true;
			}
			bHandled = FALSE;
			break;
		case WM_MOUSELEAVE:
		case WM_NCMOUSELEAVE:
			m_bMouseTracking = false;
			bHandled = FALSE;
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
	// CToast

	HWND CToast::Show(LPCTSTR text, const CToastOptions& opts)
	{
		CToastOptions o = opts;
		if( text && *text ) o.Text(text);
		CToastWnd* pWnd = new CToastWnd(o);
		HWND h = pWnd->CreateToast();
		if( h == NULL ) {
			delete pWnd;
			return NULL;
		}
		return h;
	}

	HWND CToast::Show(LPCTSTR title, LPCTSTR text, const CToastOptions& opts)
	{
		CToastOptions o = opts;
		if( title ) o.Title(title);
		if( text ) o.Text(text);
		CToastWnd* pWnd = new CToastWnd(o);
		HWND h = pWnd->CreateToast();
		if( h == NULL ) {
			delete pWnd;
			return NULL;
		}
		return h;
	}

	HWND CToast::ShowSuccess(LPCTSTR text, int duration)
	{
		return Show(text, CToastOptions().Kind(CONTROLKIND_SUCCESS).Duration(duration));
	}

	HWND CToast::ShowDanger(LPCTSTR text, int duration)
	{
		return Show(text, CToastOptions().Kind(CONTROLKIND_DANGER).Duration(duration));
	}

	HWND CToast::ShowWarning(LPCTSTR text, int duration)
	{
		return Show(text, CToastOptions().Kind(CONTROLKIND_WARNING).Duration(duration));
	}

	HWND CToast::ShowInfo(LPCTSTR text, int duration)
	{
		return Show(text, CToastOptions().Kind(CONTROLKIND_INFO).Duration(duration));
	}

	void CToast::SetMaxCount(int n)
	{
		g_nMaxCount = n > 0 ? n : 0;
	}

	int CToast::GetMaxCount()
	{
		return g_nMaxCount;
	}

	void CToast::Dismiss(HWND hToast)
	{
		if( hToast == NULL || !::IsWindow(hToast) ) return;

		TCHAR szClass[64] = { 0 };
		::GetClassName(hToast, szClass, 63);
		if( _tcscmp(szClass, _T("DuiToastWnd")) != 0 ) return;

		::PostMessage(hToast, WM_CLOSE, (WPARAM)ToastDismiss_Manual, 0);
	}

	void CToast::DismissAll()
	{
		std::vector<HWND> copy = g_activeToasts;
		g_activeToasts.clear();
		for( size_t i = 0; i < copy.size(); ++i ) {
			HWND h = copy[i];
			if( ::IsWindow(h) )
				::PostMessage(h, WM_CLOSE, (WPARAM)ToastDismiss_Manual, 0);
		}
	}

} // namespace DuiLib
