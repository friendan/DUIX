#ifndef __UICOMBOBOX_H__
#define __UICOMBOBOX_H__

#pragma once

namespace DuiLib
{
	/// 扩展下拉列表框
	/// 增加 arrow-image 属性,一张图片平均分成5份,Normal/Hot/Pushed/Focused/Disabled(必须有source属性)
	/// <Default name="ComboBox" value="arrow-image=&quot;file='sys_combo_btn.png' source='0,0,16,16'&quot; "/>
	class UILIB_API CComboBoxUI : public CComboUI
	{
		DECLARE_DUICONTROL(CComboBoxUI)
	public:
		CComboBoxUI();
		LPCTSTR GetClass() const;

		void SetAttribute(LPCTSTR pstrName, LPCTSTR pstrValue);

		void PaintText(IRenderContext& ctx);
		void PaintStatusImage(IRenderContext& ctx);

	protected:
		CDuiString m_sArrowImage;
		int        m_nArrowWidth;
	};
}

#endif // __UICOMBOBOX_H__
