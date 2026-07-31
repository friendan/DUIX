#ifndef __UISVGBOX_H__
#define __UISVGBOX_H__

#pragma once

#include <string>

namespace DuiLib
{
	class UILIB_API CSvgBoxUI : public CControlUI
	{
		DECLARE_DUICONTROL(CSvgBoxUI)
	public:
		CSvgBoxUI();
		~CSvgBoxUI();

		LPCTSTR GetClass() const;
		LPVOID GetInterface(LPCTSTR pstrName);

		void LoadFromFile(LPCTSTR pstrPath);
		void LoadFromData(LPCTSTR pstrSvgContent);
		void LoadFromUtf8Data(const char* utf8Svg);
		void SetTintColor(DWORD dwColor);
		DWORD GetTintColor() const;

		void SetAttribute(LPCTSTR pstrName, LPCTSTR pstrValue);
		void PaintStatusImage(IRenderContext& ctx);

	protected:
		static DWORD ParseColorValue(LPCTSTR pstrValue);
		static CDuiString ResolveFilePath(LPCTSTR pstrPath);
		void ClearCache();
		bool EnsureCache(int w, int h);

	private:
		CDuiString m_sSvgPath;
		CDuiString m_sSvgData;
		std::string m_sSvgUtf8;
		DWORD m_dwTintColor;

		HBITMAP m_hCacheBitmap;
		int m_nCacheW;
		int m_nCacheH;
		DWORD m_dwCacheTint;
	};
}

#endif // __UISVGBOX_H__
