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
		UINT GetControlFlags() const;
		bool PreferClientHit() const;

		void LoadFromFile(LPCTSTR pstrPath);
		void LoadFromData(LPCTSTR pstrSvgContent);
		void LoadFromUtf8Data(const char* utf8Svg);
		void SetColor(DWORD dwColor);
		DWORD GetColor() const;
		void SetHoverColor(DWORD dwColor);
		DWORD GetHoverColor() const;
		void SetActiveColor(DWORD dwColor);
		DWORD GetActiveColor() const;
		void SetDisabledColor(DWORD dwColor);
		DWORD GetDisabledColor() const;

		void SetEnabled(bool bEnable);
		void DoEvent(TEventUI& event);
		void SetAttribute(LPCTSTR pstrName, LPCTSTR pstrValue);
		void PaintStatusImage(IRenderContext& ctx);

	protected:
		static DWORD ParseColorValue(LPCTSTR pstrValue);
		static CDuiString ResolveFilePath(LPCTSTR pstrPath);
		/// 根据 SVG 内容判断着色方式：填充图标勿加 stroke，描边图标勿强制 fill
		enum TintMode { TintFill = 0, TintStroke = 1, TintBoth = 2, TintSkip = 3 };
		static TintMode DetectTintMode(const std::string& svgUtf8);
		DWORD GetPaintColor() const;
		void ClearCache();
		bool EnsureCache(int w, int h, DWORD dwColor);

	private:
		CDuiString m_sSvgPath;
		CDuiString m_sSvgData;
		std::string m_sSvgUtf8;
		DWORD m_dwColor;
		DWORD m_dwHoverColor;
		DWORD m_dwActiveColor;
		DWORD m_dwDisabledColor;
		UINT m_uButtonState;

		HBITMAP m_hCacheBitmap;
		int m_nCacheW;
		int m_nCacheH;
		DWORD m_dwCacheColor;
	};
}

#endif // __UISVGBOX_H__
