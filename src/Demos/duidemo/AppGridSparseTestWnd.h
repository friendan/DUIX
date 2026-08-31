#pragma once

/// AppGrid sparse 独立测试窗：空位拖放 / 互换 / 删除留洞 / 压缩
class CAppGridSparseTestWnd : public WindowImplBase
{
public:
	CAppGridSparseTestWnd();
	~CAppGridSparseTestWnd();

	static void Open(HWND hParent);

	virtual void OnFinalMessage(HWND hWnd);
	virtual CDuiString GetSkinFile();
	virtual LPCTSTR GetWindowClassName() const;
	virtual void InitWindow();
	virtual void Notify(TNotifyUI& msg);

	DUI_DECLARE_MESSAGE_MAP()
	virtual void OnClick(TNotifyUI& msg);

private:
	void SeedSparseHoles();
	void UpdateStatus(LPCTSTR tip);
	CAppGridUI* GetGrid() const;
	CAppIconUI* CreateDemoIcon(LPCTSTR name, LPCTSTR text, LPCTSTR lucide);

private:
	int m_nSelected;
	int m_nAddSeq;
};
