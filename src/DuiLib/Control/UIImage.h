#ifndef __UIIMAGE_H__
#define __UIIMAGE_H__

#pragma once

namespace DuiLib
{
	/// 专用图片控件（XML：`<Img>` / `<Picture>` / `<ImageBox>`；勿用 `<Image>` 资源标签）。
	/// object-fit、圆角裁剪（基类 border-radius）、失败占位。
	class UILIB_API CImageUI : public CLabelUI
	{
		DECLARE_DUICONTROL(CImageUI)
	public:
		enum ObjectFit
		{
			FitFill = 0,
			FitContain,
			FitCover,
			FitNone,
			FitScaleDown,
		};

		CImageUI();
		virtual ~CImageUI();

		LPCTSTR GetClass() const;
		LPVOID GetInterface(LPCTSTR pstrName);

		void SetSrc(LPCTSTR pstrSrc);
		LPCTSTR GetSrc() const;
		void SetObjectFit(ObjectFit fit);
		ObjectFit GetObjectFit() const;
		void SetPlaceholder(LPCTSTR pstrImage);
		LPCTSTR GetPlaceholder() const;
		void SetErrorImage(LPCTSTR pstrImage);
		LPCTSTR GetErrorImage() const;
		void SetPlaceholderText(LPCTSTR pstrText);
		LPCTSTR GetPlaceholderText() const;
		bool IsImageLoaded() const;

		virtual void SetAttribute(LPCTSTR pstrName, LPCTSTR pstrValue);
		virtual void PaintStatusImage(IRenderContext& ctx);
		virtual void PaintText(IRenderContext& ctx);

	protected:
		const TImageInfo* ResolveImage(bool& bIsError) const;
		static void CalcObjectFit(int imgW, int imgH, const RECT& rcBox, ObjectFit fit, RECT& rcSrc, RECT& rcDest);
		void PaintResolvedImage(IRenderContext& ctx, const TImageInfo* pInfo);

	protected:
		CDuiString m_sSrc;
		CDuiString m_sPlaceholder;
		CDuiString m_sErrorImage;
		CDuiString m_sPlaceholderText;
		ObjectFit m_eObjectFit;
		mutable bool m_bLoadFailed;
	};

	/// 头像：默认圆形裁剪；无图时显示缩写文字。
	class UILIB_API CAvatarUI : public CImageUI
	{
		DECLARE_DUICONTROL(CAvatarUI)
	public:
		CAvatarUI();

		LPCTSTR GetClass() const;
		LPVOID GetInterface(LPCTSTR pstrName);

		void SetCircle(bool bCircle);
		bool IsCircle() const;
		void SetAlt(LPCTSTR pstrAlt);
		LPCTSTR GetAlt() const;
		void SetSizePreset(int nSize);
		void SetFallbackBackgroundColor(DWORD dwColor);
		void SetFallbackColor(DWORD dwColor);

		void SetAttribute(LPCTSTR pstrName, LPCTSTR pstrValue);
		void SetPos(RECT rc, bool bNeedInvalidate = true);
		SIZE EstimateSize(SIZE szAvailable);
		void PaintBackgroundColor(IRenderContext& ctx);
		void PaintStatusImage(IRenderContext& ctx);
		void PaintText(IRenderContext& ctx);

	protected:
		void SyncCircleRadius();
		CDuiString MakeInitials() const;

	protected:
		bool m_bCircle;
		CDuiString m_sAlt;
		DWORD m_dwFallbackBk;
		DWORD m_dwFallbackColor;
	};
}

#endif // __UIIMAGE_H__
