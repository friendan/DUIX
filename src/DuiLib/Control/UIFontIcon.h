#ifndef __UIFONTICON_H__
#define __UIFONTICON_H__

#pragma once

namespace DuiLib
{
	/// 文字/字体图标：圆形或圆角矩形底 + 居中原文（不缩写、不加载图）。
	/// text 可空：仅绘制背景色块。默认跟主题 color-primary / color-primary-text；可 kind 或自定义色。
	/// 默认支持悬停；clickable 时发 click 通知，光标默认手型。
	class UILIB_API CFontIconUI : public CLabelUI
	{
		DECLARE_DUICONTROL(CFontIconUI)
	public:
		enum Shape
		{
			ShapeCircle = 0,
			ShapeRounded,
		};

		CFontIconUI();

		LPCTSTR GetClass() const;
		LPVOID GetInterface(LPCTSTR pstrName);
		bool PreferClientHit() const;
		void DoEvent(TEventUI& event);
		UINT GetControlFlags() const;

		void SetShape(Shape eShape);
		Shape GetShape() const;
		void SetSizePreset(int nSize);
		void SetKind(ControlKind kind) override;
		void SetClickable(bool bClickable);

		void SetAttribute(LPCTSTR pstrName, LPCTSTR pstrValue);
		void SetPos(RECT rc, bool bNeedInvalidate = true);
		SIZE EstimateSize(SIZE szAvailable);
		void PaintBackgroundColor(IRenderContext& ctx);
		void PaintText(IRenderContext& ctx);

	protected:
		void SyncShapeRadius();
		DWORD ResolveBackgroundColor() const;
		DWORD ResolveTextColor() const;

	protected:
		Shape m_eShape;
		bool m_bBkCustom;
		bool m_bColorCustom;
		bool m_bRadiusCustom;
	};
}

#endif // __UIFONTICON_H__
