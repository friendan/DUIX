#include "StdAfx.h"
#include "UIEditBox.h"
#include "UIEdit.h"
#include "UIButton.h"
#include "UISvgBox.h"

namespace DuiLib
{
	namespace {
		enum {
			kEditBoxHistoryItemH = 28,
			kEditBoxHistoryHeaderH = 28,
			kEditBoxHistoryMaxDrop = 320,
			kEditBoxHistoryIconBtn = 18, // 热区；内边距后字形约 10px，避免 × 抢眼
		};
		static const LPCTSTR kHistPick = _T("editbox_hist_pick");
		static const LPCTSTR kHistDel = _T("editbox_hist_del");
		static const LPCTSTR kHistClose = _T("editbox_hist_close");
		static const LPCTSTR kHistClearAll = _T("editbox_hist_clearall");
		static const LPCTSTR kHistList = _T("editbox_hist_list");

		// 历史弹层小 ×：轻量常态色 + 悬停才强调，尺寸统一
		static void StyleHistoryGlyph(CSvgBoxUI* p, DWORD clr, DWORD clrHot, DWORD bgHot)
		{
			if( p == NULL ) return;
			const int k = kEditBoxHistoryIconBtn;
			p->SetFixedWidth(k);
			p->SetFixedHeight(k);
			p->SetPadding(CDuiBox(4, 4, 4, 4));
			p->SetMouseEnabled(true);
			p->SetBackgroundColor(0);
			p->SetBorderWidth(0);
			p->SetHoverBackgroundColor(bgHot);
			p->SetActiveBackgroundColor(bgHot);
			p->SetAttribute(_T("lucide"), _T("x"));
			p->SetColor(clr);
			p->SetHoverColor(clrHot);
			p->SetActiveColor(clrHot);
			SIZE r = { 3, 3 };
			p->SetBorderRadius(r);
		}
	}

	//////////////////////////////////////////////////////////////////////////
	// 历史下拉：NOACTIVATE；可关、可单条删除、点外侧关闭
	class CEditBoxHistoryWnd : public CWindowWnd, public INotifyUI, public IMessageFilterUI
	{
	public:
		CEditBoxHistoryWnd()
			: m_pOwner(NULL)
			, m_pRoot(NULL)
			, m_pList(NULL)
			, m_nDropCy(0)
			, m_bFilter(false)
			, m_dwDropBg(0xFFFFFFFF)
			, m_dwDropBd(0xDEE2E6FF)
			, m_dwHeaderBg(0xF8F9FAFF)
			, m_dwText(0x000000E0)
			, m_dwTextSec(0x000000A6)
			, m_dwItemHover(0xF1F3F5FF)
			, m_dwDelHover(0xE9ECEFFF)
			, m_dwDivider(0xE9ECEFFF)
			, m_dwDanger(0xDC3545FF)
			, m_dwIconMuted(0xADB5BDFF)
		{
		}
		void Init(CEditBoxUI* pOwner);
		void UpdatePos();
		void RebuildList();
		void ResolveThemeColors();
		int CalcContentHeight() const;
		bool CalcDropRect(RECT& rcScreen) const;
		LPCTSTR GetWindowClassName() const override { return _T("EditBoxHistoryWnd"); }
		void OnFinalMessage(HWND /*hWnd*/) override;
		LRESULT HandleMessage(UINT uMsg, WPARAM wParam, LPARAM lParam) override;
		void Notify(TNotifyUI& msg) override;
		LRESULT MessageHandler(UINT uMsg, WPARAM wParam, LPARAM lParam, bool& bHandled) override;
		bool IsPtInPopupOrOwner(POINT ptScreen) const;

		CPaintManagerUI m_pm;
		CEditBoxUI* m_pOwner;
		CVerticalLayoutUI* m_pRoot;
		CVerticalLayoutUI* m_pList;
		int m_nDropCy;
		bool m_bFilter;
		DWORD m_dwDropBg;
		DWORD m_dwDropBd;
		DWORD m_dwHeaderBg;
		DWORD m_dwText;
		DWORD m_dwTextSec;
		DWORD m_dwItemHover;
		DWORD m_dwDelHover;
		DWORD m_dwDivider;
		DWORD m_dwDanger;
		DWORD m_dwIconMuted; // RGB 灰（勿用仅 alpha 不同的 secondary，SVG 着色会忽略 alpha）
	};

	void CEditBoxHistoryWnd::ResolveThemeColors()
	{
		m_dwDropBg = 0xFFFFFFFF;
		m_dwDropBd = 0xDEE2E6FF;
		m_dwHeaderBg = 0xF8F9FAFF;
		m_dwText = 0x000000E0;
		m_dwTextSec = 0x000000A6;
		m_dwItemHover = 0xF1F3F5FF;
		m_dwDelHover = 0xE9ECEFFF;
		m_dwDivider = 0xE9ECEFFF;
		m_dwDanger = 0xDC3545FF;
		m_dwIconMuted = 0xADB5BDFF;

		CThemeManager* tm = CThemeManager::GetInstance();
		if( tm == NULL ) return;
		CTheme* th = tm->GetCurrentTheme();
		if( th == NULL ) th = tm->FindTheme(tm->GetDefaultThemeId());
		if( th == NULL ) return;

		m_dwDropBg = th->GetToken(_T("color-bg-elevated"),
			th->GetToken(_T("color-control-bg"), th->GetToken(_T("color-bg"), m_dwDropBg)));
		m_dwDropBd = th->GetToken(_T("color-control-border"),
			th->GetToken(_T("color-border"), m_dwDropBd));
		m_dwHeaderBg = th->GetToken(_T("color-bg-secondary"),
			th->GetToken(_T("color-bg-hover"), m_dwHeaderBg));
		m_dwText = th->GetToken(_T("color-text"), m_dwText);
		m_dwTextSec = th->GetToken(_T("color-text-secondary"), m_dwTextSec);
		m_dwItemHover = th->GetToken(_T("color-bg-hover"),
			th->GetToken(_T("color-control-bg-hover"), m_dwItemHover));
		m_dwDelHover = th->GetToken(_T("color-bg-hover-medium"), m_dwItemHover);
		m_dwDivider = th->GetToken(_T("color-border"), m_dwDropBd);
		m_dwDanger = th->GetToken(_T("color-danger"), m_dwDanger);
		// 图标常态用有 RGB 差的浅灰，比 text-secondary（常为黑+alpha）更轻、且 SVG 能看出区别
		m_dwIconMuted = th->GetToken(_T("color-text-disabled"),
			th->GetToken(_T("color-border-strong"), m_dwIconMuted));
	}

	int CEditBoxHistoryWnd::CalcContentHeight() const
	{
		auto scale = [this](int v) -> int {
			CPaintManagerUI* pm = (m_pOwner != NULL) ? m_pOwner->GetManager() : NULL;
			if( pm == NULL ) pm = const_cast<CPaintManagerUI*>(&m_pm);
			if( pm != NULL && pm->GetDPIObj() != NULL )
				return pm->GetDPIObj()->Scale(v);
			return v;
		};

		// 与布局一致：根边框 + 顶栏 + 列表内边距 + 行/分割线（均按 DPI）
		const int headerH = scale(kEditBoxHistoryHeaderH);
		const int itemH = scale(kEditBoxHistoryItemH);
		const int listPadY = scale(4) * 2;
		const int borderY = scale(1) * 2;
		const int sepH = scale(1);
		const int slack = scale(2);

		const int nCount = (m_pOwner != NULL) ? m_pOwner->GetHistoryCount() : 0;
		int cy = borderY + headerH + listPadY + slack;
		if( nCount > 0 ) {
			cy += nCount * itemH;
			if( nCount > 1 ) cy += (nCount - 1) * sepH;
		}
		else {
			cy += itemH;
		}

		const int maxDrop = scale(kEditBoxHistoryMaxDrop);
		if( cy > maxDrop ) cy = maxDrop;
		return cy;
	}

	bool CEditBoxHistoryWnd::CalcDropRect(RECT& rcScreen) const
	{
		rcScreen = CDuiRect();
		if( m_pOwner == NULL || m_pOwner->GetManager() == NULL ) return false;
		RECT rcOwner = m_pOwner->GetPos();
		if( rcOwner.right <= rcOwner.left || rcOwner.bottom <= rcOwner.top ) return false;

		int cy = m_nDropCy > 0 ? m_nDropCy : CalcContentHeight();

		RECT rc = rcOwner;
		rc.top = rc.bottom;
		rc.bottom = rc.top + cy;

		HWND hPaint = m_pOwner->GetManager()->GetPaintWindow();
		::MapWindowRect(hPaint, HWND_DESKTOP, &rc);

		MONITORINFO oMonitor = {};
		oMonitor.cbSize = sizeof(oMonitor);
		::GetMonitorInfo(::MonitorFromWindow(hPaint, MONITOR_DEFAULTTONEAREST), &oMonitor);
		CDuiRect rcWork = oMonitor.rcWork;
		if( rc.bottom > rcWork.bottom ) {
			rc.left = rcOwner.left;
			rc.right = rcOwner.right;
			rc.top = rcOwner.top - cy;
			rc.bottom = rcOwner.top;
			::MapWindowRect(hPaint, HWND_DESKTOP, &rc);
		}
		rcScreen = rc;
		return true;
	}

	void CEditBoxHistoryWnd::UpdatePos()
	{
		if( m_hWnd == NULL || !::IsWindow(m_hWnd) ) return;
		m_nDropCy = CalcContentHeight();
		RECT rc = {};
		if( !CalcDropRect(rc) ) return;
		::SetWindowPos(m_hWnd, HWND_TOPMOST, rc.left, rc.top,
			rc.right - rc.left, rc.bottom - rc.top, SWP_NOACTIVATE);
		if( m_pRoot != NULL )
			m_pm.NeedUpdate();
	}

	void CEditBoxHistoryWnd::RebuildList()
	{
		if( m_pList == NULL || m_pOwner == NULL ) return;
		m_pList->RemoveAll();
		const int nCount = m_pOwner->GetHistoryCount();
		if( nCount <= 0 ) {
			CLabelUI* pEmpty = new CLabelUI;
			pEmpty->SetFixedHeight(kEditBoxHistoryItemH);
			pEmpty->SetText(_T("暂无历史"));
			pEmpty->SetColor(m_dwTextSec);
			pEmpty->SetAttribute(_T("text-align"), _T("center"));
			pEmpty->SetAttribute(_T("vertical-align"), _T("vcenter"));
			pEmpty->SetPadding(CDuiBox(0, 8, 0, 8));
			m_pList->Add(pEmpty);
		}
		else {
			for( int i = 0; i < nCount; ++i ) {
				CDuiString sText = m_pOwner->GetHistoryItem(i);
				CHorizontalLayoutUI* pRow = new CHorizontalLayoutUI;
				pRow->SetFixedHeight(kEditBoxHistoryItemH);
				pRow->SetAttribute(_T("align-items"), _T("vcenter"));
				pRow->SetGap(2);
				pRow->SetPadding(CDuiBox(0, 4, 0, 4));
				pRow->SetBackgroundColor(0);
				pRow->SetMouseEnabled(false);

				// 主区域：悬停用主题 color-bg-hover（列表级轻悬停）
				CButtonUI* pPick = new CButtonUI;
				pPick->SetName(kHistPick);
				pPick->SetKind(CONTROLKIND_NONE);
				pPick->SetFixedWidth(0);
				pPick->SetText(sText.GetData());
				pPick->SetUserData(sText.GetData());
				pPick->SetColor(m_dwText);
				pPick->SetBackgroundColor(0);
				pPick->SetBorderWidth(0);
				pPick->SetHoverBackgroundColor(m_dwItemHover);
				pPick->SetActiveBackgroundColor(m_dwDelHover);
				pPick->SetAttribute(_T("text-align"), _T("left"));
				pPick->SetAttribute(_T("vertical-align"), _T("vcenter"));
				pPick->SetPadding(CDuiBox(0, 8, 0, 8));
				{
					SIZE r = { 4, 4 };
					pPick->SetBorderRadius(r);
				}
				pRow->Add(pPick);

				// 小 ×：常态浅灰，悬停 danger；热区 18，字形约 10
				CSvgBoxUI* pDel = new CSvgBoxUI;
				pDel->SetName(kHistDel);
				pDel->SetUserData(sText.GetData());
				StyleHistoryGlyph(pDel, m_dwIconMuted, m_dwDanger, m_dwItemHover);
				pDel->SetToolTip(_T("删除这条"));
				pRow->Add(pDel);

				m_pList->Add(pRow);

				// 项间分割线
				if( i + 1 < nCount ) {
					CControlUI* pSep = new CControlUI;
					pSep->SetFixedHeight(1);
					pSep->SetMouseEnabled(false);
					pSep->SetBackgroundColor(m_dwDivider);
					m_pList->Add(pSep);
				}
			}
		}
		UpdatePos();
	}

	void CEditBoxHistoryWnd::Init(CEditBoxUI* pOwner)
	{
		m_pOwner = pOwner;
		m_pRoot = NULL;
		m_pList = NULL;
		m_nDropCy = 0;
		m_bFilter = false;
		if( m_pOwner == NULL || m_pOwner->GetManager() == NULL ) return;

		RECT rcOwner = m_pOwner->GetPos();
		if( rcOwner.right <= rcOwner.left || rcOwner.bottom <= rcOwner.top ) return;

		m_nDropCy = CalcContentHeight();
		RECT rc = {};
		if( !CalcDropRect(rc) ) return;

		HWND hPaint = m_pOwner->GetManager()->GetPaintWindow();
		Create(hPaint, NULL, WS_POPUP, WS_EX_TOOLWINDOW | WS_EX_NOACTIVATE | WS_EX_TOPMOST, rc);
		if( m_hWnd == NULL ) return;
		::ShowWindow(m_hWnd, SW_SHOWNOACTIVATE);
		::SetWindowPos(m_hWnd, HWND_TOPMOST, 0, 0, 0, 0,
			SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE);

		m_pOwner->GetManager()->AddMessageFilter(this);
		m_bFilter = true;
	}

	void CEditBoxHistoryWnd::OnFinalMessage(HWND /*hWnd*/)
	{
		if( m_bFilter && m_pOwner != NULL && m_pOwner->GetManager() != NULL )
			m_pOwner->GetManager()->RemoveMessageFilter(this);
		m_bFilter = false;
		if( m_pOwner != NULL )
			m_pOwner->m_pHistoryWnd = NULL;
		delete this;
	}

	bool CEditBoxHistoryWnd::IsPtInPopupOrOwner(POINT ptScreen) const
	{
		if( m_hWnd != NULL && ::IsWindow(m_hWnd) ) {
			RECT rcPop = {};
			::GetWindowRect(m_hWnd, &rcPop);
			if( ::PtInRect(&rcPop, ptScreen) ) return true;
		}
		if( m_pOwner != NULL && m_pOwner->GetManager() != NULL ) {
			RECT rcOwner = m_pOwner->GetPos();
			HWND hPaint = m_pOwner->GetManager()->GetPaintWindow();
			::MapWindowRect(hPaint, HWND_DESKTOP, &rcOwner);
			if( ::PtInRect(&rcOwner, ptScreen) ) return true;
		}
		return false;
	}

	LRESULT CEditBoxHistoryWnd::MessageHandler(UINT uMsg, WPARAM /*wParam*/, LPARAM /*lParam*/, bool& bHandled)
	{
		bHandled = false;
		if( uMsg == WM_MOVE || uMsg == WM_MOVING || uMsg == WM_SIZE
			|| uMsg == WM_WINDOWPOSCHANGED || uMsg == WM_WINDOWPOSCHANGING
			|| uMsg == WM_EXITSIZEMOVE )
		{
			UpdatePos();
		}
		else if( uMsg == WM_LBUTTONDOWN || uMsg == WM_NCLBUTTONDOWN
			|| uMsg == WM_RBUTTONDOWN || uMsg == WM_MBUTTONDOWN )
		{
			POINT pt = {};
			::GetCursorPos(&pt);
			if( !IsPtInPopupOrOwner(pt) ) {
				if( m_pOwner != NULL )
					m_pOwner->DismissHistoryPopup();
				else
					PostMessage(WM_CLOSE);
			}
		}
		return 0;
	}

	void CEditBoxHistoryWnd::Notify(TNotifyUI& msg)
	{
		if( m_pOwner == NULL || msg.pSender == NULL ) return;
		if( msg.sType != DUI_MSGTYPE_CLICK ) return;

		CDuiString sName = msg.pSender->GetName();
		if( sName.CompareNoCase(kHistClose) == 0 ) {
			m_pOwner->DismissHistoryPopup();
			return;
		}
		if( sName.CompareNoCase(kHistClearAll) == 0 ) {
			m_pOwner->ClearHistory();
			return;
		}
		if( sName.CompareNoCase(kHistDel) == 0 ) {
			CDuiString s = msg.pSender->GetUserData();
			if( s.IsEmpty() ) s = msg.pSender->GetText();
			m_pOwner->RemoveHistory(s.GetData());
			return;
		}
		if( sName.CompareNoCase(kHistPick) == 0 ) {
			CDuiString s = msg.pSender->GetUserData();
			if( s.IsEmpty() ) s = msg.pSender->GetText();
			if( !s.IsEmpty() ) {
				m_pOwner->ApplyHistoryPick(s.GetData());
				PostMessage(WM_CLOSE);
			}
		}
	}

	LRESULT CEditBoxHistoryWnd::HandleMessage(UINT uMsg, WPARAM wParam, LPARAM lParam)
	{
		if( uMsg == WM_CREATE ) {
			m_pm.SetForceUseSharedRes(true);
			m_pm.Init(m_hWnd);
			m_pm.GetShadow()->ShowShadow(false);
			ResolveThemeColors();

			m_pRoot = new CVerticalLayoutUI;
			m_pRoot->SetBackgroundColor(m_dwDropBg);
			m_pRoot->SetBorderColor(m_dwDropBd);
			m_pRoot->SetBorderWidth(1);
			m_pRoot->SetPadding(CDuiBox(0));
			m_pRoot->SetGap(0);
			{
				SIZE r = { 4, 4 };
				m_pRoot->SetBorderRadius(r);
			}

			CHorizontalLayoutUI* pHeader = new CHorizontalLayoutUI;
			pHeader->SetFixedHeight(kEditBoxHistoryHeaderH);
			pHeader->SetAttribute(_T("align-items"), _T("vcenter"));
			pHeader->SetGap(2);
			pHeader->SetPadding(CDuiBox(0, 8, 0, 6));
			pHeader->SetBackgroundColor(m_dwHeaderBg);
			pHeader->SetBorderColor(m_dwDivider);
			pHeader->SetAttribute(_T("border-bottom-width"), _T("1"));

			CLabelUI* pTitle = new CLabelUI;
			pTitle->SetText(_T("历史记录"));
			pTitle->SetFixedWidth(0);
			pTitle->SetColor(m_dwTextSec);
			pTitle->SetAttribute(_T("vertical-align"), _T("vcenter"));
			pHeader->Add(pTitle);

			CButtonUI* pClearAll = new CButtonUI;
			pClearAll->SetName(kHistClearAll);
			pClearAll->SetKind(CONTROLKIND_NONE);
			pClearAll->SetText(_T("清空"));
			pClearAll->SetColor(m_dwTextSec);
			pClearAll->SetHoverColor(m_dwText);
			pClearAll->SetFixedWidth(36);
			pClearAll->SetFixedHeight(kEditBoxHistoryIconBtn);
			pClearAll->SetBackgroundColor(0);
			pClearAll->SetBorderWidth(0);
			pClearAll->SetHoverBackgroundColor(m_dwItemHover);
			pClearAll->SetAttribute(_T("text-align"), _T("center"));
			pClearAll->SetAttribute(_T("vertical-align"), _T("vcenter"));
			pClearAll->SetToolTip(_T("清空全部历史"));
			{
				SIZE r = { 3, 3 };
				pClearAll->SetBorderRadius(r);
			}
			pHeader->Add(pClearAll);

			CSvgBoxUI* pClose = new CSvgBoxUI;
			pClose->SetName(kHistClose);
			StyleHistoryGlyph(pClose, m_dwIconMuted, m_dwDanger, m_dwItemHover);
			pClose->SetToolTip(_T("关闭"));
			pHeader->Add(pClose);

			m_pRoot->Add(pHeader);

			m_pList = new CVerticalLayoutUI;
			m_pList->SetName(kHistList);
			m_pList->SetBackgroundColor(m_dwDropBg);
			m_pList->SetPadding(CDuiBox(4, 4, 4, 4));
			m_pList->EnableScrollBar(true, false);
			m_pRoot->Add(m_pList);

			m_pm.AttachDialog(m_pRoot);
			m_pm.AddNotifier(this);
			RebuildList();
			return 0;
		}
		else if( uMsg == WM_CLOSE ) {
			// NOACTIVATE：关窗时不抢焦点
		}
		else if( uMsg == WM_KEYDOWN ) {
			if( wParam == VK_ESCAPE ) {
				if( m_pOwner != NULL )
					m_pOwner->DismissHistoryPopup();
				else
					PostMessage(WM_CLOSE);
			}
		}
		else if( uMsg == WM_MOUSEACTIVATE ) {
			return MA_NOACTIVATE;
		}

		LRESULT lRes = 0;
		if( m_pm.MessageHandler(uMsg, wParam, lParam, lRes) ) return lRes;
		return CWindowWnd::HandleMessage(uMsg, wParam, lParam);
	}

	IMPLEMENT_DUICONTROL(CEditBoxSlotUI)
	IMPLEMENT_DUICONTROL(CEditBoxLeftUI)
	IMPLEMENT_DUICONTROL(CEditBoxRightUI)
	IMPLEMENT_DUICONTROL(CEditBoxUI)

	//////////////////////////////////////////////////////////////////////////
	CEditBoxSlotUI::CEditBoxSlotUI()
	{
		SetAttribute(_T("align-items"), _T("vcenter"));
		SetGap(4);
		SetMouseEnabled(true);
	}

	LPCTSTR CEditBoxSlotUI::GetClass() const
	{
		return _T("EditBoxSlotUI");
	}

	LPVOID CEditBoxSlotUI::GetInterface(LPCTSTR pstrName)
	{
		if( _tcsicmp(pstrName, DUI_CTR_EDITBOXSLOT) == 0 ) return static_cast<CEditBoxSlotUI*>(this);
		return CHorizontalLayoutUI::GetInterface(pstrName);
	}

	SIZE CEditBoxSlotUI::EstimateSize(SIZE szAvailable)
	{
		SIZE sz = MeasureContent(szAvailable);
		if( GetFixedHeight() > 0 ) sz.cy = GetFixedHeight();
		else sz.cy = 0;
		if( sz.cx < 0 ) sz.cx = 0;
		return sz;
	}

	//////////////////////////////////////////////////////////////////////////
	LPCTSTR CEditBoxLeftUI::GetClass() const
	{
		return _T("EditBoxLeftUI");
	}

	LPVOID CEditBoxLeftUI::GetInterface(LPCTSTR pstrName)
	{
		if( _tcsicmp(pstrName, DUI_CTR_EDITBOXLEFT) == 0
			|| _tcsicmp(pstrName, DUI_CTR_EDITBOXPREFIX) == 0 )
			return static_cast<CEditBoxLeftUI*>(this);
		return CEditBoxSlotUI::GetInterface(pstrName);
	}

	//////////////////////////////////////////////////////////////////////////
	LPCTSTR CEditBoxRightUI::GetClass() const
	{
		return _T("EditBoxRightUI");
	}

	LPVOID CEditBoxRightUI::GetInterface(LPCTSTR pstrName)
	{
		if( _tcsicmp(pstrName, DUI_CTR_EDITBOXRIGHT) == 0
			|| _tcsicmp(pstrName, DUI_CTR_EDITBOXSUFFIX) == 0 )
			return static_cast<CEditBoxRightUI*>(this);
		return CEditBoxSlotUI::GetInterface(pstrName);
	}

	//////////////////////////////////////////////////////////////////////////
	CEditBoxUI::CEditBoxUI()
		: m_pPrefix(NULL)
		, m_pSuffix(NULL)
		, m_pEdit(NULL)
		, m_pClearBtn(NULL)
		, m_pEyeBtn(NULL)
		, m_pHistoryWnd(NULL)
		, m_bClearable(false)
		, m_bPasswordToggle(false)
		, m_bPasswordVisible(false)
		, m_bChromeReady(false)
		, m_bSubmitOnEnter(true)
		, m_bHistoryEnabled(false)
		, m_bSkipHistoryPopup(false)
		, m_nIconBtnWidth(28)
		, m_nHistoryMax(10)
	{
		SetFixedHeight(32);
		SetAttribute(_T("align-items"), _T("vcenter"));
		SetGap(4);
		SetPadding(CDuiBox(0, 6, 0, 6));
		SetBorderWidth(1);
		SIZE szR = { 4, 4 };
		SetBorderRadius(szR);
		EnsureChrome();
	}

	CEditBoxUI::~CEditBoxUI()
	{
		CloseHistoryPopup();
	}

	LPCTSTR CEditBoxUI::GetClass() const
	{
		return _T("EditBoxUI");
	}

	LPVOID CEditBoxUI::GetInterface(LPCTSTR pstrName)
	{
		if( _tcsicmp(pstrName, DUI_CTR_EDITBOX) == 0 ) return static_cast<CEditBoxUI*>(this);
		return CHorizontalLayoutUI::GetInterface(pstrName);
	}

	bool CEditBoxUI::IsFocused() const
	{
		if( CHorizontalLayoutUI::IsFocused() ) return true;
		return m_pEdit != NULL && m_pEdit->IsFocused();
	}

	bool CEditBoxUI::ParseBoolValue(LPCTSTR pstrValue)
	{
		if( pstrValue == NULL ) return false;
		return ( _tcsicmp(pstrValue, _T("true")) == 0
			|| _tcsicmp(pstrValue, _T("1")) == 0
			|| _tcsicmp(pstrValue, _T("yes")) == 0 );
	}

	void CEditBoxUI::ApplyIconButtonStyle(CButtonUI* pBtn)
	{
		if( pBtn == NULL ) return;
		pBtn->SetKind(CONTROLKIND_NONE);
		pBtn->SetBackgroundColor(0);
		pBtn->SetBorderColor(0);
		pBtn->SetBorderWidth(0);
		pBtn->SetHoverBackgroundColor(0x00000014);
		pBtn->SetActiveBackgroundColor(0x00000022);
		pBtn->SetTextStyle(DT_SINGLELINE | DT_VCENTER | DT_CENTER);
	}

	void CEditBoxUI::ApplyBuiltinIcons()
	{
		if( m_pClearBtn != NULL ) {
			m_pClearBtn->SetIconLib(_T("lucide"), _T("x"));
			m_pClearBtn->SetIconSize(14);
			m_pClearBtn->SetText(_T(""));
			m_pClearBtn->SetToolTip(_T("清除"));
		}
		if( m_pEyeBtn != NULL ) {
			m_pEyeBtn->SetIconLib(_T("lucide"), m_bPasswordVisible ? _T("eye-off") : _T("eye"));
			m_pEyeBtn->SetIconSize(14);
			m_pEyeBtn->SetText(_T(""));
			m_pEyeBtn->SetToolTip(m_bPasswordVisible ? _T("隐藏密码") : _T("显示密码"));
		}
	}

	void CEditBoxUI::EnsureChrome()
	{
		if( m_bChromeReady ) return;
		m_bChromeReady = true;

		m_pPrefix = new CEditBoxLeftUI;
		m_pPrefix->SetName(_T("editbox_left"));
		CHorizontalLayoutUI::Add(m_pPrefix);

		m_pEdit = new CEditUI;
		m_pEdit->SetName(_T("editbox_edit"));
		m_pEdit->SetFixedWidth(0);
		m_pEdit->SetBorderWidth(0);
		m_pEdit->SetBorderColor(0);
		m_pEdit->SetBackgroundColor(0);
		m_pEdit->SetNativeEditBackgroundColor(0xFFFFFFFF);
		m_pEdit->SetPadding(CDuiBox(0, 4, 0, 4));
		m_pEdit->OnNotify += MakeDelegate(this, &CEditBoxUI::OnChildNotify);
		m_pEdit->OnEvent += MakeDelegate(this, &CEditBoxUI::OnEditEvent);
		CHorizontalLayoutUI::Add(m_pEdit);

		m_pSuffix = new CEditBoxRightUI;
		m_pSuffix->SetName(_T("editbox_right"));
		CHorizontalLayoutUI::Add(m_pSuffix);

		m_pEyeBtn = new CButtonUI;
		m_pEyeBtn->SetName(_T("editbox_eye"));
		ApplyIconButtonStyle(m_pEyeBtn);
		m_pEyeBtn->SetVisible(false);
		m_pEyeBtn->OnNotify += MakeDelegate(this, &CEditBoxUI::OnChildNotify);
		m_pSuffix->Add(m_pEyeBtn);

		m_pClearBtn = new CButtonUI;
		m_pClearBtn->SetName(_T("editbox_clear"));
		ApplyIconButtonStyle(m_pClearBtn);
		m_pClearBtn->SetVisible(false);
		m_pClearBtn->OnNotify += MakeDelegate(this, &CEditBoxUI::OnChildNotify);
		m_pSuffix->Add(m_pClearBtn);

		ApplyBuiltinIcons();
		SyncSlotMetrics();
		SyncBuiltinButtons();
		SyncInnerEditChrome();
	}

	void CEditBoxUI::SyncSlotMetrics()
	{
		const int h = m_cxyFixed.cy > 0 ? m_cxyFixed.cy : 32;
		const int btn = m_nIconBtnWidth > 0 ? m_nIconBtnWidth : 28;
		if( m_pPrefix != NULL ) m_pPrefix->SetFixedHeight(h);
		if( m_pSuffix != NULL ) m_pSuffix->SetFixedHeight(h);
		if( m_pEdit != NULL ) m_pEdit->SetFixedHeight(h > 4 ? h - 2 : h);
		CButtonUI* btns[] = { m_pClearBtn, m_pEyeBtn };
		for( int i = 0; i < 2; ++i ) {
			if( btns[i] == NULL ) continue;
			btns[i]->SetFixedWidth(btn);
			btns[i]->SetFixedHeight(btn > h ? h : btn);
		}
	}

	void CEditBoxUI::SyncInnerEditChrome()
	{
		if( m_pEdit == NULL ) return;
		m_pEdit->SetBorderWidth(0);
		m_pEdit->SetBorderColor(0);
		m_pEdit->SetBackgroundColor(0);
		DWORD dwBk = GetBackgroundColor();
		if( dwBk == 0 ) dwBk = 0xFFFFFFFF;
		m_pEdit->SetNativeEditBackgroundColor(dwBk);
		DWORD dwText = 0;
		CLabelUI* pLab = static_cast<CLabelUI*>(m_pEdit->GetInterface(DUI_CTR_LABEL));
		if( pLab != NULL ) dwText = pLab->GetColor();
		if( dwText != 0 ) {
			TCHAR buf[16] = { 0 };
			_stprintf_s(buf, _countof(buf), _T("#%08X"), dwText);
			m_pEdit->SetNativeEditColor(buf);
		}
		m_pEdit->SyncNativeEditColors();
	}

	static bool EditBoxSlotHasSelfVisibleChild(CContainerUI* pSlot)
	{
		if( pSlot == NULL ) return false;
		for( int i = 0; i < pSlot->GetCount(); ++i ) {
			CControlUI* p = pSlot->GetItemAt(i);
			if( p != NULL && p->IsSelfVisible() ) return true;
		}
		return false;
	}

	void CEditBoxUI::SyncSlotVisibility()
	{
		// 左右 HBox：没有自身可见子项就隐藏；显示时 SetVisible(true) 恢复 InternVisible
		CEditBoxSlotUI* slots[2] = { m_pPrefix, m_pSuffix };
		for( int i = 0; i < 2; ++i ) {
			CEditBoxSlotUI* pSlot = slots[i];
			if( pSlot == NULL ) continue;
			const bool bShow = EditBoxSlotHasSelfVisibleChild(pSlot);
			if( bShow )
				pSlot->SetMaxWidth(9999);
			if( pSlot->IsSelfVisible() != bShow )
				pSlot->SetVisible(bShow);
		}
	}

	void CEditBoxUI::SyncClearVisible()
	{
		if( m_pClearBtn == NULL || m_pEdit == NULL ) return;
		const bool bShow = m_bClearable
			&& m_pEdit->IsEnabled()
			&& !m_pEdit->IsReadOnly()
			&& !m_pEdit->GetText().IsEmpty();
		if( m_pClearBtn->IsVisible() != bShow ) {
			m_pClearBtn->SetVisible(bShow);
			SyncSlotVisibility();
			NeedUpdate();
		}
	}

	void CEditBoxUI::SyncBuiltinButtons()
	{
		EnsureChrome();
		if( m_pEyeBtn != NULL ) {
			const bool bShow = m_bPasswordToggle && m_pEdit != NULL && m_pEdit->IsPasswordMode();
			m_pEyeBtn->SetVisible(bShow);
			if( bShow ) ApplyBuiltinIcons();
		}
		SyncClearVisible();
		SyncSlotVisibility();
		NeedUpdate();
	}

	int CEditBoxUI::FindBuiltinInsertIndex() const
	{
		if( m_pSuffix == NULL ) return 0;
		int idx = m_pSuffix->GetCount();
		for( int i = 0; i < m_pSuffix->GetCount(); ++i ) {
			CControlUI* p = m_pSuffix->GetItemAt(i);
			if( IsBuiltinControl(p) ) {
				idx = i;
				break;
			}
		}
		return idx;
	}

	bool CEditBoxUI::IsBuiltinControl(CControlUI* pControl) const
	{
		return pControl != NULL && (pControl == m_pClearBtn || pControl == m_pEyeBtn);
	}

	bool CEditBoxUI::IsLeftSlotControl(CControlUI* pControl) const
	{
		if( pControl == NULL ) return false;
		return pControl->GetInterface(DUI_CTR_EDITBOXLEFT) != NULL
			|| pControl->GetInterface(DUI_CTR_EDITBOXPREFIX) != NULL;
	}

	bool CEditBoxUI::IsRightSlotControl(CControlUI* pControl) const
	{
		if( pControl == NULL ) return false;
		return pControl->GetInterface(DUI_CTR_EDITBOXRIGHT) != NULL
			|| pControl->GetInterface(DUI_CTR_EDITBOXSUFFIX) != NULL;
	}

	int CEditBoxUI::IndexOfChild(CControlUI* pControl) const
	{
		for( int i = 0; i < GetCount(); ++i ) {
			if( GetItemAt(i) == pControl ) return i;
		}
		return -1;
	}

	void CEditBoxUI::DetachFromParent(CContainerUI* pParent, CControlUI* pChild)
	{
		if( pParent == NULL || pChild == NULL ) return;
		pParent->SetAutoDestroy(false);
		pParent->Remove(pChild);
		pParent->SetAutoDestroy(true);
	}

	bool CEditBoxUI::AdoptLeftSlot(CEditBoxSlotUI* pNew)
	{
		if( pNew == NULL ) return false;
		EnsureChrome();
		if( pNew == m_pPrefix )
			return true;

		// 旧槽里若有直接塞进来的子项，并入新槽
		if( m_pPrefix != NULL ) {
			while( m_pPrefix->GetCount() > 0 ) {
				CControlUI* p = m_pPrefix->GetItemAt(0);
				DetachFromParent(m_pPrefix, p);
				pNew->Add(p);
			}
			const int idx = IndexOfChild(m_pPrefix);
			DetachFromParent(this, m_pPrefix);
			delete m_pPrefix;
			m_pPrefix = pNew;
			if( m_pPrefix->GetName().IsEmpty() )
				m_pPrefix->SetName(_T("editbox_left"));
			const bool ok = CHorizontalLayoutUI::AddAt(m_pPrefix, idx >= 0 ? idx : 0);
			SyncSlotMetrics();
			SyncSlotVisibility();
			return ok;
		}

		m_pPrefix = pNew;
		if( m_pPrefix->GetName().IsEmpty() )
			m_pPrefix->SetName(_T("editbox_left"));
		const bool ok = CHorizontalLayoutUI::AddAt(m_pPrefix, 0);
		SyncSlotMetrics();
		SyncSlotVisibility();
		return ok;
	}

	bool CEditBoxUI::AdoptRightSlot(CEditBoxSlotUI* pNew)
	{
		if( pNew == NULL ) return false;
		EnsureChrome();
		if( pNew == m_pSuffix )
			return true;

		// 内建钮始终挂在右侧最末
		if( m_pSuffix != NULL ) {
			if( m_pEyeBtn != NULL ) {
				DetachFromParent(m_pSuffix, m_pEyeBtn);
				pNew->Add(m_pEyeBtn);
			}
			if( m_pClearBtn != NULL ) {
				DetachFromParent(m_pSuffix, m_pClearBtn);
				pNew->Add(m_pClearBtn);
			}
			while( m_pSuffix->GetCount() > 0 ) {
				CControlUI* p = m_pSuffix->GetItemAt(0);
				DetachFromParent(m_pSuffix, p);
				pNew->AddAt(p, FindBuiltinInsertIndex());
			}
			const int idx = IndexOfChild(m_pSuffix);
			DetachFromParent(this, m_pSuffix);
			delete m_pSuffix;
			m_pSuffix = pNew;
			if( m_pSuffix->GetName().IsEmpty() )
				m_pSuffix->SetName(_T("editbox_right"));
			const int insertAt = idx >= 0 ? idx : GetCount();
			const bool ok = CHorizontalLayoutUI::AddAt(m_pSuffix, insertAt);
			SyncSlotMetrics();
			SyncBuiltinButtons();
			return ok;
		}

		m_pSuffix = pNew;
		if( m_pSuffix->GetName().IsEmpty() )
			m_pSuffix->SetName(_T("editbox_right"));
		if( m_pEyeBtn != NULL ) m_pSuffix->Add(m_pEyeBtn);
		if( m_pClearBtn != NULL ) m_pSuffix->Add(m_pClearBtn);
		const bool ok = CHorizontalLayoutUI::Add(m_pSuffix);
		SyncSlotMetrics();
		SyncBuiltinButtons();
		return ok;
	}

	void CEditBoxUI::SetText(LPCTSTR pstrText)
	{
		EnsureChrome();
		if( m_pEdit != NULL ) m_pEdit->SetText(pstrText);
		SyncClearVisible();
	}

	CDuiString CEditBoxUI::GetText() const
	{
		if( m_pEdit != NULL ) return m_pEdit->GetText();
		return CDuiString();
	}

	void CEditBoxUI::SetClearable(bool b)
	{
		if( m_bClearable == b ) return;
		m_bClearable = b;
		SyncBuiltinButtons();
	}

	void CEditBoxUI::SetPasswordToggle(bool b)
	{
		if( m_bPasswordToggle == b ) return;
		m_bPasswordToggle = b;
		SyncBuiltinButtons();
	}

	void CEditBoxUI::SetIconBtnWidth(int n)
	{
		if( n <= 0 ) n = 28;
		if( m_nIconBtnWidth == n ) return;
		m_nIconBtnWidth = n;
		EnsureChrome();
		SyncSlotMetrics();
		NeedUpdate();
	}

	void CEditBoxUI::SetSubmitOnEnter(bool b)
	{
		m_bSubmitOnEnter = b;
	}

	void CEditBoxUI::SetSubmitButton(LPCTSTR pstrName)
	{
		m_sSubmitButton = pstrName ? pstrName : _T("");
	}

	void CEditBoxUI::SetHistoryEnabled(bool b)
	{
		m_bHistoryEnabled = b;
		if( !m_bHistoryEnabled )
			CloseHistoryPopup();
	}

	void CEditBoxUI::SetHistoryMaxCount(int n)
	{
		if( n < 1 ) n = 1;
		if( n > 100 ) n = 100;
		m_nHistoryMax = n;
		while( (int)m_aHistory.size() > m_nHistoryMax )
			m_aHistory.pop_back();
	}

	bool CEditBoxUI::AddHistory(LPCTSTR text)
	{
		if( !m_bHistoryEnabled ) return false;
		if( text == NULL || text[0] == _T('\0') ) return false;
		CDuiString s(text);
		// 去掉首尾空白
		while( s.GetLength() > 0 && (s[0] == _T(' ') || s[0] == _T('\t')) )
			s = s.Right(s.GetLength() - 1);
		while( s.GetLength() > 0 ) {
			TCHAR c = s.GetAt(s.GetLength() - 1);
			if( c != _T(' ') && c != _T('\t') ) break;
			s = s.Left(s.GetLength() - 1);
		}
		if( s.IsEmpty() ) return false;
		for( size_t i = 0; i < m_aHistory.size(); ++i ) {
			if( m_aHistory[i] == s ) {
				m_aHistory.erase(m_aHistory.begin() + (std::ptrdiff_t)i);
				break;
			}
		}
		m_aHistory.insert(m_aHistory.begin(), s);
		while( (int)m_aHistory.size() > m_nHistoryMax )
			m_aHistory.pop_back();
		// 弹层已开时实时刷新（回车加入等）
		if( m_pHistoryWnd != NULL && ::IsWindow(m_pHistoryWnd->GetHWND()) )
			m_pHistoryWnd->RebuildList();
		NotifyHistoryChanged(DUI_HISTORYCHANGE_ADD, 0);
		return true;
	}

	bool CEditBoxUI::RemoveHistory(LPCTSTR text)
	{
		if( text == NULL || text[0] == _T('\0') ) return false;
		CDuiString s(text);
		for( size_t i = 0; i < m_aHistory.size(); ++i ) {
			if( m_aHistory[i] == s ) {
				const LPARAM idx = (LPARAM)i;
				m_aHistory.erase(m_aHistory.begin() + (std::ptrdiff_t)i);
				if( m_pHistoryWnd != NULL && ::IsWindow(m_pHistoryWnd->GetHWND()) ) {
					if( m_aHistory.empty() )
						DismissHistoryPopup();
					else
						m_pHistoryWnd->RebuildList();
				}
				NotifyHistoryChanged(DUI_HISTORYCHANGE_REMOVE, idx);
				return true;
			}
		}
		return false;
	}

	bool CEditBoxUI::RemoveHistoryAt(int index)
	{
		if( index < 0 || index >= (int)m_aHistory.size() ) return false;
		m_aHistory.erase(m_aHistory.begin() + (std::ptrdiff_t)index);
		if( m_pHistoryWnd != NULL && ::IsWindow(m_pHistoryWnd->GetHWND()) ) {
			if( m_aHistory.empty() )
				DismissHistoryPopup();
			else
				m_pHistoryWnd->RebuildList();
		}
		NotifyHistoryChanged(DUI_HISTORYCHANGE_REMOVE, (LPARAM)index);
		return true;
	}

	void CEditBoxUI::ClearHistory()
	{
		const bool bHad = !m_aHistory.empty();
		m_aHistory.clear();
		m_bSkipHistoryPopup = false;
		CloseHistoryPopup();
		if( bHad )
			NotifyHistoryChanged(DUI_HISTORYCHANGE_CLEAR, 0);
	}

	void CEditBoxUI::SetHistory(LPCTSTR const* items, int count)
	{
		m_aHistory.clear();
		if( items != NULL && count > 0 ) {
			for( int i = 0; i < count; ++i ) {
				if( items[i] == NULL || items[i][0] == _T('\0') ) continue;
				CDuiString s(items[i]);
				while( s.GetLength() > 0 && (s[0] == _T(' ') || s[0] == _T('\t')) )
					s = s.Right(s.GetLength() - 1);
				while( s.GetLength() > 0 ) {
					TCHAR c = s.GetAt(s.GetLength() - 1);
					if( c != _T(' ') && c != _T('\t') ) break;
					s = s.Left(s.GetLength() - 1);
				}
				if( s.IsEmpty() ) continue;
				bool bDup = false;
				for( size_t j = 0; j < m_aHistory.size(); ++j ) {
					if( m_aHistory[j] == s ) { bDup = true; break; }
				}
				if( !bDup ) m_aHistory.push_back(s);
			}
			while( (int)m_aHistory.size() > m_nHistoryMax )
				m_aHistory.pop_back();
		}
		if( m_pHistoryWnd != NULL && ::IsWindow(m_pHistoryWnd->GetHWND()) ) {
			if( m_aHistory.empty() )
				CloseHistoryPopup();
			else
				m_pHistoryWnd->RebuildList();
		}
		NotifyHistoryChanged(DUI_HISTORYCHANGE_SET, (LPARAM)m_aHistory.size());
	}

	void CEditBoxUI::NotifyHistoryChanged(WPARAM reason, LPARAM lParam)
	{
		if( m_pManager != NULL )
			m_pManager->SendNotify(this, DUI_MSGTYPE_HISTORYCHANGED, reason, lParam);
	}

	int CEditBoxUI::GetHistoryCount() const
	{
		return (int)m_aHistory.size();
	}

	CDuiString CEditBoxUI::GetHistoryItem(int index) const
	{
		if( index < 0 || index >= (int)m_aHistory.size() ) return CDuiString();
		return m_aHistory[(size_t)index];
	}

	void CEditBoxUI::CloseHistoryPopup()
	{
		if( m_pHistoryWnd != NULL && ::IsWindow(m_pHistoryWnd->GetHWND()) )
			m_pHistoryWnd->Close();
		m_pHistoryWnd = NULL;
	}

	void CEditBoxUI::DismissHistoryPopup()
	{
		// 主动关掉后输入框往往仍有焦点；抑制本轮内再点/聚焦自动弹出
		m_bSkipHistoryPopup = true;
		CloseHistoryPopup();
	}

	bool CEditBoxUI::IsHistoryPopupVisible() const
	{
		return m_pHistoryWnd != NULL && ::IsWindow(m_pHistoryWnd->GetHWND());
	}

	void CEditBoxUI::RequestShowHistoryPopup()
	{
		if( m_bSkipHistoryPopup ) return;
		if( !m_bHistoryEnabled || m_aHistory.empty() ) return;
		if( m_pEdit != NULL && m_pEdit->IsPasswordMode() ) return;
		if( !IsEnabled() || m_pManager == NULL ) return;
		if( m_pHistoryWnd != NULL ) return;
		OpenHistoryPopupNow();
	}

	void CEditBoxUI::TryShowHistoryOnFocus()
	{
		// 选中历史项后 SetFocus：吞掉一次，避免立刻又弹
		if( m_bSkipHistoryPopup ) {
			m_bSkipHistoryPopup = false;
			return;
		}
		RequestShowHistoryPopup();
	}

	void CEditBoxUI::OnInnerEditNativeClick()
	{
		// 已 Dismiss 且仍聚焦：再点不弹；失焦后会清 skip
		RequestShowHistoryPopup();
	}

	void CEditBoxUI::OpenHistoryPopupNow()
	{
		if( !m_bHistoryEnabled || m_aHistory.empty() ) return;
		if( m_pEdit != NULL && m_pEdit->IsPasswordMode() ) return;
		if( !IsEnabled() || m_pManager == NULL ) return;
		if( m_pHistoryWnd != NULL ) return;
		m_pHistoryWnd = new CEditBoxHistoryWnd;
		m_pHistoryWnd->Init(this);
		if( m_pHistoryWnd->GetHWND() == NULL ) {
			delete m_pHistoryWnd;
			m_pHistoryWnd = NULL;
		}
	}

	void CEditBoxUI::ShowHistoryPopup()
	{
		// 手动弹出：清掉 Dismiss 抑制
		m_bSkipHistoryPopup = false;
		if( m_pHistoryWnd != NULL ) return;
		OpenHistoryPopupNow();
	}

	void CEditBoxUI::ApplyHistoryPick(LPCTSTR text)
	{
		CloseHistoryPopup();
		SetText(text);
		if( m_pEdit != NULL ) {
			if( m_pEdit->IsFocused() ) {
				m_bSkipHistoryPopup = false;
			}
			else {
				m_bSkipHistoryPopup = true;
				m_pEdit->SetFocus();
			}
		}
	}

	CControlUI* CEditBoxUI::ResolveSubmitButton() const
	{
		if( m_sSubmitButton.IsEmpty() ) return NULL;
		CEditBoxUI* pSelf = const_cast<CEditBoxUI*>(this);
		CControlUI* p = pSelf->FindSubControl(m_sSubmitButton.GetData());
		if( p != NULL ) return p;
		if( m_pManager != NULL )
			return m_pManager->FindControl(m_sSubmitButton.GetData());
		return NULL;
	}

	void CEditBoxUI::TrySubmitOnEnter()
	{
		if( !m_bSubmitOnEnter || !IsEnabled() ) return;
		CControlUI* pBtn = ResolveSubmitButton();
		if( pBtn == NULL || !pBtn->IsEnabled() || !pBtn->IsVisible() ) return;
		// AddHistory 已在 RETURN 通知里做过；此处再调一次只是去重提到最前并刷新弹层
		AddHistory(GetText().GetData());
		pBtn->Activate();
	}

	void CEditBoxUI::ApplyEnabledToChild(CControlUI* pControl)
	{
		if( pControl == NULL ) return;
		pControl->SetEnabled(IsEnabled());
	}

	void CEditBoxUI::SetEnabled(bool bEnable)
	{
		CHorizontalLayoutUI::SetEnabled(bEnable);
		EnsureChrome();
		if( m_pPrefix != NULL ) {
			ApplyEnabledToChild(m_pPrefix);
			for( int i = 0; i < m_pPrefix->GetCount(); ++i )
				ApplyEnabledToChild(m_pPrefix->GetItemAt(i));
		}
		if( m_pEdit != NULL ) ApplyEnabledToChild(m_pEdit);
		if( m_pSuffix != NULL ) {
			ApplyEnabledToChild(m_pSuffix);
			for( int i = 0; i < m_pSuffix->GetCount(); ++i )
				ApplyEnabledToChild(m_pSuffix->GetItemAt(i));
		}
		if( !bEnable ) CloseHistoryPopup();
	}

	void CEditBoxUI::SetFixedHeight(int cy)
	{
		CHorizontalLayoutUI::SetFixedHeight(cy);
		if( m_bChromeReady ) SyncSlotMetrics();
	}

	void CEditBoxUI::SetPos(RECT rc, bool bNeedInvalidate)
	{
		CHorizontalLayoutUI::SetPos(rc, bNeedInvalidate);
		if( m_pHistoryWnd != NULL )
			m_pHistoryWnd->UpdatePos();
	}

	void CEditBoxUI::Move(SIZE szOffset, bool bNeedInvalidate)
	{
		CHorizontalLayoutUI::Move(szOffset, bNeedInvalidate);
		if( m_pHistoryWnd != NULL )
			m_pHistoryWnd->UpdatePos();
	}

	void CEditBoxUI::SetManager(CPaintManagerUI* pManager, CControlUI* pParent, bool bInit)
	{
		CHorizontalLayoutUI::SetManager(pManager, pParent, bInit);
		if( bInit ) SyncInnerEditChrome();
	}

	bool CEditBoxUI::Add(CControlUI* pControl)
	{
		EnsureChrome();
		if( pControl == NULL ) return false;
		if( pControl == m_pPrefix || pControl == m_pSuffix || pControl == m_pEdit )
			return CHorizontalLayoutUI::Add(pControl);
		if( IsBuiltinControl(pControl) )
			return m_pSuffix != NULL && m_pSuffix->Add(pControl);

		if( IsLeftSlotControl(pControl) )
			return AdoptLeftSlot(static_cast<CEditBoxSlotUI*>(pControl));
		if( IsRightSlotControl(pControl) )
			return AdoptRightSlot(static_cast<CEditBoxSlotUI*>(pControl));

		// 直接子控件默认进左侧（简单图标场景）
		if( m_pPrefix == NULL ) return false;
		const bool ok = m_pPrefix->Add(pControl);
		if( ok ) {
			ApplyEnabledToChild(pControl);
			SyncSlotVisibility();
		}
		return ok;
	}

	bool CEditBoxUI::AddAt(CControlUI* pControl, int iIndex)
	{
		EnsureChrome();
		if( pControl == NULL ) return false;
		if( pControl == m_pPrefix || pControl == m_pSuffix || pControl == m_pEdit )
			return CHorizontalLayoutUI::AddAt(pControl, iIndex);
		if( IsLeftSlotControl(pControl) )
			return AdoptLeftSlot(static_cast<CEditBoxSlotUI*>(pControl));
		if( IsRightSlotControl(pControl) )
			return AdoptRightSlot(static_cast<CEditBoxSlotUI*>(pControl));

		if( m_pPrefix == NULL ) return false;
		const bool ok = m_pPrefix->AddAt(pControl, iIndex);
		if( ok ) {
			ApplyEnabledToChild(pControl);
			SyncSlotVisibility();
		}
		return ok;
	}

	bool CEditBoxUI::OnEditEvent(void* param)
	{
		TEventUI* pEvent = static_cast<TEventUI*>(param);
		if( pEvent == NULL ) return true;
		if( pEvent->Type == UIEVENT_BUTTONDOWN || pEvent->Type == UIEVENT_DBLCLICK ) {
			if( m_bHistoryEnabled && !m_aHistory.empty() )
				RequestShowHistoryPopup();
		}
		return true;
	}

	bool CEditBoxUI::OnChildNotify(void* param)
	{
		TNotifyUI* pMsg = static_cast<TNotifyUI*>(param);
		if( pMsg == NULL || pMsg->pSender == NULL ) return true;

		if( pMsg->pSender == m_pEdit ) {
			if( pMsg->sType == DUI_MSGTYPE_TEXTCHANGED )
				SyncClearVisible();
			else if( pMsg->sType == DUI_MSGTYPE_SETFOCUS
				|| pMsg->sType == DUI_MSGTYPE_KILLFOCUS )
			{
				Invalidate();
			}
			if( m_pManager != NULL && (
				pMsg->sType == DUI_MSGTYPE_TEXTCHANGED
				|| pMsg->sType == DUI_MSGTYPE_SETFOCUS
				|| pMsg->sType == DUI_MSGTYPE_KILLFOCUS
				|| pMsg->sType == DUI_MSGTYPE_RETURN ) )
			{
				m_pManager->SendNotify(this, pMsg->sType.GetData(), pMsg->wParam, pMsg->lParam);
			}
			if( pMsg->sType == DUI_MSGTYPE_SETFOCUS )
				TryShowHistoryOnFocus();
			else if( pMsg->sType == DUI_MSGTYPE_KILLFOCUS ) {
				// 离开输入框后允许下次聚焦再弹
				m_bSkipHistoryPopup = false;
				CloseHistoryPopup();
			}
			else if( pMsg->sType == DUI_MSGTYPE_RETURN ) {
				AddHistory(GetText().GetData());
				TrySubmitOnEnter();
			}
			return true;
		}

		if( pMsg->sType == DUI_MSGTYPE_CLICK ) {
			if( pMsg->pSender == m_pClearBtn && m_pEdit != NULL ) {
				m_pEdit->SetText(_T(""));
				SyncClearVisible();
				if( m_pManager != NULL )
					m_pManager->SendNotify(this, DUI_MSGTYPE_TEXTCHANGED);
				m_pEdit->SetFocus();
				return true;
			}
			if( pMsg->pSender == m_pEyeBtn && m_pEdit != NULL ) {
				m_bPasswordVisible = !m_bPasswordVisible;
				m_pEdit->SetPasswordMode(!m_bPasswordVisible);
				ApplyBuiltinIcons();
				m_pEdit->SetFocus();
				Invalidate();
				return true;
			}
		}
		return true;
	}

	void CEditBoxUI::DoEvent(TEventUI& event)
	{
		if( event.Type == UIEVENT_BUTTONDOWN || event.Type == UIEVENT_DBLCLICK ) {
			if( IsEnabled() && m_pEdit != NULL ) {
				POINT pt = event.ptMouse;
				bool bOnBuiltin = false;
				if( m_pClearBtn != NULL && m_pClearBtn->IsVisible()
					&& ::PtInRect(&m_pClearBtn->GetPos(), pt) )
					bOnBuiltin = true;
				if( m_pEyeBtn != NULL && m_pEyeBtn->IsVisible()
					&& ::PtInRect(&m_pEyeBtn->GetPos(), pt) )
					bOnBuiltin = true;
				if( !bOnBuiltin ) {
					bool bInEdit = ::PtInRect(&m_pEdit->GetPos(), pt);
					if( !bInEdit )
						m_pEdit->SetFocus();
					if( m_bHistoryEnabled && !m_aHistory.empty() )
						RequestShowHistoryPopup();
				}
			}
		}
		CHorizontalLayoutUI::DoEvent(event);
	}

	bool CEditBoxUI::DoPaint(IRenderContext& ctx, const RECT& rcPaint, CControlUI* pStopControl)
	{
		bool bRet = CHorizontalLayoutUI::DoPaint(ctx, rcPaint, pStopControl);
		PaintBorder(ctx);
		return bRet;
	}

	bool CEditBoxUI::IsEditForwardAttr(LPCTSTR pstrName)
	{
		if( pstrName == NULL ) return false;
		static const LPCTSTR kAttrs[] = {
			_T("placeholder"), _T("placeholder-color"), _T("tip"), _T("tooltip"), _T("title"),
			_T("font"), _T("font-size"), _T("font-family"), _T("text-color"), _T("textcolor"),
			_T("color"), _T("disabled-text-color"), _T("disabled-color"),
			_T("align"), _T("text-align"), _T("vertical-align"),
			_T("password"), _T("password-char"), _T("type"),
			_T("readonly"), _T("read-only"), _T("maxchar"), _T("max-char"), _T("maxlength"),
			_T("number-only"), _T("number"), _T("want-return"),
			_T("native-bkcolor"), _T("native-bk-color"), _T("native-background-color"),
			_T("native-text-color"), _T("native-color"),
			_T("image"), _T("image-hot"), _T("image-focused"), _T("image-focus"), _T("image-disabled"),
			_T("normalimage"), _T("hotimage"), _T("focusedimage"), _T("disabledimage"),
			_T("text-padding"), _T("textpadding"),
			_T("auto-sel-all"), _T("autoselectall"), _T("select-on-focus"), _T("autoselect"),
			_T("disabled"), _T("enabled"),
		};
		for( size_t i = 0; i < sizeof(kAttrs) / sizeof(kAttrs[0]); ++i ) {
			if( _tcsicmp(pstrName, kAttrs[i]) == 0 ) return true;
		}
		return false;
	}

	void CEditBoxUI::SetAttribute(LPCTSTR pstrName, LPCTSTR pstrValue)
	{
		EnsureChrome();
		if( _tcsicmp(pstrName, _T("clearable")) == 0
			|| _tcsicmp(pstrName, _T("clear")) == 0 ) {
			SetClearable(ParseBoolValue(pstrValue));
		}
		else if( _tcsicmp(pstrName, _T("password-toggle")) == 0
			|| _tcsicmp(pstrName, _T("show-password-toggle")) == 0 ) {
			SetPasswordToggle(ParseBoolValue(pstrValue));
		}
		else if( _tcsicmp(pstrName, _T("icon-btn-width")) == 0
			|| _tcsicmp(pstrName, _T("addon-btn-width")) == 0 ) {
			SetIconBtnWidth(_ttoi(pstrValue));
		}
		else if( _tcsicmp(pstrName, _T("submit-on-enter")) == 0
			|| _tcsicmp(pstrName, _T("enter-submit")) == 0 ) {
			SetSubmitOnEnter(ParseBoolValue(pstrValue));
		}
		else if( _tcsicmp(pstrName, _T("submit-button")) == 0
			|| _tcsicmp(pstrName, _T("enter-button")) == 0 ) {
			SetSubmitButton(pstrValue);
		}
		else if( _tcsicmp(pstrName, _T("history")) == 0
			|| _tcsicmp(pstrName, _T("history-enabled")) == 0
			|| _tcsicmp(pstrName, _T("show-history")) == 0 ) {
			SetHistoryEnabled(ParseBoolValue(pstrValue));
		}
		else if( _tcsicmp(pstrName, _T("history-max")) == 0
			|| _tcsicmp(pstrName, _T("history-max-count")) == 0 ) {
			SetHistoryMaxCount(_ttoi(pstrValue));
		}
		else if( _tcsicmp(pstrName, _T("text")) == 0
			|| _tcsicmp(pstrName, _T("value")) == 0 ) {
			SetText(pstrValue);
		}
		else if( _tcsicmp(pstrName, _T("tip")) == 0 ) {
			SetToolTip(pstrValue);
			if( m_pEdit != NULL ) m_pEdit->SetAttribute(_T("tooltip"), pstrValue);
		}
		else if( _tcsnicmp(pstrName, _T("left-"), 5) == 0 && m_pPrefix != NULL ) {
			m_pPrefix->SetAttribute(pstrName + 5, pstrValue);
		}
		else if( _tcsnicmp(pstrName, _T("prefix-"), 7) == 0 && m_pPrefix != NULL ) {
			m_pPrefix->SetAttribute(pstrName + 7, pstrValue);
		}
		else if( _tcsnicmp(pstrName, _T("right-"), 6) == 0 && m_pSuffix != NULL ) {
			m_pSuffix->SetAttribute(pstrName + 6, pstrValue);
		}
		else if( _tcsnicmp(pstrName, _T("suffix-"), 7) == 0 && m_pSuffix != NULL ) {
			m_pSuffix->SetAttribute(pstrName + 7, pstrValue);
		}
		else if( m_pEdit != NULL && IsEditForwardAttr(pstrName) ) {
			if( _tcsicmp(pstrName, _T("tip")) == 0 )
				m_pEdit->SetAttribute(_T("tooltip"), pstrValue);
			else
				m_pEdit->SetAttribute(pstrName, pstrValue);
			if( _tcsicmp(pstrName, _T("type")) == 0
				|| _tcsicmp(pstrName, _T("password")) == 0 )
				SyncBuiltinButtons();
			else if( _tcsicmp(pstrName, _T("readonly")) == 0
				|| _tcsicmp(pstrName, _T("enabled")) == 0
				|| _tcsicmp(pstrName, _T("disabled")) == 0 )
				SyncClearVisible();
		}
		else if( _tcsicmp(pstrName, _T("background-color")) == 0
			|| _tcsicmp(pstrName, _T("bkcolor")) == 0 ) {
			CHorizontalLayoutUI::SetAttribute(pstrName, pstrValue);
			SyncInnerEditChrome();
		}
		else if( _tcsicmp(pstrName, _T("enabled")) == 0
			|| _tcsicmp(pstrName, _T("disabled")) == 0 ) {
			bool b = ParseBoolValue(pstrValue);
			if( _tcsicmp(pstrName, _T("disabled")) == 0 ) b = !b;
			SetEnabled(b);
		}
		else {
			CHorizontalLayoutUI::SetAttribute(pstrName, pstrValue);
		}
	}
}
