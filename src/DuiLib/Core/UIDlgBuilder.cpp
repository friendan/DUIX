#include "StdAfx.h"

namespace DuiLib {

	static void ParseCssBlock(CPaintManagerUI* pManager, LPCTSTR pCssText);
	static void LoadCssFile(CPaintManagerUI* pManager, LPCTSTR pstrSrc);
	static void ApplyWindowAttribute(CPaintManagerUI* pManager, LPCTSTR pstrName, LPCTSTR pstrValue);
	static void ApplyWindowAttributeList(CPaintManagerUI* pManager, LPCTSTR pstrList);
	static void ApplyWindowCssRules(CPaintManagerUI* pManager);

	static void ApplyWindowAttribute(CPaintManagerUI* pManager, LPCTSTR pstrName, LPCTSTR pstrValue)
	{
		if( pManager == NULL || pstrName == NULL || pstrValue == NULL ) return;
		if( _tcsicmp(pstrName, _T("size")) == 0 ) {
			LPTSTR pstr = NULL;
			int cx = _tcstol(pstrValue, &pstr, 10);  ASSERT(pstr);
			int cy = _tcstol(pstr + 1, &pstr, 10);    ASSERT(pstr);
			pManager->SetInitSize(pManager->GetDPIObj()->Scale(cx), pManager->GetDPIObj()->Scale(cy));
		}
		else if( _tcsicmp(pstrName, _T("sizebox")) == 0 ) {
			RECT rcSizeBox = { 0 };
			LPTSTR pstr = NULL;
			rcSizeBox.left = _tcstol(pstrValue, &pstr, 10);  ASSERT(pstr);
			rcSizeBox.top = _tcstol(pstr + 1, &pstr, 10);    ASSERT(pstr);
			rcSizeBox.right = _tcstol(pstr + 1, &pstr, 10);  ASSERT(pstr);
			rcSizeBox.bottom = _tcstol(pstr + 1, &pstr, 10); ASSERT(pstr);
			pManager->SetSizeBox(rcSizeBox);
		}
		else if( _tcsicmp(pstrName, _T("caption")) == 0 ) {
			RECT rcCaption = { 0 };
			LPTSTR pstr = NULL;
			rcCaption.left = _tcstol(pstrValue, &pstr, 10);  ASSERT(pstr);
			rcCaption.top = _tcstol(pstr + 1, &pstr, 10);    ASSERT(pstr);
			rcCaption.right = _tcstol(pstr + 1, &pstr, 10);  ASSERT(pstr);
			rcCaption.bottom = _tcstol(pstr + 1, &pstr, 10); ASSERT(pstr);
			pManager->SetCaptionRect(rcCaption);
		}
		else if( _tcsicmp(pstrName, _T("roundcorner")) == 0 ) {
			LPTSTR pstr = NULL;
			int cx = _tcstol(pstrValue, &pstr, 10);  ASSERT(pstr);
			int cy = _tcstol(pstr + 1, &pstr, 10);    ASSERT(pstr);
			pManager->SetRoundCorner(cx, cy);
		}
		else if( _tcsicmp(pstrName, _T("mininfo")) == 0 ) {
			LPTSTR pstr = NULL;
			int cx = _tcstol(pstrValue, &pstr, 10);  ASSERT(pstr);
			int cy = _tcstol(pstr + 1, &pstr, 10);    ASSERT(pstr);
			pManager->SetMinInfo(cx, cy);
		}
		else if( _tcsicmp(pstrName, _T("maxinfo")) == 0 ) {
			LPTSTR pstr = NULL;
			int cx = _tcstol(pstrValue, &pstr, 10);  ASSERT(pstr);
			int cy = _tcstol(pstr + 1, &pstr, 10);    ASSERT(pstr);
			pManager->SetMaxInfo(cx, cy);
		}
		else if( _tcsicmp(pstrName, _T("showdirty")) == 0 ) {
			pManager->SetShowUpdateRect(_tcsicmp(pstrValue, _T("true")) == 0);
		}
		else if( _tcsicmp(pstrName, _T("opacity")) == 0 || _tcsicmp(pstrName, _T("alpha")) == 0 ) {
			pManager->SetOpacity(_ttoi(pstrValue));
		}
		else if( _tcscmp(pstrName, _T("layeredopacity")) == 0 ) {
			pManager->SetLayeredOpacity(_ttoi(pstrValue));
		}
		else if( _tcscmp(pstrName, _T("layered")) == 0 || _tcscmp(pstrName, _T("bktrans")) == 0) {
			pManager->SetLayered(_tcsicmp(pstrValue, _T("true")) == 0);
		}
		else if( _tcscmp(pstrName, _T("layeredimage")) == 0 ) {
			pManager->SetLayered(true);
			pManager->SetLayeredImage(pstrValue);
		}
		else if( _tcscmp(pstrName, _T("noactivate")) == 0 ) {
			pManager->SetNoActivate(_tcsicmp(pstrValue, _T("true")) == 0);
		}
		else if( _tcsicmp(pstrName, _T("disabledfontcolor")) == 0 ) {
			if( *pstrValue == _T('#')) pstrValue = ::CharNext(pstrValue);
			LPTSTR pstr = NULL;
			DWORD clrColor = _tcstoul(pstrValue, &pstr, 16);
			pManager->SetDefaultDisabledColor(clrColor);
		}
		else if( _tcsicmp(pstrName, _T("defaultfontcolor")) == 0 ) {
			if( *pstrValue == _T('#')) pstrValue = ::CharNext(pstrValue);
			LPTSTR pstr = NULL;
			DWORD clrColor = _tcstoul(pstrValue, &pstr, 16);
			pManager->SetDefaultFontColor(clrColor);
		}
		else if( _tcsicmp(pstrName, _T("linkfontcolor")) == 0 ) {
			if( *pstrValue == _T('#')) pstrValue = ::CharNext(pstrValue);
			LPTSTR pstr = NULL;
			DWORD clrColor = _tcstoul(pstrValue, &pstr, 16);
			pManager->SetDefaultLinkFontColor(clrColor);
		}
		else if( _tcsicmp(pstrName, _T("linkhoverfontcolor")) == 0 ) {
			if( *pstrValue == _T('#')) pstrValue = ::CharNext(pstrValue);
			LPTSTR pstr = NULL;
			DWORD clrColor = _tcstoul(pstrValue, &pstr, 16);
			pManager->SetDefaultLinkHoverFontColor(clrColor);
		}
		else if( _tcsicmp(pstrName, _T("selectedcolor")) == 0 ) {
			if( *pstrValue == _T('#')) pstrValue = ::CharNext(pstrValue);
			LPTSTR pstr = NULL;
			DWORD clrColor = _tcstoul(pstrValue, &pstr, 16);
			pManager->SetDefaultSelectedBkColor(clrColor);
		}
		else if( _tcsicmp(pstrName, _T("bkcolor")) == 0 || _tcsicmp(pstrName, _T("bkcolor1")) == 0 ) {
			DWORD clrColor = 0;
			if( ParseColorString(pstrValue, clrColor) )
				pManager->SetWindowBkColor(clrColor);
		}
		else if( _tcsicmp(pstrName, _T("action")) == 0 ) {
			// html/Window：title/move 等落到窗口级，Attach 后赋给 root（见 SetWindowAction）
			UIAction act = UIACTION_NONE;
			if( _tcsicmp(pstrValue, _T("close")) == 0 )         act = UIACTION_CLOSE;
			else if( _tcsicmp(pstrValue, _T("min")) == 0
			      || _tcsicmp(pstrValue, _T("mini")) == 0 )     act = UIACTION_MIN;
			else if( _tcsicmp(pstrValue, _T("max")) == 0 )      act = UIACTION_MAX;
			else if( _tcsicmp(pstrValue, _T("title")) == 0 )    act = UIACTION_TITLE;
			else if( _tcsicmp(pstrValue, _T("move")) == 0
			      || _tcsicmp(pstrValue, _T("movewindow")) == 0) act = UIACTION_MOVEWINDOW;
			else if( _tcsicmp(pstrValue, _T("copy")) == 0 )     act = UIACTION_COPY;
			pManager->SetWindowAction(act);
		}
		else if( _tcsicmp(pstrName, _T("shadowsize")) == 0 ) {
			pManager->GetShadow()->SetSize(_ttoi(pstrValue));
		}
		else if( _tcsicmp(pstrName, _T("shadowsharpness")) == 0 ) {
			pManager->GetShadow()->SetSharpness(_ttoi(pstrValue));
		}
		else if( _tcsicmp(pstrName, _T("shadowdarkness")) == 0 ) {
			pManager->GetShadow()->SetDarkness(_ttoi(pstrValue));
		}
		else if( _tcsicmp(pstrName, _T("shadowposition")) == 0 ) {
			LPTSTR pstr = NULL;
			int cx = _tcstol(pstrValue, &pstr, 10);  ASSERT(pstr);
			int cy = _tcstol(pstr + 1, &pstr, 10);    ASSERT(pstr);
			pManager->GetShadow()->SetPosition(cx, cy);
		}
		else if( _tcsicmp(pstrName, _T("shadowcolor")) == 0 ) {
			if( *pstrValue == _T('#')) pstrValue = ::CharNext(pstrValue);
			LPTSTR pstr = NULL;
			DWORD clrColor = _tcstoul(pstrValue, &pstr, 16);
			pManager->GetShadow()->SetColor(clrColor);
		}
		else if( _tcsicmp(pstrName, _T("shadowcorner")) == 0 ) {
			RECT rcCorner = { 0 };
			LPTSTR pstr = NULL;
			rcCorner.left = _tcstol(pstrValue, &pstr, 10);  ASSERT(pstr);
			rcCorner.top = _tcstol(pstr + 1, &pstr, 10);    ASSERT(pstr);
			rcCorner.right = _tcstol(pstr + 1, &pstr, 10);  ASSERT(pstr);
			rcCorner.bottom = _tcstol(pstr + 1, &pstr, 10); ASSERT(pstr);
			pManager->GetShadow()->SetShadowCorner(rcCorner);
		}
		else if( _tcsicmp(pstrName, _T("shadowimage")) == 0 ) {
			pManager->GetShadow()->SetImage(pstrValue);
		}
		else if( _tcsicmp(pstrName, _T("showshadow")) == 0 ) {
			pManager->GetShadow()->ShowShadow(_tcsicmp(pstrValue, _T("true")) == 0);
		}
		else if( _tcsicmp(pstrName, _T("gdiplustext")) == 0 ) {
			pManager->SetUseGdiplusText(_tcsicmp(pstrValue, _T("true")) == 0);
		}
		else if( _tcsicmp(pstrName, _T("textrenderinghint")) == 0 ) {
			pManager->SetGdiplusTextRenderingHint(_ttoi(pstrValue));
		}
		else if( _tcsicmp(pstrName, _T("tooltiphovertime")) == 0 ) {
			pManager->SetHoverTime(_ttoi(pstrValue));
		}
	}

	static void ApplyWindowAttributeList(CPaintManagerUI* pManager, LPCTSTR pstrList)
	{
		if( pManager == NULL || pstrList == NULL || *pstrList == _T('\0') ) return;
		CDuiString sXmlData = pstrList;
		sXmlData.Replace(_T("&quot;"), _T("\""));
		sXmlData.Replace(_T("\r"), _T(" "));
		sXmlData.Replace(_T("\n"), _T(" "));
		sXmlData.Replace(_T("\t"), _T(" "));
		sXmlData.Trim();
		LPCTSTR p = sXmlData.GetData();
		CDuiString sItem;
		CDuiString sValue;
		while( *p != _T('\0') ) {
			sItem.Empty();
			sValue.Empty();
			while( *p == _T(' ') ) p++;
			while( *p != _T('\0') && *p != _T('=') ) {
				LPTSTR pTemp = ::CharNext(p);
				while( p < pTemp ) sItem += *p++;
			}
			if( *p++ != _T('=') ) break;
			if( *p++ != _T('\"') ) break;
			while( *p != _T('\0') && *p != _T('\"') ) {
				LPTSTR pTemp = ::CharNext(p);
				while( p < pTemp ) sValue += *p++;
			}
			if( *p == _T('\"') ) p++;
			ApplyWindowAttribute(pManager, sItem, sValue);
			if( *p != _T(' ') && *p != _T(',') && *p != _T('\0') ) break;
			while( *p == _T(' ') || *p == _T(',') ) p++;
		}
	}

	static void ApplyWindowCssRules(CPaintManagerUI* pManager)
	{
		if( pManager == NULL || pManager->GetPaintWindow() == NULL ) return;
		LPCTSTR pHtml = pManager->GetCssTypeRule(_T("html"));
		if( pHtml != NULL ) ApplyWindowAttributeList(pManager, pHtml);
		LPCTSTR pWindow = pManager->GetCssTypeRule(_T("window"));
		if( pWindow != NULL ) ApplyWindowAttributeList(pManager, pWindow);
	}

	CDialogBuilder::CDialogBuilder() : m_pCallback(NULL), m_pstrtype(NULL)
	{
		m_instance = NULL;
	}

	CControlUI* CDialogBuilder::Create(STRINGorID xml, LPCTSTR type, IDialogBuilderCallback* pCallback, 
		CPaintManagerUI* pManager, CControlUI* pParent)
	{
		//资源ID为0-65535，两个字节；字符串指针为4个字节
		//字符串以<开头认为是XML字符串，否则认为是XML文件
		if(HIWORD(xml.m_lpstr) != NULL && *(xml.m_lpstr) != _T('<')) {
			LPCTSTR xmlpath = CResourceManager::GetInstance()->GetXmlPath(xml.m_lpstr);
			if (xmlpath != NULL) {
				xml = xmlpath;
			}
		}

		if( HIWORD(xml.m_lpstr) != NULL ) {
			if( *(xml.m_lpstr) == _T('<') ) {
				if( !m_xml.Load(xml.m_lpstr) ) return NULL;
			}
			else {
				if( !m_xml.LoadFromFile(xml.m_lpstr) ) return NULL;
			}
		}
		else {
			HINSTANCE dll_instence = NULL;
			if (m_instance)
			{
				dll_instence = m_instance;
			}
			else
			{
				dll_instence = CPaintManagerUI::GetResourceDll();
			}
			HRSRC hResource = ::FindResource(dll_instence, xml.m_lpstr, type);
			if( hResource == NULL ) return NULL;
			HGLOBAL hGlobal = ::LoadResource(dll_instence, hResource);
			if( hGlobal == NULL ) {
				FreeResource(hResource);
				return NULL;
			}

			m_pCallback = pCallback;
			if( !m_xml.LoadFromMem((BYTE*)::LockResource(hGlobal), ::SizeofResource(dll_instence, hResource) )) return NULL;
			::FreeResource(hGlobal);
			m_pstrtype = type;
		}

		return Create(pCallback, pManager, pParent);
	}

	CControlUI* CDialogBuilder::Create(IDialogBuilderCallback* pCallback, CPaintManagerUI* pManager, CControlUI* pParent)
	{
		m_pCallback = pCallback;
		CMarkupNode root = m_xml.GetRoot();
		if( !root.IsValid() ) return NULL;

		if( pManager ) {
			LPCTSTR pstrClass = NULL;
			int nAttributes = 0;
			LPCTSTR pstrName = NULL;
			LPCTSTR pstrValue = NULL;
			LPTSTR pstr = NULL;
			for( CMarkupNode node = root.GetChild() ; node.IsValid(); node = node.GetSibling() ) {
				pstrClass = node.GetName();
				if( _tcsicmp(pstrClass, _T("Image")) == 0 ) {
					nAttributes = node.GetAttributeCount();
					LPCTSTR pImageName = NULL;
					LPCTSTR pImageResType = NULL;
					bool shared = false;
					DWORD mask = 0;
					for( int i = 0; i < nAttributes; i++ ) {
						pstrName = node.GetAttributeName(i);
						pstrValue = node.GetAttributeValue(i);
						if( _tcsicmp(pstrName, _T("name")) == 0 ) {
							pImageName = pstrValue;
						}
						else if( _tcsicmp(pstrName, _T("restype")) == 0 ) {
							pImageResType = pstrValue;
						}
						else if( _tcsicmp(pstrName, _T("mask")) == 0 ) {
							if( *pstrValue == _T('#')) pstrValue = ::CharNext(pstrValue);
							mask = _tcstoul(pstrValue, &pstr, 16);
						}
						else if( _tcsicmp(pstrName, _T("shared")) == 0 ) {
							shared = (_tcsicmp(pstrValue, _T("true")) == 0);
						}
					}
					if( pImageName ) pManager->AddImage(pImageName, pImageResType, mask, false, shared);
				}
				else if( _tcsicmp(pstrClass, _T("Font")) == 0 ) {
					nAttributes = node.GetAttributeCount();
					int id = -1;
					LPCTSTR pFontName = NULL;
					int size = 12;
					bool bold = false;
					bool underline = false;
					bool italic = false;
					bool defaultfont = false;
					bool shared = false;
					bool strikeout = false;
					for( int i = 0; i < nAttributes; i++ ) {
						pstrName = node.GetAttributeName(i);
						pstrValue = node.GetAttributeValue(i);
						if( _tcsicmp(pstrName, _T("id")) == 0 ) {
							id = _tcstol(pstrValue, &pstr, 10);
						}
						else if( _tcsicmp(pstrName, _T("name")) == 0 ) {
							pFontName = pstrValue;
						}
						else if( _tcsicmp(pstrName, _T("size")) == 0 ) {
							size = _tcstol(pstrValue, &pstr, 10);
						}
						else if( _tcsicmp(pstrName, _T("bold")) == 0 ) {
							bold = (_tcsicmp(pstrValue, _T("true")) == 0);
						}
						else if( _tcsicmp(pstrName, _T("underline")) == 0 ) {
							underline = (_tcsicmp(pstrValue, _T("true")) == 0);
						}
						else if( _tcsicmp(pstrName, _T("italic")) == 0 ) {
							italic = (_tcsicmp(pstrValue, _T("true")) == 0);
						}
						else if (_tcsicmp(pstrName, _T("strikeout")) == 0) {
							strikeout = (_tcsicmp(pstrValue, _T("true")) == 0);
						}
						else if( _tcsicmp(pstrName, _T("default")) == 0 ) {
							defaultfont = (_tcsicmp(pstrValue, _T("true")) == 0);
						}
						else if( _tcsicmp(pstrName, _T("shared")) == 0 ) {
							shared = (_tcsicmp(pstrValue, _T("true")) == 0);
						}
					}
					if( id >= 0 ) {
						pManager->AddFont(id, pFontName, size, bold, underline, italic, strikeout, shared);
						if( defaultfont ) pManager->SetDefaultFont(pFontName, size, bold, underline, italic, strikeout, shared);
					}
				}
				else if( _tcsicmp(pstrClass, _T("Default")) == 0 ) {
					nAttributes = node.GetAttributeCount();
					LPCTSTR pControlName = NULL;
					LPCTSTR pControlValue = NULL;
					bool shared = false;
					for( int i = 0; i < nAttributes; i++ ) {
						pstrName = node.GetAttributeName(i);
						pstrValue = node.GetAttributeValue(i);
						if( _tcsicmp(pstrName, _T("name")) == 0 ) {
							pControlName = pstrValue;
						}
						else if( _tcsicmp(pstrName, _T("value")) == 0 ) {
							pControlValue = pstrValue;
						}
						else if( _tcsicmp(pstrName, _T("shared")) == 0 ) {
							shared = (_tcsicmp(pstrValue, _T("true")) == 0);
						}
					}
					if( pControlName ) {
						pManager->AddDefaultAttributeList(pControlName, pControlValue, shared);
					}
				}
				else if( _tcsicmp(pstrClass, _T("Style")) == 0 ) {
					nAttributes = node.GetAttributeCount();
					LPCTSTR pName = NULL;
					LPCTSTR pStyle = NULL;
					bool shared = false;
					for( int i = 0; i < nAttributes; i++ ) {
						pstrName = node.GetAttributeName(i);
						pstrValue = node.GetAttributeValue(i);
						if( _tcsicmp(pstrName, _T("name")) == 0 ) {
							pName = pstrValue;
						}
						else if( _tcsicmp(pstrName, _T("value")) == 0 ) {
							pStyle = pstrValue;
						}
						else if( _tcsicmp(pstrName, _T("shared")) == 0 ) {
							shared = (_tcsicmp(pstrValue, _T("true")) == 0);
						}
					}
					if( pName ) {
						pManager->AddStyle(pName, pStyle, shared);
					}
					else {
						LPCTSTR pCssText = node.GetValue();
						if (pCssText && *pCssText) {
							ParseCssBlock(pManager, pCssText);
						}
					}
				}
				else if (_tcsicmp(pstrClass, _T("Import")) == 0) {
					nAttributes = node.GetAttributeCount();
					LPCTSTR pstrPath = NULL;
					for (int i = 0; i < nAttributes; i++) {
						pstrName = node.GetAttributeName(i);
						pstrValue = node.GetAttributeValue(i);
						if (_tcsicmp(pstrName, _T("fontfile")) == 0) {
							pstrPath = pstrValue;
						}
					}
					if (pstrPath) {
						pManager->AddFontArray(pstrPath);
					}
				}
			}

			pstrClass = root.GetName();
			const bool bWindowRoot = (_tcsicmp(pstrClass, _T("Window")) == 0 || _tcsicmp(pstrClass, _T("html")) == 0);
			// 先记下根节点，等全部 <style> 注册后再：CSS 类型选择器 → 内联属性（内联最高）
			CMarkupNode windowRoot = root;

			// Scan sibling nodes of root (e.g. <style> after </html>)
			for (CMarkupNode sibling = root.GetSibling(); sibling.IsValid(); sibling = sibling.GetSibling()) {
				LPCTSTR pstrSiblingClass = sibling.GetName();
				if (_tcsicmp(pstrSiblingClass, _T("Style")) == 0 || _tcsicmp(pstrSiblingClass, _T("style")) == 0) {
					int nAttr = sibling.GetAttributeCount();
					if (nAttr == 0) {
						LPCTSTR pCssText = sibling.GetValue();
						if (pCssText && *pCssText) {
							ParseCssBlock(pManager, pCssText);
						}
					}
				}
			}

			// 优先级：html/window 类型选择器 < 根节点内联属性（与 HTML 一致）
			if( bWindowRoot && pManager->GetPaintWindow() ) {
				ApplyWindowCssRules(pManager);
				int nAttributes = windowRoot.GetAttributeCount();
				for( int i = 0; i < nAttributes; i++ ) {
					ApplyWindowAttribute(pManager, windowRoot.GetAttributeName(i), windowRoot.GetAttributeValue(i));
				}
			}
		}

		return _Parse(&root, pParent, pManager);
	}

	CMarkup* CDialogBuilder::GetMarkup()
	{
		return &m_xml;
	}

	void CDialogBuilder::GetLastErrorMessage(LPTSTR pstrMessage, SIZE_T cchMax) const
	{
		return m_xml.GetLastErrorMessage(pstrMessage, cchMax);
	}

	void CDialogBuilder::GetLastErrorLocation(LPTSTR pstrSource, SIZE_T cchMax) const
	{
		return m_xml.GetLastErrorLocation(pstrSource, cchMax);
	}

	static void LoadCssFile(CPaintManagerUI* pManager, LPCTSTR pstrSrc)
	{
		if (pManager == NULL || pstrSrc == NULL || *pstrSrc == _T('\0')) return;

		CDuiString sPath = CPaintManagerUI::GetResourcePath();
		sPath += pstrSrc;

		HANDLE hFile = ::CreateFile(sPath, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
		if (hFile == INVALID_HANDLE_VALUE) return;

		DWORD dwSize = ::GetFileSize(hFile, NULL);
		if (dwSize == 0 || dwSize == INVALID_FILE_SIZE) {
			::CloseHandle(hFile);
			return;
		}

		char* pBuf = new char[dwSize + 1];
		DWORD dwRead = 0;
		::ReadFile(hFile, pBuf, dwSize, &dwRead, NULL);
		::CloseHandle(hFile);
		pBuf[dwRead] = '\0';

		char* pText = pBuf;
		if (dwRead >= 3 && (BYTE)pText[0] == 0xEF && (BYTE)pText[1] == 0xBB && (BYTE)pText[2] == 0xBF) {
			pText += 3;
			dwRead -= 3;
		}

#ifdef _UNICODE
		int nLen = ::MultiByteToWideChar(CP_UTF8, 0, pText, dwRead, NULL, 0);
		WCHAR* pWide = new WCHAR[nLen + 1];
		::MultiByteToWideChar(CP_UTF8, 0, pText, dwRead, pWide, nLen);
		pWide[nLen] = L'\0';
		ParseCssBlock(pManager, pWide);
		delete[] pWide;
#else
		ParseCssBlock(pManager, pText);
#endif

		delete[] pBuf;
	}

	static void SkipCssCommentsAndSpace(LPCTSTR& p)
	{
		for (;;) {
			while (*p != _T('\0') && (*p == _T(' ') || *p == _T('\t') || *p == _T('\r') || *p == _T('\n')))
				++p;
			// /* ... */（含跨行）；不支持嵌套
			if (p[0] == _T('/') && p[1] == _T('*')) {
				p += 2;
				while (*p != _T('\0') && !(p[0] == _T('*') && p[1] == _T('/')))
					++p;
				if (p[0] == _T('*') && p[1] == _T('/'))
					p += 2;
				continue;
			}
			// // 行注释
			if (p[0] == _T('/') && p[1] == _T('/')) {
				p += 2;
				while (*p != _T('\0') && *p != _T('\r') && *p != _T('\n'))
					++p;
				continue;
			}
			break;
		}
	}

	// :hover / :active / :disabled → 映射到控件已有状态属性（解析期，全控件通用）
	enum CssPseudoKind { CSS_PSEUDO_NONE = 0, CSS_PSEUDO_HOVER, CSS_PSEUDO_ACTIVE, CSS_PSEUDO_DISABLED };

	static CssPseudoKind SplitCssSelectorPseudo(CDuiString& sSelector)
	{
		sSelector.Trim();
		const int nColon = sSelector.ReverseFind(_T(':'));
		if( nColon < 0 ) return CSS_PSEUDO_NONE;
		CDuiString sPseudo = sSelector.Mid(nColon + 1);
		sPseudo.Trim();
		sPseudo.MakeLower();
		CssPseudoKind kind = CSS_PSEUDO_NONE;
		if( sPseudo == _T("hover") ) kind = CSS_PSEUDO_HOVER;
		else if( sPseudo == _T("active") ) kind = CSS_PSEUDO_ACTIVE;
		else if( sPseudo == _T("disabled") ) kind = CSS_PSEUDO_DISABLED;
		else return CSS_PSEUDO_NONE;
		sSelector = sSelector.Left(nColon);
		sSelector.Trim();
		return kind;
	}

	static bool CssAttrLooksAlreadyStateful(LPCTSTR pstrKey)
	{
		if( pstrKey == NULL || *pstrKey == _T('\0') ) return false;
		if( _tcsnicmp(pstrKey, _T("hot"), 3) == 0 ) return true;
		if( _tcsnicmp(pstrKey, _T("pushed"), 6) == 0 ) return true;
		if( _tcsnicmp(pstrKey, _T("disabled"), 8) == 0 ) return true;
		if( _tcsnicmp(pstrKey, _T("focused"), 7) == 0 ) return true;
		if( _tcsnicmp(pstrKey, _T("selected"), 8) == 0 ) return true;
		if( _tcsnicmp(pstrKey, _T("color-"), 6) == 0 ) return true;
		if( _tcsnicmp(pstrKey, _T("fill-"), 5) == 0 ) return true;
		return false;
	}

	static CDuiString RemapCssAttrKeyForPseudo(LPCTSTR pstrKey, CssPseudoKind pseudo)
	{
		CDuiString sKey = pstrKey ? pstrKey : _T("");
		sKey.Trim();
		if( sKey.IsEmpty() ) return sKey;

		// fill-* → color-*（SvgBox）
		if( _tcsicmp(sKey.GetData(), _T("fill-hover")) == 0 ) return _T("color-hover");
		if( _tcsicmp(sKey.GetData(), _T("fill-active")) == 0 ) return _T("color-active");
		if( _tcsicmp(sKey.GetData(), _T("fill-disabled")) == 0 ) return _T("color-disabled");
		if( _tcsicmp(sKey.GetData(), _T("fill")) == 0 ) sKey = _T("color");

		if( CssAttrLooksAlreadyStateful(sKey.GetData()) )
			return sKey;

		if( pseudo == CSS_PSEUDO_NONE ) {
			if( _tcsicmp(sKey.GetData(), _T("color")) == 0 ) return _T("color");
			return sKey;
		}

		// SvgBox：color → color-hover / color-active / color-disabled
		if( _tcsicmp(sKey.GetData(), _T("color")) == 0 ) {
			switch( pseudo ) {
			case CSS_PSEUDO_HOVER: return _T("color-hover");
			case CSS_PSEUDO_ACTIVE: return _T("color-active");
			case CSS_PSEUDO_DISABLED: return _T("color-disabled");
			default: return _T("color");
			}
		}

		struct TCssStateMap {
			LPCTSTR pBase;
			LPCTSTR pHover;
			LPCTSTR pActive;
			LPCTSTR pDisabled; // NULL = 伪类下不改写
		};
		static const TCssStateMap kMap[] = {
			{ _T("bkcolor"),     _T("hotbkcolor"),      _T("pushedbkcolor"),      _T("disabledbkcolor") },
			{ _T("textcolor"),   _T("hottextcolor"),    _T("pushedtextcolor"),    _T("disabledtextcolor") },
			{ _T("bordercolor"), _T("hotbordercolor"),  _T("pushedbordercolor"),  _T("disabledbordercolor") },
			{ _T("image"),       _T("hotimage"),        _T("pushedimage"),        _T("disabledimage") },
			{ _T("bkimage"),     _T("hotimage"),        _T("pushedimage"),        _T("disabledimage") },
			{ _T("foreimage"),   _T("hotforeimage"),    _T("pushedforeimage"),    NULL },
			{ _T("font"),        _T("hotfont"),         _T("pushedfont"),         NULL },
		};
		for( int i = 0; i < (int)(sizeof(kMap) / sizeof(kMap[0])); ++i ) {
			if( _tcsicmp(sKey.GetData(), kMap[i].pBase) != 0 ) continue;
			LPCTSTR pMapped = NULL;
			switch( pseudo ) {
			case CSS_PSEUDO_HOVER: pMapped = kMap[i].pHover; break;
			case CSS_PSEUDO_ACTIVE: pMapped = kMap[i].pActive; break;
			case CSS_PSEUDO_DISABLED: pMapped = kMap[i].pDisabled; break;
			default: break;
			}
			if( pMapped != NULL ) return pMapped;
			return sKey;
		}
		return sKey;
	}

	static void AppendCssAttr(CDuiString& sAttrList, LPCTSTR pstrKey, LPCTSTR pstrVal)
	{
		if( pstrKey == NULL || *pstrKey == _T('\0') || pstrVal == NULL || *pstrVal == _T('\0') ) return;
		if( !sAttrList.IsEmpty() ) sAttrList += _T(' ');
		sAttrList += pstrKey;
		sAttrList += _T("=\"");
		sAttrList += pstrVal;
		sAttrList += _T("\"");
	}

	static void ParseCssBlock(CPaintManagerUI* pManager, LPCTSTR pCssText)
	{
		if (pManager == NULL || pCssText == NULL) return;
		LPCTSTR p = pCssText;
		while (*p != _T('\0')) {
			SkipCssCommentsAndSpace(p);
			if (*p == _T('\0')) break;
			CDuiString sSelector;
			while (*p != _T('\0') && *p != _T('{')) {
				// 选择器列表前的注释
				if (p[0] == _T('/') && (p[1] == _T('*') || p[1] == _T('/'))) {
					SkipCssCommentsAndSpace(p);
					continue;
				}
				sSelector += *p++;
			}
			sSelector.Trim();
			if (*p != _T('{')) break;
			p++;

			struct TCssDecl { CDuiString sKey; CDuiString sVal; };
			CStdPtrArray aDecls;
			while (*p != _T('\0') && *p != _T('}')) {
				SkipCssCommentsAndSpace(p);
				if (*p == _T('\0') || *p == _T('}')) break;
				CDuiString sKey, sVal;
				while (*p != _T('\0') && *p != _T(':') && *p != _T('}') && *p != _T(';')) {
					if (p[0] == _T('/') && (p[1] == _T('*') || p[1] == _T('/'))) {
						SkipCssCommentsAndSpace(p);
						continue;
					}
					sKey += *p++;
				}
				sKey.Trim();
				if (*p != _T(':')) {
					// 容错：跳过损坏声明直到 ; 或 }
					while (*p != _T('\0') && *p != _T(';') && *p != _T('}')) ++p;
					if (*p == _T(';')) ++p;
					continue;
				}
				p++;
				SkipCssCommentsAndSpace(p);
				while (*p != _T('\0') && *p != _T(';') && *p != _T('}')) {
					// 值后同行注释：bkcolor: #F00; // test  或  bkcolor: #F00 // test
					if (p[0] == _T('/') && (p[1] == _T('*') || p[1] == _T('/'))) break;
					sVal += *p++;
				}
				sVal.Trim();
				if (*p == _T(';')) p++;
				SkipCssCommentsAndSpace(p); // 吃掉分号后的 // / /* 注释
				if (!sKey.IsEmpty() && !sVal.IsEmpty()) {
					TCssDecl* pDecl = new TCssDecl;
					pDecl->sKey = sKey;
					pDecl->sVal = sVal;
					aDecls.Add(pDecl);
				}
			}
			if (*p == _T('}')) p++;

			if (!sSelector.IsEmpty() && aDecls.GetSize() > 0) {
				// 逗号分隔选择器列表
				LPCTSTR pSel = sSelector.GetData();
				while (*pSel != _T('\0')) {
					CDuiString sOne;
					while (*pSel != _T('\0') && *pSel != _T(','))
						sOne += *pSel++;
					if (*pSel == _T(',')) ++pSel;
					sOne.Trim();
					if (sOne.IsEmpty()) continue;

					const CssPseudoKind pseudo = SplitCssSelectorPseudo(sOne);
					if (sOne.IsEmpty()) continue;

					CDuiString sAttrList;
					for (int i = 0; i < aDecls.GetSize(); ++i) {
						TCssDecl* pDecl = static_cast<TCssDecl*>(aDecls.GetAt(i));
						if (pDecl == NULL) continue;
						CDuiString sMapped = RemapCssAttrKeyForPseudo(pDecl->sKey.GetData(), pseudo);
						AppendCssAttr(sAttrList, sMapped.GetData(), pDecl->sVal.GetData());
					}
					if (!sAttrList.IsEmpty())
						pManager->AddCssRule(sOne, sAttrList);
				}
			}

			for (int i = 0; i < aDecls.GetSize(); ++i) {
				TCssDecl* pDecl = static_cast<TCssDecl*>(aDecls.GetAt(i));
				delete pDecl;
			}
			aDecls.Empty();
		}
	}

	CControlUI* CDialogBuilder::_Parse(CMarkupNode* pRoot, CControlUI* pParent, CPaintManagerUI* pManager)
	{
		IContainerUI* pContainer = NULL;
		CControlUI* pReturn = NULL;
		for( CMarkupNode node = pRoot->GetChild() ; node.IsValid(); node = node.GetSibling() ) {
			LPCTSTR pstrClass = node.GetName();
			if( _tcsicmp(pstrClass, _T("Image")) == 0 || _tcsicmp(pstrClass, _T("Font")) == 0 \
				|| _tcsicmp(pstrClass, _T("Default")) == 0 || _tcsicmp(pstrClass, _T("Style")) == 0 ) continue;

			CControlUI* pControl = NULL;
			if (_tcsicmp(pstrClass, _T("Import")) == 0) continue;
			if( _tcsicmp(pstrClass, _T("Include")) == 0 ) {
				if( !node.HasAttributes() ) continue;
				int count = 1;
				LPTSTR pstr = NULL;
				TCHAR szValue[500] = { 0 };
				SIZE_T cchLen = lengthof(szValue) - 1;
				if ( node.GetAttributeValue(_T("count"), szValue, cchLen) )
					count = _tcstol(szValue, &pstr, 10);
				cchLen = lengthof(szValue) - 1;
				if ( !node.GetAttributeValue(_T("source"), szValue, cchLen) ) continue;
				for ( int i = 0; i < count; i++ ) {
					CDialogBuilder builder;
					if( m_pstrtype != NULL ) { // 使用资源dll，从资源中读取
						WORD id = (WORD)_tcstol(szValue, &pstr, 10); 
						pControl = builder.Create((UINT)id, m_pstrtype, m_pCallback, pManager, pParent);
					}
					else {
						pControl = builder.Create((LPCTSTR)szValue, (UINT)0, m_pCallback, pManager, pParent);
					}
				}
				continue;
			}
			else {
				CDuiString strClass;
				strClass.Format(_T("C%sUI"), pstrClass);
				pControl = dynamic_cast<CControlUI*>(CControlFactory::GetInstance()->CreateControl(strClass));

				// 检查插件
				if( pControl == NULL ) {
					CStdPtrArray* pPlugins = CPaintManagerUI::GetPlugins();
					LPCREATECONTROL lpCreateControl = NULL;
					for( int i = 0; i < pPlugins->GetSize(); ++i ) {
						lpCreateControl = (LPCREATECONTROL)pPlugins->GetAt(i);
						if( lpCreateControl != NULL ) {
							pControl = lpCreateControl(pstrClass);
							if( pControl != NULL ) break;
						}
					}
				}
				// 回掉创建
				if( pControl == NULL && m_pCallback != NULL ) {
					pControl = m_pCallback->CreateControl(pstrClass);
				}
			}

			if( pControl == NULL ) {
#ifdef _DEBUG
				DUITRACE(_T("未知控件:%s"), pstrClass);
#else
				continue;
#endif
			}

			// Add children
			if( node.HasChildren() ) {
				_Parse(&node, pControl, pManager);
			}
			// Attach to parent
			// 因为某些属性和父窗口相关，比如selected，必须先Add到父窗口
			CTreeViewUI* pTreeView = NULL;
			if( pParent != NULL && pControl != NULL ) {
				CTreeNodeUI* pParentTreeNode = static_cast<CTreeNodeUI*>(pParent->GetInterface(_T("TreeNode")));
				CTreeNodeUI* pTreeNode = static_cast<CTreeNodeUI*>(pControl->GetInterface(_T("TreeNode")));
				pTreeView = static_cast<CTreeViewUI*>(pParent->GetInterface(_T("TreeView")));
				// TreeNode子节点
				if(pTreeNode != NULL) {
					if(pParentTreeNode) {
						pTreeView = pParentTreeNode->GetTreeView();
						if(!pParentTreeNode->Add(pTreeNode)) {
							delete pTreeNode;
							pTreeNode = NULL;
							continue;
						}
					}
					else {
						if(pTreeView != NULL) {
							if(!pTreeView->Add(pTreeNode)) {
								delete pTreeNode;
								pTreeNode = NULL;
								continue;
							}
						}
					}
				}
				// TreeNode子控件
				else if(pParentTreeNode != NULL) {
					pParentTreeNode->GetTreeNodeHoriznotal()->Add(pControl);
				}
				// 普通控件
				else {
					if( pContainer == NULL ) pContainer = static_cast<IContainerUI*>(pParent->GetInterface(_T("IContainer")));
					ASSERT(pContainer);
					if( pContainer == NULL ) return NULL;
					if( !pContainer->Add(pControl) ) {
						delete pControl;
						continue;
					}
				}
			}
			if( pControl == NULL ) continue;

			// Init default attributes
			if( pManager ) {
				if(pTreeView != NULL) {
					pControl->SetManager(pManager, pTreeView, true);
				}
				else {
					pControl->SetManager(pManager, NULL, false);
				}
				LPCTSTR pDefaultAttributes = pManager->GetDefaultAttributeList(pstrClass);
				if( pDefaultAttributes ) {
					pControl->ApplyAttributeList(pDefaultAttributes);
				}
				LPCTSTR pCssTypeAttrs = pManager->GetCssTypeRule(pstrClass);
				if (pCssTypeAttrs) {
					pControl->ApplyAttributeList(pCssTypeAttrs);
				}
				LPCTSTR pControlName = node.GetAttributeValue(_T("name"));
				if (!pControlName || !*pControlName) pControlName = node.GetAttributeValue(_T("id"));
				if (pControlName && *pControlName) {
					LPCTSTR pCssIdAttrs = pManager->GetCssIdRule(pControlName);
					if (pCssIdAttrs) {
						pControl->ApplyAttributeList(pCssIdAttrs);
					}
				}
			}
			// Process attributes
			if( node.HasAttributes() ) {
				// Set ordinary attributes
				int nAttributes = node.GetAttributeCount();
				for( int i = 0; i < nAttributes; i++ ) {
					pControl->SetAttribute(node.GetAttributeName(i), node.GetAttributeValue(i));
				}
			}
			if( pManager ) {
				if(pTreeView == NULL) {
					pControl->SetManager(NULL, NULL, false);
				}
			}
			// Return first item
			if( pReturn == NULL ) pReturn = pControl;
		}
		return pReturn;
	}

} // namespace DuiLib
