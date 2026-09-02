#pragma once

// Carousel 测试窗：加载 carouseltest.html
class CCarouselTestWnd : public WindowImplBase
{
public:
	CCarouselTestWnd();
	~CCarouselTestWnd();

	static void Open(HWND hParent);

	void OnFinalMessage(HWND hWnd) override;
	CDuiString GetSkinFile() override;
	LPCTSTR GetWindowClassName() const override;
	void InitWindow() override;
	void Notify(TNotifyUI& msg) override;

	DUI_DECLARE_MESSAGE_MAP()
	void OnClick(TNotifyUI& msg) override;

private:
	void SetStatus(LPCTSTR pstrText);

private:
	CLabelUI* m_pStatus;
};
