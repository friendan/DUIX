#pragma once

class CFrameWnd: public WindowImplBase
{
public:
	explicit CFrameWnd(LPCTSTR pszXMLPath);

	LPCTSTR GetWindowClassName() const override;
	CDuiString GetSkinFile() override;
	CDuiString GetSkinFolder();

	LRESULT OnDPIChanged(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled) override;
	LRESULT OnDestroy(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled) override;
	void InitWindow() override;
	void Notify(TNotifyUI& msg) override;
	LRESULT HandleCustomMessage(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled) override;
	void setDPI(int DPI);

private:
	CDuiString		m_strXMLPath;
	CStdStringPtrMap m_MenuCheckInfo; //保存菜单的单选复选信息
};