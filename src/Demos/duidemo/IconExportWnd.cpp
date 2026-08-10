#include "StdAfx.h"
#include "IconExportWnd.h"
#include "Core/UITheme.h"
#include <shlobj.h>
#include <shellapi.h>
#include <commdlg.h>
#include <string>
#include <memory>
#include <lunasvg.h>
namespace {
	DWORD ExportThemeToken(LPCTSTR pstrName, DWORD dwFallback)
	{
		CThemeManager* pTm = CThemeManager::GetInstance();
		if( pTm == NULL ) return dwFallback;
		CTheme* pTh = pTm->GetCurrentTheme();
		if( pTh == NULL ) pTh = pTm->FindTheme(pTm->GetDefaultThemeId());
		if( pTh == NULL ) return dwFallback;
		return pTh->GetToken(pstrName, dwFallback);
	}
	void FormatColorLabel(CDuiString& sOut, DWORD dw, bool bNoTint)
	{
		if( bNoTint ) {
			sOut = _T("不着色（保留 SVG 原色）");
			return;
		}
		sOut.Format(_T("#%08X"), dw);
	}
	int CALLBACK BrowseDirCallback(HWND hwnd, UINT uMsg, LPARAM /*lParam*/, LPARAM lpData)
	{
		if( uMsg == BFFM_INITIALIZED && lpData != 0 )
			::SendMessage(hwnd, BFFM_SETSELECTION, TRUE, lpData);
		return 0;
	}
	int ClampExportSize(int v)
	{
		if( v < 1 ) return 1;
		if( v > 4096 ) return 4096;
		return v;
	}
} // namespace
DUI_BEGIN_MESSAGE_MAP(CIconExportWnd, WindowImplBase)
	DUI_ON_MSGTYPE(DUI_MSGTYPE_CLICK, CIconExportWnd::OnClick)
DUI_END_MESSAGE_MAP()
CDuiString CIconExportWnd::GetDefaultExportDir()
{
	TCHAR szPath[MAX_PATH] = { 0 };
	DWORD n = ::GetModuleFileName(NULL, szPath, MAX_PATH);
	if( n == 0 || n >= MAX_PATH ) return _T("svgimg");
	TCHAR* pSlash = _tcsrchr(szPath, _T('\\'));
	if( pSlash == NULL ) pSlash = _tcsrchr(szPath, _T('/'));
	if( pSlash != NULL ) *pSlash = _T('\0');
	CDuiString sDir = szPath;
	sDir += _T("\\svgimg");
	return sDir;
}
CDuiString& CIconExportWnd::SharedExportDir()
{
	static CDuiString sDir;
	if( sDir.IsEmpty() )
		sDir = GetDefaultExportDir();
	return sDir;
}
CDuiString CIconExportWnd::FileBaseName(LPCTSTR pstrPath)
{
	if( pstrPath == NULL || *pstrPath == _T('\0') ) return _T("icon");
	LPCTSTR pName = pstrPath;
	LPCTSTR pSlash = _tcsrchr(pstrPath, _T('\\'));
	if( pSlash == NULL ) pSlash = _tcsrchr(pstrPath, _T('/'));
	if( pSlash != NULL ) pName = pSlash + 1;
	CDuiString s = pName;
	int nDot = s.ReverseFind(_T('.'));
	if( nDot > 0 ) s = s.Left(nDot);
	if( s.IsEmpty() ) s = _T("icon");
	return s;
}
CIconExportWnd::CIconExportWnd(LPCTSTR pstrLib, LPCTSTR pstrName, const char* utf8Svg, int nCtrlW, int nCtrlH)
	: m_sLib(pstrLib ? pstrLib : _T(""))
	, m_sName(pstrName ? pstrName : _T(""))
	, m_sSvgUtf8(utf8Svg ? utf8Svg : "")
	, m_nCtrlW(nCtrlW > 0 ? nCtrlW : 0)
	, m_nCtrlH(nCtrlH > 0 ? nCtrlH : 0)
	, m_pPreview(NULL)
	, m_pFormat(NULL)
	, m_pPalette(NULL)
	, m_pColorLabel(NULL)
	, m_pDirLabel(NULL)
	, m_pEditW(NULL)
	, m_pEditH(NULL)
	, m_pColorRow(NULL)
	, m_dwTint(0x333333FF)
	, m_bNoTint(false)
{
	m_dwTint = ExportThemeToken(_T("color-text"), 0x333333FF);
}
CIconExportWnd::~CIconExportWnd()
{
}
void CIconExportWnd::Open(HWND hOwner, LPCTSTR pstrLib, LPCTSTR pstrName, const char* utf8Svg, int nCtrlW, int nCtrlH)
{
	if( pstrName == NULL || *pstrName == _T('\0') ) return;
	if( utf8Svg == NULL || *utf8Svg == '\0' ) return;
	CIconExportWnd* pWnd = new CIconExportWnd(pstrLib, pstrName, utf8Svg, nCtrlW, nCtrlH);
	pWnd->Create(hOwner, _T("导出图标"), UI_WNDSTYLE_FRAME, WS_EX_WINDOWEDGE, 0, 0, 440, 600);
	if( pWnd->GetHWND() == NULL ) {
		delete pWnd;
		return;
	}
	pWnd->CenterWindow();
	pWnd->ShowModal();
}
void CIconExportWnd::OpenBlank(HWND hOwner, bool bPickFile)
{
	CDuiString sPath;
	std::string utf8;
	if( bPickFile ) {
		if( !PickSvgPath(hOwner, sPath) ) return;
		HANDLE hFile = ::CreateFile(sPath.GetData(), GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
		if( hFile == INVALID_HANDLE_VALUE ) {
			CToast::ShowDanger(_T("无法读取 SVG 文件"), 2500);
			return;
		}
		LARGE_INTEGER li = {};
		bool bReadOk = false;
		if( ::GetFileSizeEx(hFile, &li) && li.QuadPart > 0 && li.QuadPart <= 8 * 1024 * 1024 ) {
			utf8.resize((size_t)li.QuadPart);
			DWORD nRead = 0;
			bReadOk = ::ReadFile(hFile, &utf8[0], (DWORD)utf8.size(), &nRead, NULL) && nRead == utf8.size();
			if( bReadOk ) utf8.resize(nRead);
		}
		::CloseHandle(hFile);
		if( !bReadOk ) {
			CToast::ShowDanger(_T("读取 SVG 失败或文件过大"), 2500);
			return;
		}
		if( !ContainsSvgRootTag(utf8) ) {
			CToast::ShowWarning(_T("文件不是 SVG（未找到合法 <svg> 标签）"), 2800);
			return;
		}
		if( !IsValidSvgUtf8(utf8) ) {
			CToast::ShowDanger(_T("SVG 无法解析，请确认文件内容合法"), 2800);
			return;
		}
	}
	CIconExportWnd* pWnd = new CIconExportWnd(_T("file"),
		sPath.IsEmpty() ? _T("") : FileBaseName(sPath.GetData()).GetData(),
		utf8.empty() ? NULL : utf8.c_str(), 256, 256);
	if( !sPath.IsEmpty() ) {
		pWnd->m_sSvgPath = sPath;
		pWnd->m_sLib = _T("file");
	}
	pWnd->Create(hOwner, _T("导出 SVG"), UI_WNDSTYLE_FRAME, WS_EX_WINDOWEDGE, 0, 0, 440, 600);
	if( pWnd->GetHWND() == NULL ) {
		delete pWnd;
		return;
	}
	pWnd->CenterWindow();
	pWnd->ShowModal();
}
bool CIconExportWnd::ContainsSvgRootTag(const std::string& s)
{
	size_t i = 0;
	if( s.size() >= 3 && (unsigned char)s[0] == 0xEF && (unsigned char)s[1] == 0xBB && (unsigned char)s[2] == 0xBF )
		i = 3;
	while( i < s.size() && (s[i] == ' ' || s[i] == '\t' || s[i] == '\r' || s[i] == '\n') ) ++i;
	const size_t nProbe = (s.size() - i < 4096 ? s.size() - i : (size_t)4096);
	if( nProbe == 0 ) return false;
	std::string head = s.substr(i, nProbe);
	for( size_t k = 0; k < head.size(); ++k ) {
		if( head[k] >= 'A' && head[k] <= 'Z' ) head[k] = (char)(head[k] - 'A' + 'a');
	}
	// 要求出现独立的 <svg 标签（避免仅含 svg 字样的普通文本）
	size_t pos = 0;
	while( (pos = head.find("<svg", pos)) != std::string::npos ) {
		const size_t end = pos + 4;
		if( end >= head.size() ) return true;
		const char c = head[end];
		if( c == ' ' || c == '\t' || c == '\r' || c == '\n' || c == '/' || c == '>' )
			return true;
		pos = end;
	}
	return false;
}
bool CIconExportWnd::IsValidSvgUtf8(const std::string& s)
{
	if( s.empty() ) return false;
	if( s.size() > 8 * 1024 * 1024 ) return false; // 过大不接受
	if( !ContainsSvgRootTag(s) ) return false;
	std::unique_ptr<lunasvg::Document> doc = lunasvg::Document::loadFromData(s);
	if( doc.get() == NULL ) return false;
	// 能成功解析即视为合法；部分 SVG 宽高为 0 仍可按导出尺寸渲染
	return true;
}
bool CIconExportWnd::ReadClipboardTextUtf8(HWND hOwner, std::string& utf8Out)
{
	utf8Out.clear();
	if( !::OpenClipboard(hOwner) ) return false;
	HANDLE hUni = ::GetClipboardData(CF_UNICODETEXT);
	if( hUni != NULL ) {
		LPCWSTR p = (LPCWSTR)::GlobalLock(hUni);
		if( p != NULL ) {
			int n = ::WideCharToMultiByte(CP_UTF8, 0, p, -1, NULL, 0, NULL, NULL);
			if( n > 1 ) {
				utf8Out.resize((size_t)n - 1);
				::WideCharToMultiByte(CP_UTF8, 0, p, -1, &utf8Out[0], n, NULL, NULL);
			}
			::GlobalUnlock(hUni);
		}
	}
	if( utf8Out.empty() ) {
		HANDLE hAnsi = ::GetClipboardData(CF_TEXT);
		if( hAnsi != NULL ) {
			LPCSTR p = (LPCSTR)::GlobalLock(hAnsi);
			if( p != NULL ) {
				int nWide = ::MultiByteToWideChar(CP_ACP, 0, p, -1, NULL, 0);
				if( nWide > 1 ) {
					std::wstring w((size_t)nWide - 1, L'\0');
					::MultiByteToWideChar(CP_ACP, 0, p, -1, &w[0], nWide);
					int n = ::WideCharToMultiByte(CP_UTF8, 0, w.c_str(), -1, NULL, 0, NULL, NULL);
					if( n > 1 ) {
						utf8Out.resize((size_t)n - 1);
						::WideCharToMultiByte(CP_UTF8, 0, w.c_str(), -1, &utf8Out[0], n, NULL, NULL);
					}
				}
				::GlobalUnlock(hAnsi);
			}
		}
	}
	::CloseClipboard();
	return !utf8Out.empty();
}
bool CIconExportWnd::ApplySvgClipboard(bool bSilentFail)
{
	std::string utf8;
	if( !ReadClipboardTextUtf8(m_hWnd, utf8) ) {
		if( !bSilentFail )
			CToast::ShowWarning(_T("剪贴板为空或无法读取文本"), 2500);
		return false;
	}
	if( !ContainsSvgRootTag(utf8) ) {
		if( !bSilentFail )
			CToast::ShowWarning(_T("剪贴板不是 SVG（未找到合法 <svg> 标签）"), 2800);
		return false;
	}
	if( !IsValidSvgUtf8(utf8) ) {
		if( !bSilentFail )
			CToast::ShowDanger(_T("SVG 无法解析，请确认内容合法"), 2800);
		return false;
	}
	m_sSvgPath.Empty();
	m_sSvgUtf8 = utf8;
	m_sLib = _T("clipboard");
	m_sName = _T("clipboard");
	if( m_pPreview != NULL ) {
		m_pPreview->LoadFromUtf8Data(m_sSvgUtf8.c_str());
		SyncPreview();
	}
	SyncSourceLabels();
	return true;
}
void CIconExportWnd::OpenFromClipboard(HWND hOwner)
{
	std::string utf8;
	if( !ReadClipboardTextUtf8(hOwner, utf8) ) {
		CToast::ShowWarning(_T("剪贴板为空或无法读取文本"), 2500);
		return;
	}
	if( !ContainsSvgRootTag(utf8) ) {
		CToast::ShowWarning(_T("剪贴板不是 SVG（未找到合法 <svg> 标签）"), 2800);
		return;
	}
	if( !IsValidSvgUtf8(utf8) ) {
		CToast::ShowDanger(_T("SVG 无法解析，请确认内容合法"), 2800);
		return;
	}
	Open(hOwner, _T("clipboard"), _T("clipboard"), utf8.c_str(), 256, 256);
}
void CIconExportWnd::OnFinalMessage(HWND hWnd)
{
	WindowImplBase::OnFinalMessage(hWnd);
	delete this;
}
CDuiString CIconExportWnd::GetSkinFile()
{
	return _T("iconexport.html");
}
LPCTSTR CIconExportWnd::GetWindowClassName() const
{
	return _T("IconExportWnd");
}
DWORD CIconExportWnd::ThemeToken(LPCTSTR pstrName, DWORD dwFallback) const
{
	return ExportThemeToken(pstrName, dwFallback);
}
void CIconExportWnd::BuildColorSwatches()
{
	if( m_pColorRow == NULL ) return;
	m_pColorRow->RemoveAll();
	struct Swatch { LPCTSTR name; DWORD color; bool theme; };
	const Swatch items[] = {
		{ _T("主题字"), 0, true },
		{ _T("主色"), 0, true },
		{ _T("黑"), 0x000000FF, false },
		{ _T("白"), 0xFFFFFFFF, false },
		{ _T("灰"), 0x6C757DFF, false },
		{ _T("红"), 0xDC3545FF, false },
		{ _T("绿"), 0x198754FF, false },
		{ _T("蓝"), 0x0D6EFDFF, false },
		{ _T("橙"), 0xFD7E14FF, false },
		{ _T("紫"), 0x722ED1FF, false },
	};
	for( int i = 0; i < (int)(sizeof(items) / sizeof(items[0])); ++i ) {
		DWORD c = items[i].color;
		if( items[i].theme ) {
			if( i == 0 ) c = ThemeToken(_T("color-text"), 0x333333FF);
			else c = ThemeToken(_T("color-primary"), 0x0D6EFDFF);
		}
		CFontIconUI* p = new CFontIconUI;
		p->SetSizePreset(28);
		p->SetShape(CFontIconUI::ShapeRounded);
		CDuiString sName;
		sName.Format(_T("swatch_%d"), i);
		p->SetName(sName.GetData());
		p->SetClickable(true);
		p->SetToolTip(items[i].name);
		CDuiString sBk;
		sBk.Format(_T("#%08X"), c);
		p->SetAttribute(_T("background-color"), sBk.GetData());
		CDuiString sTag;
		sTag.Format(_T("%08X"), c);
		p->SetUserData(sTag.GetData());
		m_pColorRow->Add(p);
	}
}
void CIconExportWnd::ApplyTint(DWORD dwColor, bool bNoTint)
{
	m_bNoTint = bNoTint;
	if( !bNoTint ) m_dwTint = dwColor;
	SyncPreview();
	if( m_pColorLabel != NULL ) {
		CDuiString s;
		FormatColorLabel(s, m_dwTint, m_bNoTint);
		m_pColorLabel->SetText(s.GetData());
	}
	if( m_pPalette != NULL && !bNoTint )
		m_pPalette->SetSelectColor(m_dwTint);
}
void CIconExportWnd::SyncPreview()
{
	if( m_pPreview == NULL ) return;
	if( m_bNoTint )
		m_pPreview->SetColor(0);
	else
		m_pPreview->SetColor(m_dwTint);
	m_pPreview->Invalidate();
}
void CIconExportWnd::SyncFormatHint()
{
	CLabelUI* pHint = static_cast<CLabelUI*>(m_pm.FindControl(_T("lbl_fmt_hint")));
	if( pHint == NULL ) return;
	CDuiString ext = GetFormatExt();
	if( ext == _T("ico") )
		pHint->SetText(_T("ICO：按边长导出正方形（最大 512）"));
	else if( ext == _T("jpg") )
		pHint->SetText(_T("JPG：叠白底（无透明）"));
	else if( ext == _T("bmp") )
		pHint->SetText(_T("BMP：位图导出"));
	else
		pHint->SetText(_T("PNG：保留透明"));
}
void CIconExportWnd::SyncDirLabel()
{
	if( m_pDirLabel == NULL ) return;
	m_pDirLabel->SetText(SharedExportDir().GetData());
	m_pDirLabel->SetToolTip(SharedExportDir().GetData());
}
void CIconExportWnd::SyncSourceLabels()
{
	CLabelUI* pName = static_cast<CLabelUI*>(m_pm.FindControl(_T("lbl_name")));
	if( pName != NULL ) {
		if( !m_sName.IsEmpty() ) pName->SetText(m_sName.GetData());
		else pName->SetText(_T("（未选择 SVG）"));
	}
	CLabelUI* pLib = static_cast<CLabelUI*>(m_pm.FindControl(_T("lbl_lib")));
	if( pLib != NULL ) {
		if( !m_sSvgPath.IsEmpty() ) {
			pLib->SetText(m_sSvgPath.GetData());
			pLib->SetToolTip(m_sSvgPath.GetData());
		}
		else if( m_sLib.CompareNoCase(_T("clipboard")) == 0 ) {
			pLib->SetText(_T("来源：剪贴板"));
			pLib->SetToolTip(_T("SVG 文本来自剪贴板"));
		}
		else if( !m_sLib.IsEmpty() && !m_sName.IsEmpty() ) {
			CDuiString s;
			s.Format(_T("%s=\"%s\""), m_sLib.GetData(), m_sName.GetData());
			pLib->SetText(s.GetData());
			pLib->SetToolTip(s.GetData());
		}
		else {
			pLib->SetText(_T("打开 / 剪贴板 / 拖入 SVG 文件"));
			pLib->SetToolTip(_T(""));
		}
	}
	CTitleBarUI* pBar = static_cast<CTitleBarUI*>(m_pm.FindControl(_T("titlebar")));
	if( pBar != NULL ) {
		CDuiString s;
		if( !m_sName.IsEmpty() ) s.Format(_T("导出 — %s"), m_sName.GetData());
		else s = _T("导出 SVG");
		pBar->SetTitle(s.GetData());
	}
}
void CIconExportWnd::SetSizeEdits(int w, int h)
{
	w = ClampExportSize(w);
	h = ClampExportSize(h);
	CDuiString sw, sh;
	sw.Format(_T("%d"), w);
	sh.Format(_T("%d"), h);
	if( m_pEditW != NULL ) m_pEditW->SetText(sw.GetData());
	if( m_pEditH != NULL ) m_pEditH->SetText(sh.GetData());
}
void CIconExportWnd::ResetSizeToControl()
{
	int w = m_nCtrlW;
	int h = m_nCtrlH;
	if( w <= 0 && m_pPreview != NULL ) {
		w = m_pPreview->GetFixedWidth();
		if( w <= 0 ) {
			RECT rc = m_pPreview->GetPos();
			w = rc.right - rc.left;
		}
	}
	if( h <= 0 && m_pPreview != NULL ) {
		h = m_pPreview->GetFixedHeight();
		if( h <= 0 ) {
			RECT rc = m_pPreview->GetPos();
			h = rc.bottom - rc.top;
		}
	}
	if( w <= 0 ) w = 28;
	if( h <= 0 ) h = 28;
	SetSizeEdits(w, h);
}
bool CIconExportWnd::ReadExportSize(int& w, int& h) const
{
	w = 0;
	h = 0;
	if( m_pEditW != NULL )
		w = _ttoi(m_pEditW->GetText().GetData());
	if( m_pEditH != NULL )
		h = _ttoi(m_pEditH->GetText().GetData());
	if( w <= 0 || h <= 0 ) {
		int cw = m_nCtrlW > 0 ? m_nCtrlW : 28;
		int ch = m_nCtrlH > 0 ? m_nCtrlH : 28;
		if( w <= 0 ) w = cw;
		if( h <= 0 ) h = ch;
	}
	w = ClampExportSize(w);
	h = ClampExportSize(h);
	return true;
}
bool CIconExportWnd::EnsureExportDir() const
{
	const CDuiString& sDir = SharedExportDir();
	if( sDir.IsEmpty() ) return false;
	DWORD attr = ::GetFileAttributes(sDir.GetData());
	if( attr != INVALID_FILE_ATTRIBUTES ) {
		if( (attr & FILE_ATTRIBUTE_DIRECTORY) == 0 ) return false;
		return true;
	}
	return ::CreateDirectory(sDir.GetData(), NULL) != FALSE
		|| ::GetLastError() == ERROR_ALREADY_EXISTS;
}
bool CIconExportWnd::BrowseExportDir()
{
	TCHAR szDisplay[MAX_PATH] = { 0 };
	BROWSEINFO bi;
	::ZeroMemory(&bi, sizeof(bi));
	bi.hwndOwner = m_hWnd;
	bi.pszDisplayName = szDisplay;
	bi.lpszTitle = _T("选择导出目录");
	bi.ulFlags = BIF_RETURNONLYFSDIRS | BIF_NEWDIALOGSTYLE;
	bi.lpfn = BrowseDirCallback;
	bi.lParam = (LPARAM)SharedExportDir().GetData();
	LPITEMIDLIST pidl = ::SHBrowseForFolder(&bi);
	if( pidl == NULL ) return false;
	TCHAR szPath[MAX_PATH] = { 0 };
	BOOL ok = ::SHGetPathFromIDList(pidl, szPath);
	::CoTaskMemFree(pidl);
	if( !ok || szPath[0] == _T('\0') ) return false;
	SharedExportDir() = szPath;
	SyncDirLabel();
	return true;
}
bool CIconExportWnd::ApplySvgFile(LPCTSTR pstrPath)
{
	if( pstrPath == NULL || *pstrPath == _T('\0') ) return false;
	if( ::GetFileAttributes(pstrPath) == INVALID_FILE_ATTRIBUTES ) {
		CToast::ShowDanger(_T("无法打开 SVG 文件"), 2500);
		return false;
	}
	HANDLE hFile = ::CreateFile(pstrPath, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if( hFile == INVALID_HANDLE_VALUE ) {
		CToast::ShowDanger(_T("无法读取 SVG 文件"), 2500);
		return false;
	}
	LARGE_INTEGER li = {};
	std::string utf8;
	bool bReadOk = false;
	if( ::GetFileSizeEx(hFile, &li) && li.QuadPart > 0 && li.QuadPart <= 8 * 1024 * 1024 ) {
		utf8.resize((size_t)li.QuadPart);
		DWORD nRead = 0;
		bReadOk = ::ReadFile(hFile, &utf8[0], (DWORD)utf8.size(), &nRead, NULL) && nRead == utf8.size();
		if( bReadOk ) utf8.resize(nRead);
	}
	::CloseHandle(hFile);
	if( !bReadOk ) {
		CToast::ShowDanger(_T("读取 SVG 失败或文件过大"), 2500);
		return false;
	}
	if( !ContainsSvgRootTag(utf8) ) {
		CToast::ShowWarning(_T("文件不是 SVG（未找到合法 <svg> 标签）"), 2800);
		return false;
	}
	if( !IsValidSvgUtf8(utf8) ) {
		CToast::ShowDanger(_T("SVG 无法解析，请确认文件内容合法"), 2800);
		return false;
	}
	m_sSvgPath = pstrPath;
	m_sSvgUtf8 = utf8;
	m_sLib = _T("file");
	m_sName = FileBaseName(pstrPath);
	if( m_pPreview != NULL ) {
		m_pPreview->LoadFromUtf8Data(m_sSvgUtf8.c_str());
		SyncPreview();
	}
	SyncSourceLabels();
	return true;
}
bool CIconExportWnd::PickSvgPath(HWND hOwner, CDuiString& sOutPath)
{
	sOutPath.Empty();
	TCHAR szFile[MAX_PATH] = { 0 };
	static TCHAR sFilter[] = _T("SVG (*.svg)\0*.svg\0All Files (*.*)\0*.*\0");
	OPENFILENAME ofn;
	::ZeroMemory(&ofn, sizeof(ofn));
	ofn.lStructSize = sizeof(ofn);
	ofn.hwndOwner = hOwner;
	ofn.lpstrFile = szFile;
	ofn.nMaxFile = MAX_PATH;
	ofn.lpstrFilter = sFilter;
	ofn.nFilterIndex = 1;
	ofn.lpstrDefExt = _T("svg");
	ofn.Flags = OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST | OFN_HIDEREADONLY;
	if( !::GetOpenFileName(&ofn) ) return false;
	sOutPath = szFile;
	return true;
}
bool CIconExportWnd::BrowseOpenSvgFile()
{
	CDuiString sPath;
	if( !PickSvgPath(m_hWnd, sPath) ) return false;
	return ApplySvgFile(sPath.GetData());
}
bool CIconExportWnd::HasSvgSource() const
{
	if( !m_sSvgPath.IsEmpty() ) return true;
	if( !m_sSvgUtf8.empty() ) return true;
	return false;
}
CDuiString CIconExportWnd::GetFormatExt() const
{
	if( m_pFormat == NULL ) return _T("png");
	LPCTSTR v = m_pFormat->GetSelectedValue();
	if( v == NULL || *v == _T('\0') ) return _T("png");
	return v;
}
CDuiString CIconExportWnd::MakeSafeFileName() const
{
	TCHAR sz[MAX_PATH] = { 0 };
	_tcsncpy(sz, m_sName.GetData(), MAX_PATH - 1);
	for( TCHAR* p = sz; *p; ++p ) {
		if( *p == _T('\\') || *p == _T('/') || *p == _T(':') || *p == _T('*')
			|| *p == _T('?') || *p == _T('\"') || *p == _T('<') || *p == _T('>') || *p == _T('|') )
			*p = _T('_');
	}
	if( sz[0] == _T('\0') ) return _T("icon");
	return sz;
}
void CIconExportWnd::InitWindow()
{
	m_pPreview = static_cast<CSvgBoxUI*>(m_pm.FindControl(_T("preview")));
	m_pFormat = static_cast<CSegmentedUI*>(m_pm.FindControl(_T("seg_format")));
	m_pPalette = static_cast<CColorPaletteUI*>(m_pm.FindControl(_T("palette")));
	m_pColorLabel = static_cast<CLabelUI*>(m_pm.FindControl(_T("lbl_color")));
	m_pDirLabel = static_cast<CLabelUI*>(m_pm.FindControl(_T("lbl_dir")));
	m_pEditW = static_cast<CEditUI*>(m_pm.FindControl(_T("edt_w")));
	m_pEditH = static_cast<CEditUI*>(m_pm.FindControl(_T("edt_h")));
	m_pColorRow = static_cast<CHorizontalLayoutUI*>(m_pm.FindControl(_T("color_row")));
	if( m_pPreview != NULL ) {
		if( !m_sSvgUtf8.empty() )
			m_pPreview->LoadFromUtf8Data(m_sSvgUtf8.c_str());
		else if( !m_sSvgPath.IsEmpty() )
			m_pPreview->LoadFromFile(m_sSvgPath.GetData());
		SyncPreview();
	}
	if( m_nCtrlW <= 0 || m_nCtrlH <= 0 ) {
		if( m_pPreview != NULL ) {
			if( m_nCtrlW <= 0 ) m_nCtrlW = m_pPreview->GetFixedWidth();
			if( m_nCtrlH <= 0 ) m_nCtrlH = m_pPreview->GetFixedHeight();
		}
		if( m_nCtrlW <= 0 ) m_nCtrlW = 28;
		if( m_nCtrlH <= 0 ) m_nCtrlH = 28;
	}
	ResetSizeToControl();
	CLabelUI* pSizeHint = static_cast<CLabelUI*>(m_pm.FindControl(_T("lbl_size_hint")));
	if( pSizeHint != NULL ) {
		CDuiString s;
		s.Format(_T("px（控件 %d×%d）"), m_nCtrlW, m_nCtrlH);
		pSizeHint->SetText(s.GetData());
	}
	BuildColorSwatches();
	ApplyTint(m_dwTint, false);
	SyncFormatHint();
	EnsureExportDir();
	SyncDirLabel();
	SyncSourceLabels();
	if( m_hWnd != NULL )
		::DragAcceptFiles(m_hWnd, TRUE);
}
void CIconExportWnd::Notify(TNotifyUI& msg)
{
	if( msg.sType == DUI_MSGTYPE_SELECTCHANGED && msg.pSender == m_pFormat ) {
		SyncFormatHint();
		return;
	}
	if( (msg.sType == DUI_MSGTYPE_COLORCHANGING || msg.sType == DUI_MSGTYPE_COLORCHANGED)
		&& msg.pSender == m_pPalette ) {
		ApplyTint((DWORD)msg.wParam, false);
		return;
	}
	WindowImplBase::Notify(msg);
}
bool CIconExportWnd::DoExport()
{
	if( m_pPreview == NULL ) return false;
	if( !HasSvgSource() ) {
		CToast::ShowWarning(_T("请先打开 SVG 文件或从图标库选择"), 2500);
		return false;
	}
	if( !EnsureExportDir() ) {
		CToast::ShowDanger(_T("无法创建导出目录"), 2500);
		return false;
	}
	int w = 0, h = 0;
	ReadExportSize(w, h);
	CDuiString ext = GetFormatExt();
	CDuiString sFile = MakeSafeFileName();
	CDuiString sPath;
	sPath.Format(_T("%s\\%s.%s"), SharedExportDir().GetData(), sFile.GetData(), ext.GetData());
	if( ::GetFileAttributes(sPath.GetData()) != INVALID_FILE_ATTRIBUTES ) {
		CDuiString sAsk;
		sAsk.Format(_T("文件已存在，是否覆盖？\n%s"), sPath.GetData());
		if( ::MessageBox(m_hWnd, sAsk.GetData(), _T("导出图标"), MB_YESNO | MB_ICONQUESTION) != IDYES )
			return false;
	}
	DWORD tint = m_bNoTint ? 0 : m_dwTint;
	bool ok = false;
	if( ext == _T("ico") ) {
		int s = w;
		if( h > 0 && (s <= 0 || h < s) ) s = h;
		if( s > 512 ) s = 512;
		if( s < 1 ) s = 1;
		ok = m_pPreview->ExportToIcoFile(sPath.GetData(), &s, 1, tint);
	}
	else {
		ok = m_pPreview->ExportToFile(sPath.GetData(), w, h, tint, 90);
	}
	if( ok ) {
		CDuiString sTip;
		sTip.Format(_T("已导出：%s（%d×%d）"), sPath.GetData(), w, h);
		CToast::ShowSuccess(sTip.GetData(), 2800);
	}
	else {
		CToast::ShowDanger(_T("导出失败"), 2500);
	}
	return ok;
}
bool CIconExportWnd::ApplyDroppedFiles(HDROP hDrop)
{
	if( hDrop == NULL ) return false;
	const UINT nFiles = ::DragQueryFile(hDrop, 0xFFFFFFFF, NULL, 0);
	CDuiString sSvg;
	for( UINT i = 0; i < nFiles; ++i ) {
		TCHAR szPath[MAX_PATH] = { 0 };
		const UINT nLen = ::DragQueryFile(hDrop, i, szPath, MAX_PATH);
		if( nLen == 0 || nLen >= MAX_PATH ) continue;
		CDuiString s = szPath;
		CDuiString sLower = s;
		sLower.MakeLower();
		if( sLower.GetLength() < 4 ) continue;
		if( sLower.Right(4) != _T(".svg") ) continue;
		sSvg = s;
		break;
	}
	if( sSvg.IsEmpty() ) {
		CToast::ShowWarning(_T("请拖入 .svg 文件"), 2500);
		return false;
	}
	return ApplySvgFile(sSvg.GetData());
}
LRESULT CIconExportWnd::HandleCustomMessage(UINT uMsg, WPARAM wParam, LPARAM /*lParam*/, BOOL& bHandled)
{
	if( uMsg == WM_DROPFILES ) {
		HDROP hDrop = (HDROP)wParam;
		ApplyDroppedFiles(hDrop);
		::DragFinish(hDrop);
		bHandled = TRUE;
		return 0;
	}
	bHandled = FALSE;
	return 0;
}
void CIconExportWnd::OnClick(TNotifyUI& msg)
{
	CDuiString sName = msg.pSender->GetName();
	if( sName.CompareNoCase(_T("btn_cancel")) == 0 || sName.CompareNoCase(_T("closebtn")) == 0 ) {
		Close(0);
		return;
	}
	if( sName.CompareNoCase(_T("btn_export")) == 0 ) {
		DoExport();
		return;
	}
	if( sName.CompareNoCase(_T("btn_open_svg")) == 0 ) {
		BrowseOpenSvgFile();
		return;
	}
	if( sName.CompareNoCase(_T("btn_clipboard_svg")) == 0 ) {
		ApplySvgClipboard(false);
		return;
	}
	if( sName.CompareNoCase(_T("btn_browse_dir")) == 0 ) {
		BrowseExportDir();
		return;
	}
	if( sName.CompareNoCase(_T("btn_size_ctrl")) == 0 ) {
		ResetSizeToControl();
		return;
	}
	if( sName.CompareNoCase(_T("btn_size_32")) == 0 ) { SetSizeEdits(32, 32); return; }
	if( sName.CompareNoCase(_T("btn_size_64")) == 0 ) { SetSizeEdits(64, 64); return; }
	if( sName.CompareNoCase(_T("btn_size_128")) == 0 ) { SetSizeEdits(128, 128); return; }
	if( sName.CompareNoCase(_T("btn_size_256")) == 0 ) { SetSizeEdits(256, 256); return; }
	if( sName.CompareNoCase(_T("btn_size_512")) == 0 ) { SetSizeEdits(512, 512); return; }
	if( sName.CompareNoCase(_T("btn_no_tint")) == 0 ) {
		ApplyTint(0, true);
		return;
	}
	if( sName.CompareNoCase(_T("btn_custom_color")) == 0 ) {
		if( m_pPalette != NULL ) {
			bool bShow = !m_pPalette->IsVisible();
			m_pPalette->SetVisible(bShow);
			if( bShow ) {
				m_pPalette->SetSelectColor(m_dwTint);
				m_bNoTint = false;
			}
		}
		return;
	}
	if( sName.Find(_T("swatch_")) == 0 ) {
		LPCTSTR ud = msg.pSender->GetUserData().GetData();
		DWORD c = 0;
		if( ud != NULL && _stscanf(ud, _T("%08X"), &c) == 1 )
			ApplyTint(c, false);
		return;
	}
	WindowImplBase::OnClick(msg);
}
