#pragma once



class CShapeDemoWnd : public WindowImplBase

{

public:

	CShapeDemoWnd() {}

	~CShapeDemoWnd() {}



	virtual void OnFinalMessage(HWND hWnd)

	{

		WindowImplBase::OnFinalMessage(hWnd);

		delete this;

	}

	virtual CDuiString GetSkinFile() { return _T("shapedemo.html"); }

	virtual LPCTSTR GetWindowClassName() const { return _T("DuiShapeDemoWnd"); }



	virtual void InitWindow()

	{

		m_pm.SetLayeredCompositionEnabled(false);

		FitToShapeImage(true, true);

	}



	DUI_DECLARE_MESSAGE_MAP()

	virtual void OnClick(TNotifyUI& msg)

	{

		if( msg.pSender != NULL && msg.pSender->GetName() == _T("closebtn") )

			Close(0);

	}

};


