#pragma once

class CShapeDemoWnd : public WindowImplBase
{
public:
	CShapeDemoWnd() {}
	~CShapeDemoWnd() {}

	void OnFinalMessage(HWND hWnd) override
	{
		WindowImplBase::OnFinalMessage(hWnd);
		delete this;
	}
	CDuiString GetSkinFile() override { return _T("shapedemo.html"); }
	LPCTSTR GetWindowClassName() const override { return _T("DuiShapeDemoWnd"); }

	void InitWindow() override
	{
		m_pm.SetLayeredCompositionEnabled(false);
		FitToShapeImage(true, true);
	}

	DUI_DECLARE_MESSAGE_MAP()
	void OnClick(TNotifyUI& msg) override
	{
		if( msg.pSender != NULL && msg.pSender->GetName() == _T("closebtn") )
			Close(0);
	}
};
