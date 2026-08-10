#include "StdAfx.h"
#include "UIDateTime.h"

namespace DuiLib
{
	static int DaysInMonth(int y, int m)
	{
		static const int kDays[] = { 0, 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 };
		if( m == 2 && ((y % 4 == 0 && y % 100 != 0) || (y % 400 == 0)) ) return 29;
		if( m < 1 || m > 12 ) return 30;
		return kDays[m];
	}

	static WORD DayOfWeek(int y, int m, int d)
	{
		SYSTEMTIME st = { 0 };
		st.wYear = (WORD)y;
		st.wMonth = (WORD)m;
		st.wDay = (WORD)d;
		FILETIME ft = { 0 };
		if( !::SystemTimeToFileTime(&st, &ft) ) return 0;
		SYSTEMTIME out = { 0 };
		::FileTimeToSystemTime(&ft, &out);
		return out.wDayOfWeek;
	}

	static bool SameYMD(const SYSTEMTIME& a, const SYSTEMTIME& b)
	{
		return a.wYear == b.wYear && a.wMonth == b.wMonth && a.wDay == b.wDay;
	}

	static bool SameHMS(const SYSTEMTIME& a, const SYSTEMTIME& b)
	{
		return a.wHour == b.wHour && a.wMinute == b.wMinute && a.wSecond == b.wSecond;
	}

	static void ShiftMonth(int& y, int& m, int delta)
	{
		m += delta;
		while( m < 1 ) { m += 12; --y; }
		while( m > 12 ) { m -= 12; ++y; }
	}

	static int ClampDay(int y, int m, int d)
	{
		int maxd = DaysInMonth(y, m);
		if( d < 1 ) d = 1;
		if( d > maxd ) d = maxd;
		return d;
	}

	//////////////////////////////////////////////////////////////////////////
	class CCalendarPanelUI : public CControlUI
	{
	public:
		enum ViewMode { ViewDay = 0, ViewMonth, ViewYear };

		CCalendarPanelUI(CDateTimeUI* pOwner)
			: m_pOwner(pOwner)
			, m_eView(ViewDay)
			, m_nViewYear(0)
			, m_nViewMonth(0)
			, m_nDecadeStart(0)
			, m_nHoverIndex(-1)
			, m_nHoverMonth(-1)
			, m_nHoverYear(-1)
			, m_nHoverTimePart(-1)
			, m_bPendingClose(false)
		{
			::ZeroMemory(m_rcDays, sizeof(m_rcDays));
			::ZeroMemory(m_nDayNums, sizeof(m_nDayNums));
			::ZeroMemory(m_bOtherMonth, sizeof(m_bOtherMonth));
			::ZeroMemory(m_rcMonths, sizeof(m_rcMonths));
			::ZeroMemory(m_rcYears, sizeof(m_rcYears));
			::ZeroMemory(&m_rcPrev, sizeof(m_rcPrev));
			::ZeroMemory(&m_rcNext, sizeof(m_rcNext));
			::ZeroMemory(&m_rcToday, sizeof(m_rcToday));
			::ZeroMemory(&m_rcOk, sizeof(m_rcOk));
			::ZeroMemory(&m_rcTitle, sizeof(m_rcTitle));
			::ZeroMemory(m_rcTimeBtn, sizeof(m_rcTimeBtn));
			::ZeroMemory(m_rcTimeVal, sizeof(m_rcTimeVal));
			::ZeroMemory(&m_stPending, sizeof(m_stPending));
			DWORD bg = 0xFFFFFFFF, bd = 0xD9D9D9FF;
			CThemeManager* tm = CThemeManager::GetInstance();
			if( tm != NULL ) {
				CTheme* th = tm->GetCurrentTheme();
				if( th == NULL ) th = tm->FindTheme(tm->GetDefaultThemeId());
				if( th != NULL ) {
					bg = th->GetToken(_T("color-control-bg"), th->GetToken(_T("color-bg"), bg));
					bd = th->GetToken(_T("color-control-border"), th->GetToken(_T("color-border"), bd));
				}
			}
			SetBackgroundColor(bg);
			SetBorderColor(bd);
			SetBorderWidth(1);
			SIZE szR = { 6, 6 };
			SetBorderRadius(szR);
		}

		LPCTSTR GetClass() const { return _T("CalendarPanelUI"); }

		void SetView(int y, int m)
		{
			m_nViewYear = y;
			m_nViewMonth = m;
			m_nDecadeStart = (y / 10) * 10;
			RebuildGrid();
			Invalidate();
		}

		SIZE EstimateSize(SIZE /*szAvailable*/)
		{
			SIZE sz = m_pOwner->GetDropBoxSize();
			if( m_pManager ) m_pManager->GetDPIObj()->Scale(&sz);
			return sz;
		}

		void SetPos(RECT rc, bool bNeedInvalidate = true)
		{
			CControlUI::SetPos(rc, bNeedInvalidate);
			LayoutCells();
		}

		void DoEvent(TEventUI& event)
		{
			if( !IsMouseEnabled() && event.Type > UIEVENT__MOUSEBEGIN && event.Type < UIEVENT__MOUSEEND ) {
				CControlUI::DoEvent(event);
				return;
			}

			if( event.Type == UIEVENT_SCROLLWHEEL ) {
				HandleWheel(event);
				return;
			}

			if( event.Type == UIEVENT_MOUSEMOVE ) {
				UpdateHover(event.ptMouse);
				return;
			}
			if( event.Type == UIEVENT_MOUSELEAVE ) {
				ClearHover();
				return;
			}

			if( event.Type == UIEVENT_BUTTONDOWN || event.Type == UIEVENT_DBLCLICK ) {
				HandleClick(event.ptMouse);
				return;
			}
			if( event.Type == UIEVENT_BUTTONUP ) {
				// 必须在 UP（已 ReleaseCapture）后再关弹层，DOWN 里 Destroy 会卡死
				if( m_bPendingClose ) {
					m_bPendingClose = false;
					m_pOwner->OnPicked(m_stPending, true);
				}
				return;
			}
			CControlUI::DoEvent(event);
		}

		void PaintStatusImage(IRenderContext& ctx)
		{
			LayoutChrome();
			PaintHeader(ctx);
			if( m_pOwner->IsShowDate() ) {
				if( m_eView == ViewDay ) PaintDayBody(ctx);
				else if( m_eView == ViewMonth ) PaintMonthBody(ctx);
				else PaintYearBody(ctx);
			}
			if( m_pOwner->IsShowTime() ) PaintTimeBar(ctx);
			PaintFooter(ctx);
		}

	private:
		int Scale(int v) const
		{
			return m_pManager ? m_pManager->GetDPIObj()->Scale(v) : v;
		}

		int HeaderH() const { return Scale(40); }
		int WeekH() const { return Scale(28); }
		int TimeH() const { return m_pOwner->IsShowTime() ? Scale(44) : 0; }
		int FooterH() const
		{
			bool bToday = m_pOwner->IsShowToday() && m_pOwner->IsShowDate();
			bool bOk = m_pOwner->IsShowTime();
			if( bToday || bOk ) return Scale(36);
			return 0;
		}

		void LayoutChrome()
		{
			RECT rc = m_rcItem;
			int pad = Scale(8);
			RECT rcHeader = { rc.left + pad, rc.top + Scale(4), rc.right - pad, rc.top + HeaderH() };
			m_rcPrev = rcHeader;
			m_rcPrev.right = m_rcPrev.left + Scale(28);
			m_rcNext = rcHeader;
			m_rcNext.left = m_rcNext.right - Scale(28);
			m_rcTitle = rcHeader;
			m_rcTitle.left = m_rcPrev.right;
			m_rcTitle.right = m_rcNext.left;

			bool bToday = m_pOwner->IsShowToday() && m_pOwner->IsShowDate();
			bool bOk = m_pOwner->IsShowTime();
			m_rcToday.left = m_rcToday.right = m_rcToday.top = m_rcToday.bottom = 0;
			m_rcOk.left = m_rcOk.right = m_rcOk.top = m_rcOk.bottom = 0;
			if( bToday || bOk ) {
				RECT rcFoot = { rc.left + pad, rc.bottom - Scale(34), rc.right - pad, rc.bottom - Scale(6) };
				if( bToday && bOk ) {
					int mid = (rcFoot.left + rcFoot.right) / 2;
					m_rcToday = rcFoot;
					m_rcToday.right = mid - Scale(4);
					m_rcOk = rcFoot;
					m_rcOk.left = mid + Scale(4);
				}
				else if( bToday ) {
					m_rcToday = rcFoot;
				}
				else {
					m_rcOk = rcFoot;
				}
			}

			if( m_pOwner->IsShowTime() ) {
				int bottom = (bToday || bOk) ? (rc.bottom - FooterH()) : (rc.bottom - Scale(6));
				RECT rcTime = { rc.left + pad, bottom - TimeH() + Scale(4), rc.right - pad, bottom };
				LayoutTimeBar(rcTime);
			}
		}

		void LayoutTimeBar(const RECT& rcTime)
		{
			int parts = m_pOwner->IsShowSeconds() ? 3 : 2;
			int gap = Scale(4);
			int colonW = Scale(10);
			int unitW = (rcTime.right - rcTime.left - (parts - 1) * colonW - (parts * 2) * gap) / parts;
			if( unitW < Scale(40) ) unitW = Scale(40);
			int x = rcTime.left + ((rcTime.right - rcTime.left) - (parts * unitW + (parts - 1) * colonW)) / 2;
			int btnH = Scale(14);
			int valH = rcTime.bottom - rcTime.top - btnH * 2;
			for( int i = 0; i < 3; ++i ) {
				::ZeroMemory(&m_rcTimeBtn[i][0], sizeof(RECT));
				::ZeroMemory(&m_rcTimeBtn[i][1], sizeof(RECT));
				::ZeroMemory(&m_rcTimeVal[i], sizeof(RECT));
			}
			for( int i = 0; i < parts; ++i ) {
				RECT rcU = { x, rcTime.top, x + unitW, rcTime.bottom };
				m_rcTimeBtn[i][0] = rcU;
				m_rcTimeBtn[i][0].bottom = rcU.top + btnH;
				m_rcTimeBtn[i][1] = rcU;
				m_rcTimeBtn[i][1].top = rcU.bottom - btnH;
				m_rcTimeVal[i] = rcU;
				m_rcTimeVal[i].top = m_rcTimeBtn[i][0].bottom;
				m_rcTimeVal[i].bottom = m_rcTimeBtn[i][1].top;
				x += unitW + colonW;
			}
		}

		void LayoutCells()
		{
			LayoutChrome();
			RECT rc = m_rcItem;
			int pad = Scale(8);
			int top = rc.top + HeaderH();
			int bottom = rc.bottom - FooterH() - TimeH() - Scale(4);
			if( !m_pOwner->IsShowDate() ) return;

			if( m_eView == ViewDay ) {
				top += WeekH();
				RECT rcGrid = { rc.left + pad, top, rc.right - pad, bottom };
				int cellW = (rcGrid.right - rcGrid.left) / 7;
				int cellH = (rcGrid.bottom - rcGrid.top) / 6;
				if( cellW < 1 ) cellW = 1;
				if( cellH < 1 ) cellH = 1;
				for( int r = 0; r < 6; ++r ) {
					for( int c = 0; c < 7; ++c ) {
						int i = r * 7 + c;
						RECT& d = m_rcDays[i];
						d.left = rcGrid.left + c * cellW + Scale(1);
						d.top = rcGrid.top + r * cellH + Scale(1);
						d.right = (c == 6) ? rcGrid.right : (rcGrid.left + (c + 1) * cellW - Scale(1));
						d.bottom = (r == 5) ? rcGrid.bottom : (rcGrid.top + (r + 1) * cellH - Scale(1));
					}
				}
			}
			else {
				RECT rcGrid = { rc.left + pad, top + Scale(4), rc.right - pad, bottom };
				int cols = 3, rows = 4;
				int cellW = (rcGrid.right - rcGrid.left) / cols;
				int cellH = (rcGrid.bottom - rcGrid.top) / rows;
				for( int i = 0; i < 12; ++i ) {
					int r = i / cols, c = i % cols;
					RECT rcC = {
						rcGrid.left + c * cellW + Scale(2),
						rcGrid.top + r * cellH + Scale(2),
						(c == cols - 1) ? rcGrid.right : (rcGrid.left + (c + 1) * cellW - Scale(2)),
						(r == rows - 1) ? rcGrid.bottom : (rcGrid.top + (r + 1) * cellH - Scale(2))
					};
					if( m_eView == ViewMonth ) m_rcMonths[i] = rcC;
					else m_rcYears[i] = rcC;
				}
			}
		}

		void RebuildGrid()
		{
			int first = m_pOwner->GetFirstDayOfWeek();
			int dow = (int)DayOfWeek(m_nViewYear, m_nViewMonth, 1);
			int offset = (dow - first + 7) % 7;
			int days = DaysInMonth(m_nViewYear, m_nViewMonth);
			int prevY = m_nViewYear, prevM = m_nViewMonth;
			ShiftMonth(prevY, prevM, -1);
			int prevDays = DaysInMonth(prevY, prevM);

			for( int i = 0; i < 42; ++i ) {
				m_bOtherMonth[i] = false;
				if( i < offset ) {
					m_nDayNums[i] = prevDays - offset + i + 1;
					m_bOtherMonth[i] = true;
				}
				else if( i - offset < days ) {
					m_nDayNums[i] = i - offset + 1;
				}
				else {
					m_nDayNums[i] = i - offset - days + 1;
					m_bOtherMonth[i] = true;
				}
			}
		}

		void ClearHover()
		{
			if( m_nHoverIndex >= 0 || m_nHoverMonth >= 0 || m_nHoverYear >= 0 || m_nHoverTimePart >= 0 ) {
				m_nHoverIndex = m_nHoverMonth = m_nHoverYear = m_nHoverTimePart = -1;
				Invalidate();
			}
		}

		void UpdateHover(POINT pt)
		{
			int day = -1, mon = -1, yr = -1, tp = -1;
			if( m_eView == ViewDay && m_pOwner->IsShowDate() ) {
				for( int i = 0; i < 42; ++i )
					if( ::PtInRect(&m_rcDays[i], pt) ) { day = i; break; }
			}
			else if( m_eView == ViewMonth ) {
				for( int i = 0; i < 12; ++i )
					if( ::PtInRect(&m_rcMonths[i], pt) ) { mon = i; break; }
			}
			else if( m_eView == ViewYear ) {
				for( int i = 0; i < 12; ++i )
					if( ::PtInRect(&m_rcYears[i], pt) ) { yr = i; break; }
			}
			if( m_pOwner->IsShowTime() ) {
				int parts = m_pOwner->IsShowSeconds() ? 3 : 2;
				for( int i = 0; i < parts; ++i ) {
					if( ::PtInRect(&m_rcTimeVal[i], pt) || ::PtInRect(&m_rcTimeBtn[i][0], pt) || ::PtInRect(&m_rcTimeBtn[i][1], pt) )
						tp = i;
				}
			}
			if( day != m_nHoverIndex || mon != m_nHoverMonth || yr != m_nHoverYear || tp != m_nHoverTimePart ) {
				m_nHoverIndex = day;
				m_nHoverMonth = mon;
				m_nHoverYear = yr;
				m_nHoverTimePart = tp;
				Invalidate();
			}
		}

		void NudgeTime(int part, int delta)
		{
			SYSTEMTIME st = m_pOwner->GetTime();
			if( part == 0 ) {
				int h = ((int)st.wHour + delta) % 24;
				if( h < 0 ) h += 24;
				st.wHour = (WORD)h;
			}
			else if( part == 1 ) {
				int m = ((int)st.wMinute + delta) % 60;
				if( m < 0 ) m += 60;
				st.wMinute = (WORD)m;
			}
			else {
				int s = ((int)st.wSecond + delta) % 60;
				if( s < 0 ) s += 60;
				st.wSecond = (WORD)s;
			}
			m_pOwner->OnLiveTimeChanged(st);
			Invalidate();
		}

		void HandleWheel(TEventUI& event)
		{
			bool bDown = (LOWORD(event.wParam) == SB_LINEDOWN);
			int delta = bDown ? -1 : 1;
			POINT pt = event.ptMouse;
			if( m_pOwner->IsShowTime() ) {
				int parts = m_pOwner->IsShowSeconds() ? 3 : 2;
				for( int i = 0; i < parts; ++i ) {
					if( ::PtInRect(&m_rcTimeVal[i], pt) || ::PtInRect(&m_rcTimeBtn[i][0], pt) || ::PtInRect(&m_rcTimeBtn[i][1], pt) ) {
						NudgeTime(i, delta);
						return;
					}
				}
			}
			if( m_eView == ViewDay ) {
				ShiftMonth(m_nViewYear, m_nViewMonth, delta > 0 ? -1 : 1);
				RebuildGrid();
				LayoutCells();
				Invalidate();
			}
			else if( m_eView == ViewMonth ) {
				m_nViewYear += (delta > 0 ? -1 : 1);
				Invalidate();
			}
			else {
				m_nDecadeStart += (delta > 0 ? -10 : 10);
				Invalidate();
			}
		}

		void HandleClick(POINT pt)
		{
			if( m_pOwner->IsShowDate() && ::PtInRect(&m_rcPrev, pt) ) {
				if( m_eView == ViewDay ) {
					ShiftMonth(m_nViewYear, m_nViewMonth, -1);
					RebuildGrid();
				}
				else if( m_eView == ViewMonth ) {
					--m_nViewYear;
				}
				else {
					m_nDecadeStart -= 10;
				}
				LayoutCells();
				Invalidate();
				return;
			}
			if( m_pOwner->IsShowDate() && ::PtInRect(&m_rcNext, pt) ) {
				if( m_eView == ViewDay ) {
					ShiftMonth(m_nViewYear, m_nViewMonth, 1);
					RebuildGrid();
				}
				else if( m_eView == ViewMonth ) {
					++m_nViewYear;
				}
				else {
					m_nDecadeStart += 10;
				}
				LayoutCells();
				Invalidate();
				return;
			}
			if( m_pOwner->IsShowDate() && ::PtInRect(&m_rcTitle, pt) ) {
				if( m_eView == ViewDay ) m_eView = ViewMonth;
				else if( m_eView == ViewMonth ) {
					m_eView = ViewYear;
					m_nDecadeStart = (m_nViewYear / 10) * 10;
				}
				LayoutCells();
				Invalidate();
				return;
			}

			if( m_pOwner->IsShowTime() ) {
				int parts = m_pOwner->IsShowSeconds() ? 3 : 2;
				for( int i = 0; i < parts; ++i ) {
					if( ::PtInRect(&m_rcTimeBtn[i][0], pt) ) { NudgeTime(i, 1); return; }
					if( ::PtInRect(&m_rcTimeBtn[i][1], pt) ) { NudgeTime(i, -1); return; }
				}
			}

			if( m_rcOk.right > m_rcOk.left && ::PtInRect(&m_rcOk, pt) ) {
				m_stPending = m_pOwner->GetTime();
				m_bPendingClose = true;
				return;
			}
			if( m_rcToday.right > m_rcToday.left && ::PtInRect(&m_rcToday, pt) ) {
				SYSTEMTIME now = { 0 };
				::GetLocalTime(&now);
				SYSTEMTIME st = m_pOwner->GetTime();
				st.wYear = now.wYear;
				st.wMonth = now.wMonth;
				st.wDay = now.wDay;
				st.wDayOfWeek = now.wDayOfWeek;
				if( !m_pOwner->IsShowTime() ) {
					st.wHour = now.wHour;
					st.wMinute = now.wMinute;
					st.wSecond = now.wSecond;
					st.wMilliseconds = now.wMilliseconds;
				}
				m_nViewYear = st.wYear;
				m_nViewMonth = st.wMonth;
				m_eView = ViewDay;
				RebuildGrid();
				LayoutCells();
				if( m_pOwner->IsShowTime() ) {
					m_pOwner->OnPicked(st, false);
					Invalidate();
				}
				else {
					m_stPending = st;
					m_bPendingClose = true;
					m_pOwner->OnLiveTimeChanged(st);
					Invalidate();
				}
				return;
			}

			if( m_eView == ViewMonth ) {
				for( int i = 0; i < 12; ++i ) {
					if( ::PtInRect(&m_rcMonths[i], pt) ) {
						m_nViewMonth = i + 1;
						m_eView = ViewDay;
						RebuildGrid();
						LayoutCells();
						Invalidate();
						return;
					}
				}
			}
			if( m_eView == ViewYear ) {
				for( int i = 0; i < 12; ++i ) {
					if( ::PtInRect(&m_rcYears[i], pt) ) {
						m_nViewYear = m_nDecadeStart - 1 + i;
						m_eView = ViewMonth;
						LayoutCells();
						Invalidate();
						return;
					}
				}
			}
			if( m_eView == ViewDay && m_pOwner->IsShowDate() ) {
				for( int i = 0; i < 42; ++i ) {
					if( !::PtInRect(&m_rcDays[i], pt) || m_nDayNums[i] <= 0 ) continue;
					SYSTEMTIME st = m_pOwner->GetTime();
					int y = m_nViewYear;
					int m = m_nViewMonth;
					if( m_bOtherMonth[i] ) {
						if( m_nDayNums[i] > 20 ) ShiftMonth(y, m, -1);
						else ShiftMonth(y, m, 1);
					}
					st.wYear = (WORD)y;
					st.wMonth = (WORD)m;
					st.wDay = (WORD)m_nDayNums[i];
					st.wDayOfWeek = DayOfWeek(y, m, m_nDayNums[i]);
					m_nViewYear = y;
					m_nViewMonth = m;
					RebuildGrid();
					LayoutCells();
					if( m_pOwner->IsShowTime() ) {
						m_pOwner->OnPicked(st, false);
						Invalidate();
					}
					else {
						m_stPending = st;
						m_bPendingClose = true;
						m_pOwner->OnLiveTimeChanged(st);
						Invalidate();
					}
					return;
				}
			}
		}

		void PaintHeader(IRenderContext& ctx)
		{
			if( !m_pOwner->IsShowDate() ) {
				// 仅时间：画「选择时间」标题
				RECT rc = m_rcItem;
				RECT rcT = { rc.left, rc.top + Scale(8), rc.right, rc.top + HeaderH() };
				ctx.DrawText(rcT, _T("选择时间"), GetAdjustColor(m_pOwner->m_dwHeaderColor), -1,
					DT_SINGLELINE | DT_CENTER | DT_VCENTER);
				return;
			}

			DWORD clr = m_pOwner->m_dwHeaderColor;
			ctx.DrawText(m_rcPrev, _T("<"), GetAdjustColor(clr), -1, DT_SINGLELINE | DT_CENTER | DT_VCENTER);
			ctx.DrawText(m_rcNext, _T(">"), GetAdjustColor(clr), -1, DT_SINGLELINE | DT_CENTER | DT_VCENTER);
			CDuiString sTitle;
			if( m_eView == ViewDay )
				sTitle.SmallFormat(_T("%d年%d月"), m_nViewYear, m_nViewMonth);
			else if( m_eView == ViewMonth )
				sTitle.SmallFormat(_T("%d年"), m_nViewYear);
			else
				sTitle.SmallFormat(_T("%d-%d"), m_nDecadeStart, m_nDecadeStart + 9);
			ctx.DrawText(m_rcTitle, sTitle.GetData(), GetAdjustColor(clr), -1, DT_SINGLELINE | DT_CENTER | DT_VCENTER);
		}

		void PaintDayBody(IRenderContext& ctx)
		{
			RECT rc = m_rcItem;
			int pad = Scale(8);
			static const LPCTSTR kWeekCN[] = { _T("日"), _T("一"), _T("二"), _T("三"), _T("四"), _T("五"), _T("六") };
			RECT rcWeek = { rc.left + pad, rc.top + HeaderH(), rc.right - pad, rc.top + HeaderH() + WeekH() };
			int cellW = (rcWeek.right - rcWeek.left) / 7;
			int first = m_pOwner->GetFirstDayOfWeek();
			DWORD clrMuted = m_pOwner->m_dwOtherMonthColor;
			for( int i = 0; i < 7; ++i ) {
				RECT rcC = rcWeek;
				rcC.left = rcWeek.left + i * cellW;
				rcC.right = (i == 6) ? rcWeek.right : (rcC.left + cellW);
				ctx.DrawText(rcC, kWeekCN[(first + i) % 7], GetAdjustColor(clrMuted), -1,
					DT_SINGLELINE | DT_CENTER | DT_VCENTER);
			}

			SYSTEMTIME today = { 0 };
			::GetLocalTime(&today);
			SYSTEMTIME sel = m_pOwner->GetTime();
			DWORD clrText = m_pOwner->GetDayTextColor();
			DWORD clrSelBk = m_pOwner->m_dwSelectedBk;
			DWORD clrHover = m_pOwner->m_dwHoverBk;
			DWORD clrToday = m_pOwner->m_dwTodayColor;
			DWORD clrSelText = m_pOwner->m_dwSelectedText;
			SIZE szR = { Scale(4), Scale(4) };

			for( int i = 0; i < 42; ++i ) {
				RECT rcD = m_rcDays[i];
				if( rcD.right <= rcD.left ) continue;
				int day = m_nDayNums[i];
				if( day <= 0 ) continue;

				int y = m_nViewYear, m = m_nViewMonth;
				if( m_bOtherMonth[i] ) {
					if( day > 20 ) ShiftMonth(y, m, -1);
					else ShiftMonth(y, m, 1);
				}
				SYSTEMTIME cell = { 0 };
				cell.wYear = (WORD)y;
				cell.wMonth = (WORD)m;
				cell.wDay = (WORD)day;

				bool bSel = SameYMD(cell, sel);
				bool bToday = SameYMD(cell, today);
				bool bHover = (i == m_nHoverIndex);
				if( bSel ) ctx.FillRoundRect(rcD, szR.cx, szR.cy, GetAdjustColor(clrSelBk));
				else if( bHover ) ctx.FillRoundRect(rcD, szR.cx, szR.cy, GetAdjustColor(clrHover));

				DWORD clr = clrText;
				if( m_bOtherMonth[i] ) clr = clrMuted;
				if( bSel ) clr = clrSelText;
				else if( bToday ) clr = clrToday;
				CDuiString s;
				s.SmallFormat(_T("%d"), day);
				ctx.DrawText(rcD, s.GetData(), GetAdjustColor(clr), -1, DT_SINGLELINE | DT_CENTER | DT_VCENTER);
			}
		}

		void PaintMonthBody(IRenderContext& ctx)
		{
			static const LPCTSTR kMon[] = {
				_T("一月"), _T("二月"), _T("三月"), _T("四月"),
				_T("五月"), _T("六月"), _T("七月"), _T("八月"),
				_T("九月"), _T("十月"), _T("十一月"), _T("十二月")
			};
			SYSTEMTIME sel = m_pOwner->GetTime();
			SIZE szR = { Scale(4), Scale(4) };
			for( int i = 0; i < 12; ++i ) {
				RECT rc = m_rcMonths[i];
				bool bSel = (m_nViewYear == (int)sel.wYear && (i + 1) == (int)sel.wMonth);
				bool bHover = (i == m_nHoverMonth);
				if( bSel ) ctx.FillRoundRect(rc, szR.cx, szR.cy, GetAdjustColor(m_pOwner->m_dwSelectedBk));
				else if( bHover ) ctx.FillRoundRect(rc, szR.cx, szR.cy, GetAdjustColor(m_pOwner->m_dwHoverBk));
				DWORD clr = bSel ? m_pOwner->m_dwSelectedText : m_pOwner->GetDayTextColor();
				ctx.DrawText(rc, kMon[i], GetAdjustColor(clr), -1, DT_SINGLELINE | DT_CENTER | DT_VCENTER);
			}
		}

		void PaintYearBody(IRenderContext& ctx)
		{
			SYSTEMTIME sel = m_pOwner->GetTime();
			SIZE szR = { Scale(4), Scale(4) };
			for( int i = 0; i < 12; ++i ) {
				int y = m_nDecadeStart - 1 + i;
				RECT rc = m_rcYears[i];
				bool bSel = (y == (int)sel.wYear);
				bool bHover = (i == m_nHoverYear);
				bool bOut = (y < m_nDecadeStart || y > m_nDecadeStart + 9);
				if( bSel ) ctx.FillRoundRect(rc, szR.cx, szR.cy, GetAdjustColor(m_pOwner->m_dwSelectedBk));
				else if( bHover ) ctx.FillRoundRect(rc, szR.cx, szR.cy, GetAdjustColor(m_pOwner->m_dwHoverBk));
				DWORD clr = bSel ? m_pOwner->m_dwSelectedText
					: (bOut ? m_pOwner->m_dwOtherMonthColor : m_pOwner->GetDayTextColor());
				CDuiString s;
				s.SmallFormat(_T("%d"), y);
				ctx.DrawText(rc, s.GetData(), GetAdjustColor(clr), -1, DT_SINGLELINE | DT_CENTER | DT_VCENTER);
			}
		}

		void PaintTimeBar(IRenderContext& ctx)
		{
			SYSTEMTIME st = m_pOwner->GetTime();
			int parts = m_pOwner->IsShowSeconds() ? 3 : 2;
			int vals[3] = { st.wHour, st.wMinute, st.wSecond };
			DWORD clr = m_pOwner->GetDayTextColor();
			DWORD clrBtn = m_pOwner->m_dwMutedBtnColor;
			SIZE szR = { Scale(3), Scale(3) };
			for( int i = 0; i < parts; ++i ) {
				if( i == m_nHoverTimePart )
					ctx.FillRoundRect(m_rcTimeVal[i], szR.cx, szR.cy, GetAdjustColor(m_pOwner->m_dwHoverBk));
				ctx.DrawText(m_rcTimeBtn[i][0], _T("▲"), GetAdjustColor(clrBtn), -1, DT_SINGLELINE | DT_CENTER | DT_VCENTER);
				ctx.DrawText(m_rcTimeBtn[i][1], _T("▼"), GetAdjustColor(clrBtn), -1, DT_SINGLELINE | DT_CENTER | DT_VCENTER);
				CDuiString s;
				s.SmallFormat(_T("%02d"), vals[i]);
				ctx.DrawText(m_rcTimeVal[i], s.GetData(), GetAdjustColor(clr), -1, DT_SINGLELINE | DT_CENTER | DT_VCENTER);
				if( i + 1 < parts ) {
					RECT rcColon = m_rcTimeVal[i];
					rcColon.left = m_rcTimeVal[i].right;
					rcColon.right = m_rcTimeVal[i + 1].left;
					ctx.DrawText(rcColon, _T(":"), GetAdjustColor(clr), -1, DT_SINGLELINE | DT_CENTER | DT_VCENTER);
				}
			}
		}

		void PaintFooter(IRenderContext& ctx)
		{
			DWORD clr = m_pOwner->m_dwTodayColor;
			if( m_rcToday.right > m_rcToday.left )
				ctx.DrawText(m_rcToday, _T("今天"), GetAdjustColor(clr), -1, DT_SINGLELINE | DT_CENTER | DT_VCENTER);
			if( m_rcOk.right > m_rcOk.left )
				ctx.DrawText(m_rcOk, _T("确定"), GetAdjustColor(clr), -1, DT_SINGLELINE | DT_CENTER | DT_VCENTER);
		}

		CDateTimeUI* m_pOwner;
		ViewMode m_eView;
		int m_nViewYear;
		int m_nViewMonth;
		int m_nDecadeStart;
		int m_nHoverIndex;
		int m_nHoverMonth;
		int m_nHoverYear;
		int m_nHoverTimePart;
		RECT m_rcPrev, m_rcNext, m_rcTitle, m_rcToday, m_rcOk;
		RECT m_rcDays[42];
		int m_nDayNums[42];
		bool m_bOtherMonth[42];
		RECT m_rcMonths[12];
		RECT m_rcYears[12];
		RECT m_rcTimeBtn[3][2];
		RECT m_rcTimeVal[3];
		bool m_bPendingClose;
		SYSTEMTIME m_stPending;
	};

	//////////////////////////////////////////////////////////////////////////
	class CDateTimeWnd : public CWindowWnd
	{
	public:
		CDateTimeWnd() : m_pOwner(NULL), m_pPanel(NULL) {}

		void Init(CDateTimeUI* pOwner)
		{
			m_pOwner = pOwner;
			SIZE sz = pOwner->GetDropBoxSize();
			if( pOwner->GetManager() )
				pOwner->GetManager()->GetDPIObj()->Scale(&sz);

			RECT rcOwner = pOwner->GetPos();
			RECT rc = rcOwner;
			rc.top = rc.bottom + 2;
			rc.bottom = rc.top + sz.cy;
			rc.right = rc.left + sz.cx;
			::MapWindowRect(pOwner->GetManager()->GetPaintWindow(), HWND_DESKTOP, &rc);

			MONITORINFO oMonitor = {};
			oMonitor.cbSize = sizeof(oMonitor);
			::GetMonitorInfo(::MonitorFromWindow(pOwner->GetManager()->GetPaintWindow(), MONITOR_DEFAULTTONEAREST), &oMonitor);
			CDuiRect rcWork = oMonitor.rcWork;
			if( rc.bottom > rcWork.bottom ) {
				RECT rcAbove = rcOwner;
				::MapWindowRect(pOwner->GetManager()->GetPaintWindow(), HWND_DESKTOP, &rcAbove);
				rc.bottom = rcAbove.top - 2;
				rc.top = rc.bottom - sz.cy;
				if( rc.top < rcWork.top ) {
					rc.top = rcWork.top + 4;
					rc.bottom = rc.top + sz.cy;
				}
			}
			if( rc.right > rcWork.right ) {
				int w = rc.right - rc.left;
				rc.right = rcWork.right - 4;
				rc.left = rc.right - w;
			}
			if( rc.left < rcWork.left ) {
				int w = rc.right - rc.left;
				rc.left = rcWork.left + 4;
				rc.right = rc.left + w;
			}

			Create(pOwner->GetManager()->GetPaintWindow(), NULL, WS_POPUP, WS_EX_TOOLWINDOW, rc);
			HWND hWndParent = m_hWnd;
			while( ::GetParent(hWndParent) != NULL ) hWndParent = ::GetParent(hWndParent);
			::ShowWindow(m_hWnd, SW_SHOW);
			::SendMessage(hWndParent, WM_NCACTIVATE, TRUE, 0L);
		}

		LPCTSTR GetWindowClassName() const { return _T("DateTimeWnd"); }

		void OnFinalMessage(HWND /*hWnd*/)
		{
			if( m_pOwner ) {
				m_pOwner->m_pWindow = NULL;
				m_pOwner->Invalidate();
			}
			delete this;
		}

		LRESULT HandleMessage(UINT uMsg, WPARAM wParam, LPARAM lParam)
		{
			if( uMsg == WM_CREATE ) {
				m_pm.SetForceUseSharedRes(true);
				m_pm.Init(m_hWnd);
				m_pm.SetLayered(true);
				if( m_pOwner && m_pOwner->GetManager() )
					m_pm.GetDPIObj()->SetScale(m_pOwner->GetManager()->GetDPIObj()->GetDPI());

				m_pPanel = new CCalendarPanelUI(m_pOwner);
				m_pPanel->SetManager(&m_pm, NULL, true);
				SYSTEMTIME st = m_pOwner->GetTime();
				m_pPanel->SetView(st.wYear, st.wMonth);

				CShadowUI* pShadow = m_pOwner->GetManager()->GetShadow();
				if( pShadow ) {
					pShadow->CopyShadow(m_pm.GetShadow());
					m_pm.GetShadow()->ShowShadow(pShadow->IsShowShadow());
				}
				m_pm.AttachDialog(m_pPanel);
				return 0;
			}
			else if( uMsg == WM_CLOSE ) {
				if( m_pOwner ) m_pOwner->SetFocus();
			}
			else if( uMsg == WM_KEYDOWN ) {
				if( wParam == VK_ESCAPE ) {
					PostMessage(WM_CLOSE);
					return 0;
				}
				if( wParam == VK_RETURN && m_pOwner && m_pOwner->IsShowTime() ) {
					// 键盘确认：先放掉捕获再关
					::ReleaseCapture();
					m_pOwner->OnPicked(m_pOwner->GetTime(), true);
					return 0;
				}
			}
			else if( uMsg == WM_KILLFOCUS ) {
				if( m_hWnd != (HWND)wParam ) {
					::ReleaseCapture();
					PostMessage(WM_CLOSE);
				}
			}

			LRESULT lRes = 0;
			if( m_pm.MessageHandler(uMsg, wParam, lParam, lRes) ) return lRes;
			return CWindowWnd::HandleMessage(uMsg, wParam, lParam);
		}

		void SyncThemeShell()
		{
			if( m_pPanel == NULL ) return;
			DWORD bg = 0xFFFFFFFF, bd = 0xD9D9D9FF;
			CThemeManager* tm = CThemeManager::GetInstance();
			if( tm != NULL ) {
				CTheme* th = tm->GetCurrentTheme();
				if( th == NULL ) th = tm->FindTheme(tm->GetDefaultThemeId());
				if( th != NULL ) {
					bg = th->GetToken(_T("color-control-bg"), th->GetToken(_T("color-bg"), bg));
					bd = th->GetToken(_T("color-control-border"), th->GetToken(_T("color-border"), bd));
				}
			}
			m_pPanel->SetBackgroundColor(bg);
			m_pPanel->SetBorderColor(bd);
			m_pPanel->Invalidate();
			m_pm.NeedUpdate();
		}

	private:
		CPaintManagerUI m_pm;
		CDateTimeUI* m_pOwner;
		CCalendarPanelUI* m_pPanel;
	};

	//////////////////////////////////////////////////////////////////////////
	IMPLEMENT_DUICONTROL(CDateTimeUI)

	CDateTimeUI::CDateTimeUI()
		: m_bReadOnly(false)
		, m_bShowToday(true)
		, m_bShowTime(false)
		, m_bShowSeconds(true)
		, m_bShowDate(true)
		, m_bFormatDrivesFlags(true)
		, m_nFirstDayOfWeek(0)
		, m_pWindow(NULL)
		, m_dwSelectedBk(0x1677FFFF)
		, m_dwHoverBk(0xE6F4FFFF)
		, m_dwTodayColor(0x1677FFFF)
		, m_dwOtherMonthColor(0xBFBFBFFF)
		, m_dwHeaderColor(0x333333FF)
		, m_dwDayColor(0x333333FF)
		, m_dwSelectedText(0xFFFFFFFF)
		, m_dwMutedBtnColor(0x8C8C8CFF)
	{
		::GetLocalTime(&m_sysTime);
		m_sFormat = _T("yyyy-MM-dd");
		SetCursor(DUI_HAND);
		SetKind(CONTROLKIND_NONE);
		SyncFormatFlags();
		UpdateText();
	}

	CDateTimeUI::~CDateTimeUI()
	{
		CloseDropDown();
	}

	LPCTSTR CDateTimeUI::GetClass() const
	{
		return _T("DateTimeUI");
	}

	LPVOID CDateTimeUI::GetInterface(LPCTSTR pstrName)
	{
		if( _tcsicmp(pstrName, DUI_CTR_DATETIME) == 0 ) return static_cast<CDateTimeUI*>(this);
		return CLabelUI::GetInterface(pstrName);
	}

	UINT CDateTimeUI::GetControlFlags() const
	{
		return UIFLAG_SETCURSOR;
	}

	bool CDateTimeUI::PreferClientHit() const
	{
		return true;
	}

	SYSTEMTIME& CDateTimeUI::GetTime()
	{
		return m_sysTime;
	}

	void CDateTimeUI::SetTime(const SYSTEMTIME& st)
	{
		m_sysTime = st;
		m_sysTime.wDay = (WORD)ClampDay(st.wYear, st.wMonth, st.wDay);
		UpdateText();
		Invalidate();
	}

	void CDateTimeUI::SetTime(SYSTEMTIME* pst)
	{
		if( pst ) SetTime(*pst);
	}

	void CDateTimeUI::SetReadOnly(bool bReadOnly)
	{
		m_bReadOnly = bReadOnly;
		Invalidate();
	}

	bool CDateTimeUI::IsReadOnly() const
	{
		return m_bReadOnly;
	}

	void CDateTimeUI::SyncFormatFlags()
	{
		if( !m_bFormatDrivesFlags ) return;

		if( m_sFormat.CompareNoCase(_T("yyyy-MM-dd")) == 0 ) {
			m_bShowDate = true; m_bShowTime = false; m_bShowSeconds = false;
		}
		else if( m_sFormat.CompareNoCase(_T("HH:mm:ss")) == 0 ) {
			m_bShowDate = false; m_bShowTime = true; m_bShowSeconds = true;
		}
		else if( m_sFormat.CompareNoCase(_T("HH:mm")) == 0 ) {
			m_bShowDate = false; m_bShowTime = true; m_bShowSeconds = false;
		}
		else if( m_sFormat.CompareNoCase(_T("yyyy-MM-dd HH:mm:ss")) == 0 ) {
			m_bShowDate = true; m_bShowTime = true; m_bShowSeconds = true;
		}
		else if( m_sFormat.CompareNoCase(_T("yyyy-MM-dd HH:mm")) == 0 ) {
			m_bShowDate = true; m_bShowTime = true; m_bShowSeconds = false;
		}
		else {
			bool bDate = (m_sFormat.Find(_T("yyyy")) >= 0 || m_sFormat.Find(_T("MM")) >= 0 || m_sFormat.Find(_T("dd")) >= 0);
			bool bTime = (m_sFormat.Find(_T("HH")) >= 0);
			bool bSec = (m_sFormat.Find(_T("ss")) >= 0 || m_sFormat.Find(_T("SS")) >= 0);
			if( !bDate && !bTime ) bDate = true;
			if( bTime && m_sFormat.Find(_T("mm")) >= 0 ) { /* minutes present */ }
			m_bShowDate = bDate;
			m_bShowTime = bTime;
			m_bShowSeconds = bSec;
		}
	}

	void CDateTimeUI::SetFormat(LPCTSTR pstrFormat)
	{
		m_sFormat = pstrFormat ? pstrFormat : _T("yyyy-MM-dd");
		if( m_bFormatDrivesFlags ) SyncFormatFlags();
		UpdateText();
		Invalidate();
	}

	CDuiString CDateTimeUI::GetFormat() const
	{
		return m_sFormat;
	}

	void CDateTimeUI::SetShowToday(bool bShow)
	{
		m_bShowToday = bShow;
	}

	bool CDateTimeUI::IsShowToday() const
	{
		return m_bShowToday;
	}

	void CDateTimeUI::SetShowTime(bool bShow)
	{
		m_bFormatDrivesFlags = false;
		m_bShowTime = bShow;
		if( bShow && !m_bShowDate && !m_bShowSeconds ) m_bShowSeconds = true;
	}

	bool CDateTimeUI::IsShowTime() const
	{
		return m_bShowTime;
	}

	void CDateTimeUI::SetShowSeconds(bool bShow)
	{
		m_bFormatDrivesFlags = false;
		m_bShowSeconds = bShow;
	}

	bool CDateTimeUI::IsShowSeconds() const
	{
		return m_bShowSeconds && m_bShowTime;
	}

	bool CDateTimeUI::IsShowDate() const
	{
		return m_bShowDate;
	}

	void CDateTimeUI::SetFirstDayOfWeek(int nFirst)
	{
		if( nFirst < 0 ) nFirst = 0;
		if( nFirst > 6 ) nFirst = 6;
		m_nFirstDayOfWeek = nFirst;
	}

	int CDateTimeUI::GetFirstDayOfWeek() const
	{
		return m_nFirstDayOfWeek;
	}

	SIZE CDateTimeUI::GetDropBoxSize() const
	{
		SIZE sz = { 280, 0 };
		if( m_bShowDate ) {
			sz.cy = 272;
			if( m_bShowToday ) sz.cy = 308;
		}
		else {
			sz.cx = 220;
			sz.cy = 120;
		}
		if( m_bShowTime ) {
			sz.cy += 44;
			if( m_bShowDate ) sz.cy += 0; // footer 已含确定行（替换/并列今天）
			else sz.cy += 36; // 仅时间：确定
			if( !m_bShowDate ) { /* already */ }
			else if( !m_bShowToday ) sz.cy += 36; // 无今天时补确定行
		}
		return sz;
	}

	bool CDateTimeUI::IsDropDownOpened() const
	{
		return m_pWindow != NULL;
	}

	void CDateTimeUI::ActivateDropDown()
	{
		if( !IsEnabled() || m_bReadOnly ) return;
		if( m_pWindow != NULL ) {
			CloseDropDown();
			return;
		}
		m_pWindow = new CDateTimeWnd();
		m_pWindow->Init(this);
	}

	void CDateTimeUI::CloseDropDown()
	{
		if( m_pWindow != NULL && m_pWindow->GetHWND() != NULL ) {
			::ReleaseCapture();
			::PostMessage(m_pWindow->GetHWND(), WM_CLOSE, 0, 0);
		}
	}

	void CDateTimeUI::SyncOpenCalendarShell()
	{
		if( m_pWindow == NULL ) return;
		m_pWindow->SyncThemeShell();
	}

	void CDateTimeUI::OnLiveTimeChanged(const SYSTEMTIME& st)
	{
		m_sysTime = st;
		UpdateText();
		Invalidate();
	}

	void CDateTimeUI::OnPicked(const SYSTEMTIME& st, bool bClose)
	{
		bool bChanged = !SameYMD(m_sysTime, st) || !SameHMS(m_sysTime, st);
		m_sysTime = st;
		UpdateText();
		Invalidate();
		if( bChanged && m_pManager )
			m_pManager->SendNotify(this, DUI_MSGTYPE_VALUECHANGED, 0, 0, true);
		if( bClose ) CloseDropDown();
	}

	int CDateTimeUI::ScaleValue(int v) const
	{
		return m_pManager ? m_pManager->GetDPIObj()->Scale(v) : v;
	}

	void CDateTimeUI::UpdateText()
	{
		CDuiString sText;
		if( m_sFormat.CompareNoCase(_T("yyyy-MM-dd")) == 0 ) {
			sText.SmallFormat(_T("%04d-%02d-%02d"), m_sysTime.wYear, m_sysTime.wMonth, m_sysTime.wDay);
		}
		else if( m_sFormat.CompareNoCase(_T("HH:mm:ss")) == 0 ) {
			sText.SmallFormat(_T("%02d:%02d:%02d"), m_sysTime.wHour, m_sysTime.wMinute, m_sysTime.wSecond);
		}
		else if( m_sFormat.CompareNoCase(_T("HH:mm")) == 0 ) {
			sText.SmallFormat(_T("%02d:%02d"), m_sysTime.wHour, m_sysTime.wMinute);
		}
		else if( m_sFormat.CompareNoCase(_T("yyyy-MM-dd HH:mm:ss")) == 0 ) {
			sText.SmallFormat(_T("%04d-%02d-%02d %02d:%02d:%02d"),
				m_sysTime.wYear, m_sysTime.wMonth, m_sysTime.wDay,
				m_sysTime.wHour, m_sysTime.wMinute, m_sysTime.wSecond);
		}
		else if( m_sFormat.CompareNoCase(_T("yyyy-MM-dd HH:mm")) == 0 ) {
			sText.SmallFormat(_T("%04d-%02d-%02d %02d:%02d"),
				m_sysTime.wYear, m_sysTime.wMonth, m_sysTime.wDay,
				m_sysTime.wHour, m_sysTime.wMinute);
		}
		else if( m_bShowDate && m_bShowTime ) {
			if( m_bShowSeconds )
				sText.SmallFormat(_T("%04d-%02d-%02d %02d:%02d:%02d"),
					m_sysTime.wYear, m_sysTime.wMonth, m_sysTime.wDay,
					m_sysTime.wHour, m_sysTime.wMinute, m_sysTime.wSecond);
			else
				sText.SmallFormat(_T("%04d-%02d-%02d %02d:%02d"),
					m_sysTime.wYear, m_sysTime.wMonth, m_sysTime.wDay,
					m_sysTime.wHour, m_sysTime.wMinute);
		}
		else if( m_bShowTime ) {
			if( m_bShowSeconds )
				sText.SmallFormat(_T("%02d:%02d:%02d"), m_sysTime.wHour, m_sysTime.wMinute, m_sysTime.wSecond);
			else
				sText.SmallFormat(_T("%02d:%02d"), m_sysTime.wHour, m_sysTime.wMinute);
		}
		else {
			sText.SmallFormat(_T("%04d-%02d-%02d"), m_sysTime.wYear, m_sysTime.wMonth, m_sysTime.wDay);
		}
		SetText(sText.GetData());
	}

	SIZE CDateTimeUI::EstimateSize(SIZE szAvailable)
	{
		SIZE sz = CLabelUI::EstimateSize(szAvailable);
		int minW = m_bShowTime ? 180 : 140;
		if( GetFixedWidth() <= 0 && sz.cx < ScaleValue(minW) ) sz.cx = ScaleValue(minW);
		if( GetFixedHeight() <= 0 && sz.cy < ScaleValue(28) ) sz.cy = ScaleValue(28);
		return sz;
	}

	void CDateTimeUI::DoEvent(TEventUI& event)
	{
		if( !IsMouseEnabled() && event.Type > UIEVENT__MOUSEBEGIN && event.Type < UIEVENT__MOUSEEND ) {
			if( !BubbleEvent(event) ) CLabelUI::DoEvent(event);
			return;
		}

		if( event.Type == UIEVENT_BUTTONDOWN || event.Type == UIEVENT_DBLCLICK ) {
			if( IsEnabled() && !m_bReadOnly ) ActivateDropDown();
			return;
		}
		if( event.Type == UIEVENT_KEYDOWN ) {
			if( event.chKey == VK_SPACE || event.chKey == VK_RETURN ) {
				ActivateDropDown();
				return;
			}
			if( event.chKey == VK_ESCAPE && m_pWindow ) {
				CloseDropDown();
				return;
			}
		}

		CLabelUI::DoEvent(event);
	}

	void CDateTimeUI::PaintStatusImage(IRenderContext& ctx)
	{
		RECT rc = m_rcItem;
		RECT rcArrow = { rc.right - ScaleValue(18), rc.top, rc.right - ScaleValue(6), rc.bottom };
		DWORD clr = IsEnabled() ? m_dwMutedBtnColor : m_dwOtherMonthColor;
		ctx.DrawText(rcArrow, _T("▼"), GetAdjustColor(clr), -1, DT_SINGLELINE | DT_CENTER | DT_VCENTER);
	}

	DWORD CDateTimeUI::GetDayTextColor() const
	{
		DWORD c = GetColor();
		if( c != 0 ) return c;
		return m_dwDayColor;
	}

	void CDateTimeUI::SetAttribute(LPCTSTR pstrName, LPCTSTR pstrValue)
	{
		if( _tcsicmp(pstrName, _T("format")) == 0 || _tcsicmp(pstrName, _T("timeformat")) == 0 ) {
			SetFormat(pstrValue);
		}
		else if( _tcsicmp(pstrName, _T("readonly")) == 0 ) {
			SetReadOnly(_tcsicmp(pstrValue, _T("true")) == 0);
		}
		else if( _tcsicmp(pstrName, _T("show-today")) == 0 ) {
			SetShowToday(_tcsicmp(pstrValue, _T("true")) == 0);
		}
		else if( _tcsicmp(pstrName, _T("show-time")) == 0 ) {
			SetShowTime(_tcsicmp(pstrValue, _T("true")) == 0);
			if( IsShowTime() && !IsShowDate() ) { /* time-only ok */ }
		}
		else if( _tcsicmp(pstrName, _T("show-seconds")) == 0 ) {
			SetShowSeconds(_tcsicmp(pstrValue, _T("true")) == 0);
		}
		else if( _tcsicmp(pstrName, _T("show-date")) == 0 ) {
			m_bFormatDrivesFlags = false;
			m_bShowDate = (_tcsicmp(pstrValue, _T("true")) == 0);
		}
		else if( _tcsicmp(pstrName, _T("first-day-of-week")) == 0 ) {
			if( _tcsicmp(pstrValue, _T("mon")) == 0 || _tcsicmp(pstrValue, _T("monday")) == 0 )
				SetFirstDayOfWeek(1);
			else if( _tcsicmp(pstrValue, _T("sun")) == 0 || _tcsicmp(pstrValue, _T("sunday")) == 0 )
				SetFirstDayOfWeek(0);
			else
				SetFirstDayOfWeek(_ttoi(pstrValue));
		}
		else if( _tcsicmp(pstrName, _T("selected-background-color")) == 0 ) {
			DWORD clr = 0;
			if( ParseColorString(pstrValue, clr) ) m_dwSelectedBk = clr;
		}
		else if( _tcsicmp(pstrName, _T("day-hover-background-color")) == 0 ) {
			DWORD clr = 0;
			if( ParseColorString(pstrValue, clr) ) m_dwHoverBk = clr;
		}
		else if( _tcsicmp(pstrName, _T("today-color")) == 0 ) {
			DWORD clr = 0;
			if( ParseColorString(pstrValue, clr) ) m_dwTodayColor = clr;
		}
		else if( _tcsicmp(pstrName, _T("other-month-color")) == 0 ) {
			DWORD clr = 0;
			if( ParseColorString(pstrValue, clr) ) m_dwOtherMonthColor = clr;
		}
		else if( _tcsicmp(pstrName, _T("header-color")) == 0 ) {
			DWORD clr = 0;
			if( ParseColorString(pstrValue, clr) ) m_dwHeaderColor = clr;
		}
		else if( _tcsicmp(pstrName, _T("day-color")) == 0 ) {
			DWORD clr = 0;
			if( ParseColorString(pstrValue, clr) ) m_dwDayColor = clr;
		}
		else if( _tcsicmp(pstrName, _T("selected-color")) == 0 ) {
			DWORD clr = 0;
			if( ParseColorString(pstrValue, clr) ) m_dwSelectedText = clr;
		}
		else if( _tcsicmp(pstrName, _T("muted-color")) == 0 || _tcsicmp(pstrName, _T("arrow-color")) == 0 ) {
			DWORD clr = 0;
			if( ParseColorString(pstrValue, clr) ) m_dwMutedBtnColor = clr;
		}
		else {
			CLabelUI::SetAttribute(pstrName, pstrValue);
		}
	}
}
