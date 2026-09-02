#pragma once

/// AppGrid sparse 独立测试窗：空位拖放 / 互换 / 删除留洞 / 压缩
class CAppGridSparseTestWnd : public WindowImplBase
{
public:
	CAppGridSparseTestWnd();
	~CAppGridSparseTestWnd();

	static void Open(HWND hParent);

	void OnFinalMessage(HWND hWnd) override;
	CDuiString GetSkinFile() override;
	LPCTSTR GetWindowClassName() const override;
	void InitWindow() override;
	void Notify(TNotifyUI& msg) override;

	DUI_DECLARE_MESSAGE_MAP()
	void OnClick(TNotifyUI& msg) override;

private:
	void SeedSparseHoles();
	void UpdateStatus(LPCTSTR tip);
	CAppGridUI* GetGrid() const;
	CAppIconUI* CreateDemoIcon(LPCTSTR name, LPCTSTR text, LPCTSTR lucide);

private:
	int m_nSelected;
	int m_nAddSeq;
};
