#pragma once

// 图标选择控件测试窗：加载 iconpicker_wnd.html
// 演示 CIconPickerUI（<IconPicker>）：点击触发器 → 弹出图标选择窗 → 确定后 selectchanged。
// 三个例子：例1 libs 白名单（2 库）；例2 不设 libs（全部 7 库 + default-lib）；例3 隐藏大小/颜色设置（固定宽高颜色）。
class CIconPickerDemoWnd : public WindowImplBase
{
public:
	CIconPickerDemoWnd();
	~CIconPickerDemoWnd();

	static void Open(HWND hParent);

	virtual void OnFinalMessage(HWND hWnd);
	virtual CDuiString GetSkinFile();
	virtual LPCTSTR GetWindowClassName() const;
	virtual void InitWindow();
	virtual void Notify(TNotifyUI& msg);

	DUI_DECLARE_MESSAGE_MAP()
	virtual void OnClick(TNotifyUI& msg);

private:
	void UpdateStatus();

private:
	CIconPickerUI* m_pPickerFilter;   // 例1：白名单
	CIconPickerUI* m_pPickerAll;      // 例2：全部
	CIconPickerUI* m_pPickerFixed;    // 例3：固定宽高/颜色
	CLabelUI* m_pStatusFilter;
	CLabelUI* m_pStatusAll;
	CLabelUI* m_pStatusFixed;
};
