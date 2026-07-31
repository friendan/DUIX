#ifndef __UIFLOWLAYOUT_H__
#define __UIFLOWLAYOUT_H__

#pragma once

namespace DuiLib
{
	// 流式布局：子项从左到右排列，自动换行后自上而下堆叠。
	// childalign / align：每行内容左/中/右对齐；inset/padding：内边距；margin：外边距（控件 padding）。
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
