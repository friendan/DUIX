#include "StdAfx.h"
#include "UIShapeButton.h"
#include "Utils/UIShape.h"

namespace DuiLib
{
	IMPLEMENT_DUICONTROL(CShapeButtonUI)

	CShapeButtonUI::CShapeButtonUI()
		: m_nShapeAlphaThreshold(16)
		, m_pShapeBits(NULL)
		, m_nShapeW(0)
		, m_nShapeH(0)
		, m_nShapeStride(0)
	{
		SetBackgroundColor(0);
		SetKind(CONTROLKIND_NONE);
		SetCursor((WORD)DUI_HAND);
	}

	CShapeButtonUI::~CShapeButtonUI()
	{
		InvalidateShapeMask();
	}

	LPCTSTR CShapeButtonUI::GetClass() const
	{
		return _T("ShapeButtonUI");
	}

	LPVOID CShapeButtonUI::GetInterface(LPCTSTR pstrName)
	{
		if( _tcsicmp(pstrName, DUI_CTR_SHAPEBUTTON) == 0 )
			return static_cast<CShapeButtonUI*>(this);
		return CButtonUI::GetInterface(pstrName);
	}

	UINT CShapeButtonUI::GetControlFlags() const
	{
		return CButtonUI::GetControlFlags() | UIFLAG_SETCURSOR;
	}

	void CShapeButtonUI::InvalidateShapeMask()
	{
		delete[] m_pShapeBits;
		m_pShapeBits = NULL;
		m_nShapeW = m_nShapeH = m_nShapeStride = 0;
		m_sShapeLoaded.Empty();
	}

	void CShapeButtonUI::SetShapeImage(LPCTSTR pstrImage)
	{
		m_sShapeImage = pstrImage ? pstrImage : _T("");
		InvalidateShapeMask();
		// 未另设 Button image 时，shape-image 兼作常态图
		if( !m_sShapeImage.IsEmpty() && (GetImage() == NULL || *GetImage() == _T('\0')) ) {
			SetImage(m_sShapeImage.GetData());
			SetBackgroundColor(0);
		}
		Invalidate();
	}

	LPCTSTR CShapeButtonUI::GetShapeImage() const
	{
		return m_sShapeImage.GetData();
	}

	void CShapeButtonUI::SetShapeMask(LPCTSTR pstrMask)
	{
		m_sShapeMask = pstrMask ? pstrMask : _T("");
		InvalidateShapeMask();
		Invalidate();
	}

	LPCTSTR CShapeButtonUI::GetShapeMask() const
	{
		return m_sShapeMask.GetData();
	}

	LPCTSTR CShapeButtonUI::GetShapeHitImage() const
	{
		if( !m_sShapeMask.IsEmpty() ) return m_sShapeMask.GetData();
		return m_sShapeImage.GetData();
	}

	void CShapeButtonUI::SetShapeAlphaThreshold(BYTE nThreshold)
	{
		m_nShapeAlphaThreshold = nThreshold;
	}

	BYTE CShapeButtonUI::GetShapeAlphaThreshold() const
	{
		return m_nShapeAlphaThreshold;
	}

	bool CShapeButtonUI::EnsureShapeMask()
	{
		LPCTSTR pHit = GetShapeHitImage();
		if( pHit == NULL || *pHit == _T('\0') || m_pManager == NULL ) {
			InvalidateShapeMask();
			return false;
		}
		if( m_pShapeBits != NULL && m_sShapeLoaded == pHit )
			return true;

		InvalidateShapeMask();
		const TImageInfo* pInfo = m_pManager->GetImageEx(pHit);
		if( pInfo == NULL || pInfo->hBitmap == NULL ) return false;
		if( !CopyBitmapAlphaBits(pInfo->hBitmap, &m_pShapeBits, &m_nShapeW, &m_nShapeH, &m_nShapeStride) )
			return false;
		m_sShapeLoaded = pHit;
		return true;
	}

	bool CShapeButtonUI::HitTestShape(POINT pt) const
	{
		LPCTSTR pHit = GetShapeHitImage();
		if( pHit == NULL || *pHit == _T('\0') ) return true;
		if( !const_cast<CShapeButtonUI*>(this)->EnsureShapeMask() ) return true;
		return HitTestAlphaInDestRect(m_pShapeBits, m_nShapeW, m_nShapeH, m_rcItem, pt,
			m_nShapeStride, m_nShapeAlphaThreshold);
	}

	CControlUI* CShapeButtonUI::FindControl(FINDCONTROLPROC Proc, LPVOID pData, UINT uFlags)
	{
		if( (uFlags & UIFIND_HITTEST) != 0 ) {
			if( (uFlags & UIFIND_VISIBLE) != 0 && !IsVisible() ) return NULL;
			if( (uFlags & UIFIND_ENABLED) != 0 && !IsEnabled() ) return NULL;
			POINT pt = *static_cast<LPPOINT>(pData);
			if( !m_bMouseEnabled || !::PtInRect(&m_rcItem, pt) ) return NULL;
			if( !HitTestShape(pt) ) return NULL;
			return Proc(this, pData);
		}
		return CButtonUI::FindControl(Proc, pData, uFlags);
	}

	void CShapeButtonUI::SetAttribute(LPCTSTR pstrName, LPCTSTR pstrValue)
	{
		if( _tcsicmp(pstrName, _T("shape-image")) == 0 || _tcsicmp(pstrName, _T("src")) == 0 ) {
			SetShapeImage(pstrValue);
		}
		else if( _tcsicmp(pstrName, _T("shape-mask")) == 0 ) {
			SetShapeMask(pstrValue);
		}
		else if( _tcsicmp(pstrName, _T("shape-alpha-threshold")) == 0
			|| _tcsicmp(pstrName, _T("alpha-threshold")) == 0 ) {
			SetShapeAlphaThreshold((BYTE)_ttoi(pstrValue));
		}
		else {
			CButtonUI::SetAttribute(pstrName, pstrValue);
		}
	}

	/////////////////////////////////////////////////////////////////////////////////////

	IMPLEMENT_DUICONTROL(CShapeBoxUI)

	CShapeBoxUI::CShapeBoxUI()
		: m_nShapeAlphaThreshold(16)
		, m_pShapeBits(NULL)
		, m_nShapeW(0)
		, m_nShapeH(0)
		, m_nShapeStride(0)
	{
		SetBackgroundColor(0);
	}

	CShapeBoxUI::~CShapeBoxUI()
	{
		InvalidateShapeMask();
	}

	LPCTSTR CShapeBoxUI::GetClass() const
	{
		return _T("ShapeBoxUI");
	}

	LPVOID CShapeBoxUI::GetInterface(LPCTSTR pstrName)
	{
		if( _tcsicmp(pstrName, DUI_CTR_SHAPEBOX) == 0 )
			return static_cast<CShapeBoxUI*>(this);
		return CContainerUI::GetInterface(pstrName);
	}

	void CShapeBoxUI::InvalidateShapeMask()
	{
		delete[] m_pShapeBits;
		m_pShapeBits = NULL;
		m_nShapeW = m_nShapeH = m_nShapeStride = 0;
		m_sShapeLoaded.Empty();
	}

	void CShapeBoxUI::SetShapeImage(LPCTSTR pstrImage)
	{
		m_sShapeImage = pstrImage ? pstrImage : _T("");
		InvalidateShapeMask();
		Invalidate();
	}

	LPCTSTR CShapeBoxUI::GetShapeImage() const
	{
		return m_sShapeImage.GetData();
	}

	void CShapeBoxUI::SetShapeMask(LPCTSTR pstrMask)
	{
		m_sShapeMask = pstrMask ? pstrMask : _T("");
		InvalidateShapeMask();
		Invalidate();
	}

	LPCTSTR CShapeBoxUI::GetShapeMask() const
	{
		return m_sShapeMask.GetData();
	}

	LPCTSTR CShapeBoxUI::GetShapeHitImage() const
	{
		if( !m_sShapeMask.IsEmpty() ) return m_sShapeMask.GetData();
		return m_sShapeImage.GetData();
	}

	void CShapeBoxUI::SetShapeAlphaThreshold(BYTE nThreshold)
	{
		m_nShapeAlphaThreshold = nThreshold;
	}

	BYTE CShapeBoxUI::GetShapeAlphaThreshold() const
	{
		return m_nShapeAlphaThreshold;
	}

	bool CShapeBoxUI::EnsureShapeMask()
	{
		LPCTSTR pHit = GetShapeHitImage();
		if( pHit == NULL || *pHit == _T('\0') || m_pManager == NULL ) {
			InvalidateShapeMask();
			return false;
		}
		if( m_pShapeBits != NULL && m_sShapeLoaded == pHit )
			return true;
		InvalidateShapeMask();
		const TImageInfo* pInfo = m_pManager->GetImageEx(pHit);
		if( pInfo == NULL || pInfo->hBitmap == NULL ) return false;
		if( !CopyBitmapAlphaBits(pInfo->hBitmap, &m_pShapeBits, &m_nShapeW, &m_nShapeH, &m_nShapeStride) )
			return false;
		m_sShapeLoaded = pHit;
		return true;
	}

	bool CShapeBoxUI::HitTestShape(POINT pt) const
	{
		LPCTSTR pHit = GetShapeHitImage();
		if( pHit == NULL || *pHit == _T('\0') ) return true;
		if( !const_cast<CShapeBoxUI*>(this)->EnsureShapeMask() ) return true;
		return HitTestAlphaInDestRect(m_pShapeBits, m_nShapeW, m_nShapeH, m_rcItem, pt,
			m_nShapeStride, m_nShapeAlphaThreshold);
	}

	CControlUI* CShapeBoxUI::FindControl(FINDCONTROLPROC Proc, LPVOID pData, UINT uFlags)
	{
		if( (uFlags & UIFIND_HITTEST) != 0 ) {
			if( (uFlags & UIFIND_VISIBLE) != 0 && !IsVisible() ) return NULL;
			if( (uFlags & UIFIND_ENABLED) != 0 && !IsEnabled() ) return NULL;
			POINT pt = *static_cast<LPPOINT>(pData);
			if( !::PtInRect(&m_rcItem, pt) ) return NULL;
			if( !HitTestShape(pt) ) return NULL;
		}
		return CContainerUI::FindControl(Proc, pData, uFlags);
	}

	void CShapeBoxUI::PaintStatusImage(IRenderContext& ctx)
	{
		if( !m_sShapeImage.IsEmpty() )
			DrawImage(ctx, m_sShapeImage.GetData());
		CContainerUI::PaintStatusImage(ctx);
	}

	void CShapeBoxUI::SetAttribute(LPCTSTR pstrName, LPCTSTR pstrValue)
	{
		if( _tcsicmp(pstrName, _T("shape-image")) == 0 || _tcsicmp(pstrName, _T("src")) == 0 ) {
			SetShapeImage(pstrValue);
		}
		else if( _tcsicmp(pstrName, _T("shape-mask")) == 0 ) {
			SetShapeMask(pstrValue);
		}
		else if( _tcsicmp(pstrName, _T("shape-alpha-threshold")) == 0
			|| _tcsicmp(pstrName, _T("alpha-threshold")) == 0 ) {
			SetShapeAlphaThreshold((BYTE)_ttoi(pstrValue));
		}
		else {
			CContainerUI::SetAttribute(pstrName, pstrValue);
		}
	}
}
