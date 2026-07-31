#include "StdAfx.h"
#include "ShortCutUI.h"


CShortCutUI::CShortCutUI()
{
}


CShortCutUI::~CShortCutUI(void)
{
	if(m_hIcon != NULL)
		::DestroyIcon(m_hIcon);
}

void CShortCutUI::PaintStatusImage(IRenderContext& ctx)
{
	CButtonUI::PaintStatusImage(ctx);
	if(m_hIcon != NULL)
	{
		HDC hDC = ctx.GetDC();
		ICONINFO ii;
		if(::GetIconInfo(m_hIcon, &ii))
		{
			BITMAP bmp;
			if(::GetObject(ii.hbmColor, sizeof(bmp), &bmp))
				::DrawIcon(hDC, m_rcItem.left, m_rcItem.top, m_hIcon);
			::DeleteObject(ii.hbmColor);
			::DeleteObject(ii.hbmMask);
		}
	}
}

void CShortCutUI::SetText(LPCTSTR pstrText)
{
	m_sText = pstrText;
}

void CShortCutUI::SetIcon(HICON hIcon)
{
	m_hIcon = hIcon;
}
