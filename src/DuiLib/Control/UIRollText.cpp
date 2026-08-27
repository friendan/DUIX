#include "StdAfx.h"
#include "UIRollText.h"

namespace
{
	VOID CALLBACK RollTextRollQueueTimerProc(PVOID lpParameter, BOOLEAN /*TimerOrWaitFired*/)
	{
		DuiLib::CRollTextUI* pSelf = static_cast<DuiLib::CRollTextUI*>(lpParameter);
		if( pSelf == NULL ) return;
		DuiLib::CPaintManagerUI* pm = pSelf->GetManager();
		if( pm == NULL ) return;
		HWND hWnd = pm->GetPaintWindow();
		if( hWnd == NULL || !::IsWindow(hWnd) ) return;
		::PostMessage(hWnd, DuiLib::UIMSG_ROLLTEXT_TICK, (WPARAM)pSelf, ROLLTEXT_TIMERID);
	}

	VOID CALLBACK RollTextEndQueueTimerProc(PVOID lpParameter, BOOLEAN /*TimerOrWaitFired*/)
	{
		DuiLib::CRollTextUI* pSelf = static_cast<DuiLib::CRollTextUI*>(lpParameter);
		if( pSelf == NULL ) return;
		DuiLib::CPaintManagerUI* pm = pSelf->GetManager();
		if( pm == NULL ) return;
		HWND hWnd = pm->GetPaintWindow();
		if( hWnd == NULL || !::IsWindow(hWnd) ) return;
		::PostMessage(hWnd, DuiLib::UIMSG_ROLLTEXT_TICK, (WPARAM)pSelf, ROLLTEXT_ROLL_END);
	}
}

namespace DuiLib
{
	IMPLEMENT_DUICONTROL(CRollTextUI)

	CRollTextUI::CRollTextUI(void)
		: m_nStep(1)
		, m_nScrollPos(0)
		, m_bUseRoll(FALSE)
		, m_bAutoRoll(false)
		, m_bPendingRollEnd(false)
		, m_bPauseOnHover(true)
		, m_bHoverPaused(false)
		, m_bPaused(false)
		, m_nRollDirection(ROLLTEXT_LEFT)
		, m_lTimeSpan(ROLLTEXT_TIMERID_SPAN)
		, m_lMaxTimeLimited(0)
		, m_dwDurationDeadline(0)
		, m_nLoopLimit(0)
		, m_nLoopDone(0)
		, m_nText_W_H(0)
		, m_hRollQueueTimer(NULL)
		, m_hEndQueueTimer(NULL)
	{
	}

	CRollTextUI::~CRollTextUI(void)
	{
		EndRoll();
	}

	LPCTSTR CRollTextUI::GetClass() const
	{
		return _T("RollTextUI");
	}

	LPVOID CRollTextUI::GetInterface(LPCTSTR pstrName)
	{
		if( _tcsicmp(pstrName, DUI_CTR_ROLLTEXT) == 0 ) return static_cast<CRollTextUI*>(this);
		if( _tcsicmp(pstrName, DUI_CTR_MARQUEE) == 0 ) return static_cast<CRollTextUI*>(this);
		return CLabelUI::GetInterface(pstrName);
	}

	int CRollTextUI::ParseDirection(LPCTSTR pstrValue)
	{
		if( pstrValue == NULL || *pstrValue == _T('\0') ) return ROLLTEXT_LEFT;
		if( _tcsicmp(pstrValue, _T("left")) == 0 || _tcsicmp(pstrValue, _T("l")) == 0
			|| _tcscmp(pstrValue, _T("0")) == 0 ) return ROLLTEXT_LEFT;
		if( _tcsicmp(pstrValue, _T("right")) == 0 || _tcsicmp(pstrValue, _T("r")) == 0
			|| _tcscmp(pstrValue, _T("1")) == 0 ) return ROLLTEXT_RIGHT;
		if( _tcsicmp(pstrValue, _T("up")) == 0 || _tcsicmp(pstrValue, _T("u")) == 0
			|| _tcscmp(pstrValue, _T("2")) == 0 ) return ROLLTEXT_UP;
		if( _tcsicmp(pstrValue, _T("down")) == 0 || _tcsicmp(pstrValue, _T("d")) == 0
			|| _tcscmp(pstrValue, _T("3")) == 0 ) return ROLLTEXT_DOWN;
		return ROLLTEXT_LEFT;
	}

	bool CRollTextUI::ParseBoolAttr(LPCTSTR pstrValue)
	{
		if( pstrValue == NULL || *pstrValue == _T('\0') ) return false;
		return _tcsicmp(pstrValue, _T("true")) == 0 || _tcscmp(pstrValue, _T("1")) == 0
			|| _tcsicmp(pstrValue, _T("yes")) == 0;
	}

	bool CRollTextUI::PreferClientHit() const
	{
		if( !IsEnabled() ) return false;
		if( m_bPauseOnHover && m_bUseRoll ) return true;
		return CLabelUI::PreferClientHit();
	}

	void CRollTextUI::DoInit()
	{
		CLabelUI::DoInit();
		if( m_bAutoRoll ) BeginRoll(m_nRollDirection, m_lTimeSpan, m_lMaxTimeLimited);
	}

	void CRollTextUI::RestartRollIfNeeded()
	{
		if( !m_bAutoRoll || m_pManager == NULL ) return;
		if( m_bUseRoll ) BeginRoll(m_nRollDirection, m_lTimeSpan, m_lMaxTimeLimited);
	}

	void CRollTextUI::SetAutoRoll(bool bAuto)
	{
		if( m_bAutoRoll == bAuto ) return;
		m_bAutoRoll = bAuto;
		if( m_pManager == NULL ) return;
		if( m_bAutoRoll ) BeginRoll(m_nRollDirection, m_lTimeSpan, m_lMaxTimeLimited);
		else EndRoll();
	}

	void CRollTextUI::SetRollDirection(int nDirect)
	{
		if( nDirect < ROLLTEXT_LEFT || nDirect > ROLLTEXT_DOWN ) nDirect = ROLLTEXT_LEFT;
		if( m_nRollDirection == nDirect ) return;
		m_nRollDirection = nDirect;
		m_nText_W_H = 0;
		m_nScrollPos = 0;
		RestartRollIfNeeded();
	}

	void CRollTextUI::SetRollInterval(LONG lTimeSpan)
	{
		if( lTimeSpan < 1 ) lTimeSpan = 1;
		if( m_lTimeSpan == lTimeSpan ) return;
		m_lTimeSpan = lTimeSpan;
		RestartRollIfNeeded();
	}

	void CRollTextUI::SetRollDuration(LONG lSeconds)
	{
		if( m_lMaxTimeLimited == lSeconds ) return;
		m_lMaxTimeLimited = lSeconds;
		RestartRollIfNeeded();
	}

	void CRollTextUI::SetRollStep(int nStep)
	{
		if( nStep < 1 ) nStep = 1;
		if( m_nStep == nStep ) return;
		m_nStep = nStep;
	}

	void CRollTextUI::SetRollLoop(int nLoop)
	{
		if( nLoop < 0 ) nLoop = 0;
		if( m_nLoopLimit == nLoop ) return;
		m_nLoopLimit = nLoop;
		RestartRollIfNeeded();
	}

	void CRollTextUI::SetPauseOnHover(bool bPause)
	{
		if( m_bPauseOnHover == bPause ) return;
		m_bPauseOnHover = bPause;
		if( !m_bPauseOnHover && m_bHoverPaused ) {
			m_bHoverPaused = false;
			ApplyPauseTimers();
		}
	}

	void CRollTextUI::Pause()
	{
		if( m_bPaused ) return;
		m_bPaused = true;
		ApplyPauseTimers();
	}

	void CRollTextUI::Resume()
	{
		if( !m_bPaused ) return;
		m_bPaused = false;
		ApplyPauseTimers();
	}

	void CRollTextUI::StopRollQueueTimer()
	{
		if( m_hRollQueueTimer != NULL ) {
			::DeleteTimerQueueTimer(NULL, m_hRollQueueTimer, INVALID_HANDLE_VALUE);
			m_hRollQueueTimer = NULL;
		}
	}

	void CRollTextUI::StopEndQueueTimer()
	{
		if( m_hEndQueueTimer != NULL ) {
			::DeleteTimerQueueTimer(NULL, m_hEndQueueTimer, INVALID_HANDLE_VALUE);
			m_hEndQueueTimer = NULL;
		}
	}

	void CRollTextUI::StartRollQueueTimer()
	{
		StopRollQueueTimer();
		if( m_pManager == NULL || !m_bUseRoll || m_lTimeSpan < 1 ) return;
		HANDLE hTimer = NULL;
		if( ::CreateTimerQueueTimer(&hTimer, NULL, RollTextRollQueueTimerProc,
			reinterpret_cast<PVOID>(this),
			(UINT)m_lTimeSpan, (UINT)m_lTimeSpan, WT_EXECUTEDEFAULT) ) {
			m_hRollQueueTimer = hTimer;
		}
	}

	void CRollTextUI::StartEndQueueTimer(UINT uElapse)
	{
		StopEndQueueTimer();
		if( m_pManager == NULL || uElapse == 0 ) return;
		HANDLE hTimer = NULL;
		if( ::CreateTimerQueueTimer(&hTimer, NULL, RollTextEndQueueTimerProc,
			reinterpret_cast<PVOID>(this), uElapse, 0, WT_EXECUTEONLYONCE) ) {
			m_hEndQueueTimer = hTimer;
		}
	}

	void CRollTextUI::ApplyPauseTimers()
	{
		if( m_pManager == NULL || !m_bUseRoll ) return;
		const bool bPause = m_bPaused || m_bHoverPaused;
		StopRollQueueTimer();
		StopEndQueueTimer();
		if( bPause ) return;
		StartRollQueueTimer();
		if( m_lMaxTimeLimited > 0 && m_dwDurationDeadline != 0 ) {
			const DWORD now = ::GetTickCount();
			const DWORD remain = m_dwDurationDeadline - now;
			if( remain == 0 || remain > (DWORD)(m_lMaxTimeLimited * 1000) )
				m_bPendingRollEnd = true;
			else
				StartEndQueueTimer(remain);
		}
	}

	void CRollTextUI::FinishRoll()
	{
		if( !m_bUseRoll && !m_bPendingRollEnd ) return;
		m_bPendingRollEnd = false;
		EndRoll();
		if( m_pManager != NULL )
			m_pManager->SendNotify(this, DUI_MSGTYPE_TEXTROLLEND);
	}

	void CRollTextUI::BeginRoll(int nDirect, LONG lTimeSpan, LONG lMaxTimeLimited)
	{
		if( m_pManager == NULL ) return;
		if( nDirect < ROLLTEXT_LEFT || nDirect > ROLLTEXT_DOWN ) nDirect = ROLLTEXT_LEFT;
		if( lTimeSpan < 1 ) lTimeSpan = ROLLTEXT_TIMERID_SPAN;

		m_nRollDirection = nDirect;
		m_lTimeSpan = lTimeSpan;
		m_lMaxTimeLimited = lMaxTimeLimited;

		if( m_bUseRoll ) EndRoll();

		m_nText_W_H = 0;
		m_nScrollPos = 0;
		m_nLoopDone = 0;
		m_bPendingRollEnd = false;
		m_bHoverPaused = false;
		m_bPaused = false;
		m_dwDurationDeadline = 0;

		m_bUseRoll = TRUE;
		StartRollQueueTimer();
		if( m_lMaxTimeLimited > 0 ) {
			m_dwDurationDeadline = ::GetTickCount() + (DWORD)(m_lMaxTimeLimited * 1000);
			StartEndQueueTimer((UINT)(m_lMaxTimeLimited * 1000));
		}

		Invalidate();
	}

	void CRollTextUI::EndRoll()
	{
		StopEndQueueTimer();
		StopRollQueueTimer();
		if( !m_bUseRoll ) return;
		m_bUseRoll = FALSE;
		m_bPendingRollEnd = false;
		m_bHoverPaused = false;
		m_bPaused = false;
		m_nScrollPos = 0;
		Invalidate();
	}

	void CRollTextUI::SetPos(RECT rc, bool bNeedInvalidate)
	{
		CLabelUI::SetPos(rc, bNeedInvalidate);
		m_nText_W_H = 0;
	}

	void CRollTextUI::SetText(LPCTSTR pstrText)
	{
		CLabelUI::SetText(pstrText);
		m_nText_W_H = 0;
		m_nScrollPos = 0;
		m_nLoopDone = 0;
	}

	void CRollTextUI::SetAttribute(LPCTSTR pstrName, LPCTSTR pstrValue)
	{
		if( pstrName == NULL ) return;

		if( _tcsicmp(pstrName, _T("roll")) == 0 || _tcsicmp(pstrName, _T("rolling")) == 0
			|| _tcsicmp(pstrName, _T("marquee")) == 0 ) {
			SetAutoRoll(_tcsicmp(pstrValue, _T("true")) == 0 || _tcscmp(pstrValue, _T("1")) == 0);
		}
		else if( _tcsicmp(pstrName, _T("roll-direction")) == 0 || _tcsicmp(pstrName, _T("direction")) == 0 ) {
			SetRollDirection(ParseDirection(pstrValue));
		}
		else if( _tcsicmp(pstrName, _T("roll-interval")) == 0 || _tcsicmp(pstrName, _T("roll-span")) == 0 ) {
			if( pstrValue != NULL ) SetRollInterval(_ttoi(pstrValue));
		}
		else if( _tcsicmp(pstrName, _T("roll-duration")) == 0 || _tcsicmp(pstrName, _T("roll-timeout")) == 0 ) {
			if( pstrValue != NULL ) SetRollDuration(_ttoi(pstrValue));
		}
		else if( _tcsicmp(pstrName, _T("roll-step")) == 0 ) {
			if( pstrValue != NULL ) SetRollStep(_ttoi(pstrValue));
		}
		else if( _tcsicmp(pstrName, _T("roll-loop")) == 0 || _tcsicmp(pstrName, _T("roll-count")) == 0
			|| _tcsicmp(pstrName, _T("loop")) == 0 ) {
			if( pstrValue != NULL ) SetRollLoop(_ttoi(pstrValue));
		}
		else if( _tcsicmp(pstrName, _T("roll-pause-hover")) == 0
			|| _tcsicmp(pstrName, _T("pause-on-hover")) == 0
			|| _tcsicmp(pstrName, _T("roll-hover-pause")) == 0 ) {
			SetPauseOnHover(ParseBoolAttr(pstrValue));
		}
		else {
			CLabelUI::SetAttribute(pstrName, pstrValue);
		}
	}

	void CRollTextUI::DoEvent(TEventUI& event)
	{
		if( event.Type == UIEVENT_TIMER && event.wParam == ROLLTEXT_ROLL_END )
		{
			FinishRoll();
			return;
		}
		if( event.Type == UIEVENT_TIMER && event.wParam == ROLLTEXT_TIMERID )
		{
			if( m_bPendingRollEnd ) {
				FinishRoll();
				return;
			}
			if( m_bUseRoll && !IsPaused() ) Invalidate();
			return;
		}
		if( event.Type == UIEVENT_MOUSEENTER )
		{
			if( m_bPauseOnHover && m_bUseRoll && IsEnabled() ) {
				m_bHoverPaused = true;
				ApplyPauseTimers();
			}
		}
		else if( event.Type == UIEVENT_MOUSELEAVE )
		{
			if( m_bHoverPaused ) {
				m_bHoverPaused = false;
				ApplyPauseTimers();
			}
		}
		CLabelUI::DoEvent(event);
	}

	static CDuiString ReverseUtf16(LPCTSTR src)
	{
		CDuiString out;
		if( src == NULL || *src == _T('\0') ) return out;
		const int n = (int)_tcslen(src);
		for( int i = n - 1; i >= 0; --i ) {
			if( i > 0 && src[i] >= 0xDC00 && src[i] <= 0xDFFF
				&& src[i - 1] >= 0xD800 && src[i - 1] <= 0xDBFF ) {
				TCHAR pair[3] = { src[i - 1], src[i], 0 };
				out += pair;
				--i;
			}
			else {
				out += src[i];
			}
		}
		return out;
	}

	// 强制按字符序绘制，避免 DWrite/Uniscribe 把后半段英文/标点再 BiDi 重排
	static CDuiString WrapLtrOverride(const CDuiString& s)
	{
		CDuiString out;
		out += (TCHAR)0x202D;
		out += s;
		out += (TCHAR)0x202C;
		return out;
	}

	void CRollTextUI::PaintText(IRenderContext& ctx)
	{
		if( !m_bUseRoll ) {
			CLabelUI::PaintText(ctx);
			return;
		}

		if( m_dwColor == 0 && m_pManager != NULL )
			m_dwColor = m_pManager->GetDefaultFontColor();
		if( m_dwDisabledColor == 0 && m_pManager != NULL )
			m_dwDisabledColor = m_pManager->GetDefaultDisabledColor();
		DWORD dwColor = IsEnabled() ? m_dwColor : m_dwDisabledColor;
		CDuiString sLogical = GetText();
		if( sLogical.IsEmpty() ) return;

		const bool bRtl = (m_nRollDirection == ROLLTEXT_RIGHT && !m_bShowHtml);
		CDuiString sGlyphs = bRtl ? ReverseUtf16(sLogical.GetData()) : sLogical;
		CDuiString sPaint = bRtl ? WrapLtrOverride(sGlyphs) : sGlyphs;

		RECT rcTextPadding = GetTextPadding();
		RECT rcPadding = GetPadding();
		CDuiRect rcClient = m_rcItem;
		rcClient.left += rcPadding.left + rcTextPadding.left;
		rcClient.right -= rcPadding.right + rcTextPadding.right;
		rcClient.top += rcPadding.top + rcTextPadding.top;
		rcClient.bottom -= rcPadding.bottom + rcTextPadding.bottom;
		if( rcClient.right <= rcClient.left || rcClient.bottom <= rcClient.top ) return;

		if( m_nText_W_H <= 0 ) {
			SIZE sz = ctx.GetTextSize(sGlyphs.GetData(), m_iFont, DT_SINGLELINE | DT_NOPREFIX);
			m_nText_W_H = (m_nRollDirection == ROLLTEXT_LEFT || m_nRollDirection == ROLLTEXT_RIGHT)
				? sz.cx : sz.cy;
			if( m_nText_W_H < 1 ) m_nText_W_H = 1;
		}

		CRenderClipScope clip(ctx, rcClient);

		CDuiRect rcDraw = rcClient;
		const bool bHorizontal = (m_nRollDirection == ROLLTEXT_LEFT || m_nRollDirection == ROLLTEXT_RIGHT);
		if( m_nText_W_H > 0 ) {
			int nScrollRange = 0;
			if( bHorizontal ) {
				nScrollRange = m_nText_W_H + rcClient.GetWidth();
				int x = 0;
				if( m_nRollDirection == ROLLTEXT_LEFT )
					x = rcClient.left + rcClient.GetWidth() - m_nScrollPos;
				else
					x = rcClient.left - m_nText_W_H + m_nScrollPos;
				rcDraw.left = x;
				rcDraw.right = x + m_nText_W_H;
			}
			else {
				nScrollRange = m_nText_W_H + rcClient.GetHeight();
				int y = 0;
				if( m_nRollDirection == ROLLTEXT_UP )
					y = rcClient.top + rcClient.GetHeight() - m_nScrollPos;
				else
					y = rcClient.top + m_nScrollPos;
				rcDraw.top = y;
				rcDraw.bottom = y + m_nText_W_H;
			}

			if( !m_bPaused && !m_bHoverPaused ) {
				m_nScrollPos += m_nStep;
				if( m_nScrollPos > nScrollRange ) {
					m_nScrollPos = 0;
					++m_nLoopDone;
					if( m_nLoopLimit > 0 && m_nLoopDone >= m_nLoopLimit )
						m_bPendingRollEnd = true;
				}
			}
		}

		RECT rc = rcDraw;
		UINT uTextStyle = DT_SINGLELINE | DT_VCENTER | DT_NOPREFIX | DT_LEFT;
		if( m_bShowHtml ) {
			int nLinks = 0;
			ctx.DrawHtmlText(rc, sPaint.GetData(), GetAdjustColor(dwColor), NULL, NULL, nLinks, m_iFont, uTextStyle);
		}
		else {
			ctx.DrawText(rc, sPaint.GetData(), GetAdjustColor(dwColor), m_iFont, uTextStyle);
		}
	}
}
