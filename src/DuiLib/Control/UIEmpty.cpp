#include "StdAfx.h"
#include "UIEmpty.h"

namespace DuiLib
{
	IMPLEMENT_DUICONTROL(CEmptyUI)

	CEmptyUI::CEmptyUI()
		: m_bBuilt(false)
		, m_bShowImage(true)
		, m_sDescription(_T("暂无数据"))
		, m_dwDescColor(0x00000073)
		, m_pImageHost(NULL)
		, m_pDesc(NULL)
		, m_pExtra(NULL)
	{
		m_szImage.cx = 96;
		m_szImage.cy = 96;
		SetKind(CONTROLKIND_NONE);
		SetAlignItems(DT_CENTER);
		SetGap(12);
		SetPadding(CDuiBox(16, 16, 16, 16));
	}

	CEmptyUI::~CEmptyUI()
	{
	}

	LPCTSTR CEmptyUI::GetClass() const { return _T("EmptyUI"); }

	LPVOID CEmptyUI::GetInterface(LPCTSTR pstrName)
	{
		if( _tcsicmp(pstrName, DUI_CTR_EMPTY) == 0 ) return static_cast<CEmptyUI*>(this);
		return CVerticalLayoutUI::GetInterface(pstrName);
	}

	int CEmptyUI::ScaleValue(int v) const
	{
		return m_pManager ? m_pManager->GetDPIObj()->Scale(v) : v;
	}

	void CEmptyUI::SetDescription(LPCTSTR pstr)
	{
		m_sDescription = pstr ? pstr : _T("");
		if( m_pDesc ) m_pDesc->SetText(m_sDescription.GetData());
		Invalidate();
	}

	LPCTSTR CEmptyUI::GetDescription() const { return m_sDescription.GetData(); }

	void CEmptyUI::SetImage(LPCTSTR pstr)
	{
		m_sImage = pstr ? pstr : _T("");
		if( m_bBuilt ) {
			m_bBuilt = false;
			EnsureBuilt();
		}
		Invalidate();
	}

	LPCTSTR CEmptyUI::GetImage() const { return m_sImage.GetData(); }

	void CEmptyUI::SetImageSize(SIZE sz)
	{
		if( sz.cx < 16 ) sz.cx = 16;
		if( sz.cy < 16 ) sz.cy = 16;
		m_szImage = sz;
		if( m_pImageHost ) {
			m_pImageHost->SetFixedWidth(ScaleValue(m_szImage.cx));
			m_pImageHost->SetFixedHeight(ScaleValue(m_szImage.cy));
		}
		NeedUpdate();
	}

	SIZE CEmptyUI::GetImageSize() const { return m_szImage; }

	void CEmptyUI::SetShowImage(bool b)
	{
		m_bShowImage = b;
		if( m_pImageHost ) m_pImageHost->SetVisible(b);
	}

	bool CEmptyUI::IsShowImage() const { return m_bShowImage; }

	void CEmptyUI::SetDescriptionColor(DWORD dwColor)
	{
		m_dwDescColor = dwColor;
		if( m_pDesc ) m_pDesc->SetColor(dwColor);
		Invalidate();
	}

	void CEmptyUI::EnsureBuilt()
	{
		if( m_bBuilt ) return;

		CStdPtrArray extras;
		SetAutoDestroy(false);
		while( GetCount() > 0 ) {
			CControlUI* p = GetItemAt(0);
			Remove(p);
			extras.Add(p);
		}
		SetAutoDestroy(true);

		if( m_bShowImage ) {
			if( !m_sImage.IsEmpty() ) {
				CLabelUI* pImg = new CLabelUI();
				pImg->SetFixedWidth(ScaleValue(m_szImage.cx));
				pImg->SetFixedHeight(ScaleValue(m_szImage.cy));
				pImg->SetBackgroundImage(m_sImage.GetData());
				pImg->SetMouseEnabled(false);
				m_pImageHost = pImg;
			}
			else {
				CControlUI* pHost = new CControlUI();
				pHost->SetFixedWidth(ScaleValue(m_szImage.cx));
				pHost->SetFixedHeight(ScaleValue(m_szImage.cy));
				pHost->SetMouseEnabled(false);
				pHost->SetName(_T("__empty_icon"));
				m_pImageHost = pHost;
			}
			Add(m_pImageHost);
		}

		m_pDesc = new CLabelUI();
		m_pDesc->SetText(m_sDescription.GetData());
		m_pDesc->SetColor(m_dwDescColor);
		m_pDesc->SetTextStyle(DT_CENTER | DT_VCENTER | DT_SINGLELINE | DT_END_ELLIPSIS);
		m_pDesc->SetFixedHeight(ScaleValue(22));
		m_pDesc->SetMouseEnabled(false);
		Add(m_pDesc);

		if( extras.GetSize() > 0 ) {
			m_pExtra = new CHorizontalLayoutUI();
			m_pExtra->SetFixedHeight(ScaleValue(36));
			m_pExtra->SetAlignItems(DT_VCENTER);
			m_pExtra->SetGap(ScaleValue(8));
			m_pExtra->SetAttribute(_T("justify-content"), _T("center"));
			for( int i = 0; i < extras.GetSize(); ++i ) {
				CControlUI* p = static_cast<CControlUI*>(extras[i]);
				if( p ) m_pExtra->Add(p);
			}
			Add(m_pExtra);
		}

		m_bBuilt = true;
	}

	void CEmptyUI::PaintDefaultIllustration(IRenderContext& ctx, const RECT& rc)
	{
		if( rc.right - rc.left < 8 || rc.bottom - rc.top < 8 ) return;
		int w = rc.right - rc.left;
		int h = rc.bottom - rc.top;
		DWORD bg = 0xF5F5F5FF, bd = 0xD9D9D9FF, lid = 0xE6E6E6FF, lidBd = 0xBFBFBFFF, dot = 0xBFBFBFFF;
		CThemeManager* tm = CThemeManager::GetInstance();
		if( tm != NULL && tm->IsEnabled() ) {
			CTheme* th = tm->GetCurrentTheme();
			if( th == NULL ) th = tm->FindTheme(tm->GetDefaultThemeId());
			if( th != NULL ) {
				bg = th->GetToken(_T("color-bg-elevated"), bg);
				bd = th->GetToken(_T("color-border"), bd);
				lid = th->GetToken(_T("color-bg-hover"), lid);
				lidBd = th->GetToken(_T("color-border-strong"), lidBd);
				dot = lidBd;
			}
		}
		int pad = ScaleValue(8);
		RECT rcBox = { rc.left + pad, rc.top + pad + ScaleValue(8), rc.right - pad, rc.bottom - pad };
		int rx = ScaleValue(8);
		ctx.FillRoundRect(rcBox, rx, rx, GetAdjustColor(bg));
		ctx.DrawRoundRect(rcBox, ScaleValue(1), rx, rx, GetAdjustColor(bd), PS_SOLID);

		RECT rcLid = rcBox;
		rcLid.bottom = rcLid.top + ScaleValue(14);
		rcLid.left -= ScaleValue(4);
		rcLid.right += ScaleValue(4);
		ctx.FillRoundRect(rcLid, ScaleValue(4), ScaleValue(4), GetAdjustColor(lid));
		ctx.DrawRoundRect(rcLid, ScaleValue(1), ScaleValue(4), ScaleValue(4), GetAdjustColor(lidBd), PS_SOLID);

		int cx = (rc.left + rc.right) / 2;
		int cy = (rcBox.top + rcBox.bottom) / 2 + ScaleValue(6);
		int r = ScaleValue(4);
		RECT rcDot = { cx - r, cy - r, cx + r, cy + r };
		ctx.FillRoundRect(rcDot, r, r, GetAdjustColor(dot));
		(void)w; (void)h;
	}

	bool CEmptyUI::DoPaint(IRenderContext& ctx, const RECT& rcPaint, CControlUI* pStopControl)
	{
		bool b = CVerticalLayoutUI::DoPaint(ctx, rcPaint, pStopControl);
		if( m_pImageHost != NULL && m_sImage.IsEmpty() && m_bShowImage ) {
			RECT rc = m_pImageHost->GetPos();
			PaintDefaultIllustration(ctx, rc);
		}
		return b;
	}

	void CEmptyUI::DoInit()
	{
		CVerticalLayoutUI::DoInit();
		EnsureBuilt();
	}

	void CEmptyUI::SetAttribute(LPCTSTR pstrName, LPCTSTR pstrValue)
	{
		if( _tcsicmp(pstrName, _T("description")) == 0 || _tcsicmp(pstrName, _T("desc")) == 0
			|| _tcsicmp(pstrName, _T("text")) == 0 ) {
			SetDescription(pstrValue);
		}
		else if( _tcsicmp(pstrName, _T("image")) == 0 || _tcsicmp(pstrName, _T("src")) == 0 ) {
			SetImage(pstrValue);
		}
		else if( _tcsicmp(pstrName, _T("image-size")) == 0 || _tcsicmp(pstrName, _T("imagesize")) == 0 ) {
			SIZE sz = m_szImage;
			LPTSTR p = NULL;
			sz.cx = _tcstol(pstrValue, &p, 10);
			if( p && *p == _T(',') ) sz.cy = _tcstol(p + 1, &p, 10);
			else sz.cy = sz.cx;
			SetImageSize(sz);
		}
		else if( _tcsicmp(pstrName, _T("show-image")) == 0 || _tcsicmp(pstrName, _T("showimage")) == 0 ) {
			SetShowImage(_tcsicmp(pstrValue, _T("true")) == 0);
		}
		else if( _tcsicmp(pstrName, _T("description-color")) == 0 || _tcsicmp(pstrName, _T("color")) == 0 ) {
			DWORD clr = 0;
			if( ParseColorString(pstrValue, clr) ) SetDescriptionColor(clr);
		}
		else {
			CVerticalLayoutUI::SetAttribute(pstrName, pstrValue);
		}
	}
}
