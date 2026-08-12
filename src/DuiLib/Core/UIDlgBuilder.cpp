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
		else if( _tcsicmp(pstrName, _T("width")) == 0 ) {
			SIZE sz = pManager->GetInitSize();
			int cx = _ttoi(pstrValue);
			pManager->SetInitSize(pManager->GetDPIObj()->Scale(cx), sz.cy);
		}
		else if( _tcsicmp(pstrName, _T("height")) == 0 ) {
			SIZE sz = pManager->GetInitSize();
			int cy = _ttoi(pstrValue);
			pManager->SetInitSize(sz.cx, pManager->GetDPIObj()->Scale(cy));
		}
		else if( _tcsicmp(pstrName, _T("size-box")) == 0 ) {
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
		else if( _tcsicmp(pstrName, _T("border-radius")) == 0 ) {
			SIZE szRound = { 0 };
			if( ParseBorderRadiusValue(pstrValue, szRound) )
				pManager->SetBorderRadius(szRound.cx, szRound.cy);
		}
		else if( _tcsicmp(pstrName, _T("min-size")) == 0 ) {
			LPTSTR pstr = NULL;
			int cx = _tcstol(pstrValue, &pstr, 10);  ASSERT(pstr);
			int cy = _tcstol(pstr + 1, &pstr, 10);    ASSERT(pstr);
			pManager->SetMinSize(cx, cy);
		}
		else if( _tcsicmp(pstrName, _T("min-width")) == 0 ) {
			SIZE sz = pManager->GetMinSize();
			CDPI* pDpi = pManager->GetDPIObj();
			int cy = (pDpi != NULL) ? pDpi->ScaleBack(sz.cy) : sz.cy;
			pManager->SetMinSize(_ttoi(pstrValue), cy);
		}
		else if( _tcsicmp(pstrName, _T("min-height")) == 0 ) {
			SIZE sz = pManager->GetMinSize();
			CDPI* pDpi = pManager->GetDPIObj();
			int cx = (pDpi != NULL) ? pDpi->ScaleBack(sz.cx) : sz.cx;
			pManager->SetMinSize(cx, _ttoi(pstrValue));
		}
		else if( _tcsicmp(pstrName, _T("max-size")) == 0 ) {
			LPTSTR pstr = NULL;
			int cx = _tcstol(pstrValue, &pstr, 10);  ASSERT(pstr);
			int cy = _tcstol(pstr + 1, &pstr, 10);    ASSERT(pstr);
			pManager->SetMaxSize(cx, cy);
		}
		else if( _tcsicmp(pstrName, _T("max-width")) == 0 ) {
			SIZE sz = pManager->GetMaxSize();
			CDPI* pDpi = pManager->GetDPIObj();
			int cy = (pDpi != NULL) ? pDpi->ScaleBack(sz.cy) : sz.cy;
			pManager->SetMaxSize(_ttoi(pstrValue), cy);
		}
		else if( _tcsicmp(pstrName, _T("max-height")) == 0 ) {
			SIZE sz = pManager->GetMaxSize();
			CDPI* pDpi = pManager->GetDPIObj();
			int cx = (pDpi != NULL) ? pDpi->ScaleBack(sz.cx) : sz.cx;
			pManager->SetMaxSize(cx, _ttoi(pstrValue));
		}
		else if( _tcsicmp(pstrName, _T("show-dirty")) == 0 ) {
			pManager->SetShowUpdateRect(_tcsicmp(pstrValue, _T("true")) == 0);
		}
		else if( _tcsicmp(pstrName, _T("opacity")) == 0 || _tcsicmp(pstrName, _T("alpha")) == 0 ) {
			BYTE nOpacity = 255;
			if( ParseCssOpacity(pstrValue, nOpacity) )
				pManager->SetOpacity(nOpacity);
			else
				pManager->SetOpacity((BYTE)_ttoi(pstrValue));
		}
		else if( _tcsicmp(pstrName, _T("wallpaper-bleed")) == 0 || _tcsicmp(pstrName, _T("bg-bleed")) == 0 ) {
			if( _tcsicmp(pstrValue, _T("false")) == 0 || _tcsicmp(pstrValue, _T("none")) == 0
				|| _tcsicmp(pstrValue, _T("off")) == 0 || _tcsicmp(pstrValue, _T("solid")) == 0
				|| _tcsicmp(pstrValue, _T("opaque")) == 0 ) {
				pManager->SetWallpaperBleed(255);
			}
			else {
				BYTE nBleed = 255;
				if( ParseCssOpacity(pstrValue, nBleed) )
					pManager->SetWallpaperBleed(nBleed);
				else
					pManager->SetWallpaperBleed((BYTE)_ttoi(pstrValue));
			}
		}
		else if( _tcsicmp(pstrName, _T("wallpaper-bleed-need-image")) == 0
			|| _tcsicmp(pstrName, _T("bg-bleed-need-image")) == 0 ) {
			pManager->SetWallpaperBleedNeedImage(_tcsicmp(pstrValue, _T("false")) != 0);
		}
		else if( _tcscmp(pstrName, _T("layered-opacity")) == 0 ) {
			pManager->SetLayeredOpacity(_ttoi(pstrValue));
		}
		else if( _tcsicmp(pstrName, _T("layered")) == 0 ) {
			pManager->SetLayered(_tcsicmp(pstrValue, _T("true")) == 0);
		}
		else if( _tcscmp(pstrName, _T("layered-image")) == 0 ) {
			pManager->SetLayered(true);
			pManager->SetLayeredImage(pstrValue);
		}
		else if( _tcsicmp(pstrName, _T("shape-image")) == 0 ) {
			pManager->SetShapeImage(pstrValue);
		}
		else if( _tcsicmp(pstrName, _T("shape-mask")) == 0 ) {
			pManager->SetShapeMask(pstrValue);
		}
		else if( _tcsicmp(pstrName, _T("shape-alpha-threshold")) == 0 ) {
			pManager->SetShapeAlphaThreshold((BYTE)_ttoi(pstrValue));
		}
		else if( _tcsicmp(pstrName, _T("shape-drag")) == 0 ) {
			pManager->SetShapeDragEnabled(_tcsicmp(pstrValue, _T("true")) == 0);
		}
		else if( _tcscmp(pstrName, _T("no-activate")) == 0 ) {
			pManager->SetNoActivate(_tcsicmp(pstrValue, _T("true")) == 0);
		}
		else if( _tcsicmp(pstrName, _T("disabled-font-color")) == 0 ) {
			DWORD clrColor = 0;
			if( ParseColorString(pstrValue, clrColor) )
				pManager->SetDefaultDisabledColor(clrColor);
		}
		else if( _tcsicmp(pstrName, _T("default-font-color")) == 0 || _tcsicmp(pstrName, _T("color")) == 0 ) {
			DWORD clrColor = 0;
			if( ParseColorString(pstrValue, clrColor) )
				pManager->SetDefaultFontColor(clrColor);
		}
		else if( _tcsicmp(pstrName, _T("link-font-color")) == 0 ) {
			DWORD clrColor = 0;
			if( ParseColorString(pstrValue, clrColor) )
				pManager->SetDefaultLinkFontColor(clrColor);
		}
		else if( _tcsicmp(pstrName, _T("link-hover-font-color")) == 0 ) {
			DWORD clrColor = 0;
			if( ParseColorString(pstrValue, clrColor) )
				pManager->SetDefaultLinkHoverFontColor(clrColor);
		}
		else if( _tcsicmp(pstrName, _T("selected-color")) == 0 ) {
			DWORD clrColor = 0;
			if( ParseColorString(pstrValue, clrColor) )
				pManager->SetDefaultSelectedBackgroundColor(clrColor);
		}
		else if( _tcsicmp(pstrName, _T("background-color")) == 0 ) {
			DWORD clrColor = 0;
			if( ParseColorString(pstrValue, clrColor) )
				pManager->SetWindowBackgroundColor(clrColor);
		}
		else if( _tcsicmp(pstrName, _T("background-image")) == 0 ) {
			pManager->SetWindowBackgroundImage(pstrValue);
		}
		else if( _tcsicmp(pstrName, _T("font-family")) == 0
			|| _tcsicmp(pstrName, _T("font-size")) == 0
			|| _tcsicmp(pstrName, _T("font-weight")) == 0
			|| _tcsicmp(pstrName, _T("font-style")) == 0
			|| _tcsicmp(pstrName, _T("text-decoration")) == 0 ) {
			TFontInfo* pFi = pManager->GetDefaultFontInfo();
			if( pFi != NULL ) {
				CDuiString sName = pFi->sFontName;
				int nSize = pFi->iSize;
				bool bBold = pFi->bBold;
				bool bUnderline = pFi->bUnderline;
				bool bItalic = pFi->bItalic;
				bool bStrikeout = pFi->bStrikeout;
				if( _tcsicmp(pstrName, _T("font-family")) == 0 ) sName = pstrValue;
				else if( _tcsicmp(pstrName, _T("font-size")) == 0 ) nSize = _ttoi(pstrValue);
				else if( _tcsicmp(pstrName, _T("font-weight")) == 0 ) ParseCssFontWeightBold(pstrValue, bBold);
				else if( _tcsicmp(pstrName, _T("font-style")) == 0 ) ParseCssFontStyleItalic(pstrValue, bItalic);
				else if( _tcsicmp(pstrName, _T("text-decoration")) == 0 ) ParseCssTextDecoration(pstrValue, bUnderline, bStrikeout);
				pManager->SetDefaultFont(sName.GetData(), nSize, bBold, bUnderline, bItalic, bStrikeout);
			}
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
		else if( _tcsicmp(pstrName, _T("theme")) == 0 ) {
			// html/Window：chrome 等落到窗口级，Attach 后赋给 root（见 SetWindowTheme）
			pManager->SetWindowTheme(pstrValue);
		}
		else if( _tcsicmp(pstrName, _T("theme-id")) == 0 ) {
			pManager->SetWindowThemeId(pstrValue);
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
			DWORD clrColor = 0;
			if( ParseColorString(pstrValue, clrColor) )
				pManager->GetShadow()->SetColor(DuiColorToCOLORREF(clrColor));
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
		else if( _tcsicmp(pstrName, _T("gdiplus-text")) == 0 ) {
			pManager->SetUseGdiplusText(_tcsicmp(pstrValue, _T("true")) == 0);
		}
		else if( _tcsicmp(pstrName, _T("text-rendering-hint")) == 0 ) {
			pManager->SetGdiplusTextRenderingHint(_ttoi(pstrValue));
		}
		else if( _tcsicmp(pstrName, _T("tooltip-hover-time")) == 0 ) {
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
			ApplyWindowAttribute(pManager, sItem.GetData(), sValue.GetData());
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
						else if( _tcsicmp(pstrName, _T("res-type")) == 0 ) {
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
						else if( _tcsicmp(pstrName, _T("font-family")) == 0 ) {
							pFontName = pstrValue;
						}
						else if( _tcsicmp(pstrName, _T("font-size")) == 0 ) {
							size = _tcstol(pstrValue, &pstr, 10);
						}
						else if( _tcsicmp(pstrName, _T("bold")) == 0 ) {
							bold = (_tcsicmp(pstrValue, _T("true")) == 0);
						}
						else if( _tcsicmp(pstrName, _T("font-weight")) == 0 ) {
							ParseCssFontWeightBold(pstrValue, bold);
						}
						else if( _tcsicmp(pstrName, _T("underline")) == 0 ) {
							underline = (_tcsicmp(pstrValue, _T("true")) == 0);
						}
						else if( _tcsicmp(pstrName, _T("italic")) == 0 ) {
							italic = (_tcsicmp(pstrValue, _T("true")) == 0);
						}
						else if( _tcsicmp(pstrName, _T("font-style")) == 0 ) {
							ParseCssFontStyleItalic(pstrValue, italic);
						}
						else if (_tcsicmp(pstrName, _T("strikeout")) == 0) {
							strikeout = (_tcsicmp(pstrValue, _T("true")) == 0);
						}
						else if( _tcsicmp(pstrName, _T("text-decoration")) == 0 ) {
							ParseCssTextDecoration(pstrValue, underline, strikeout);
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
						if (_tcsicmp(pstrName, _T("font-file")) == 0) {
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

		BYTE* pRaw = NULL;
		DWORD dwSize = 0;
		if (!CPaintManagerUI::LoadResourceData(pstrSrc, &pRaw, &dwSize) || pRaw == NULL || dwSize == 0)
			return;

		char* pBuf = new char[dwSize + 1];
		::CopyMemory(pBuf, pRaw, dwSize);
		pBuf[dwSize] = '\0';
		delete[] pRaw;

		DWORD dwRead = dwSize;
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

	// :hover / :active / :disabled / :focus / :checked|:selected → 状态属性（解析期）
	enum CssPseudoKind {
		CSS_PSEUDO_NONE = 0,
		CSS_PSEUDO_HOVER,
		CSS_PSEUDO_ACTIVE,
		CSS_PSEUDO_DISABLED,
		CSS_PSEUDO_FOCUS,
		CSS_PSEUDO_CHECKED
	};

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
		else if( sPseudo == _T("focus") ) kind = CSS_PSEUDO_FOCUS;
		else if( sPseudo == _T("checked") || sPseudo == _T("selected") ) kind = CSS_PSEUDO_CHECKED;
		else return CSS_PSEUDO_NONE;
		sSelector = sSelector.Left(nColon);
		sSelector.Trim();
		return kind;
	}

	static bool CssAttrLooksAlreadyStateful(LPCTSTR pstrKey)
	{
		if( pstrKey == NULL || *pstrKey == _T('\0') ) return false;
		size_t nLen = _tcslen(pstrKey);
		auto endsWith = [&](LPCTSTR suf, size_t nSuf) -> bool {
			return nLen >= nSuf && _tcsicmp(pstrKey + (nLen - nSuf), suf) == 0;
		};
		if( endsWith(_T("-hover"), 6) ) return true;
		if( endsWith(_T("-active"), 7) ) return true;
		if( endsWith(_T("-disabled"), 9) ) return true;
		if( endsWith(_T("-focus"), 6) ) return true;
		if( endsWith(_T("-selected"), 9) ) return true;
		if( _tcsnicmp(pstrKey, _T("focused"), 7) == 0 ) return true;
		if( _tcsnicmp(pstrKey, _T("selected"), 8) == 0 ) return true;
		if( _tcsnicmp(pstrKey, _T("color-"), 6) == 0 ) return true;
		return false;
	}

	static CDuiString RemapCssAttrKeyForPseudo(LPCTSTR pstrKey, CssPseudoKind pseudo)
	{
		CDuiString sKey = pstrKey ? pstrKey : _T("");
		sKey.Trim();
		if( sKey.IsEmpty() ) return sKey;

		if( CssAttrLooksAlreadyStateful(sKey.GetData()) )
			return sKey;

		if( pseudo == CSS_PSEUDO_NONE ) {
			if( _tcsicmp(sKey.GetData(), _T("color")) == 0 ) return _T("color");
			return sKey;
		}

		// color → color-hover / … / color-focus / color-selected
		if( _tcsicmp(sKey.GetData(), _T("color")) == 0 ) {
			switch( pseudo ) {
			case CSS_PSEUDO_HOVER: return _T("color-hover");
			case CSS_PSEUDO_ACTIVE: return _T("color-active");
			case CSS_PSEUDO_DISABLED: return _T("color-disabled");
			case CSS_PSEUDO_FOCUS: return _T("color-focus");
			case CSS_PSEUDO_CHECKED: return _T("color-selected");
			default: return _T("color");
			}
		}

		struct TCssStateMap {
			LPCTSTR pBase;
			LPCTSTR pHover;
			LPCTSTR pActive;
			LPCTSTR pDisabled;
			LPCTSTR pFocus;
			LPCTSTR pChecked;
		};
		static const TCssStateMap kMap[] = {
			{ _T("background-color"),   _T("background-color-hover"),   _T("background-color-active"),   _T("background-color-disabled"), _T("background-color-focus"), _T("background-color-selected") },
			{ _T("border-color"),       _T("border-color-hover"),       _T("border-color-active"),       _T("border-color-disabled"),     _T("border-color-focus"),     NULL },
			{ _T("image"),              _T("image-hover"),              _T("image-active"),              _T("image-disabled"),            _T("image-focus"),            _T("image-selected") },
			{ _T("background-image"),   _T("background-image-hover"),   _T("background-image-active"),   _T("background-image-disabled"), _T("background-image-focus"), _T("background-image-selected") },
			{ _T("foreground-image"),   _T("foreground-image-hover"),   _T("foreground-image-active"),   NULL,                            NULL,                       _T("foreground-image-selected") },
			{ _T("icon-tint"),          _T("icon-tint-hover"),          _T("icon-tint-active"),          _T("icon-tint-disabled"),        _T("icon-tint-focus"),        _T("icon-tint-selected") },
			{ _T("icon-color"),         _T("icon-color-hover"),         _T("icon-color-active"),         _T("icon-color-disabled"),       _T("icon-color-focus"),       _T("icon-color-selected") },
			// List / VirtualList / Combo / Menu 行皮肤
			{ _T("item-color"),              _T("item-color-hover"),              _T("item-color-selected"),              _T("item-color-disabled"),              NULL, _T("item-color-selected") },
			{ _T("item-background-color"),   _T("item-background-color-hover"),   _T("item-background-color-selected"),   _T("item-background-color-disabled"),   NULL, _T("item-background-color-selected") },
			{ _T("item-image"),              _T("item-image-hover"),              _T("item-image-selected"),              _T("item-image-disabled"),              NULL, _T("item-image-selected") },
			{ _T("item-foreground-image"),   _T("item-foreground-image-hover"),   NULL,                                    NULL,                                    NULL, _T("item-foreground-image-selected") },
		};
		for( int i = 0; i < (int)(sizeof(kMap) / sizeof(kMap[0])); ++i ) {
			if( _tcsicmp(sKey.GetData(), kMap[i].pBase) != 0 ) continue;
			LPCTSTR pMapped = NULL;
			switch( pseudo ) {
			case CSS_PSEUDO_HOVER: pMapped = kMap[i].pHover; break;
			case CSS_PSEUDO_ACTIVE: pMapped = kMap[i].pActive; break;
			case CSS_PSEUDO_DISABLED: pMapped = kMap[i].pDisabled; break;
			case CSS_PSEUDO_FOCUS: pMapped = kMap[i].pFocus; break;
			case CSS_PSEUDO_CHECKED: pMapped = kMap[i].pChecked; break;
			default: break;
			}
			if( pMapped != NULL ) return pMapped;
			// 未实现的伪类态：跳过，避免写回基属性覆盖常态
			return _T("");
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
					// 值后同行注释：background-color: #F00; // test  或  background-color: #F00 // test
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
						pManager->AddCssRule(sOne.GetData(), sAttrList.GetData());
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
