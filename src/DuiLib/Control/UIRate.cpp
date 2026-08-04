#include "StdAfx.h"
#include "UIRate.h"

namespace DuiLib
{
	IMPLEMENT_DUICONTROL(CRateUI)

	CRateUI::CRateUI()
		: m_dbValue(0)
		, m_dbHover(-1)
		, m_nCount(5)
		, m_bAllowHalf(false)
		, m_bAllowClear(true)
		, m_bReadOnly(false)
		, m_nStarSize(24)
		, m_nStarGap(8)
		, m_dwStarColor(0xFADB14FF)
		, m_dwVoidColor(0x00000026)
	{
		m_sCharacter = _T("★");
		SetKind(CONTROLKIND_NONE);
		SetCursor(DUI_HAND);
	}

	LPCTSTR CRateUI::GetClass() const { return _T("RateUI"); }

	LPVOID CRateUI::GetInterface(LPCTSTR pstrName)
	{
		if( _tcsicmp(pstrName, DUI_CTR_RATE) == 0 ) return static_cast<CRateUI*>(this);
		return CControlUI::GetInterface(pstrName);
	}

	UINT CRateUI::GetControlFlags() const
	{
		if( !IsEnabled() || m_bReadOnly ) return 0;
		return UIFLAG_SETCURSOR | UIFLAG_TABSTOP;
	}

	bool CRateUI::PreferClientHit() const
	{
		return true;
	}

	int CRateUI::ScaleValue(int v) const
	{
		return m_pManager ? m_pManager->GetDPIObj()->Scale(v) : v;
	}

	double CRateUI::ClampValue(double v) const
	{
		if( v < 0 ) v = 0;
		if( v > (double)m_nCount ) v = (double)m_nCount;
		return v;
	}

	double CRateUI::Quantize(double v) const
	{
		v = ClampValue(v);
		if( m_bAllowHalf ) {
			// 四舍五入到 0.5
			v = floor(v * 2.0 + 0.5) / 2.0;
		}
		else {
			v = floor(v + 0.5);
		}
		return ClampValue(v);
	}

	void CRateUI::SetValue(double v, bool bNotify)
	{
		v = Quantize(v);
		if( m_dbValue == v ) return;
		m_dbValue = v;
		Invalidate();
		if( bNotify && m_pManager )
			m_pManager->SendNotify(this, DUI_MSGTYPE_VALUECHANGED);
	}

	double CRateUI::GetValue() const { return m_dbValue; }

	void CRateUI::SetCount(int n)
	{
		if( n < 1 ) n = 1;
		if( n > 20 ) n = 20;
		if( m_nCount == n ) return;
		m_nCount = n;
		m_dbValue = ClampValue(m_dbValue);
		NeedParentUpdate();
		Invalidate();
	}

	int CRateUI::GetCount() const { return m_nCount; }

	void CRateUI::SetAllowHalf(bool b)
	{
		m_bAllowHalf = b;
		m_dbValue = Quantize(m_dbValue);
		Invalidate();
	}

	bool CRateUI::IsAllowHalf() const { return m_bAllowHalf; }

	void CRateUI::SetAllowClear(bool b) { m_bAllowClear = b; }
	bool CRateUI::IsAllowClear() const { return m_bAllowClear; }

	void CRateUI::SetReadOnly(bool b)
	{
		m_bReadOnly = b;
		Invalidate();
	}

	bool CRateUI::IsReadOnly() const { return m_bReadOnly; }

	void CRateUI::SetStarSize(int n)
	{
		if( n < 12 ) n = 12;
		m_nStarSize = n;
		NeedParentUpdate();
		Invalidate();
	}

	int CRateUI::GetStarSize() const { return m_nStarSize; }

	void CRateUI::SetStarGap(int n)
	{
		if( n < 0 ) n = 0;
		m_nStarGap = n;
		NeedParentUpdate();
		Invalidate();
	}

	int CRateUI::GetStarGap() const { return m_nStarGap; }

	void CRateUI::SetCharacter(LPCTSTR pstr)
	{
		m_sCharacter = (pstr && *pstr) ? pstr : _T("★");
		Invalidate();
	}

	LPCTSTR CRateUI::GetCharacter() const { return m_sCharacter.GetData(); }

	void CRateUI::SetStarColor(DWORD dw)
	{
		m_dwStarColor = dw;
		Invalidate();
	}

	DWORD CRateUI::GetStarColor() const { return m_dwStarColor; }

	void CRateUI::SetVoidColor(DWORD dw)
	{
		m_dwVoidColor = dw;
		Invalidate();
	}

	DWORD CRateUI::GetVoidColor() const { return m_dwVoidColor; }

	RECT CRateUI::GetStarRect(int index) const
	{
		RECT rc = { 0, 0, 0, 0 };
		if( index < 0 || index >= m_nCount ) return rc;
		CDuiBox pad = GetPadding();
		int sz = ScaleValue(m_nStarSize);
		int gap = ScaleValue(m_nStarGap);
		rc.left = m_rcItem.left + pad.left + index * (sz + gap);
		rc.right = rc.left + sz;
		int h = m_rcItem.bottom - m_rcItem.top - pad.top - pad.bottom;
		rc.top = m_rcItem.top + pad.top + (h - sz) / 2;
		rc.bottom = rc.top + sz;
		return rc;
	}

	int CRateUI::HitStarIndex(POINT pt) const
	{
		CDuiBox pad = GetPadding();
		if( pt.y < m_rcItem.top + pad.top || pt.y >= m_rcItem.bottom - pad.bottom )
			return -1;
		for( int i = 0; i < m_nCount; ++i ) {
			RECT rc = GetStarRect(i);
			RECT rcHit = rc;
			if( i + 1 < m_nCount ) {
				RECT rcNext = GetStarRect(i + 1);
				rcHit.right = (rc.right + rcNext.left) / 2;
			}
			else {
				rcHit.right = m_rcItem.right - pad.right;
			}
			if( i == 0 ) rcHit.left = m_rcItem.left + pad.left;
			if( ::PtInRect(&rcHit, pt) ) return i + 1;
		}
		return -1;
	}

	double CRateUI::NextValueForStar(int star1) const
	{
		if( star1 < 1 || star1 > m_nCount ) return m_dbValue;
		double full = (double)star1;
		if( m_bAllowHalf ) {
			double half = full - 0.5;
			// 同星循环：半星 → 全星 →（可清空时）取消
			if( m_dbValue == half ) return full;
			if( m_dbValue == full ) return m_bAllowClear ? 0.0 : half;
			return half;
		}
		if( m_dbValue == full && m_bAllowClear ) return 0.0;
		return full;
	}

	double CRateUI::DisplayValue() const
	{
		if( m_dbHover >= 0 && IsEnabled() && !m_bReadOnly )
			return m_dbHover;
		return m_dbValue;
	}

	int CRateUI::ResolveFont() const
	{
		if( m_pManager == NULL ) return -1;
		LPCTSTR name = _T("Segoe UI Symbol");
		TFontInfo* pDef = m_pManager->GetDefaultFontInfo();
		if( pDef != NULL && !pDef->sFontName.IsEmpty() )
			name = pDef->sFontName.GetData();
		int id = m_pManager->EnsureFont(name, m_nStarSize, false, false, false, false, false);
		return id >= 0 ? id : -1;
	}

	void CRateUI::PaintStar(IRenderContext& ctx, int index, double display)
	{
		RECT rc = GetStarRect(index);
		if( rc.right <= rc.left ) return;

		double fullAt = (double)(index + 1);
		DWORD clrVoid = m_dwVoidColor;
		DWORD clrStar = m_dwStarColor;
		if( !IsEnabled() ) {
			clrVoid = DuiColorSetA(clrVoid, (BYTE)(DuiColorA(clrVoid) / 2));
			clrStar = DuiColorSetA(clrStar, (BYTE)(DuiColorA(clrStar) / 2));
		}

		LPCTSTR ch = m_sCharacter.GetData();
		UINT style = DT_SINGLELINE | DT_CENTER | DT_VCENTER;
		int font = ResolveFont();

		if( display >= fullAt ) {
			ctx.DrawText(rc, ch, GetAdjustColor(clrStar), font, style);
			return;
		}

		ctx.DrawText(rc, ch, GetAdjustColor(clrVoid), font, style);

		if( m_bAllowHalf && display >= fullAt - 0.5 ) {
			RECT rcHalf = rc;
			rcHalf.right = (rc.left + rc.right) / 2;
			CRenderClipScope clip(ctx, rcHalf);
			ctx.DrawText(rc, ch, GetAdjustColor(clrStar), font, style);
		}
	}

	SIZE CRateUI::EstimateSize(SIZE /*szAvailable*/)
	{
		SIZE sz = GetFixedSize();
		CDuiBox pad = GetPadding();
		int star = ScaleValue(m_nStarSize);
		int gap = ScaleValue(m_nStarGap);
		int w = m_nCount * star + (m_nCount > 0 ? (m_nCount - 1) * gap : 0) + pad.left + pad.right;
		int h = star + pad.top + pad.bottom;
		if( sz.cx <= 0 ) sz.cx = w;
		if( sz.cy <= 0 ) sz.cy = h;
		return sz;
	}

	bool CRateUI::DoPaint(IRenderContext& ctx, const RECT& /*rcPaint*/, CControlUI* /*pStopControl*/)
	{
		PaintBackgroundColor(ctx);
		PaintBackgroundImage(ctx);
		double display = DisplayValue();
		for( int i = 0; i < m_nCount; ++i )
			PaintStar(ctx, i, display);
		PaintBorder(ctx);
		return true;
	}

	void CRateUI::DoEvent(TEventUI& event)
	{
		if( !IsMouseEnabled() && event.Type > UIEVENT__MOUSEBEGIN && event.Type < UIEVENT__MOUSEEND ) {
			if( m_pParent != NULL ) m_pParent->DoEvent(event);
			else CControlUI::DoEvent(event);
			return;
		}

		if( IsEnabled() && !m_bReadOnly ) {
			if( event.Type == UIEVENT_SETCURSOR ) {
				::SetCursor(::LoadCursor(NULL, MAKEINTRESOURCE(IDC_HAND)));
				return;
			}
			if( event.Type == UIEVENT_MOUSEMOVE || event.Type == UIEVENT_MOUSEENTER ) {
				int star = HitStarIndex(event.ptMouse);
				double hv = (star > 0) ? NextValueForStar(star) : -1;
				if( hv != m_dbHover ) {
					m_dbHover = hv;
					Invalidate();
				}
				return;
			}
			if( event.Type == UIEVENT_MOUSELEAVE ) {
				if( m_dbHover >= 0 ) {
					m_dbHover = -1;
					Invalidate();
				}
				return;
			}
			if( event.Type == UIEVENT_BUTTONDOWN || event.Type == UIEVENT_DBLCLICK ) {
				int star = HitStarIndex(event.ptMouse);
				if( star > 0 ) {
					SetValue(NextValueForStar(star), true);
					m_dbHover = NextValueForStar(star);
					Invalidate();
				}
				return;
			}
			if( event.Type == UIEVENT_KEYDOWN ) {
				if( event.chKey == VK_LEFT || event.chKey == VK_DOWN ) {
					double step = m_bAllowHalf ? 0.5 : 1.0;
					SetValue(m_dbValue - step, true);
					return;
				}
				if( event.chKey == VK_RIGHT || event.chKey == VK_UP ) {
					double step = m_bAllowHalf ? 0.5 : 1.0;
					SetValue(m_dbValue + step, true);
					return;
				}
				if( event.chKey == VK_HOME ) { SetValue(0, true); return; }
				if( event.chKey == VK_END ) { SetValue((double)m_nCount, true); return; }
			}
		}

		CControlUI::DoEvent(event);
	}

	void CRateUI::SetAttribute(LPCTSTR pstrName, LPCTSTR pstrValue)
	{
		if( _tcsicmp(pstrName, _T("value")) == 0 || _tcsicmp(pstrName, _T("default-value")) == 0 ) {
			SetValue(_tcstod(pstrValue, NULL), false);
		}
		else if( _tcsicmp(pstrName, _T("count")) == 0 || _tcsicmp(pstrName, _T("max")) == 0 ) {
			SetCount(_ttoi(pstrValue));
		}
		else if( _tcsicmp(pstrName, _T("allow-half")) == 0 || _tcsicmp(pstrName, _T("allowhalf")) == 0 ) {
			SetAllowHalf(_tcsicmp(pstrValue, _T("true")) == 0);
		}
		else if( _tcsicmp(pstrName, _T("allow-clear")) == 0 || _tcsicmp(pstrName, _T("allowclear")) == 0 ) {
			SetAllowClear(_tcsicmp(pstrValue, _T("true")) == 0);
		}
		else if( _tcsicmp(pstrName, _T("readonly")) == 0 || _tcsicmp(pstrName, _T("read-only")) == 0 ) {
			SetReadOnly(_tcsicmp(pstrValue, _T("true")) == 0);
		}
		else if( _tcsicmp(pstrName, _T("star-size")) == 0 || _tcsicmp(pstrName, _T("size")) == 0 ) {
			SetStarSize(_ttoi(pstrValue));
		}
		else if( _tcsicmp(pstrName, _T("star-gap")) == 0 || _tcsicmp(pstrName, _T("gap")) == 0 ) {
			SetStarGap(_ttoi(pstrValue));
		}
		else if( _tcsicmp(pstrName, _T("character")) == 0 || _tcsicmp(pstrName, _T("char")) == 0 ) {
			SetCharacter(pstrValue);
		}
		else if( _tcsicmp(pstrName, _T("color")) == 0 || _tcsicmp(pstrName, _T("star-color")) == 0 ) {
			DWORD clr = 0; if( ParseColorString(pstrValue, clr) ) SetStarColor(clr);
		}
		else if( _tcsicmp(pstrName, _T("void-color")) == 0 || _tcsicmp(pstrName, _T("voidcolor")) == 0 ) {
			DWORD clr = 0; if( ParseColorString(pstrValue, clr) ) SetVoidColor(clr);
		}
		else {
			CControlUI::SetAttribute(pstrName, pstrValue);
		}
	}
}
