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
		void SetHotTintColor(DWORD dwColor);
		DWORD GetHotTintColor() const;
		void SetPushedTintColor(DWORD dwColor);
		DWORD GetPushedTintColor() const;
		void SetDisabledTintColor(DWORD dwColor);
		DWORD GetDisabledTintColor() const;

		void SetEnabled(bool bEnable);
		void DoEvent(TEventUI& event);
		void SetAttribute(LPCTSTR pstrName, LPCTSTR pstrValue);
		void PaintStatusImage(IRenderContext& ctx);

	protected:
		static DWORD ParseColorValue(LPCTSTR pstrValue);
		static CDuiString ResolveFilePath(LPCTSTR pstrPath);
		DWORD GetPaintTintColor() const;
		void ClearCache();
		bool EnsureCache(int w, int h, DWORD dwTint);

	private:
		CDuiString m_sSvgPath;
		CDuiString m_sSvgData;
		std::string m_sSvgUtf8;
		DWORD m_dwTintColor;
		DWORD m_dwHotTintColor;
		DWORD m_dwPushedTintColor;
		DWORD m_dwDisabledTintColor;
		UINT m_uButtonState;

		HBITMAP m_hCacheBitmap;
		int m_nCacheW;
		int m_nCacheH;
		DWORD m_dwCacheTint;
	};
}

#endif // __UISVGBOX_H__
