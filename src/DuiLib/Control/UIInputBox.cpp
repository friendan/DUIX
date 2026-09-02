#include "StdAfx.h"
#include "UIInputBox.h"

namespace DuiLib {

namespace {

	/// 工程固定 UNICODE；内嵌皮肤以 '<' 开头，DialogBuilder 按内联 XML 加载。
	LPCTSTR GetBuiltinInputBoxSkin()
	{
		return LR"dui(<html theme="chrome">
  <VBox name="root" gap="0">
    <TitleBar name="titlebar" title="输入" height="36"
        show-min="false" show-max="false" show-close="true" action="title" />
    <VBox padding="16,16,16,16" gap="10">
      <Label name="prompt" theme="secondary" height="20" />
      <Edit name="input" height="30" border-width="1" border-radius="4" />
      <HBox height="36" gap="8" align-items="vcenter">
        <Control flexible="true" />
        <Button name="btn_ok" text="确定" kind="primary" width="72" height="30" />
        <Button name="btn_cancel" text="取消" kind="default" width="72" height="30" />
      </HBox>
    </VBox>
  </VBox>
</html>
<style>
  html {
    size: 420,180;
    caption: 0,0,0,36;
    min-size: 320,160;
  }
</style>
)dui";
	}

} // namespace

/////////////////////////////////////////////////////////////////////////////////////
// CInputBoxOptions

CInputBoxOptions::CInputBoxOptions()
	: m_bPassword(false)
	, m_bNumberOnly(false)
	, m_nMaxLength(0)
	, m_sOkText(_T("确定"))
	, m_sCancelText(_T("取消"))
	, m_nWidth(420)
	, m_nHeight(180)
	, m_bSelectAll(true)
	, m_hOwner(NULL)
{
	m_sTitle = _T("输入");
}

CInputBoxOptions& CInputBoxOptions::Title(LPCTSTR text)
{
	m_sTitle = text ? text : _T("");
	return *this;
}

CInputBoxOptions& CInputBoxOptions::Prompt(LPCTSTR text)
{
	m_sPrompt = text ? text : _T("");
	return *this;
}

CInputBoxOptions& CInputBoxOptions::Value(LPCTSTR text)
{
	m_sValue = text ? text : _T("");
	return *this;
}

CInputBoxOptions& CInputBoxOptions::Placeholder(LPCTSTR text)
{
	m_sPlaceholder = text ? text : _T("");
	return *this;
}

CInputBoxOptions& CInputBoxOptions::Password(bool password)
{
	m_bPassword = password;
	return *this;
}

CInputBoxOptions& CInputBoxOptions::Number(bool numberOnly)
{
	m_bNumberOnly = numberOnly;
	return *this;
}

CInputBoxOptions& CInputBoxOptions::MaxLength(UINT nMax)
{
	m_nMaxLength = nMax;
	return *this;
}

CInputBoxOptions& CInputBoxOptions::OkText(LPCTSTR text)
{
	m_sOkText = text ? text : _T("");
	return *this;
}

CInputBoxOptions& CInputBoxOptions::CancelText(LPCTSTR text)
{
	m_sCancelText = text ? text : _T("");
	return *this;
}

CInputBoxOptions& CInputBoxOptions::Width(int w)
{
	m_nWidth = w;
	return *this;
}

CInputBoxOptions& CInputBoxOptions::Height(int h)
{
	m_nHeight = h;
	return *this;
}

CInputBoxOptions& CInputBoxOptions::SelectAll(bool selectAll)
{
	m_bSelectAll = selectAll;
	return *this;
}

CInputBoxOptions& CInputBoxOptions::Owner(HWND hOwner)
{
	m_hOwner = hOwner;
	return *this;
}

/////////////////////////////////////////////////////////////////////////////////////
// CInputBoxWnd

class CInputBoxWnd : public WindowImplBase
{
public:
	CInputBoxWnd(const CInputBoxOptions& opts, CDuiString* pOutText)
		: m_opts(opts)
		, m_pOutText(pOutText)
		, m_pEdit(NULL)
		, m_pPrompt(NULL)
		, m_pBtnOk(NULL)
		, m_pBtnCancel(NULL)
	{
	}

	void OnFinalMessage(HWND hWnd) override
	{
		WindowImplBase::OnFinalMessage(hWnd);
		delete this;
	}

	CDuiString GetSkinFile() override
	{
		return GetBuiltinInputBoxSkin();
	}

	LPCTSTR GetWindowClassName() const override
	{
		return _T("DuiInputBoxWnd");
	}

	void InitWindow() override
	{
		m_pPrompt = static_cast<CLabelUI*>(m_pm.FindControl(_T("prompt")));
		m_pEdit = static_cast<CEditUI*>(m_pm.FindControl(_T("input")));
		m_pBtnOk = static_cast<CButtonUI*>(m_pm.FindControl(_T("btn_ok")));
		m_pBtnCancel = static_cast<CButtonUI*>(m_pm.FindControl(_T("btn_cancel")));

		CControlUI* pTb = m_pm.FindControl(_T("titlebar"));
		CTitleBarUI* pTitleBar = pTb
			? static_cast<CTitleBarUI*>(pTb->GetInterface(DUI_CTR_TITLEBAR))
			: NULL;
		if( pTitleBar != NULL && !m_opts.m_sTitle.IsEmpty() )
			pTitleBar->SetTitle(m_opts.m_sTitle.GetData());

		if( m_pPrompt != NULL ) {
			if( m_opts.m_sPrompt.IsEmpty() ) {
				m_pPrompt->SetVisible(false);
				m_pPrompt->SetFixedHeight(0);
			}
			else {
				m_pPrompt->SetText(m_opts.m_sPrompt.GetData());
			}
		}

		if( m_pEdit != NULL ) {
			m_pEdit->SetText(m_opts.m_sValue.GetData());
			if( !m_opts.m_sPlaceholder.IsEmpty() )
				m_pEdit->SetPlaceholder(m_opts.m_sPlaceholder.GetData());
			if( m_opts.m_bPassword )
				m_pEdit->SetPasswordMode(true);
			if( m_opts.m_bNumberOnly )
				m_pEdit->SetNumberOnly(true);
			if( m_opts.m_nMaxLength > 0 )
				m_pEdit->SetMaxChar(m_opts.m_nMaxLength);
			m_pEdit->SetFocus();
			if( m_opts.m_bSelectAll )
				m_pEdit->SetSelAll();
		}

		if( m_pBtnOk != NULL && !m_opts.m_sOkText.IsEmpty() )
			m_pBtnOk->SetText(m_opts.m_sOkText.GetData());
		if( m_pBtnCancel != NULL && !m_opts.m_sCancelText.IsEmpty() )
			m_pBtnCancel->SetText(m_opts.m_sCancelText.GetData());

		if( m_opts.m_nWidth > 0 && m_opts.m_nHeight > 0 && m_hWnd != NULL ) {
			RECT rc = { 0 };
			::GetWindowRect(m_hWnd, &rc);
			int w = m_opts.m_nWidth;
			int h = m_opts.m_nHeight;
			if( m_pm.GetDPIObj() != NULL ) {
				w = m_pm.GetDPIObj()->Scale(w);
				h = m_pm.GetDPIObj()->Scale(h);
			}
			::SetWindowPos(m_hWnd, NULL, 0, 0, w, h, SWP_NOMOVE | SWP_NOZORDER | SWP_NOACTIVATE);
		}
	}

	DUI_DECLARE_MESSAGE_MAP()

	void Notify(TNotifyUI& msg) override
	{
		if( msg.sType == DUI_MSGTYPE_RETURN && msg.pSender == m_pEdit ) {
			CommitOk();
			return;
		}
		WindowImplBase::Notify(msg);
	}

	void OnClick(TNotifyUI& msg) override
	{
		CDuiString sName = msg.pSender ? msg.pSender->GetName() : _T("");
		if( sName.CompareNoCase(_T("btn_ok")) == 0 ) {
			CommitOk();
			return;
		}
		if( sName.CompareNoCase(_T("btn_cancel")) == 0
			|| sName.CompareNoCase(_T("closebtn")) == 0 ) {
			Close(INPUTBOX_CANCEL);
			return;
		}
		WindowImplBase::OnClick(msg);
	}

private:
	void CommitOk()
	{
		if( m_pOutText != NULL ) {
			if( m_pEdit != NULL )
				*m_pOutText = m_pEdit->GetText();
			else
				m_pOutText->Empty();
		}
		Close(INPUTBOX_OK);
	}

	CInputBoxOptions m_opts;
	CDuiString* m_pOutText;
	CEditUI* m_pEdit;
	CLabelUI* m_pPrompt;
	CButtonUI* m_pBtnOk;
	CButtonUI* m_pBtnCancel;
};

DUI_BEGIN_MESSAGE_MAP(CInputBoxWnd, WindowImplBase)
	DUI_ON_MSGTYPE(DUI_MSGTYPE_CLICK, CInputBoxWnd::OnClick)
DUI_END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////////////
// CInputBox

int CInputBox::Show(HWND hOwner, LPCTSTR title, LPCTSTR prompt, CDuiString& outText)
{
	return Show(hOwner,
		CInputBoxOptions()
			.Title(title)
			.Prompt(prompt),
		outText);
}

int CInputBox::Show(HWND hOwner, LPCTSTR title, LPCTSTR prompt, LPCTSTR defaultValue, CDuiString& outText)
{
	return Show(hOwner,
		CInputBoxOptions()
			.Title(title)
			.Prompt(prompt)
			.Value(defaultValue),
		outText);
}

int CInputBox::Show(HWND hOwner, const CInputBoxOptions& opts, CDuiString& outText)
{
	CInputBoxOptions o = opts;
	if( hOwner != NULL )
		o.Owner(hOwner);

	CInputBoxWnd* pWnd = new CInputBoxWnd(o, &outText);
	HWND hCreateOwner = o.m_hOwner;
	pWnd->Create(hCreateOwner, o.m_sTitle.IsEmpty() ? _T("输入") : o.m_sTitle.GetData(),
		WS_POPUP | WS_CLIPCHILDREN, WS_EX_WINDOWEDGE | WS_EX_DLGMODALFRAME,
		0, 0, o.m_nWidth, o.m_nHeight);
	if( pWnd->GetHWND() == NULL ) {
		delete pWnd;
		return INPUTBOX_CANCEL;
	}
	pWnd->CenterWindow();
	return (int)pWnd->ShowModal();
}

} // namespace DuiLib
