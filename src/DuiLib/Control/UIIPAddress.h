#ifndef __UIIPADDRESS_H__
#define __UIIPADDRESS_H__

#pragma once

//给该控件添加一个属性dtstyle

namespace DuiLib
{
	class CIPAddressWnd;

	/// IP 地址控件（聚焦时创建 SysIPAddress32）
	class UILIB_API CIPAddressUI : public CLabelUI
	{
		DECLARE_DUICONTROL(CIPAddressUI)

		friend class CIPAddressWnd;
	public:
		CIPAddressUI();
		LPCTSTR GetClass() const;
		LPVOID GetInterface(LPCTSTR pstrName);

		DWORD GetIP();
		void SetIP(DWORD dwIP);

		void SetReadOnly(bool bReadOnly);
		bool IsReadOnly() const;

		void SetNativeBackgroundColor(DWORD dwBackgroundColor);
		DWORD GetNativeBackgroundColor() const;
		void SetNativeColor(DWORD dwColor);
		DWORD GetNativeColor() const;
		/// 主题热切时：已打开的原生 HWND 重刷底/字色
		void SyncNativeShellColors();

		void UpdateText();

		void DoEvent(TEventUI& event);

		void SetAttribute(LPCTSTR pstrName, LPCTSTR pstrValue);

	protected:
		DWORD	m_dwIP;
		bool       m_bReadOnly;
		int		m_nIPUpdateFlag;
		DWORD	m_dwNativeBkColor;
		DWORD	m_dwNativeTextColor;
		bool	m_bNativeTextColorSet;

		CIPAddressWnd* m_pWindow;
	};
}
#endif // __UIIPADDRESS_H__
