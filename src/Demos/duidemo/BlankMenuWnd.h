#pragma once

// 空白右键菜单测试窗：加载 blankmenu.html
// 演示 CPaintManagerUI::SetBlankContextMenuEnabled / SetBlankContextMenuUseDeepestContainer
// 在页面未铺满的空白处右键 -> DUI_MSGTYPE_MENU（sender=根/最内层容器），弹出菜单。
class CBlankMenuWnd : public WindowImplBase
{
public:
	CBlankMenuWnd();
	~CBlankMenuWnd();

	static void Open(HWND hParent);

	void OnFinalMessage(HWND hWnd) override;
	CDuiString GetSkinFile() override;
	LPCTSTR GetWindowClassName() const override;
	void InitWindow() override;
	void Notify(TNotifyUI& msg) override;

	DUI_DECLARE_MESSAGE_MAP()
	void OnClick(TNotifyUI& msg) override;

	private:
	// 空白右键 OnNotify 回调（MakeDelegate 绑定到目标容器），签名 bool(void*)
	bool OnBlankMenu(void* p);
	void ShowBlankMenu(CControlUI* pSender, POINT ptScreen);
	void SetStatus(LPCTSTR pstrText);

private:
	CLabelUI* m_pStatus;
	CMenuWnd* m_pMenu;
};
