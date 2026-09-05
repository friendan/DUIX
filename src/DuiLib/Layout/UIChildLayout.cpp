#include "StdAfx.h"
#include "UIChildLayout.h"

namespace DuiLib
{
	IMPLEMENT_DUICONTROL(CChildLayoutUI)

	CChildLayoutUI::CChildLayoutUI()
	{

	}

	void CChildLayoutUI::Init()
	{
		if (!m_pstrXMLFile.IsEmpty())
		{
			// 幂等加载：皮肤树已挂载（本控件已有子项）则不再重建。
			// DUIX 对已激活容器每次 Add/AddAt 都会重调 SetManager→Init（标签换序
			// MoveItem=Remove+AddAt 即触发），此处无条件 RemoveAll+重建会删掉存活皮肤树，
			// 使页面缓存的 siteGrid 等后代控件指针悬垂 → 下次布局对其调虚函数即 UAF 崩溃。
			// 子皮肤仅在首次入树时创建一次，此后重复入树仅重链 manager，树保持原样。
			if (GetCount() > 0)
				return;
			this->RemoveAll();
			CDialogBuilder builder;
			CContainerUI* pChildWindow = static_cast<CContainerUI*>(builder.Create(m_pstrXMLFile.GetData(), NULL, NULL, m_pManager));
			if (pChildWindow)
			{
				this->Add(pChildWindow);
			}
		}
	}

	void CChildLayoutUI::SetAttribute( LPCTSTR pstrName, LPCTSTR pstrValue )
	{
		if( _tcsicmp(pstrName, _T("xml-file")) == 0 )
			SetChildLayoutXML(pstrValue);
		else
			CContainerUI::SetAttribute(pstrName,pstrValue);
	}

	void CChildLayoutUI::SetChildLayoutXML( DuiLib::CDuiString pXML )
	{
		m_pstrXMLFile=pXML;
	}

	DuiLib::CDuiString CChildLayoutUI::GetChildLayoutXML()
	{
		return m_pstrXMLFile;
	}

	LPVOID CChildLayoutUI::GetInterface( LPCTSTR pstrName )
	{
		if( _tcsicmp(pstrName, DUI_CTR_CHILDLAYOUT) == 0 ) return static_cast<CChildLayoutUI*>(this);
		return CContainerUI::GetInterface(pstrName);
	}

	LPCTSTR CChildLayoutUI::GetClass() const
	{
		return _T("ChildLayoutUI");
	}
} // namespace DuiLib
