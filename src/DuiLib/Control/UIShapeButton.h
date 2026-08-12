#ifndef __UISHAPEBUTTON_H__
#define __UISHAPEBUTTON_H__

#pragma once

namespace DuiLib
{
	/// 异形按钮：shape-mask（或 shape-image）决定命中；绘制可用 image / image-hover 等。
	class UILIB_API CShapeButtonUI : public CButtonUI
	{
		DECLARE_DUICONTROL(CShapeButtonUI)
	public:
		CShapeButtonUI();
		~CShapeButtonUI();

		LPCTSTR GetClass() const;
		LPVOID GetInterface(LPCTSTR pstrName);

		void SetShapeImage(LPCTSTR pstrImage);
		LPCTSTR GetShapeImage() const;
		void SetShapeMask(LPCTSTR pstrMask);
		LPCTSTR GetShapeMask() const;
		LPCTSTR GetShapeHitImage() const;
		void SetShapeAlphaThreshold(BYTE nThreshold);
		BYTE GetShapeAlphaThreshold() const;

		void SetAttribute(LPCTSTR pstrName, LPCTSTR pstrValue);
		CControlUI* FindControl(FINDCONTROLPROC Proc, LPVOID pData, UINT uFlags);
		UINT GetControlFlags() const;

	protected:
		void InvalidateShapeMask();
		bool EnsureShapeMask();
		bool HitTestShape(POINT pt) const;

		CDuiString m_sShapeImage;
		CDuiString m_sShapeMask;
		BYTE m_nShapeAlphaThreshold;
		BYTE* m_pShapeBits;
		int m_nShapeW;
		int m_nShapeH;
		int m_nShapeStride;
		CDuiString m_sShapeLoaded;
	};

	/// 异形容器：外形外不命中；可画 shape-image，命中可用 shape-mask。
	class UILIB_API CShapeBoxUI : public CContainerUI
	{
		DECLARE_DUICONTROL(CShapeBoxUI)
	public:
		CShapeBoxUI();
		~CShapeBoxUI();

		LPCTSTR GetClass() const;
		LPVOID GetInterface(LPCTSTR pstrName);

		void SetShapeImage(LPCTSTR pstrImage);
		LPCTSTR GetShapeImage() const;
		void SetShapeMask(LPCTSTR pstrMask);
		LPCTSTR GetShapeMask() const;
		LPCTSTR GetShapeHitImage() const;
		void SetShapeAlphaThreshold(BYTE nThreshold);
		BYTE GetShapeAlphaThreshold() const;

		void SetAttribute(LPCTSTR pstrName, LPCTSTR pstrValue);
		CControlUI* FindControl(FINDCONTROLPROC Proc, LPVOID pData, UINT uFlags);
		void PaintStatusImage(IRenderContext& ctx);

	protected:
		void InvalidateShapeMask();
		bool EnsureShapeMask();
		bool HitTestShape(POINT pt) const;

		CDuiString m_sShapeImage;
		CDuiString m_sShapeMask;
		BYTE m_nShapeAlphaThreshold;
		BYTE* m_pShapeBits;
		int m_nShapeW;
		int m_nShapeH;
		int m_nShapeStride;
		CDuiString m_sShapeLoaded;
	};
}

#endif // __UISHAPEBUTTON_H__
