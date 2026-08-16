#ifndef __UISVGBOX_H__
#define __UISVGBOX_H__

#pragma once

	#include <string>
	#include <vector>

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

		/// 从文件加载内容（透明分流）：扩展名为 `.ico/.png/.jpg/.jpeg/.bmp/.gif/.webp` 时按本地位图显示；
		/// 否则（`.svg` 等）按 SVG 显示。相对路径基于资源路径；也可传绝对路径。
		void LoadFromFile(LPCTSTR pstrPath);
		/// 从二进制内存加载**位图**（.ico/.png/.jpg/.jpeg/.bmp/.gif/.webp 字节），不做 SVG 着色。
		/// 数据会被拷贝一份，可安全释放调用方缓冲。
		void LoadFromMemory(const BYTE* pData, size_t nLen);
		/// 从资源加载**位图**字节（如 .rc 里的 ICO/PNG）。pstrType 默认 RT_RCDATA，可给自定义类型。
		void LoadFromResource(LPCTSTR pstrType, UINT nID);
		/// 用**字符串形式**的 SVG 内容加载（等效 `src` 内联 SVG）。
		void LoadFromData(LPCTSTR pstrSvgContent);
		/// 用 UTF-8 编码的 SVG 字节加载。
		void LoadFromUtf8Data(const char* utf8Svg);
		void SetColor(DWORD dwColor, bool bInvalidate = true);
		DWORD GetColor() const;
		void SetHoverColor(DWORD dwColor, bool bInvalidate = true);
		DWORD GetHoverColor() const;
		void SetActiveColor(DWORD dwColor, bool bInvalidate = true);
		DWORD GetActiveColor() const;
		void SetDisabledColor(DWORD dwColor, bool bInvalidate = true);
		DWORD GetDisabledColor() const;

		void SetEnabled(bool bEnable = true);
		void Invalidate() override;
		void DoEvent(TEventUI& event);
		void SetAttribute(LPCTSTR pstrName, LPCTSTR pstrValue);
		void PaintStatusImage(IRenderContext& ctx);
		/// 仅绘制缓存位图（供 Button 等父控件内嵌时调用，不走完整 DoPaint）
		void PaintIcon(IRenderContext& ctx, const RECT& rcPaint);

		/// 导出当前 SVG 为 PNG / JPEG / BMP（由扩展名决定）。`.ico` 请用 ExportToIcoFile。
		/// width/height <= 0：用控件当前尺寸，仍无效则 256。
		/// dwTintColor == (DWORD)-1：用当前绘制色 GetPaintColor()；0：不着色。
		/// jpegQuality：1~100，仅 JPEG 有效。
		bool ExportToFile(LPCTSTR pstrPath, int width = 0, int height = 0,
			DWORD dwTintColor = (DWORD)-1, int jpegQuality = 90) const;

		/// ICO 专用（PNG-in-ICO，保留 alpha）。默认 16/24/32/48/64/128/256，适合 Windows 壳图标。
		bool ExportToIcoFile(LPCTSTR pstrPath, DWORD dwTintColor = (DWORD)-1) const;
		/// ICO 专用：自定义正方形边长列表（如 16,32,48,256；单边最大 512）；非法边长跳过。
		bool ExportToIcoFile(LPCTSTR pstrPath, const int* pSizes, int nCount,
			DWORD dwTintColor = (DWORD)-1) const;

		/// 栅格化 SVG → 预乘 alpha HBITMAP（调用方 DeleteObject）。width/height<=0 用文档尺寸，仍无效则 256。
		static HBITMAP RasterizeToHBitmap(const char* utf8Svg, size_t nBytes,
			int width = 0, int height = 0, DWORD dwTintColor = 0,
			int* pOutW = NULL, int* pOutH = NULL);
		static HBITMAP RasterizeToHBitmap(LPCTSTR pstrSvg,
			int width = 0, int height = 0, DWORD dwTintColor = 0,
			int* pOutW = NULL, int* pOutH = NULL);

	protected:
		static DWORD ParseColorValue(LPCTSTR pstrValue);
		static CDuiString ResolveFilePath(LPCTSTR pstrPath);
		/// 根据 SVG 内容判断着色方式：填充图标勿加 stroke，描边图标勿强制 fill
		enum TintMode { TintFill = 0, TintStroke = 1, TintBoth = 2, TintSkip = 3 };
		static TintMode DetectTintMode(const std::string& svgUtf8);
		DWORD GetPaintColor() const;
		void SyncControlStateFromButton();
		void EnsureInteractiveCursor();
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
		std::vector<BYTE> m_vBitmapData;   // 位图内存/资源字节（非空则按位图显示）
	};
}

#endif // __UISVGBOX_H__
