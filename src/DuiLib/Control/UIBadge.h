#ifndef __UIBADGE_H__
#define __UIBADGE_H__

#pragma once

namespace DuiLib
{
	/// 标签芯片：圆角底 + 文案；可选关闭钮。
	class UILIB_API CTagUI : public CLabelUI
	{
		DECLARE_DUICONTROL(CTagUI)
	public:
		enum Status { StatusDefault = 0, StatusSuccess, StatusProcessing, StatusError, StatusWarning };

		CTagUI();

		LPCTSTR GetClass() const;
		LPVOID GetInterface(LPCTSTR pstrName);
		UINT GetControlFlags() const;

		void SetClosable(bool b);
		bool IsClosable() const;
		void SetStatus(Status e);
		Status GetStatus() const;
		void ApplyStatusColors();

		void SetAttribute(LPCTSTR pstrName, LPCTSTR pstrValue);
		SIZE EstimateSize(SIZE szAvailable);
		void DoEvent(TEventUI& event);
		void PaintBackgroundColor(IRenderContext& ctx);
		void PaintText(IRenderContext& ctx);
		void PaintBorder(IRenderContext& ctx);

	protected:
		int ScaleValue(int v) const;
		RECT GetCloseRect() const;
		int HitClose(POINT pt) const;

	protected:
		bool m_bClosable;
		bool m_bCloseHover;
		Status m_eStatus;
		int m_nCloseSize;
	};

	/// 角标：数字 / 小红点；可包裹子控件画在右上角，无子控件时独立显示。
	class UILIB_API CBadgeUI : public CContainerUI
	{
		DECLARE_DUICONTROL(CBadgeUI)
	public:
		CBadgeUI();

		LPCTSTR GetClass() const;
		LPVOID GetInterface(LPCTSTR pstrName);
		UINT GetControlFlags() const;

		void SetCount(int n);
		int GetCount() const;
		void SetOverflowCount(int n);
		int GetOverflowCount() const;
		void SetShowZero(bool b);
		bool IsShowZero() const;
		void SetDot(bool b);
		bool IsDot() const;
		void SetOffset(SIZE sz);
		SIZE GetOffset() const;
		void SetBadgeColor(DWORD dw);
		DWORD GetBadgeColor() const;
		void SetBadgeTextColor(DWORD dw);

		void SetAttribute(LPCTSTR pstrName, LPCTSTR pstrValue);
		void SetPos(RECT rc, bool bNeedInvalidate = true);
		SIZE EstimateSize(SIZE szAvailable);
		bool DoPaint(IRenderContext& ctx, const RECT& rcPaint, CControlUI* pStopControl);

	protected:
		int ScaleValue(int v) const;
		bool ShouldShow() const;
		CDuiString FormatCount() const;
		SIZE MeasureBadgeSize() const;
		void ApplyBadgeInsetPadding();
		RECT CalcBadgeRect(const RECT& rcHost) const;
		void PaintBadge(IRenderContext& ctx, const RECT& rcHost);

	protected:
		int m_nCount;
		int m_nOverflow;
		bool m_bShowZero;
		bool m_bDot;
		bool m_bHang; // true：半悬在宿主角上（靠内边距留出空间，避免被裁）
		SIZE m_szOffset;
		DWORD m_dwBadgeColor;
		DWORD m_dwBadgeTextColor;
		int m_nDotSize;
		int m_nHeight;
	};
}

#endif // __UIBADGE_H__
