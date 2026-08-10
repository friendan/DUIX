#include "StdAfx.h"
#include "UIImage.h"

namespace DuiLib
{
	IMPLEMENT_DUICONTROL(CImageUI)

	CImageUI::CImageUI()
		: m_eObjectFit(FitFill)
		, m_bLoadFailed(false)
	{
		m_sPlaceholderText = _T("");
	}

	CImageUI::~CImageUI()
	{
	}

	LPCTSTR CImageUI::GetClass() const
	{
		return _T("ImageUI");
	}

	LPVOID CImageUI::GetInterface(LPCTSTR pstrName)
	{
		if( _tcsicmp(pstrName, DUI_CTR_IMAGE) == 0 ) return static_cast<CImageUI*>(this);
		return CLabelUI::GetInterface(pstrName);
	}

	void CImageUI::SetSrc(LPCTSTR pstrSrc)
	{
		CDuiString s;
		if( pstrSrc != NULL && *pstrSrc != _T('\0') ) {
			if( !ParseCssUrlImage(pstrSrc, s) ) s = pstrSrc;
		}
		if( m_sSrc == s ) return;
		m_sSrc = s;
		m_bLoadFailed = false;
		Invalidate();
	}

	LPCTSTR CImageUI::GetSrc() const
	{
		return m_sSrc.GetData();
	}

	void CImageUI::SetObjectFit(ObjectFit fit)
	{
		if( m_eObjectFit == fit ) return;
		m_eObjectFit = fit;
		Invalidate();
	}

	CImageUI::ObjectFit CImageUI::GetObjectFit() const
	{
		return m_eObjectFit;
	}

	void CImageUI::SetPlaceholder(LPCTSTR pstrImage)
	{
		CDuiString s;
		if( pstrImage != NULL && *pstrImage != _T('\0') ) {
			if( !ParseCssUrlImage(pstrImage, s) ) s = pstrImage;
		}
		if( m_sPlaceholder == s ) return;
		m_sPlaceholder = s;
		Invalidate();
	}

	LPCTSTR CImageUI::GetPlaceholder() const
	{
		return m_sPlaceholder.GetData();
	}

	void CImageUI::SetErrorImage(LPCTSTR pstrImage)
	{
		CDuiString s;
		if( pstrImage != NULL && *pstrImage != _T('\0') ) {
			if( !ParseCssUrlImage(pstrImage, s) ) s = pstrImage;
		}
		if( m_sErrorImage == s ) return;
		m_sErrorImage = s;
		Invalidate();
	}

	LPCTSTR CImageUI::GetErrorImage() const
	{
		return m_sErrorImage.GetData();
	}

	void CImageUI::SetPlaceholderText(LPCTSTR pstrText)
	{
		m_sPlaceholderText = pstrText ? pstrText : _T("");
		Invalidate();
	}

	LPCTSTR CImageUI::GetPlaceholderText() const
	{
		return m_sPlaceholderText.GetData();
	}

	bool CImageUI::IsImageLoaded() const
	{
		bool bErr = false;
		return ResolveImage(bErr) != NULL && !bErr;
	}

	void CImageUI::CalcObjectFit(int imgW, int imgH, const RECT& rcBox, ObjectFit fit, RECT& rcSrc, RECT& rcDest)
	{
		rcSrc.left = 0;
		rcSrc.top = 0;
		rcSrc.right = imgW;
		rcSrc.bottom = imgH;
		rcDest = rcBox;

		int boxW = rcBox.right - rcBox.left;
		int boxH = rcBox.bottom - rcBox.top;
		if( imgW <= 0 || imgH <= 0 || boxW <= 0 || boxH <= 0 ) return;

		ObjectFit eFit = fit;
		if( eFit == FitScaleDown ) {
			if( imgW <= boxW && imgH <= boxH ) eFit = FitNone;
			else eFit = FitContain;
		}

		if( eFit == FitFill ) {
			rcDest = rcBox;
			return;
		}

		if( eFit == FitNone ) {
			rcDest.left = rcBox.left + (boxW - imgW) / 2;
			rcDest.top = rcBox.top + (boxH - imgH) / 2;
			rcDest.right = rcDest.left + imgW;
			rcDest.bottom = rcDest.top + imgH;
			return;
		}

		double sx = (double)boxW / (double)imgW;
		double sy = (double)boxH / (double)imgH;

		if( eFit == FitContain ) {
			double s = (sx < sy) ? sx : sy;
			int dw = (int)(imgW * s + 0.5);
			int dh = (int)(imgH * s + 0.5);
			if( dw < 1 ) dw = 1;
			if( dh < 1 ) dh = 1;
			rcDest.left = rcBox.left + (boxW - dw) / 2;
			rcDest.top = rcBox.top + (boxH - dh) / 2;
			rcDest.right = rcDest.left + dw;
			rcDest.bottom = rcDest.top + dh;
			return;
		}

		// Cover：铺满并裁剪源图
		double s = (sx > sy) ? sx : sy;
		int sw = (int)(boxW / s + 0.5);
		int sh = (int)(boxH / s + 0.5);
		if( sw < 1 ) sw = 1;
		if( sh < 1 ) sh = 1;
		if( sw > imgW ) sw = imgW;
		if( sh > imgH ) sh = imgH;
		rcSrc.left = (imgW - sw) / 2;
		rcSrc.top = (imgH - sh) / 2;
		rcSrc.right = rcSrc.left + sw;
		rcSrc.bottom = rcSrc.top + sh;
		rcDest = rcBox;
	}

	const TImageInfo* CImageUI::ResolveImage(bool& bIsError) const
	{
		bIsError = false;
		if( m_pManager == NULL ) return NULL;

		if( !m_sSrc.IsEmpty() ) {
			const TImageInfo* p = m_pManager->GetImageEx(m_sSrc.GetData());
			if( p != NULL ) {
				m_bLoadFailed = false;
				return p;
			}
			m_bLoadFailed = true;
			bIsError = true;
			if( !m_sErrorImage.IsEmpty() ) {
				p = m_pManager->GetImageEx(m_sErrorImage.GetData());
				if( p != NULL ) return p;
			}
		}

		if( !m_sPlaceholder.IsEmpty() )
			return m_pManager->GetImageEx(m_sPlaceholder.GetData());
		return NULL;
	}

	void CImageUI::PaintResolvedImage(IRenderContext& ctx, const TImageInfo* pInfo)
	{
		if( pInfo == NULL ) return;
		RECT rcPad = GetPadding();
		RECT rcBox = m_rcItem;
		rcBox.left += rcPad.left;
		rcBox.top += rcPad.top;
		rcBox.right -= rcPad.right;
		rcBox.bottom -= rcPad.bottom;
		if( rcBox.right <= rcBox.left || rcBox.bottom <= rcBox.top ) return;

		RECT rcSrc = { 0 }, rcDest = { 0 };
		CalcObjectFit(pInfo->nX, pInfo->nY, rcBox, m_eObjectFit, rcSrc, rcDest);
		RECT rcCorners = { 0, 0, 0, 0 };
		ctx.DrawImage(pInfo, rcDest, m_rcPaint, rcSrc, rcCorners);
	}

	void CImageUI::PaintStatusImage(IRenderContext& ctx)
	{
		bool bErr = false;
		const TImageInfo* pInfo = ResolveImage(bErr);
		if( pInfo != NULL ) {
			PaintResolvedImage(ctx, pInfo);
			return;
		}
		// 无图时留给 PaintText 画 placeholder-text；背景色由基类 PaintBackgroundColor
	}

	void CImageUI::PaintText(IRenderContext& ctx)
	{
		bool bErr = false;
		if( ResolveImage(bErr) != NULL ) return;
		if( m_sPlaceholderText.IsEmpty() ) return;

		DWORD clr = IsEnabled() ? GetColor() : GetDisabledColor();
		if( clr == 0 ) clr = 0x999999FF;
		RECT rc = m_rcItem;
		RECT rcPad = GetPadding();
		rc.left += rcPad.left;
		rc.top += rcPad.top;
		rc.right -= rcPad.right;
		rc.bottom -= rcPad.bottom;
		ctx.DrawText(rc, m_sPlaceholderText.GetData(), GetAdjustColor(clr), GetFont(),
			DT_SINGLELINE | DT_CENTER | DT_VCENTER | DT_END_ELLIPSIS);
	}

	void CImageUI::SetAttribute(LPCTSTR pstrName, LPCTSTR pstrValue)
	{
		if( _tcsicmp(pstrName, _T("src")) == 0 || _tcsicmp(pstrName, _T("url")) == 0 ) {
			SetSrc(pstrValue);
		}
		else if( _tcsicmp(pstrName, _T("object-fit")) == 0 ) {
			if( _tcsicmp(pstrValue, _T("contain")) == 0 ) SetObjectFit(FitContain);
			else if( _tcsicmp(pstrValue, _T("cover")) == 0 ) SetObjectFit(FitCover);
			else if( _tcsicmp(pstrValue, _T("none")) == 0 ) SetObjectFit(FitNone);
			else if( _tcsicmp(pstrValue, _T("scale-down")) == 0 ) SetObjectFit(FitScaleDown);
			else SetObjectFit(FitFill);
		}
		else if( _tcsicmp(pstrName, _T("placeholder")) == 0
			|| _tcsicmp(pstrName, _T("placeholder-image")) == 0 ) {
			SetPlaceholder(pstrValue);
		}
		else if( _tcsicmp(pstrName, _T("error")) == 0
			|| _tcsicmp(pstrName, _T("error-image")) == 0 ) {
			SetErrorImage(pstrValue);
		}
		else if( _tcsicmp(pstrName, _T("placeholder-text")) == 0 ) {
			SetPlaceholderText(pstrValue);
		}
		else {
			CLabelUI::SetAttribute(pstrName, pstrValue);
		}
	}

	//////////////////////////////////////////////////////////////////////////
	IMPLEMENT_DUICONTROL(CAvatarUI)

	CAvatarUI::CAvatarUI()
		: m_bCircle(true)
		, m_dwFallbackBk(0)
		, m_dwFallbackColor(0)
		, m_bFallbackBkCustom(false)
		, m_bFallbackColorCustom(false)
	{
		SetObjectFit(FitCover);
		SetFixedWidth(40);
		SetFixedHeight(40);
		SyncCircleRadius();
	}

	LPCTSTR CAvatarUI::GetClass() const
	{
		return _T("AvatarUI");
	}

	LPVOID CAvatarUI::GetInterface(LPCTSTR pstrName)
	{
		if( _tcsicmp(pstrName, DUI_CTR_AVATAR) == 0 ) return static_cast<CAvatarUI*>(this);
		return CImageUI::GetInterface(pstrName);
	}

	void CAvatarUI::SetCircle(bool bCircle)
	{
		if( m_bCircle == bCircle ) return;
		m_bCircle = bCircle;
		SyncCircleRadius();
		Invalidate();
	}

	bool CAvatarUI::IsCircle() const
	{
		return m_bCircle;
	}

	void CAvatarUI::SetAlt(LPCTSTR pstrAlt)
	{
		m_sAlt = pstrAlt ? pstrAlt : _T("");
		Invalidate();
	}

	LPCTSTR CAvatarUI::GetAlt() const
	{
		return m_sAlt.GetData();
	}

	void CAvatarUI::SetSizePreset(int nSize)
	{
		if( nSize < 8 ) nSize = 8;
		SetFixedWidth(nSize);
		SetFixedHeight(nSize);
		SyncCircleRadius();
		NeedParentUpdate();
		Invalidate();
	}

	void CAvatarUI::SetFallbackBackgroundColor(DWORD dwColor)
	{
		m_dwFallbackBk = dwColor;
		m_bFallbackBkCustom = true;
		Invalidate();
	}

	void CAvatarUI::SetFallbackColor(DWORD dwColor)
	{
		m_dwFallbackColor = dwColor;
		m_bFallbackColorCustom = true;
		Invalidate();
	}

	DWORD CAvatarUI::ResolveFallbackBackgroundColor() const
	{
		if( m_bFallbackBkCustom && m_dwFallbackBk != 0 )
			return m_dwFallbackBk;
		CThemeManager* tm = CThemeManager::GetInstance();
		if( tm != NULL )
			return tm->GetColor(_T("color-primary"), 0x0D6EFDFF);
		return 0x0D6EFDFF;
	}

	DWORD CAvatarUI::ResolveFallbackColor() const
	{
		if( m_bFallbackColorCustom && m_dwFallbackColor != 0 )
			return m_dwFallbackColor;
		CThemeManager* tm = CThemeManager::GetInstance();
		if( tm != NULL )
			return tm->GetColor(_T("color-primary-text"), 0xFFFFFFFF);
		return 0xFFFFFFFF;
	}

	void CAvatarUI::SyncCircleRadius()
	{
		if( !m_bCircle ) return;
		int w = GetFixedWidth();
		int h = GetFixedHeight();
		if( w <= 0 && h <= 0 ) {
			w = m_rcItem.right - m_rcItem.left;
			h = m_rcItem.bottom - m_rcItem.top;
		}
		int s = w;
		if( h > 0 && (s <= 0 || h < s) ) s = h;
		if( s <= 0 ) s = 40;
		SIZE sz = { s / 2, s / 2 };
		if( sz.cx < 1 ) sz.cx = 1;
		if( sz.cy < 1 ) sz.cy = 1;
		SetBorderRadius(sz);
	}

	static void AppendNextCodepoint(CDuiString& out, LPCTSTR& p)
	{
		if( p == NULL || *p == _T('\0') ) return;
		LPCTSTR p2 = ::CharNext(p);
		while( p < p2 ) { out += *p; ++p; }
	}

	CDuiString CAvatarUI::MakeInitials() const
	{
		if( !m_sAlt.IsEmpty() ) {
			LPCTSTR p = m_sAlt.GetData();
			CDuiString out;
			AppendNextCodepoint(out, p);
			if( p && *p ) AppendNextCodepoint(out, p);
			return out.IsEmpty() ? CDuiString(_T("?")) : out;
		}

		CDuiString sLabelText = CLabelUI::GetText();
		LPCTSTR p = sLabelText.GetData();
		if( p == NULL || *p == _T('\0') ) return _T("?");

		CDuiString sFirst;
		CDuiString sSecond;
		int nWords = 0;
		bool bInWord = false;
		while( *p ) {
			if( *p == _T(' ') || *p == _T('\t') ) {
				bInWord = false;
				p = ::CharNext(p);
				continue;
			}
			if( !bInWord ) {
				bInWord = true;
				++nWords;
				if( nWords == 1 ) AppendNextCodepoint(sFirst, p);
				else if( nWords == 2 ) {
					AppendNextCodepoint(sSecond, p);
					break;
				}
				else p = ::CharNext(p);
			}
			else {
				p = ::CharNext(p);
			}
		}

		if( nWords >= 2 ) return sFirst + sSecond;
		if( !sFirst.IsEmpty() ) {
			LPCTSTR q = sLabelText.GetData();
			CDuiString out;
			AppendNextCodepoint(out, q);
			if( q && *q && !_istspace(*q) ) AppendNextCodepoint(out, q);
			return out;
		}
		return _T("?");
	}

	void CAvatarUI::SetPos(RECT rc, bool bNeedInvalidate)
	{
		CImageUI::SetPos(rc, bNeedInvalidate);
		SyncCircleRadius();
	}

	SIZE CAvatarUI::EstimateSize(SIZE /*szAvailable*/)
	{
		SIZE sz = GetFixedSize();
		if( sz.cx <= 0 ) sz.cx = 40;
		if( sz.cy <= 0 ) sz.cy = 40;
		return sz;
	}

	void CAvatarUI::PaintBackgroundColor(IRenderContext& ctx)
	{
		bool bErr = false;
		const TImageInfo* pInfo = ResolveImage(bErr);
		if( pInfo != NULL ) {
			CLabelUI::PaintBackgroundColor(ctx);
			return;
		}
		DWORD dwBk = ResolveFallbackBackgroundColor();
		if( GetBackgroundColor() != 0 ) dwBk = GetBackgroundColor();
		if( dwBk == 0 ) return;
		SIZE szR = GetBorderRadius();
		if( szR.cx > 0 || szR.cy > 0 )
			ctx.FillRoundRect(m_rcItem, szR.cx, szR.cy, GetAdjustColor(dwBk));
		else
			ctx.DrawColor(m_rcItem, GetAdjustColor(dwBk));
	}

	void CAvatarUI::PaintStatusImage(IRenderContext& ctx)
	{
		bool bErr = false;
		const TImageInfo* pInfo = ResolveImage(bErr);
		if( pInfo != NULL )
			PaintResolvedImage(ctx, pInfo);
	}

	void CAvatarUI::PaintText(IRenderContext& ctx)
	{
		bool bErr = false;
		if( ResolveImage(bErr) != NULL ) return;

		CDuiString s = MakeInitials();
		if( s.IsEmpty() ) return;
		DWORD clr = ResolveFallbackColor();
		if( GetColor() != 0 ) clr = GetColor();
		RECT rc = m_rcItem;
		ctx.DrawText(rc, s.GetData(), GetAdjustColor(clr), GetFont(),
			DT_SINGLELINE | DT_CENTER | DT_VCENTER);
	}

	void CAvatarUI::SetAttribute(LPCTSTR pstrName, LPCTSTR pstrValue)
	{
		if( _tcsicmp(pstrName, _T("circle")) == 0 ) {
			SetCircle(_tcsicmp(pstrValue, _T("true")) == 0 || _tcscmp(pstrValue, _T("1")) == 0);
		}
		else if( _tcsicmp(pstrName, _T("alt")) == 0 ) {
			SetAlt(pstrValue);
		}
		else if( _tcsicmp(pstrName, _T("size")) == 0 ) {
			if( _tcsicmp(pstrValue, _T("small")) == 0 ) SetSizePreset(24);
			else if( _tcsicmp(pstrValue, _T("default")) == 0 || _tcsicmp(pstrValue, _T("medium")) == 0 ) SetSizePreset(32);
			else if( _tcsicmp(pstrValue, _T("large")) == 0 ) SetSizePreset(40);
			else if( _tcsicmp(pstrValue, _T("xlarge")) == 0 || _tcsicmp(pstrValue, _T("xl")) == 0 ) SetSizePreset(64);
			else SetSizePreset(_ttoi(pstrValue));
		}
		else if( _tcsicmp(pstrName, _T("fallback-background-color")) == 0 ) {
			DWORD clr = 0;
			if( ParseColorString(pstrValue, clr) ) SetFallbackBackgroundColor(clr);
		}
		else if( _tcsicmp(pstrName, _T("fallback-color")) == 0 ) {
			DWORD clr = 0;
			if( ParseColorString(pstrValue, clr) ) SetFallbackColor(clr);
		}
		else {
			CImageUI::SetAttribute(pstrName, pstrValue);
		}
	}
}
