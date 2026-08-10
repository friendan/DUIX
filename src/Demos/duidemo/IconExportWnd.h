#pragma once
#include <string>
/// 图标 / SVG 文件 / 剪贴板导出对话框
class CIconExportWnd : public WindowImplBase
{
public:
	CIconExportWnd(LPCTSTR pstrLib, LPCTSTR pstrName, const char* utf8Svg, int nCtrlW, int nCtrlH);
	~CIconExportWnd();
	static void Open(HWND hOwner, LPCTSTR pstrLib, LPCTSTR pstrName, const char* utf8Svg,
		int nCtrlW = 0, int nCtrlH = 0);
	static void OpenBlank(HWND hOwner, bool bPickFile = false);
	static void OpenFromClipboard(HWND hOwner);
	virtual void OnFinalMessage(HWND hWnd);
	virtual CDuiString GetSkinFile();
	virtual LPCTSTR GetWindowClassName() const;
	virtual void InitWindow();
	virtual void Notify(TNotifyUI& msg);
	virtual LRESULT HandleCustomMessage(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	DUI_DECLARE_MESSAGE_MAP()
	virtual void OnClick(TNotifyUI& msg);
	virtual LRESULT MessageHandler(UINT uMsg, WPARAM wParam, LPARAM /*lParam*/, bool& /*bHandled*/)
	{
		if( uMsg == WM_KEYDOWN && wParam == VK_ESCAPE ) {
			Close(0);
			return TRUE;
		}
		return FALSE;
	}
private:
	void BuildColorSwatches();
	void ApplyTint(DWORD dwColor, bool bNoTint);
	void SyncPreview();
	void SyncFormatHint();
	void SyncDirLabel();
	void SyncSourceLabels();
	void SetSizeEdits(int w, int h);
	void ResetSizeToControl();
	bool ReadExportSize(int& w, int& h) const;
	bool EnsureExportDir() const;
	bool BrowseExportDir();
	bool BrowseOpenSvgFile();
	bool ApplySvgFile(LPCTSTR pstrPath);
	bool ApplySvgClipboard(bool bSilentFail);
	bool ApplyDroppedFiles(HDROP hDrop);
	bool HasSvgSource() const;
	bool DoExport();
	CDuiString MakeSafeFileName() const;
	CDuiString GetFormatExt() const;
	DWORD ThemeToken(LPCTSTR pstrName, DWORD dwFallback) const;
	static bool PickSvgPath(HWND hOwner, CDuiString& sOutPath);
	static bool ReadClipboardTextUtf8(HWND hOwner, std::string& utf8Out);
	static bool ContainsSvgRootTag(const std::string& s);
	static bool IsValidSvgUtf8(const std::string& s);
	static CDuiString GetDefaultExportDir();
	static CDuiString& SharedExportDir();
	static CDuiString FileBaseName(LPCTSTR pstrPath);
private:
	CDuiString m_sLib;
	CDuiString m_sName;
	CDuiString m_sSvgPath;
	std::string m_sSvgUtf8;
	int m_nCtrlW;
	int m_nCtrlH;
	CSvgBoxUI* m_pPreview;
	CSegmentedUI* m_pFormat;
	CColorPaletteUI* m_pPalette;
	CLabelUI* m_pColorLabel;
	CLabelUI* m_pDirLabel;
	CEditUI* m_pEditW;
	CEditUI* m_pEditH;
	CHorizontalLayoutUI* m_pColorRow;
	DWORD m_dwTint;
	bool m_bNoTint;
};
