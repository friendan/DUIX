#include "StdAfx.h"
#include "AppGridSparseTestWnd.h"

DUI_BEGIN_MESSAGE_MAP(CAppGridSparseTestWnd, WindowImplBase)
	DUI_ON_MSGTYPE(DUI_MSGTYPE_CLICK, CAppGridSparseTestWnd::OnClick)
DUI_END_MESSAGE_MAP()

CAppGridSparseTestWnd::CAppGridSparseTestWnd()
	: m_nSelected(-1)
	, m_nAddSeq(0)
{
}

CAppGridSparseTestWnd::~CAppGridSparseTestWnd()
{
}

void CAppGridSparseTestWnd::Open(HWND hParent)
{
	CAppGridSparseTestWnd* pWnd = new CAppGridSparseTestWnd();
	pWnd->Create(hParent, _T("AppGrid Sparse"), UI_WNDSTYLE_FRAME, WS_EX_WINDOWEDGE, 0, 0, 720, 520);
	pWnd->CenterWindow();
	pWnd->ShowWindow(true);
}

void CAppGridSparseTestWnd::OnFinalMessage(HWND hWnd)
{
	WindowImplBase::OnFinalMessage(hWnd);
	delete this;
}

CDuiString CAppGridSparseTestWnd::GetSkinFile()
{
	return _T("appgridsparse.htm");
}

LPCTSTR CAppGridSparseTestWnd::GetWindowClassName() const
{
	return _T("AppGridSparseTestWnd");
}

CAppGridUI* CAppGridSparseTestWnd::GetGrid() const
{
	CControlUI* p = m_pm.FindControl(_T("sparse_grid"));
	if( p == NULL ) return NULL;
	return static_cast<CAppGridUI*>(p->GetInterface(DUI_CTR_APPGRID));
}

CAppIconUI* CAppGridSparseTestWnd::CreateDemoIcon(LPCTSTR name, LPCTSTR text, LPCTSTR lucide)
{
	CAppIconUI* pIcon = new CAppIconUI();
	if( name != NULL ) pIcon->SetName(name);
	if( text != NULL ) pIcon->SetText(text);
	if( lucide != NULL && *lucide != _T('\0') )
		pIcon->SetIconLib(_T("lucide"), lucide);
	return pIcon;
}

void CAppGridSparseTestWnd::SeedSparseHoles()
{
	CAppGridUI* pGrid = GetGrid();
	if( pGrid == NULL || !pGrid->IsSparse() ) return;

	// 在现有图标之间插入空位：I S I S I …
	int n = pGrid->GetGridItemCount();
	for( int i = n - 1; i >= 1; --i ) {
		CControlUI* pSlot = CAppGridUI::CreateSlot();
		if( pSlot != NULL )
			pGrid->AddAt(pSlot, i);
	}
	// 尾部补满至少一页容量，方便拖到空白格
	int per = pGrid->GetPerPage();
	if( per < 12 ) per = 12;
	pGrid->EnsureSlotCount(per);

	CDuiString tip;
	tip.Format(_T("已预留空位：真图标 %d / 总格 %d。拖到空格留洞，拖到图标互换。"),
		pGrid->GetRealItemCount(), pGrid->GetGridItemCount());
	UpdateStatus(tip.GetData());
}

void CAppGridSparseTestWnd::UpdateStatus(LPCTSTR tip)
{
	CLabelUI* pTip = static_cast<CLabelUI*>(m_pm.FindControl(_T("sparse_status")));
	if( pTip != NULL && tip != NULL )
		pTip->SetText(tip);
}

void CAppGridSparseTestWnd::InitWindow()
{
	SeedSparseHoles();
}

void CAppGridSparseTestWnd::Notify(TNotifyUI& msg)
{
	if( msg.pSender != NULL && msg.pSender->GetName() == _T("sparse_grid") ) {
		CAppGridUI* pGrid = GetGrid();
		if( msg.sType == DUI_MSGTYPE_ITEMCLICK ) {
			m_nSelected = (int)msg.wParam;
			CControlUI* pItem = (CControlUI*)msg.lParam;
			CDuiString tip;
			tip.Format(_T("选中 [%d] %s"), m_nSelected,
				pItem ? pItem->GetText().GetData() : _T(""));
			UpdateStatus(tip.GetData());
		}
		else if( msg.sType == DUI_MSGTYPE_ITEMMOVED ) {
			CDuiString tip;
			tip.Format(_T("itemmoved: %d ↔ %d（真图标 %d / 总格 %d）"),
				(int)msg.wParam, (int)msg.lParam,
				pGrid ? pGrid->GetRealItemCount() : 0,
				pGrid ? pGrid->GetGridItemCount() : 0);
			UpdateStatus(tip.GetData());
		}
		else if( msg.sType == DUI_MSGTYPE_DRAGEND ) {
			CDuiString tip;
			tip.Format(_T("dragend: from=%d to=%d"), (int)msg.wParam, (int)msg.lParam);
			UpdateStatus(tip.GetData());
		}
	}
	WindowImplBase::Notify(msg);
}

void CAppGridSparseTestWnd::OnClick(TNotifyUI& msg)
{
	CDuiString sName = msg.pSender->GetName();
	CAppGridUI* pGrid = GetGrid();

	if( sName == _T("btn_sparse_delete") ) {
		if( pGrid == NULL || m_nSelected < 0 ) {
			UpdateStatus(_T("请先点击选中一个图标再删除"));
			return;
		}
		if( pGrid->IsSlotEmptyAt(m_nSelected) ) {
			UpdateStatus(_T("当前下标是空位，请点图标"));
			return;
		}
		int idx = m_nSelected;
		if( pGrid->RemoveGridItemAt(idx) ) {
			m_nSelected = -1;
			CDuiString tip;
			tip.Format(_T("已删除 [%d]，该格留空。真图标 %d / 总格 %d"),
				idx, pGrid->GetRealItemCount(), pGrid->GetGridItemCount());
			UpdateStatus(tip.GetData());
		}
		return;
	}
	if( sName == _T("btn_sparse_compact") ) {
		if( pGrid == NULL ) return;
		pGrid->CompactSlots();
		m_nSelected = -1;
		CDuiString tip;
		tip.Format(_T("已压缩空位。真图标 %d / 总格 %d"),
			pGrid->GetRealItemCount(), pGrid->GetGridItemCount());
		UpdateStatus(tip.GetData());
		return;
	}
	if( sName == _T("btn_sparse_add") ) {
		if( pGrid == NULL ) return;
		++m_nAddSeq;
		CDuiString name, text;
		name.Format(_T("sparse_add_%d"), m_nAddSeq);
		text.Format(_T("新增%d"), m_nAddSeq);
		static const LPCTSTR kIcons[] = {
			_T("mail"), _T("star"), _T("heart"), _T("cloud"), _T("music"), _T("map")
		};
		LPCTSTR lucide = kIcons[m_nAddSeq % 6];
		CAppIconUI* pIcon = CreateDemoIcon(name.GetData(), text.GetData(), lucide);
		if( pIcon == NULL ) return;
		int idx = pGrid->AddToFirstEmpty(pIcon);
		if( idx >= 0 ) {
			CDuiString tip;
			tip.Format(_T("已放入空位 [%d]「%s」。真图标 %d / 总格 %d"),
				idx, text.GetData(), pGrid->GetRealItemCount(), pGrid->GetGridItemCount());
			UpdateStatus(tip.GetData());
		}
		else {
			delete pIcon;
			UpdateStatus(_T("追加失败"));
		}
		return;
	}
	if( sName == _T("btn_sparse_pad") ) {
		if( pGrid == NULL ) return;
		int per = pGrid->GetPerPage();
		if( per < 1 ) per = 12;
		int target = ((pGrid->GetGridItemCount() / per) + 1) * per;
		pGrid->EnsureSlotCount(target);
		CDuiString tip;
		tip.Format(_T("已补空位到 %d 格（便于拖到空白）"), pGrid->GetGridItemCount());
		UpdateStatus(tip.GetData());
		return;
	}

	WindowImplBase::OnClick(msg);
}
