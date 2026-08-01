#ifndef __UIFLOWLAYOUT_H__
#define __UIFLOWLAYOUT_H__

#pragma once

namespace DuiLib
{
	// 流式布局：子项从左到右排列，自动换行后自上而下堆叠。
	// EstimateSize 宽/高为 0 表示撑满（与 LinearLayout 一致）：宽→占满行宽；高→参与分配剩余行高。
	// margin / padding / inset：见 CControlUI（CSS：margin 外、padding/inset 内）。
	// childalign / align：每行内容左/中/右对齐。
	class UILIB_API CFlowLayoutUI : public CContainerUI
	{
		DECLARE_DUICONTROL(CFlowLayoutUI)
	public:
		CFlowLayoutUI();

		LPCTSTR GetClass() const;
		LPVOID GetInterface(LPCTSTR pstrName);

		void SetPos(RECT rc, bool bNeedInvalidate = true);
		SIZE EstimateSize(SIZE szAvailable);

		bool IsAutoWrap() const;
		void SetAutoWrap(bool bWrap);
		int GetLineSpacing() const;
		void SetLineSpacing(int iSpacing);

		void SetAttribute(LPCTSTR pstrName, LPCTSTR pstrValue);

	protected:
		bool m_bAutoWrap;
		int m_iLineSpacing;
	};
}
#endif // __UIFLOWLAYOUT_H__
