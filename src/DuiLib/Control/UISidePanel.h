#ifndef __UISIDEPANEL_H__
#define __UISIDEPANEL_H__

#pragma once

#include "UIAnimation.h"

namespace DuiLib
{
	class CLabelUI;
	class CButtonUI;
	class CVerticalLayoutUI;
	class CHorizontalLayoutUI;

	/// 侧滑抽屉：树内 absolute 铺满父区；遮罩 + 左/右/上/下面板；CUIAnimation 滑入。
	class UILIB_API CSidePanelUI : public CContainerUI, public CUIAnimation
	{
		DECLARE_DUICONTROL(CSidePanelUI)
	public:
		enum Placement
		{
			PlacementLeft = 0,
			PlacementRight = 1,
			PlacementTop = 2,
			PlacementBottom = 3,
		};

		CSidePanelUI();
		~CSidePanelUI();

		LPCTSTR GetClass() const override;
		LPVOID GetInterface(LPCTSTR pstrName) override;
		UINT GetControlFlags() const override;
		bool PreferClientHit() const override;

		void Show(bool bAnimate = true);
		void Hide(bool bAnimate = true);
		void Toggle(bool bAnimate = true);
		bool IsOpen() const;

		void SetPlacement(Placement e);
		Placement GetPlacement() const;
		void SetPanelWidth(int nWidth);
		int GetPanelWidth() const;
		void SetPanelWidthPercent(float fPercent);
		float GetPanelWidthPercent() const;
		void SetPanelHeight(int nHeight);
		int GetPanelHeight() const;
		void SetPanelHeightPercent(float fPercent);
		float GetPanelHeightPercent() const;
		void SetDuration(int nMs);
		int GetDuration() const;
		void SetMaskEnabled(bool b);
		bool IsMaskEnabled() const;
		void SetMaskColor(DWORD dwColor);
		DWORD GetMaskColor() const;
		void SetClickMaskClose(bool b);
		bool IsClickMaskClose() const;
		void SetEscClose(bool b);
		bool IsEscClose() const;
		void SetClosable(bool b);
		bool IsClosable() const;
		void SetTitle(LPCTSTR pstrTitle);
		LPCTSTR GetTitle() const;
		/// 标题栏 action（默认 title 可拖主窗；none 取消拖动）
		void SetHeaderAction(UIAction action);
		UIAction GetHeaderAction() const { return m_eHeaderAction; }

		/// 铺满宿主：面板厚=宿主区 100%，默认关遮罩；标题栏拖宿主；边缘可缩宿主（见 host-resize）
		void SetFillHost(bool bFill);
		bool IsFillHost() const { return m_bFillHost; }
		/// 铺满时是否用面板边缘缩放宿主窗（默认随 fill-host 开启）
		void SetHostResize(bool bResize);
		bool IsHostResize() const { return m_bHostResize; }
		/// 客户区坐标命中宿主缩放边（非 fill-host / 未开 / 未打开 → HTCLIENT）
		LRESULT HitHostResize(POINT ptClient) const;

		void ApplyThemeChrome(DWORD dwPanelBg, DWORD dwBorder, DWORD dwTitleColor);

		bool Add(CControlUI* pControl) override;
		bool AddAt(CControlUI* pControl, int iIndex) override;
		bool Remove(CControlUI* pControl) override;
		void RemoveAll() override;
		void SetPos(RECT rc, bool bNeedInvalidate = true) override;
		void DoCaptureEvent(TEventUI& event) override;
		void DoEvent(TEventUI& event) override;
		void DoInit() override;
		void SetAttribute(LPCTSTR pstrName, LPCTSTR pstrValue) override;

		void OnAnimationStep(int nTotalFrame, int nCurFrame, int nAnimationID) override;
		void OnAnimationStop(int nAnimationID) override;

	protected:
		enum
		{
			ANIM_OPEN = 1,
			ANIM_CLOSE = 2,
			ANIM_ELAPSE = 10,
		};

		void EnsureChrome();
		void SyncHeader();
		void ApplyHeaderAction();
		void SyncFillHostChrome();
		void LayoutChrome();
		RECT CalcPanelRect(float fProgress) const;
		int ResolvePanelThickness(const RECT& rcHost) const;
		int AnimFrameCount() const;
		void ApplyMaskAlpha(float fProgress);
		void ApplyCloseButtonChrome(DWORD dwTitleColor, DWORD dwPanelBg);
		void FocusInside();
		void RestoreFocus();
		static CControlUI* FindFirstTabStop(CControlUI* pRoot);
		static bool ParseSizeValue(LPCTSTR pstrValue, int& nPx, float& fPercent);
		static UIAction ParseHeaderAction(LPCTSTR pstrValue);
		bool OnCloseClick(void* param);
		bool OnMaskClick(void* param);

		Placement m_ePlacement;
		int m_nPanelWidth;
		int m_nPanelHeight;
		float m_fPanelWidthPercent;
		float m_fPanelHeightPercent;
		int m_nDuration;
		bool m_bMask;
		DWORD m_dwMaskColor;
		bool m_bClickMaskClose;
		bool m_bEscClose;
		bool m_bClosable;
		bool m_bOpen;
		bool m_bAnimating;
		bool m_bChromeReady;
		bool m_bFillHost;
		bool m_bHostResize;
		UIAction m_eHeaderAction;
		CDuiString m_sTitle;
		CControlUI* m_pRestoreFocus;

		CControlUI* m_pMask;
		CVerticalLayoutUI* m_pPanel;
		CHorizontalLayoutUI* m_pHeader;
		CLabelUI* m_pTitleLabel;
		CButtonUI* m_pCloseBtn;
		CVerticalLayoutUI* m_pBody;
	};
}

#endif // __UISIDEPANEL_H__
