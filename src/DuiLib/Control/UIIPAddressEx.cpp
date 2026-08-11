#include "StdAfx.h"
#include <Shlwapi.h>
#pragma comment(lib, "ws2_32.lib")
#pragma comment(lib, "shlwapi.lib")

namespace DuiLib
{
	IMPLEMENT_DUICONTROL(CIPAddressExUI)

    CIPAddressExUI::CIPAddressExUI()
    {
        m_nActiveSection = 0;
        SetReadOnly(true);

        m_nFirst = 0;
        m_nSecond = 0;
        m_nThird = 0;
        m_nFourth = 0;

        UpdateText();
    }

    LPCTSTR CIPAddressExUI::GetClass() const
    {
        return _T("IPAddressExUI");
    }

    LPVOID CIPAddressExUI::GetInterface(LPCTSTR pstrName)
    {
        if( _tcscmp(pstrName, DUI_CTR_IPADDRESS) == 0 )
        {
            return static_cast<CIPAddressExUI*>(this);
        }

        return CEditUI::GetInterface(pstrName);
    }

    UINT CIPAddressExUI::GetControlFlags() const
    {
        if( !IsEnabled() )
        {
            return CControlUI::GetControlFlags();
        }

        return UIFLAG_SETCURSOR | UIFLAG_TABSTOP;
    }

    void CIPAddressExUI::GetNumInput(TCHAR chKey)
    {
        if (chKey == 0x30 || chKey == VK_NUMPAD0) {m_chNum = '0';}
        else if (chKey == 0x31 || chKey == VK_NUMPAD1) {m_chNum = '1';}
        else if (chKey == 0x32 || chKey == VK_NUMPAD2) {m_chNum = '2';}
        else if (chKey == 0x33 || chKey == VK_NUMPAD3) {m_chNum = '3';}
        else if (chKey == 0x34 || chKey == VK_NUMPAD4) {m_chNum = '4';}
        else if (chKey == 0x35 || chKey == VK_NUMPAD5) {m_chNum = '5';}
        else if (chKey == 0x36 || chKey == VK_NUMPAD6) {m_chNum = '6';}
        else if (chKey == 0x37 || chKey == VK_NUMPAD7) {m_chNum = '7';}
        else if (chKey == 0x38 || chKey == VK_NUMPAD8) {m_chNum = '8';}
        else if (chKey == 0x39 || chKey == VK_NUMPAD9) {m_chNum = '9';}

        m_strNum += m_chNum;
        CharToInt();
        if ((m_strNum.GetLength() == 3) && (m_nActiveSection < 4))
        {
            m_nActiveSection++;
            m_strNum.Empty();
        }
    }

    void CIPAddressExUI::CharToInt()
    {
        TCHAR szNum[MAX_PATH] = {0};
        lstrcpyn(szNum, m_strNum.GetData(), MAX_PATH);

        int nSection = _ttoi(szNum);
        if (nSection <= 0)
        {
            nSection = 0;
        }
        else if (nSection > 255)
        {
            nSection = 255;
        }

        switch (m_nActiveSection)
        {
        case 1:
            m_nFirst = nSection;
            break;
        case 2:
            m_nSecond = nSection;
            break;
        case 3:
            m_nThird = nSection;
            break;
        case 4:
            m_nFourth = nSection;
            break;
        default:
            break;
        }
        UpdateText();
    }

    void CIPAddressExUI::DoEvent(TEventUI& event)
    {
        if( event.Type == UIEVENT_KILLFOCUS && IsEnabled() ) 
        {
            m_nActiveSection = 0;
            Invalidate();
        }
        if( event.Type == UIEVENT_BUTTONDOWN || event.Type == UIEVENT_DBLCLICK || event.Type == UIEVENT_RBUTTONDOWN)
        {
            if( !IsEnabled() )
            {
                return;
            }
            m_strNum.Empty();

            POINT p = event.ptMouse;
            RECT r = GetPos();
            RECT rcPad = GetPadding();
            RECT rcTextPad = GetTextPadding();
            r.left += rcPad.left + rcTextPad.left;
            r.right -= rcPad.right + rcTextPad.right;
            int nW = r.right - r.left;
            if( nW < 1 ) nW = 1;
            int nFocus = nW / 4;
            int nRel = p.x - r.left;
            if( nRel < 0 ) nRel = 0;
            if( nRel <= nFocus )
                m_nActiveSection = 1;
            else if( nRel <= nFocus * 2 )
                m_nActiveSection = 2;
            else if( nRel <= nFocus * 3 )
                m_nActiveSection = 3;
            else
                m_nActiveSection = 4;

            UpdateText();
        }
        else if( event.Type == UIEVENT_SCROLLWHEEL )
        {
            if( !IsEnabled() )
            {
                return;
            }

            if( event.wParam )
            {
                DecNum();
            }
            else
            {
                IncNum();
            }
        }
        else if( event.Type == UIEVENT_KEYDOWN )
        {
            if( !IsEnabled() )
            {
                return;
            }
            // 删㽤਀            椀昀 ⠀⠀攀瘀攀渀琀⸀挀栀䬀攀礀 㴀㴀 嘀䬀开䐀䔀䰀䔀吀䔀⤀ 簀簀 ⠀攀瘀攀渀琀⸀挀栀䬀攀礀 㴀㴀 嘀䬀开䈀䄀䌀䬀⤀⤀ഀ਀            笀ഀ਀                猀眀椀琀挀栀 ⠀洀开渀䄀挀琀椀瘀攀匀攀挀琀椀漀渀⤀ഀ਀                笀ഀ਀                    挀愀猀攀 ㄀㨀ഀ਀                        洀开渀䘀椀爀猀琀 㴀 　㬀ഀ਀                        戀爀攀愀欀㬀ഀ਀                    挀愀猀攀 ㈀㨀ഀ਀                        洀开渀匀攀挀漀渀搀 㴀 　㬀ഀ਀                        戀爀攀愀欀㬀ഀ਀                    挀愀猀攀 ㌀㨀ഀ਀                        洀开渀吀栀椀爀搀 㴀 　㬀ഀ਀                        戀爀攀愀欀㬀ഀ਀                    挀愀猀攀 㐀㨀ഀ਀                        洀开渀䘀漀甀爀琀栀 㴀 　㬀ഀ਀                        戀爀攀愀欀㬀ഀ਀                    搀攀昀愀甀氀琀㨀ഀ਀                        戀爀攀愀欀㬀ഀ਀                紀ഀ਀ഀ਀                洀开猀琀爀一甀洀⸀䔀洀瀀琀礀⠀⤀㬀ഀ਀                唀瀀搀愀琀攀吀攀砀琀⠀⤀㬀ഀ਀            紀ഀ਀ഀ਀            ⼀⼀ 격ꯨ릐入字符
            if ((m_nActiveSection == 1) && (event.chKey >= 0x30) && (event.chKey <= 0x39) ||
                (m_nActiveSection == 1) && (event.chKey >= VK_NUMPAD0) && (event.chKey <= VK_NUMPAD9))
            {
                GetNumInput(event.chKey);
            }
            else if ((m_nActiveSection == 2) && (event.chKey >= 0x30) && (event.chKey <= 0x39) ||
                (m_nActiveSection == 2) && (event.chKey >= VK_NUMPAD0) && (event.chKey <= VK_NUMPAD9))
            {
                GetNumInput(event.chKey);
            }
            else if ((m_nActiveSection == 3) && (event.chKey >= 0x30) && (event.chKey <= 0x39) ||
                (m_nActiveSection == 3) && (event.chKey >= VK_NUMPAD0) && (event.chKey <= VK_NUMPAD9))
            {
                GetNumInput(event.chKey);
            }
            else if ((m_nActiveSection == 4) && (event.chKey >= 0x30) && (event.chKey <= 0x39) ||
                (m_nActiveSection == 4) && (event.chKey >= VK_NUMPAD0) && (event.chKey <= VK_NUMPAD9))
            {
                GetNumInput(event.chKey);
            }

            if( event.chKey == VK_UP )
            {
                IncNum();
            }
            else if( event.chKey == VK_DOWN )
            {
                DecNum();
            }
            else if( event.chKey == VK_LEFT )
            {
                if( m_nActiveSection > 1 )
                {
                    if (!m_strNum.IsEmpty())
                    {
                        CharToInt();
                        m_strNum.Empty();
                    }
                    m_nActiveSection--;
                    Invalidate();
                }
            }
            else if( event.chKey == VK_RIGHT )
            {
                if( m_nActiveSection < 4 )
                {
                    if (!m_strNum.IsEmpty())
                    {
                        CharToInt();
                        m_strNum.Empty();
                    }
                    m_nActiveSection++;
                    Invalidate();
                }
            }
            else if ((event.chKey == VK_OEM_PERIOD) || (event.chKey == VK_DECIMAL))
            {
                if( m_nActiveSection < 4 )
                {
                    if (!m_strNum.IsEmpty())
                    {
                        CharToInt();
                        m_strNum.Empty();
                    }
                    m_nActiveSection++;
                    Invalidate();
                }
            }
        }

        CLabelUI::DoEvent(event);
    }

    void CIPAddressExUI::PaintText(IRenderContext& ctx)
    {
        if( m_dwColor == 0 ) m_dwColor = m_pManager->GetDefaultFontColor();
        if( m_dwDisabledColor == 0 ) m_dwDisabledColor = m_pManager->GetDefaultDisabledColor();

        if( m_sText.IsEmpty() ) return;

        RECT rc = m_rcItem;
        RECT rcPad = GetPadding();
        RECT rcTextPadding = GetTextPadding();
        rc.left += rcPad.left + rcTextPadding.left;
        rc.right -= rcPad.right + rcTextPadding.right;
        rc.top += rcPad.top + rcTextPadding.top;
        rc.bottom -= rcPad.bottom + rcTextPadding.bottom;
        if( rc.right <= rc.left || rc.bottom <= rc.top ) return;

        DWORD dwColor = IsEnabled() ? m_dwColor : m_dwDisabledColor;
        const UINT uMeasure = DT_SINGLELINE | DT_NOPREFIX;
        const UINT uDraw = DT_SINGLELINE | DT_CENTER | DT_VCENTER | DT_NOPREFIX;
        const DWORD dwSelTextColor = 0xFFFFFFFF;
        const DWORD dwSelBkColor = 0x3399FFFF;

        CDuiString sSec[4];
        sSec[0].Format(_T("%d"), m_nFirst);
        sSec[1].Format(_T("%d"), m_nSecond);
        sSec[2].Format(_T("%d"), m_nThird);
        sSec[3].Format(_T("%d"), m_nFourth);

        // 四段数字等分内容区；点号单独占宽并居中，避免贴数字 / 末段被裁切
        SIZE szDot = ctx.GetTextSize(_T("."), m_iFont, uMeasure);
        int nDotSlot = szDot.cx + 8;
        if( nDotSlot < 10 ) nDotSlot = 10;
        int nWidth = rc.right - rc.left;
        int nNumTotal = nWidth - nDotSlot * 3;
        if( nNumTotal < 4 ) {
            nDotSlot = 6;
            nNumTotal = nWidth - nDotSlot * 3;
            if( nNumTotal < 4 ) nNumTotal = 4;
        }
        int nCell = nNumTotal / 4;
        int nExtra = nNumTotal - nCell * 4;

        bool bShowNums = !(m_nFirst == 0 && m_nSecond == 0 && m_nThird == 0 && m_nFourth == 0 && m_nActiveSection == 0);

        int x = rc.left;
        for( int i = 0; i < 4; ++i ) {
            int cellW = nCell + (i < nExtra ? 1 : 0);
            RECT rcCell = { x, rc.top, x + cellW, rc.bottom };

            if( bShowNums ) {
                if( m_nActiveSection == i + 1 && IsEnabled() ) {
                    SIZE sz = ctx.GetTextSize(sSec[i].GetData(), m_iFont, uMeasure);
                    int nSelL = rcCell.left + (cellW - sz.cx) / 2;
                    if( nSelL < rcCell.left ) nSelL = rcCell.left;
                    int nSelR = nSelL + sz.cx;
                    if( nSelR > rcCell.right ) nSelR = rcCell.right;
                    RECT rcSel = { nSelL, rcCell.top, nSelR, rcCell.bottom };
                    ctx.DrawColor(rcSel, GetAdjustColor(dwSelBkColor));
                    ctx.DrawText(rcCell, sSec[i].GetData(), GetAdjustColor(dwSelTextColor), m_iFont, uDraw);
                }
                else {
                    ctx.DrawText(rcCell, sSec[i].GetData(), GetAdjustColor(dwColor), m_iFont, uDraw);
                }
            }

            x += cellW;
            if( i < 3 ) {
                RECT rcDot = { x, rc.top, x + nDotSlot, rc.bottom };
                ctx.DrawText(rcDot, _T("."), GetAdjustColor(dwColor), m_iFont, uDraw);
                x += nDotSlot;
            }
        }
    }

    void CIPAddressExUI::SetIP(LPCTSTR lpIP)
    {
        CDuiString sIP = lpIP;

        std::vector<CDuiString>vIPs = StrSplit(sIP, _T("."));
        if (vIPs.size() == 4) {
            m_nFirst = _ttoi(vIPs[0].GetData());
            m_nSecond = _ttoi(vIPs[1].GetData());
            m_nThird = _ttoi(vIPs[2].GetData());
            m_nFourth = _ttoi(vIPs[3].GetData());
        }
        UpdateText();
    }

    CDuiString CIPAddressExUI::GetIP()
    {
        CDuiString strIP;
        strIP.Format(_T("%d.%d.%d.%d"), m_nFirst, m_nSecond, m_nThird, m_nFourth);
        return strIP;
    }

    void CIPAddressExUI::UpdateText()
    {
        TCHAR szIP[MAX_PATH] = {0};
        _stprintf(szIP, _T("%d.%d.%d.%d"), m_nFirst, m_nSecond, m_nThird, m_nFourth);
        SetText(szIP);
    }

    void CIPAddressExUI::IncNum()
    {
        if( m_nActiveSection == 1 )
        {
            if (m_nFirst < 255)
            {
                m_nFirst++;
            }
        }
        else if(m_nActiveSection == 2)
        {
            if(m_nSecond < 255)
            {
                m_nSecond++;
            }
        }
        else if(m_nActiveSection == 3)
        {
            if(m_nThird < 255)
            {
                m_nThird++;
            }
        }
        else if(m_nActiveSection == 4)
        {
            if(m_nFourth < 255)
            {
                m_nFourth++;
            }
        }

        UpdateText();
    }

    void CIPAddressExUI::DecNum()
    {
        if(m_nActiveSection == 1)
        {
            if (m_nFirst > 0)
            {
                m_nFirst--;
            }
            
        }
        else if(m_nActiveSection == 2)
        {
            if(m_nSecond > 0)
            {
                m_nSecond--;
            }
        }
        else if(m_nActiveSection == 3)
        {
            if(m_nThird > 0)
            {
                m_nThird--;
            }
        }
        else if(m_nActiveSection == 4)
        {
            if(m_nFourth > 0)
            {
                m_nFourth--;
            }
        }

        UpdateText();
    }
}