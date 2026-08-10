#include "StdAfx.h"
#include "UIBadge.h"

namespace DuiLib
{
	//////////////////////////////////////////////////////////////////////////
	IMPLEMENT_DUICONTROL(CTagUI)

	CTagUI::CTagUI()
		: m_bClosable(false)
		, m_bCloseHover(false)
		, m_eStatus(StatusDefault)
		, m_nCloseSize(14)
	{
		SetKind(CONTROLKIND_NONE);
		SetTextStyle(DT_SINGLELINE | DT_VCENTER | DT_LEFT | DT_END_ELLIPSIS);
		SetPadding(CDuiBox(0, 8, 0, 8));
		SetAutoCalcWidth(true);
		SetAutoCalcHeight(false);
		ApplyStatusColors();
		SIZE sz = { 4, 4 };
		SetBorderRadius(sz);
	}

	LPCTSTR CTagUI::GetClass() const { return _T("TagUI"); }

	LPVOID CTagUI::GetInterface(LPCTSTR pstrName)
	{
		if( _tcsicmp(pstrName, DUI_CTR_TAG) == 0 ) return static_cast<CTagUI*>(this);
		return CLabelUI::GetInterface(pstrName);
	}

	UINT CTagUI::GetControlFlags() const
	{
		UINT f = CLabelUI::GetControlFlags();
		if( m_bClosable ) f |= UIFLAG_SETCURSOR;
		return f;
	}

	int CTagUI::ScaleValue(int v) const
	{
		return m_pManager ? m_pManager->GetDPIObj()->Scale(v) : v;
	}

	void CTagUI::SetClosable(bool b)
	{
		m_bClosable = b;
		NeedUpdate();
		Invalidate();
	}

	bool CTagUI::IsClosable() const { return m_bClosable; }

	void CTagUI::SetStatus(Status e)
	{
		m_eStatus = e;
		ApplyStatusColors();
		Invalidate();
	}

	CTagUI::Status CTagUI::GetStatus() const { return m_eStatus; }

	void CTagUI::ApplyStatusColors()
	{
		DWORD bg = 0xFAFAFAFF, fg = 0x000000E0, bd = 0xD9D9D9FF;
		CThemeManager* tm = CThemeManager::GetInstance();
		CTheme* th = NULL;
		if( tm != NULL ) {
			CDuiString mode;
			tm->ResolveEffectiveTheme(this, mode, &th);
			if( th == NULL ) th = tm->GetCurrentTheme();
			if( th == NULL ) th = tm->FindTheme(tm->GetDefaultThemeId());
		}
		DWORD baseBg = th ? th->GetToken(_T("color-bg-elevated"), 0xFAFAFAFF) : 0xFAFAFAFF;
		DWORD text = th ? th->GetToken(_T("color-text"), 0x000000E0) : 0x000000E0;
		DWORD border = th ? th->GetToken(_T("color-border"), 0xD9D9D9FF) : 0xD9D9D9FF;
		DWORD primary = th ? th->GetToken(_T("color-primary"), 0x1677FFFF) : 0x1677FFFF;
		DWORD success = th ? th->GetToken(_T("color-success"), 0x52C41AFF) : 0x52C41AFF;
		DWORD danger = th ? th->GetToken(_T("color-danger"), 0xFF4D4FFF) : 0xFF4D4FFF;
		DWORD warning = th ? th->GetToken(_T("color-warning"), 0xFAAD14FF) : 0xFAAD14FF;

		auto soft = [](DWORD accent, DWORD base) -> DWORD {
			const int w = 36, iw = 255 - w;
			return DuiColorFromRGB(
				(BYTE)((DuiColorR(accent) * w + DuiColorR(base) * iw) / 255),
				(BYTE)((DuiColorG(accent) * w + DuiColorG(base) * iw) / 255),
				(BYTE)((DuiColorB(accent) * w + DuiColorB(base) * iw) / 255),
				0xFF);
		};
		auto softBd = [](DWORD accent, DWORD base) -> DWORD {
			const int w = 90, iw = 255 - w;
			return DuiColorFromRGB(
				(BYTE)((DuiColorR(accent) * w + DuiColorR(base) * iw) / 255),
				(BYTE)((DuiColorG(accent) * w + DuiColorG(base) * iw) / 255),
				(BYTE)((DuiColorB(accent) * w + DuiColorB(base) * iw) / 255),
				0xFF);
		};

		switch( m_eStatus ) {
		case StatusSuccess:
			fg = success; bg = soft(success, baseBg); bd = softBd(success, border); break;
		case StatusProcessing:
			fg = primary; bg = soft(primary, baseBg); bd = softBd(primary, border); break;
		case StatusError:
			fg = danger; bg = soft(danger, baseBg); bd = softBd(danger, border); break;
		case StatusWarning:
			fg = warning; bg = soft(warning, baseBg); bd = softBd(warning, border); break;
		default:
			fg = text; bg = baseBg; bd = border; break;
		}
		SetBackgroundColor(bg);
		SetColor(fg);
		SetBorderColor(bd);
		SetBorderWidth(1);
	}

	RECT CTagUI::GetCloseRect() const
	{
		RECT rc = { 0, 0, 0, 0 };
		if( !m_bClosable ) return rc;
		int sz = ScaleValue(m_nCloseSize);
		CDuiBox pad = GetPadding();
		rc.right = m_rcItem.right - pad.right;
		rc.left = rc.right - sz;
		rc.top = m_rcItem.top + (m_rcItem.bottom - m_rcItem.top - sz) / 2;
		rc.bottom = rc.top + sz;
		return rc;
	}

	int CTagUI::HitClose(POINT pt) const
	{
		if( !m_bClosable ) return 0;
		RECT rc = GetCloseRect();
		return ::PtInRect(&rc, pt) ? 1 : 0;
	}

	SIZE CTagUI::EstimateSize(SIZE szAvailable)
	{
		SIZE sz = CLabelUI::EstimateSize(szAvailable);
		if( sz.cy <= 0 ) sz.cy = ScaleValue(22);
		if( GetAutoCalcWidth() || sz.cx <= 0 ) {
			CDuiString s = GetText();
			RECT rcText = { 0, 0, 9999, sz.cy };
			RenderMeasureText(m_pManager, rcText, s.GetData(), 0, GetFont(),
				DT_CALCRECT | DT_SINGLELINE | DT_VCENTER);
			CDuiBox pad = GetPadding();
			sz.cx = (rcText.right - rcText.left) + pad.left + pad.right;
			if( m_bClosable ) sz.cx += ScaleValue(m_nCloseSize) + ScaleValue(4);
			if( sz.cx < ScaleValue(28) ) sz.cx = ScaleValue(28);
		}
		return sz;
	}

	void CTagUI::DoEvent(TEventUI& event)
	{
		if( m_bClosable && IsEnabled() ) {
			if( event.Type == UIEVENT_SETCURSOR ) {
				if( HitClose(event.ptMouse) )
					::SetCursor(::LoadCursor(NULL, MAKEINTRESOURCE(IDC_HAND)));
				else
					::SetCursor(::LoadCursor(NULL, MAKEINTRESOURCE(IDC_ARROW)));
				return;
			}
			if( event.Type == UIEVENT_MOUSEMOVE ) {
				bool h = HitClose(event.ptMouse) != 0;
				if( h != m_bCloseHover ) { m_bCloseHover = h; Invalidate(); }
			}
			if( event.Type == UIEVENT_MOUSELEAVE ) {
				if( m_bCloseHover ) { m_bCloseHover = false; Invalidate(); }
			}
			if( event.Type == UIEVENT_BUTTONDOWN || event.Type == UIEVENT_DBLCLICK ) {
				if( HitClose(event.ptMouse) ) {
					if( m_pManager ) m_pManager->SendNotify(this, _T("close"));
					SetVisible(false);
					return;
				}
			}
		}
		CLabelUI::DoEvent(event);
	}

	void CTagUI::PaintBackgroundColor(IRenderContext& ctx)
	{
		DWORD clr = GetPaintBackgroundColor();
		if( clr == 0 ) return;
		SIZE br = GetBorderRadius();
		int rx = br.cx > 0 ? br.cx : ScaleValue(4);
		int ry = br.cy > 0 ? br.cy : rx;
		ctx.FillRoundRect(m_rcItem, rx, ry, GetAdjustColor(clr));
	}

	void CTagUI::PaintBorder(IRenderContext& ctx)
	{
		DWORD clr = GetBorderColor();
		int n = GetBorderWidth();
		if( clr == 0 || n <= 0 ) return;
		SIZE br = GetBorderRadius();
		int rx = br.cx > 0 ? br.cx : ScaleValue(4);
		int ry = br.cy > 0 ? br.cy : rx;
		ctx.DrawRoundRect(m_rcItem, n, rx, ry, GetAdjustColor(clr), PS_SOLID);
	}

	void CTagUI::PaintText(IRenderContext& ctx)
	{
		RECT rc = m_rcItem;
		CDuiBox pad = GetPadding();
		RECT rcTextPad = GetTextPadding();
		rc.left += pad.left + rcTextPad.left;
		rc.right -= pad.right + rcTextPad.right;
		rc.top += pad.top + rcTextPad.top;
		rc.bottom -= pad.bottom + rcTextPad.bottom;
		if( m_bClosable )
			rc.right -= ScaleValue(m_nCloseSize) + ScaleValue(2);

		CDuiString sText = GetText();
		if( !sText.IsEmpty() ) {
			DWORD clr = IsEnabled() ? GetColor() : GetDisabledColor();
			if( clr == 0 && m_pManager ) clr = m_pManager->GetDefaultFontColor();
			ctx.DrawText(rc, sText.GetData(), GetAdjustColor(clr), GetFont(),
				DT_SINGLELINE | DT_VCENTER | DT_LEFT | DT_END_ELLIPSIS);
		}

		if( m_bClosable ) {
			RECT rcClose = GetCloseRect();
			DWORD clrX = m_bCloseHover ? GetColor() : 0x00000073;
			if( GetColor() != 0 && m_bCloseHover ) clrX = GetColor();
			ctx.DrawText(rcClose, _T("×"), GetAdjustColor(clrX), GetFont(),
				DT_SINGLELINE | DT_CENTER | DT_VCENTER);
		}
	}

	void CTagUI::SetAttribute(LPCTSTR pstrName, LPCTSTR pstrValue)
	{
		if( _tcsicmp(pstrName, _T("closable")) == 0 || _tcsicmp(pstrName, _T("closeable")) == 0 ) {
			SetClosable(_tcsicmp(pstrValue, _T("true")) == 0);
		}
		else if( _tcsicmp(pstrName, _T("status")) == 0 ) {
			if( _tcsicmp(pstrValue, _T("success")) == 0 ) SetStatus(StatusSuccess);
			else if( _tcsicmp(pstrValue, _T("processing")) == 0 || _tcsicmp(pstrValue, _T("info")) == 0 )
				SetStatus(StatusProcessing);
			else if( _tcsicmp(pstrValue, _T("error")) == 0 || _tcsicmp(pstrValue, _T("danger")) == 0 )
				SetStatus(StatusError);
			else if( _tcsicmp(pstrValue, _T("warning")) == 0 || _tcsicmp(pstrValue, _T("warn")) == 0 )
				SetStatus(StatusWarning);
			else SetStatus(StatusDefault);
		}
		else {
			CLabelUI::SetAttribute(pstrName, pstrValue);
		}
	}

	//////////////////////////////////////////////////////////////////////////
	IMPLEMENT_DUICONTROL(CBadgeUI)

	CBadgeUI::CBadgeUI()
		: m_nCount(0)
		, m_nOverflow(99)
		, m_bShowZero(false)
		, m_bDot(false)
		, m_bHang(true)
		, m_dwBadgeColor(0xFF4D4FFF)
		, m_dwBadgeTextColor(0xFFFFFFFF)
		, m_nDotSize(8)
		, m_nHeight(18)
	{
		m_szOffset.cx = 0;
		m_szOffset.cy = 0;
		SetKind(CONTROLKIND_NONE);
		SetMouseChildEnabled(true);
	}

	LPCTSTR CBadgeUI::GetClass() const { return _T("BadgeUI"); }

	LPVOID CBadgeUI::GetInterface(LPCTSTR pstrName)
	{
		if( _tcsicmp(pstrName, DUI_CTR_BADGE) == 0 ) return static_cast<CBadgeUI*>(this);
		return CContainerUI::GetInterface(pstrName);
	}

	UINT CBadgeUI::GetControlFlags() const
	{
		return 0;
	}

	int CBadgeUI::ScaleValue(int v) const
	{
		return m_pManager ? m_pManager->GetDPIObj()->Scale(v) : v;
	}

	void CBadgeUI::SetCount(int n)
	{
		if( m_nCount == n ) return;
		m_nCount = n;
		Invalidate();
	}

	int CBadgeUI::GetCount() const { return m_nCount; }

	void CBadgeUI::SetOverflowCount(int n)
	{
		if( n < 1 ) n = 1;
		m_nOverflow = n;
		Invalidate();
	}

	int CBadgeUI::GetOverflowCount() const { return m_nOverflow; }

	void CBadgeUI::SetShowZero(bool b)
	{
		m_bShowZero = b;
		Invalidate();
	}

	bool CBadgeUI::IsShowZero() const { return m_bShowZero; }

	void CBadgeUI::SetDot(bool b)
	{
		m_bDot = b;
		Invalidate();
	}

	bool CBadgeUI::IsDot() const { return m_bDot; }

	void CBadgeUI::SetOffset(SIZE sz)
	{
		m_szOffset = sz;
		Invalidate();
	}

	SIZE CBadgeUI::GetOffset() const { return m_szOffset; }

	void CBadgeUI::SetBadgeColor(DWORD dw)
	{
		m_dwBadgeColor = dw;
		Invalidate();
	}

	DWORD CBadgeUI::GetBadgeColor() const { return m_dwBadgeColor; }

	void CBadgeUI::SetBadgeTextColor(DWORD dw)
	{
		m_dwBadgeTextColor = dw;
		Invalidate();
	}

	bool CBadgeUI::ShouldShow() const
	{
		if( m_bDot ) return true;
		if( m_nCount < 0 ) return false;
		if( m_nCount == 0 ) return m_bShowZero;
		return true;
	}

	CDuiString CBadgeUI::FormatCount() const
	{
		CDuiString s;
		if( m_nCount > m_nOverflow )
			s.Format(_T("%d+"), m_nOverflow);
		else
			s.Format(_T("%d"), m_nCount);
		return s;
	}

	SIZE CBadgeUI::MeasureBadgeSize() const
	{
		SIZE sz = { 0, 0 };
		if( !ShouldShow() ) return sz;
		if( m_bDot ) {
			int d = ScaleValue(m_nDotSize);
			sz.cx = sz.cy = d;
			return sz;
		}
		CDuiString s = FormatCount();
		int h = ScaleValue(m_nHeight);
		RECT rcText = { 0, 0, 9999, h };
		RenderMeasureText(m_pManager, rcText, s.GetData(), 0, -1,
			DT_CALCRECT | DT_SINGLELINE | DT_CENTER | DT_VCENTER);
		int tw = rcText.right - rcText.left;
		int pad = ScaleValue(6);
		int w = tw + pad * 2;
		if( w < h ) w = h;
		sz.cx = w;
		sz.cy = h;
		return sz;
	}

	void CBadgeUI::ApplyBadgeInsetPadding()
	{
		// 半悬角标：上/右 padding 留给外悬半边，整颗角标仍在本控件裁剪框内
		if( !m_bHang || CContainerUI::GetCount() <= 0 || !ShouldShow() )
			return;
		SIZE sz = MeasureBadgeSize();
		int padT = sz.cy / 2;
		int padR = sz.cx / 2;
		if( padT < 1 ) padT = 1;
		if( padR < 1 ) padR = 1;
		// MeasureBadgeSize 已是缩放后像素；padding 存逻辑值
		int scale = m_pManager ? (int)m_pManager->GetDPIObj()->GetScale() : 100;
		if( scale <= 0 ) scale = 100;
		int logicT = MulDiv(padT, 100, scale);
		int logicR = MulDiv(padR, 100, scale);
		if( logicT < 1 ) logicT = 1;
		if( logicR < 1 ) logicR = 1;
		CDuiBox cur = m_rcPadding;
		bool bChange = false;
		if( cur.top < logicT ) { cur.top = logicT; bChange = true; }
		if( cur.right < logicR ) { cur.right = logicR; bChange = true; }
		if( bChange ) m_rcPadding = cur;
	}

	RECT CBadgeUI::CalcBadgeRect(const RECT& rcHost) const
	{
		RECT rc = { 0, 0, 0, 0 };
		SIZE sz = MeasureBadgeSize();
		int w = sz.cx;
		int h = sz.cy;
		if( w <= 0 || h <= 0 ) return rc;

		int ox = ScaleValue(m_szOffset.cx);
		int oy = ScaleValue(m_szOffset.cy);

		if( m_szOffset.cx != 0 || m_szOffset.cy != 0 ) {
			rc.right = rcHost.right + ox;
			rc.left = rc.right - w;
			rc.top = rcHost.top + oy;
			rc.bottom = rc.top + h;
			return rc;
		}

		if( m_bHang ) {
			rc.left = rcHost.right - w / 2;
			rc.right = rc.left + w;
			rc.top = rcHost.top - h / 2;
			rc.bottom = rc.top + h;
		}
		else {
			rc.right = rcHost.right;
			rc.left = rc.right - w;
			rc.top = rcHost.top;
			rc.bottom = rc.top + h;
		}
		return rc;
	}

	void CBadgeUI::PaintBadge(IRenderContext& ctx, const RECT& rcHost)
	{
		if( !ShouldShow() ) return;
		RECT rc = CalcBadgeRect(rcHost);
		int h = rc.bottom - rc.top;
		int r = h / 2;
		if( r < 1 ) r = 1;
		ctx.FillRoundRect(rc, r, r, GetAdjustColor(m_dwBadgeColor));
		if( !m_bDot ) {
			CDuiString s = FormatCount();
			ctx.DrawText(rc, s.GetData(), GetAdjustColor(m_dwBadgeTextColor), -1,
				DT_SINGLELINE | DT_CENTER | DT_VCENTER);
		}
	}

	void CBadgeUI::SetPos(RECT rc, bool bNeedInvalidate)
	{
		ApplyBadgeInsetPadding();
		CContainerUI::SetPos(rc, bNeedInvalidate);
	}

	SIZE CBadgeUI::EstimateSize(SIZE szAvailable)
	{
		ApplyBadgeInsetPadding();
		SIZE sz = GetFixedSize();
		const int nChild = CContainerUI::GetCount();
		if( nChild > 0 ) {
			if( sz.cx <= 0 || sz.cy <= 0 ) {
				CControlUI* p = GetItemAt(0);
				if( p ) {
					SIZE c = p->EstimateSize(szAvailable);
					CDuiBox pad = GetPadding();
					if( sz.cx <= 0 ) sz.cx = c.cx + pad.left + pad.right;
					if( sz.cy <= 0 ) sz.cy = c.cy + pad.top + pad.bottom;
				}
			}
			if( sz.cy <= 0 ) sz.cy = ScaleValue(32);
		}
		else {
			SIZE b = MeasureBadgeSize();
			if( sz.cx <= 0 ) sz.cx = b.cx;
			if( sz.cy <= 0 ) sz.cy = b.cy;
		}
		return sz;
	}

	bool CBadgeUI::DoPaint(IRenderContext& ctx, const RECT& rcPaint, CControlUI* pStopControl)
	{
		RECT rcContent = m_rcItem;
		if( CContainerUI::GetCount() > 0 ) {
			CControlUI::DoPaint(ctx, rcPaint, pStopControl);
			RECT rcTemp = { 0 };
			for( int i = 0; i < CContainerUI::GetCount(); ++i ) {
				CControlUI* pControl = GetItemAt(i);
				if( pControl == NULL || !pControl->IsVisible() ) continue;
				if( !::IntersectRect(&rcTemp, &rcPaint, &pControl->GetPos()) ) continue;
				if( !pControl->DoPaint(ctx, rcPaint, pStopControl) ) return false;
			}
			CControlUI* p = GetItemAt(0);
			if( p ) rcContent = p->GetPos();
		}
		else {
			PaintBackgroundColor(ctx);
			PaintBackgroundImage(ctx);
		}
		PaintBadge(ctx, rcContent);
		return true;
	}

	void CBadgeUI::SetAttribute(LPCTSTR pstrName, LPCTSTR pstrValue)
	{
		if( _tcsicmp(pstrName, _T("count")) == 0 || _tcsicmp(pstrName, _T("value")) == 0 ) {
			SetCount(_ttoi(pstrValue));
		}
		else if( _tcsicmp(pstrName, _T("overflow-count")) == 0 || _tcsicmp(pstrName, _T("overflow")) == 0 ) {
			SetOverflowCount(_ttoi(pstrValue));
		}
		else if( _tcsicmp(pstrName, _T("show-zero")) == 0 || _tcsicmp(pstrName, _T("showzero")) == 0 ) {
			SetShowZero(_tcsicmp(pstrValue, _T("true")) == 0);
		}
		else if( _tcsicmp(pstrName, _T("dot")) == 0 ) {
			SetDot(_tcsicmp(pstrValue, _T("true")) == 0);
		}
		else if( _tcsicmp(pstrName, _T("hang")) == 0 ) {
			m_bHang = (_tcsicmp(pstrValue, _T("true")) == 0);
			NeedUpdate();
			Invalidate();
		}
		else if( _tcsicmp(pstrName, _T("offset")) == 0 ) {
			SIZE sz = { 0, 0 };
			LPTSTR p = NULL;
			sz.cx = _tcstol(pstrValue, &p, 10);
			if( p && *p == _T(',') ) sz.cy = _tcstol(p + 1, &p, 10);
			SetOffset(sz);
		}
		else if( _tcsicmp(pstrName, _T("badge-color")) == 0 || _tcsicmp(pstrName, _T("color")) == 0 ) {
			DWORD clr = 0; if( ParseColorString(pstrValue, clr) ) SetBadgeColor(clr);
		}
		else if( _tcsicmp(pstrName, _T("badge-text-color")) == 0 || _tcsicmp(pstrName, _T("text-color")) == 0 ) {
			DWORD clr = 0; if( ParseColorString(pstrValue, clr) ) SetBadgeTextColor(clr);
		}
		else {
			CContainerUI::SetAttribute(pstrName, pstrValue);
		}
	}
}
