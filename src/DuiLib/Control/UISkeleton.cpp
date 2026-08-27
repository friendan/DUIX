#include "StdAfx.h"
#include "UISkeleton.h"

namespace
{
	VOID CALLBACK SkeletonQueueTimerProc(PVOID lpParameter, BOOLEAN /*TimerOrWaitFired*/)
	{
		DuiLib::CSkeletonUI* pSelf = static_cast<DuiLib::CSkeletonUI*>(lpParameter);
		if( pSelf == NULL || !pSelf->IsActive() ) return;
		DuiLib::CPaintManagerUI* pm = pSelf->GetManager();
		if( pm == NULL ) return;
		HWND hWnd = pm->GetPaintWindow();
		if( hWnd == NULL || !::IsWindow(hWnd) ) return;
		::PostMessage(hWnd, DuiLib::UIMSG_SKELETON_TICK, (WPARAM)pSelf, 0);
	}
}

namespace DuiLib
{
	IMPLEMENT_DUICONTROL(CSkeletonUI)

	CSkeletonUI::CSkeletonUI()
		: m_eType(TypeDefault)
		, m_bActive(true)
		, m_bAvatar(true)
		, m_bTitle(true)
		, m_nParagraph(3)
		, m_nRound(4)
		, m_nPhase(0)
		, m_dwBlockColor(0x0000000F)
		, m_dwHighlightColor(0xFFFFFFA0)
		, m_hQueueTimer(NULL)
	{
		SetKind(CONTROLKIND_NONE);
		SetMouseEnabled(false);
	}

	CSkeletonUI::~CSkeletonUI()
	{
		StopAnim();
	}

	LPCTSTR CSkeletonUI::GetClass() const { return _T("SkeletonUI"); }

	LPVOID CSkeletonUI::GetInterface(LPCTSTR pstrName)
	{
		if( _tcsicmp(pstrName, DUI_CTR_SKELETON) == 0 ) return static_cast<CSkeletonUI*>(this);
		return CControlUI::GetInterface(pstrName);
	}

	int CSkeletonUI::ScaleValue(int v) const
	{
		return m_pManager ? m_pManager->GetDPIObj()->Scale(v) : v;
	}

	void CSkeletonUI::SetActive(bool b)
	{
		if( m_bActive == b ) return;
		m_bActive = b;
		if( m_bActive && IsVisible() ) StartAnim();
		else StopAnim();
		Invalidate();
	}

	bool CSkeletonUI::IsActive() const { return m_bActive; }

	void CSkeletonUI::SetAvatar(bool b)
	{
		m_bAvatar = b;
		Invalidate();
	}

	bool CSkeletonUI::IsAvatar() const { return m_bAvatar; }

	void CSkeletonUI::SetTitle(bool b)
	{
		m_bTitle = b;
		Invalidate();
	}

	bool CSkeletonUI::IsTitle() const { return m_bTitle; }

	void CSkeletonUI::SetParagraphRows(int n)
	{
		if( n < 0 ) n = 0;
		if( n > 12 ) n = 12;
		m_nParagraph = n;
		Invalidate();
	}

	int CSkeletonUI::GetParagraphRows() const { return m_nParagraph; }

	void CSkeletonUI::SetType(Type t)
	{
		m_eType = t;
		NeedParentUpdate();
		Invalidate();
	}

	CSkeletonUI::Type CSkeletonUI::GetType() const { return m_eType; }

	void CSkeletonUI::SetRound(int n)
	{
		if( n < 0 ) n = 0;
		m_nRound = n;
		Invalidate();
	}

	int CSkeletonUI::GetRound() const { return m_nRound; }

	void CSkeletonUI::SetBlockColor(DWORD dw)
	{
		m_dwBlockColor = dw;
		Invalidate();
	}

	void CSkeletonUI::SetHighlightColor(DWORD dw)
	{
		m_dwHighlightColor = dw;
		Invalidate();
	}

	void CSkeletonUI::StartAnim()
	{
		if( !m_bActive || m_pManager == NULL || !IsVisible() ) return;
		StartQueueTimer();
	}

	void CSkeletonUI::StopAnim()
	{
		StopQueueTimer();
	}

	void CSkeletonUI::StopQueueTimer()
	{
		if( m_hQueueTimer != NULL ) {
			::DeleteTimerQueueTimer(NULL, m_hQueueTimer, INVALID_HANDLE_VALUE);
			m_hQueueTimer = NULL;
		}
	}

	void CSkeletonUI::StartQueueTimer()
	{
		StopQueueTimer();
		if( !m_bActive || m_pManager == NULL || !IsVisible() ) return;
		HANDLE hTimer = NULL;
		if( ::CreateTimerQueueTimer(&hTimer, NULL, SkeletonQueueTimerProc,
			reinterpret_cast<PVOID>(this),
			kSkeletonTickMs, kSkeletonTickMs, WT_EXECUTEDEFAULT) ) {
			m_hQueueTimer = hTimer;
		}
	}

	void CSkeletonUI::OnAnimTick()
	{
		if( !m_bActive || !IsVisible() ) return;
		m_nPhase += 4;
		if( m_nPhase > 100 ) m_nPhase = 0;
		Invalidate();
	}

	void CSkeletonUI::SetManager(CPaintManagerUI* pManager, CControlUI* pParent, bool bInit)
	{
		CControlUI::SetManager(pManager, pParent, bInit);
		if( bInit && m_bActive && IsVisible() ) StartAnim();
	}

	void CSkeletonUI::SetVisible(bool bVisible)
	{
		CControlUI::SetVisible(bVisible);
		if( bVisible && m_bActive ) StartAnim();
		else StopAnim();
	}

	void CSkeletonUI::SetInternVisible(bool bVisible)
	{
		CControlUI::SetInternVisible(bVisible);
		if( bVisible && m_bActive && IsVisible() ) StartAnim();
		else if( !bVisible ) StopAnim();
	}

	SIZE CSkeletonUI::EstimateSize(SIZE /*szAvailable*/)
	{
		SIZE sz = GetFixedSize();
		int line = ScaleValue(16);
		int gap = ScaleValue(12);
		int h = 0;
		switch( m_eType ) {
		case TypeAvatar:
			if( sz.cx <= 0 ) sz.cx = ScaleValue(40);
			if( sz.cy <= 0 ) sz.cy = ScaleValue(40);
			break;
		case TypeButton:
			if( sz.cx <= 0 ) sz.cx = ScaleValue(80);
			if( sz.cy <= 0 ) sz.cy = ScaleValue(32);
			break;
		case TypeInput:
			if( sz.cx <= 0 ) sz.cx = ScaleValue(200);
			if( sz.cy <= 0 ) sz.cy = ScaleValue(32);
			break;
		case TypeParagraph:
			h = m_nParagraph > 0 ? m_nParagraph * line + (m_nParagraph - 1) * ScaleValue(10) : line;
			if( sz.cx <= 0 ) sz.cx = ScaleValue(240);
			if( sz.cy <= 0 ) sz.cy = h;
			break;
		default:
			h = 0;
			if( m_bAvatar ) h = ScaleValue(40);
			if( m_bTitle ) h = (h > 0 ? h + gap : 0) + ScaleValue(20);
			if( m_nParagraph > 0 )
				h = (h > 0 ? h + gap : 0) + m_nParagraph * line + (m_nParagraph - 1) * ScaleValue(10);
			if( h <= 0 ) h = ScaleValue(80);
			if( sz.cx <= 0 ) sz.cx = ScaleValue(280);
			if( sz.cy <= 0 ) sz.cy = h;
			break;
		}
		return sz;
	}

	void CSkeletonUI::PaintBlock(IRenderContext& ctx, const RECT& rc)
	{
		if( rc.right <= rc.left || rc.bottom <= rc.top ) return;
		int r = ScaleValue(m_nRound);
		ctx.FillRoundRect(rc, r, r, GetAdjustColor(m_dwBlockColor));
		if( !m_bActive ) return;

		int w = rc.right - rc.left;
		int band = w / 3;
		if( band < ScaleValue(24) ) band = ScaleValue(24);
		int x = rc.left + (w + band) * m_nPhase / 100 - band;
		RECT rcBand = { x, rc.top, x + band, rc.bottom };
		RECT rcClip = rc;
		CRenderClipScope clip(ctx, rcClip);
		if( ::IntersectRect(&rcBand, &rcBand, &rc) )
			ctx.DrawGradient(rcBand, GetAdjustColor(0xFFFFFF00), GetAdjustColor(m_dwHighlightColor), false, 16);
	}

	void CSkeletonUI::PaintDefault(IRenderContext& ctx)
	{
		CDuiBox pad = GetPadding();
		RECT rc = m_rcItem;
		rc.left += pad.left; rc.right -= pad.right;
		rc.top += pad.top; rc.bottom -= pad.bottom;
		int gap = ScaleValue(12);
		int y = rc.top;
		int x = rc.left;

		if( m_bAvatar ) {
			int av = ScaleValue(40);
			RECT rcAv = { x, y, x + av, y + av };
			int oldRound = m_nRound;
			m_nRound = 20;
			PaintBlock(ctx, rcAv);
			m_nRound = oldRound;
			x = rcAv.right + gap;
			if( m_bTitle ) {
				RECT rcTitle = { x, y + ScaleValue(4), rc.right, y + ScaleValue(20) };
				if( rcTitle.right > rcTitle.left + ScaleValue(40) )
					rcTitle.right = rcTitle.left + (rc.right - x) * 2 / 5;
				PaintBlock(ctx, rcTitle);
			}
			y += av + gap;
			x = rc.left;
		}
		else if( m_bTitle ) {
			RECT rcTitle = { x, y, x + (rc.right - x) * 2 / 5, y + ScaleValue(20) };
			PaintBlock(ctx, rcTitle);
			y += ScaleValue(20) + gap;
		}

		int lineH = ScaleValue(16);
		int lineGap = ScaleValue(10);
		for( int i = 0; i < m_nParagraph; ++i ) {
			RECT rcLine = { x, y, rc.right, y + lineH };
			if( i == m_nParagraph - 1 && m_nParagraph > 1 )
				rcLine.right = x + (rc.right - x) * 3 / 5;
			PaintBlock(ctx, rcLine);
			y += lineH + lineGap;
		}
	}

	void CSkeletonUI::PaintAvatarOnly(IRenderContext& ctx)
	{
		RECT rc = m_rcItem;
		int old = m_nRound;
		int side = rc.right - rc.left;
		int sideY = rc.bottom - rc.top;
		m_nRound = (side < sideY ? side : sideY) / 2;
		PaintBlock(ctx, rc);
		m_nRound = old;
	}

	void CSkeletonUI::PaintButton(IRenderContext& ctx)
	{
		PaintBlock(ctx, m_rcItem);
	}

	void CSkeletonUI::PaintInput(IRenderContext& ctx)
	{
		PaintBlock(ctx, m_rcItem);
	}

	void CSkeletonUI::PaintParagraphOnly(IRenderContext& ctx)
	{
		CDuiBox pad = GetPadding();
		RECT rc = m_rcItem;
		rc.left += pad.left; rc.right -= pad.right;
		rc.top += pad.top; rc.bottom -= pad.bottom;
		int lineH = ScaleValue(16);
		int lineGap = ScaleValue(10);
		int y = rc.top;
		for( int i = 0; i < m_nParagraph; ++i ) {
			RECT rcLine = { rc.left, y, rc.right, y + lineH };
			if( i == m_nParagraph - 1 && m_nParagraph > 1 )
				rcLine.right = rc.left + (rc.right - rc.left) * 3 / 5;
			PaintBlock(ctx, rcLine);
			y += lineH + lineGap;
		}
	}

	bool CSkeletonUI::DoPaint(IRenderContext& ctx, const RECT& /*rcPaint*/, CControlUI* /*pStopControl*/)
	{
		PaintBackgroundColor(ctx);
		switch( m_eType ) {
		case TypeAvatar: PaintAvatarOnly(ctx); break;
		case TypeButton: PaintButton(ctx); break;
		case TypeInput: PaintInput(ctx); break;
		case TypeParagraph: PaintParagraphOnly(ctx); break;
		default: PaintDefault(ctx); break;
		}
		PaintBorder(ctx);
		return true;
	}

	void CSkeletonUI::DoEvent(TEventUI& event)
	{
		CControlUI::DoEvent(event);
	}

	void CSkeletonUI::SetAttribute(LPCTSTR pstrName, LPCTSTR pstrValue)
	{
		if( _tcsicmp(pstrName, _T("active")) == 0 || _tcsicmp(pstrName, _T("animated")) == 0 ) {
			SetActive(_tcsicmp(pstrValue, _T("true")) == 0);
		}
		else if( _tcsicmp(pstrName, _T("avatar")) == 0 ) {
			SetAvatar(_tcsicmp(pstrValue, _T("true")) == 0);
		}
		else if( _tcsicmp(pstrName, _T("title")) == 0 ) {
			SetTitle(_tcsicmp(pstrValue, _T("true")) == 0);
		}
		else if( _tcsicmp(pstrName, _T("paragraph")) == 0 || _tcsicmp(pstrName, _T("rows")) == 0 ) {
			SetParagraphRows(_ttoi(pstrValue));
		}
		else if( _tcsicmp(pstrName, _T("type")) == 0 || _tcsicmp(pstrName, _T("variant")) == 0 ) {
			if( _tcsicmp(pstrValue, _T("avatar")) == 0 ) SetType(TypeAvatar);
			else if( _tcsicmp(pstrValue, _T("button")) == 0 ) SetType(TypeButton);
			else if( _tcsicmp(pstrValue, _T("input")) == 0 ) SetType(TypeInput);
			else if( _tcsicmp(pstrValue, _T("paragraph")) == 0 ) SetType(TypeParagraph);
			else SetType(TypeDefault);
		}
		else if( _tcsicmp(pstrName, _T("round")) == 0 || _tcsicmp(pstrName, _T("border-radius")) == 0 ) {
			SetRound(_ttoi(pstrValue));
		}
		else if( _tcsicmp(pstrName, _T("block-color")) == 0 || _tcsicmp(pstrName, _T("color")) == 0 ) {
			DWORD clr = 0; if( ParseColorString(pstrValue, clr) ) SetBlockColor(clr);
		}
		else if( _tcsicmp(pstrName, _T("highlight-color")) == 0 ) {
			DWORD clr = 0; if( ParseColorString(pstrValue, clr) ) SetHighlightColor(clr);
		}
		else {
			CControlUI::SetAttribute(pstrName, pstrValue);
		}
	}

	void DuiLib_SkeletonOnQueueTick(CSkeletonUI* pSkeleton)
	{
		if( pSkeleton != NULL )
			pSkeleton->OnAnimTick();
	}
}
