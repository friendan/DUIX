#include "StdAfx.h"
#include "UIFontIcon.h"

namespace DuiLib
{
	IMPLEMENT_DUICONTROL(CFontIconUI)

	CFontIconUI::CFontIconUI()
		: m_eShape(ShapeCircle)
		, m_bBkCustom(false)
		, m_bColorCustom(false)
		, m_bRadiusCustom(false)
	{
		SetFixedWidth(40);
		SetFixedHeight(40);
		SetTextStyle(DT_SINGLELINE | DT_VCENTER | DT_CENTER | DT_END_ELLIPSIS);
		SetAutoCalcWidth(false);
		SetAutoCalcHeight(false);
		SyncShapeRadius();
	}

	LPCTSTR CFontIconUI::GetClass() const
	{
		return _T("FontIconUI");
	}

	LPVOID CFontIconUI::GetInterface(LPCTSTR pstrName)
	{
		if( _tcsicmp(pstrName, DUI_CTR_FONTICON) == 0 ) return static_cast<CFontIconUI*>(this);
		return CLabelUI::GetInterface(pstrName);
	}

	bool CFontIconUI::PreferClientHit() const
	{
		// 悬停 / 可点：勿被 html{action:title} 吃成拖窗
		if( !IsEnabled() ) return false;
		return true;
	}

	UINT CFontIconUI::GetControlFlags() const
	{
		if( !IsEnabled() ) return 0;
		return UIFLAG_SETCURSOR;
	}

	void CFontIconUI::SetClickable(bool bClickable)
	{
		CLabelUI::SetClickable(bClickable);
		if( bClickable ) {
			if( GetCursor() == 0 )
				SetCursor(DUI_HAND);
		}
		else if( GetCursor() == DUI_HAND ) {
			SetCursor(0);
		}
	}

	void CFontIconUI::DoEvent(TEventUI& event)
	{
		if( IsMouseEnabled() ) {
			if( event.Type == UIEVENT_MOUSEENTER ) {
				if( IsEnabled() && (m_uControlState & UISTATE_HOT) == 0 ) {
					m_uControlState |= UISTATE_HOT;
					Invalidate();
				}
			}
			else if( event.Type == UIEVENT_MOUSELEAVE ) {
				if( (m_uControlState & UISTATE_HOT) != 0 ) {
					m_uControlState &= ~UISTATE_HOT;
					Invalidate();
				}
			}
		}

		// 可点时跟踪按下态（供 active 色）；click 通知仍由 Label 发出
		if( IsClickable() && IsMouseEnabled() ) {
			if( event.Type == UIEVENT_BUTTONDOWN || event.Type == UIEVENT_DBLCLICK ) {
				if( ::PtInRect(&m_rcItem, event.ptMouse) && IsEnabled() ) {
					m_uControlState |= UISTATE_PUSHED | UISTATE_CAPTURED;
					Invalidate();
				}
			}
			else if( event.Type == UIEVENT_MOUSEMOVE ) {
				if( (m_uControlState & UISTATE_CAPTURED) != 0 ) {
					if( ::PtInRect(&m_rcItem, event.ptMouse) )
						m_uControlState |= UISTATE_PUSHED;
					else
						m_uControlState &= ~UISTATE_PUSHED;
					Invalidate();
				}
			}
			else if( event.Type == UIEVENT_BUTTONUP ) {
				if( (m_uControlState & (UISTATE_PUSHED | UISTATE_CAPTURED)) != 0 ) {
					m_uControlState &= ~(UISTATE_PUSHED | UISTATE_CAPTURED);
					Invalidate();
				}
			}
		}

		CLabelUI::DoEvent(event);
	}

	void CFontIconUI::SetShape(Shape eShape)
	{
		if( m_eShape == eShape ) return;
		m_eShape = eShape;
		if( m_eShape == ShapeCircle )
			m_bRadiusCustom = false;
		SyncShapeRadius();
		Invalidate();
	}

	CFontIconUI::Shape CFontIconUI::GetShape() const
	{
		return m_eShape;
	}

	void CFontIconUI::SetSizePreset(int nSize)
	{
		if( nSize < 8 ) nSize = 8;
		SetFixedWidth(nSize);
		SetFixedHeight(nSize);
		SyncShapeRadius();
		NeedParentUpdate();
		Invalidate();
	}

	void CFontIconUI::SetKind(ControlKind kind)
	{
		InitKindColors();
		m_controlKind = kind;
		SetBorderWidth(0);
		SetBorderColor(0);

		if( kind == CONTROLKIND_NONE ) {
			if( !m_bBkCustom ) SetBackgroundColor(0);
			if( !m_bColorCustom ) SetColor(0);
			if( !m_bBkCustom ) SetHoverBackgroundColor(0);
			if( !m_bColorCustom ) SetHoverColor(0);
			if( !m_bBkCustom ) SetActiveBackgroundColor(0);
			if( !m_bColorCustom ) SetActiveColor(0);
			SyncShapeRadius();
			Invalidate();
			return;
		}

		int idx = (int)kind;
		if( idx < 0 || idx >= 11 ) idx = 1;
		const KindStateColors& normal = g_kindColors[idx].Normal;
		const KindStateColors& hover = g_kindColors[idx].Hover;
		const KindStateColors& active = g_kindColors[idx].Active;
		if( !m_bBkCustom ) {
			SetBackgroundColor(normal.dwBackgroundColor);
			SetHoverBackgroundColor(hover.dwBackgroundColor);
			SetActiveBackgroundColor(active.dwBackgroundColor);
		}
		if( !m_bColorCustom ) {
			SetColor(normal.dwColor);
			SetHoverColor(hover.dwColor);
			SetActiveColor(active.dwColor);
		}
		SyncShapeRadius();
		Invalidate();
	}

	void CFontIconUI::SyncShapeRadius()
	{
		if( m_eShape == ShapeRounded && m_bRadiusCustom ) return;

		int w = GetFixedWidth();
		int h = GetFixedHeight();
		if( w <= 0 && h <= 0 ) {
			w = m_rcItem.right - m_rcItem.left;
			h = m_rcItem.bottom - m_rcItem.top;
		}
		int s = w;
		if( h > 0 && (s <= 0 || h < s) ) s = h;
		if( s <= 0 ) s = 40;

		SIZE sz = { 0, 0 };
		if( m_eShape == ShapeCircle ) {
			sz.cx = s / 2;
			sz.cy = s / 2;
		}
		else {
			int r = s / 4;
			if( r < 2 ) r = 2;
			sz.cx = r;
			sz.cy = r;
		}
		if( sz.cx < 1 ) sz.cx = 1;
		if( sz.cy < 1 ) sz.cy = 1;
		SetBorderRadius(sz);
	}

	DWORD CFontIconUI::ResolveBackgroundColor() const
	{
		const bool bEnabled = IsEnabled();
		const bool bActive = bEnabled && (m_uControlState & UISTATE_PUSHED) != 0;
		const bool bHot = bEnabled && (m_uControlState & UISTATE_HOT) != 0;

		if( bActive && GetActiveBackgroundColor() != 0 )
			return GetActiveBackgroundColor();
		if( bHot && GetHoverBackgroundColor() != 0 )
			return GetHoverBackgroundColor();

		if( m_bBkCustom ) {
			DWORD c = GetBackgroundColor();
			if( c != 0 ) return c;
		}
		if( GetKind() != CONTROLKIND_NONE ) {
			int idx = (int)GetKind();
			if( idx < 0 || idx >= 11 ) idx = 1;
			if( bActive ) return g_kindColors[idx].Active.dwBackgroundColor;
			if( bHot ) return g_kindColors[idx].Hover.dwBackgroundColor;
			DWORD c = GetBackgroundColor();
			if( c != 0 ) return c;
			return g_kindColors[idx].Normal.dwBackgroundColor;
		}

		CThemeManager* tm = CThemeManager::GetInstance();
		if( tm != NULL ) {
			if( bActive )
				return tm->GetColor(_T("color-primary-active"),
					tm->GetColor(_T("color-primary-hover"), 0x0B5ED7FF));
			if( bHot )
				return tm->GetColor(_T("color-primary-hover"), 0x0B5ED7FF);
			return tm->GetColor(_T("color-primary"), 0x0D6EFDFF);
		}
		if( bHot || bActive ) return 0x0B5ED7FF;
		return 0x0D6EFDFF;
	}

	DWORD CFontIconUI::ResolveTextColor() const
	{
		const bool bEnabled = IsEnabled();
		const bool bActive = bEnabled && (m_uControlState & UISTATE_PUSHED) != 0;
		const bool bHot = bEnabled && (m_uControlState & UISTATE_HOT) != 0;

		if( !bEnabled ) {
			DWORD dis = GetDisabledColor();
			if( dis != 0 ) return dis;
		}
		if( bActive && GetActiveColor() != 0 )
			return GetActiveColor();
		if( bHot && GetHoverColor() != 0 )
			return GetHoverColor();

		if( m_bColorCustom ) {
			DWORD c = GetColor();
			if( c != 0 ) return c;
		}
		if( GetKind() != CONTROLKIND_NONE ) {
			int idx = (int)GetKind();
			if( idx < 0 || idx >= 11 ) idx = 1;
			if( bActive ) return g_kindColors[idx].Active.dwColor;
			if( bHot ) return g_kindColors[idx].Hover.dwColor;
			DWORD c = GetColor();
			if( c != 0 ) return c;
			return g_kindColors[idx].Normal.dwColor;
		}

		CThemeManager* tm = CThemeManager::GetInstance();
		if( tm != NULL )
			return tm->GetColor(_T("color-primary-text"), 0xFFFFFFFF);
		return 0xFFFFFFFF;
	}

	void CFontIconUI::SetPos(RECT rc, bool bNeedInvalidate)
	{
		CLabelUI::SetPos(rc, bNeedInvalidate);
		SyncShapeRadius();
	}

	SIZE CFontIconUI::EstimateSize(SIZE /*szAvailable*/)
	{
		SIZE sz = GetFixedSize();
		if( sz.cx <= 0 ) sz.cx = 40;
		if( sz.cy <= 0 ) sz.cy = 40;
		return sz;
	}

	void CFontIconUI::PaintBackgroundColor(IRenderContext& ctx)
	{
		DWORD dwBk = ResolveBackgroundColor();
		if( dwBk == 0 ) return;
		SIZE szR = GetBorderRadius();
		if( szR.cx > 0 || szR.cy > 0 )
			ctx.FillRoundRect(m_rcItem, szR.cx, szR.cy, GetAdjustColor(dwBk));
		else
			ctx.DrawColor(m_rcItem, GetAdjustColor(dwBk));
	}

	void CFontIconUI::PaintText(IRenderContext& ctx)
	{
		CDuiString sText = GetText();
		if( sText.IsEmpty() ) return;

		DWORD clr = ResolveTextColor();

		RECT rc = m_rcItem;
		RECT rcPadding = GetPadding();
		RECT rcTextPadding = GetTextPadding();
		rc.left += rcPadding.left + rcTextPadding.left;
		rc.right -= rcPadding.right + rcTextPadding.right;
		rc.top += rcPadding.top + rcTextPadding.top;
		rc.bottom -= rcPadding.bottom + rcTextPadding.bottom;

		ctx.DrawText(rc, sText.GetData(), GetAdjustColor(clr), GetFont(),
			DT_SINGLELINE | DT_CENTER | DT_VCENTER | DT_END_ELLIPSIS);
	}

	void CFontIconUI::SetAttribute(LPCTSTR pstrName, LPCTSTR pstrValue)
	{
		if( _tcsicmp(pstrName, _T("shape")) == 0 ) {
			if( _tcsicmp(pstrValue, _T("rounded")) == 0
				|| _tcsicmp(pstrValue, _T("roundrect")) == 0
				|| _tcsicmp(pstrValue, _T("rect")) == 0 )
				SetShape(ShapeRounded);
			else
				SetShape(ShapeCircle);
		}
		else if( _tcsicmp(pstrName, _T("size")) == 0 ) {
			if( _tcsicmp(pstrValue, _T("small")) == 0 ) SetSizePreset(24);
			else if( _tcsicmp(pstrValue, _T("default")) == 0 || _tcsicmp(pstrValue, _T("medium")) == 0 ) SetSizePreset(32);
			else if( _tcsicmp(pstrValue, _T("large")) == 0 ) SetSizePreset(40);
			else if( _tcsicmp(pstrValue, _T("xlarge")) == 0 || _tcsicmp(pstrValue, _T("xl")) == 0 ) SetSizePreset(64);
			else SetSizePreset(_ttoi(pstrValue));
		}
		else if( _tcsicmp(pstrName, _T("background-color")) == 0
			|| _tcsicmp(pstrName, _T("bkcolor")) == 0 ) {
			m_bBkCustom = true;
			CLabelUI::SetAttribute(pstrName, pstrValue);
		}
		else if( _tcsicmp(pstrName, _T("color")) == 0
			|| _tcsicmp(pstrName, _T("textcolor")) == 0
			|| _tcsicmp(pstrName, _T("text-color")) == 0 ) {
			m_bColorCustom = true;
			CLabelUI::SetAttribute(pstrName, pstrValue);
		}
		else if( _tcsicmp(pstrName, _T("clickable")) == 0 ) {
			SetClickable(_tcsicmp(pstrValue, _T("true")) == 0 || _tcscmp(pstrValue, _T("1")) == 0);
		}
		else if( _tcsicmp(pstrName, _T("border-radius")) == 0
			|| _tcsicmp(pstrName, _T("borderradius")) == 0
			|| _tcsicmp(pstrName, _T("corner")) == 0 ) {
			m_bRadiusCustom = true;
			if( m_eShape == ShapeCircle )
				m_eShape = ShapeRounded;
			CLabelUI::SetAttribute(pstrName, pstrValue);
		}
		else {
			CLabelUI::SetAttribute(pstrName, pstrValue);
		}
	}
}
