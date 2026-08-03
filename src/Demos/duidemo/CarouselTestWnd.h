#pragma once

// Carousel 测试窗：加载 carouseltest.html
class CCarouselTestWnd : public WindowImplBase
{
public:
	CCarouselTestWnd();
	~CCarouselTestWnd();

	static void Open(HWND hParent);

	virtual void OnFinalMessage(HWND hWnd);
	virtual CDuiString GetSkinFile();
	virtual LPCTSTR GetWindowClassName() const;
	virtual void InitWindow();
	virtual void Notify(TNotifyUI& msg);

	DUI_DECLARE_MESSAGE_MAP()
	virtual void OnClick(TNotifyUI& msg);

private:
	void SetStatus(LPCTSTR pstrText);

private:
	CLabelUI* m_pStatus;
};
