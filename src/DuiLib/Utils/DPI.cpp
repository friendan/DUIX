#include "StdAfx.h"
#include "DPI.h"
#include "VersionHelpers.h"
namespace DuiLib
{
	//96 DPI = 100% scaling
	//120 DPI = 125% scaling
	//144 DPI = 150% scaling
	//168 DPI = 175% scaling
	//192 DPI = 200% scaling

#ifndef DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2
	DECLARE_HANDLE(DPI_AWARENESS_CONTEXT);
#define DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE     ((DPI_AWARENESS_CONTEXT)-3)
#define DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2  ((DPI_AWARENESS_CONTEXT)-4)
#endif

	typedef HRESULT (WINAPI *LPSetProcessDpiAwareness)(
		_In_ PROCESS_DPI_AWARENESS value
		);

	typedef HRESULT (WINAPI *LPGetProcessDpiAwareness)(
		_In_  HANDLE                hprocess,
		_Out_ PROCESS_DPI_AWARENESS *value
		);


	typedef HRESULT (WINAPI *LPGetDpiForMonitor)(
		_In_  HMONITOR         hmonitor,
		_In_  MONITOR_DPI_TYPE dpiType,
		_Out_ UINT             *dpiX,
		_Out_ UINT             *dpiY
		);

	typedef BOOL (WINAPI *LPSetProcessDpiAwarenessContext)(_In_ DPI_AWARENESS_CONTEXT value);
	typedef UINT (WINAPI *LPGetDpiForWindow)(_In_ HWND hwnd);

	static BOOL s_bDpiAwarenessTried = FALSE;
	static BOOL s_bDpiAwarenessOk = FALSE;

	CDPI::CDPI()
	{
		m_nScaleFactor = 0;
		m_nScaleFactorSDA = 0;
		// 默认按 Per-Monitor 缩放；实际进程感知由 EnableProcessDpiAwareness 在 SetInstance 时启用
		m_Awareness = PROCESS_PER_MONITOR_DPI_AWARE;
		SetScale(96);
	}

	BOOL CDPI::EnableProcessDpiAwareness()
	{
		if( s_bDpiAwarenessTried )
			return s_bDpiAwarenessOk;
		s_bDpiAwarenessTried = TRUE;

		// Win10 1703+：PerMonitorV2（跨屏清晰；标题栏/菜单等非客户区也跟 DPI）
		HMODULE hUser = ::GetModuleHandle(_T("user32.dll"));
		if( hUser != NULL ) {
			LPSetProcessDpiAwarenessContext pfnCtx =
				(LPSetProcessDpiAwarenessContext)::GetProcAddress(hUser, "SetProcessDpiAwarenessContext");
			if( pfnCtx != NULL ) {
				if( pfnCtx(DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2)
					|| pfnCtx(DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE) ) {
					s_bDpiAwarenessOk = TRUE;
					return TRUE;
				}
			}
		}

		// Win8.1+：Shcore Per-Monitor
		HMODULE hShcore = ::LoadLibrary(_T("Shcore.dll"));
		if( hShcore != NULL ) {
			LPSetProcessDpiAwareness pfn =
				(LPSetProcessDpiAwareness)::GetProcAddress(hShcore, "SetProcessDpiAwareness");
			if( pfn != NULL && pfn(PROCESS_PER_MONITOR_DPI_AWARE) == S_OK ) {
				s_bDpiAwarenessOk = TRUE;
				return TRUE;
			}
			// 清单已声明感知时 Set* 会失败；再读一次进程状态
			LPGetProcessDpiAwareness pfnGet =
				(LPGetProcessDpiAwareness)::GetProcAddress(hShcore, "GetProcessDpiAwareness");
			if( pfnGet != NULL ) {
				PROCESS_DPI_AWARENESS aw = PROCESS_DPI_UNAWARE;
				if( pfnGet(::GetCurrentProcess(), &aw) == S_OK
					&& aw == PROCESS_PER_MONITOR_DPI_AWARE ) {
					s_bDpiAwarenessOk = TRUE;
					return TRUE;
				}
			}
		}
		return FALSE;
	}

	int CDPI::GetDpiForHwnd(HWND hWnd)
	{
		if( hWnd == NULL || !::IsWindow(hWnd) )
			return GetMainMonitorDPI();

		HMODULE hUser = ::GetModuleHandle(_T("user32.dll"));
		if( hUser != NULL ) {
			LPGetDpiForWindow pfn = (LPGetDpiForWindow)::GetProcAddress(hUser, "GetDpiForWindow");
			if( pfn != NULL ) {
				UINT dpi = pfn(hWnd);
				if( dpi != 0 )
					return (int)dpi;
			}
		}
		return GetDPIOfMonitor(::MonitorFromWindow(hWnd, MONITOR_DEFAULTTONEAREST));
	}

	int CDPI::GetDPIOfMonitor(HMONITOR hMonitor)
	{
		UINT dpix = 96, dpiy = 96;
		if (IsWindows8Point1OrGreater()) {
			HMODULE hModule =::LoadLibrary(_T("Shcore.dll"));
			if(hModule != NULL) {
				LPGetDpiForMonitor GetDpiForMonitor = (LPGetDpiForMonitor)GetProcAddress(hModule, "GetDpiForMonitor");
				if (GetDpiForMonitor != NULL && GetDpiForMonitor(hMonitor,MDT_EFFECTIVE_DPI, &dpix, &dpiy) != S_OK) {
					MessageBox(NULL, _T("GetDpiForMonitor failed"), _T("Notification"), MB_OK);
					return 96;
				}
			}
		}
		else {
			HDC screen = GetDC(0);
			dpix = GetDeviceCaps(screen, LOGPIXELSX);
			ReleaseDC(0, screen);
		}
		return dpix;
	}

	int CDPI::GetDPIOfMonitorNearestToPoint(POINT pt)
	{
		HMONITOR hMonitor;
		hMonitor = MonitorFromPoint(pt, MONITOR_DEFAULTTONEAREST);
		return GetDPIOfMonitor(hMonitor);
	}

	int CDPI::GetMainMonitorDPI()
	{
		POINT    pt;
		// Get the DPI for the main monitor
		pt.x = 1;
		pt.y = 1;
		return GetDPIOfMonitorNearestToPoint(pt);
	}

	PROCESS_DPI_AWARENESS CDPI::GetDPIAwareness()
	{
		if (IsWindows8Point1OrGreater()) {
			HMODULE hModule =::LoadLibrary(_T("Shcore.dll"));
			if(hModule != NULL) {
				LPGetProcessDpiAwareness GetProcessDpiAwareness = (LPGetProcessDpiAwareness)GetProcAddress(hModule, "GetProcessDpiAwareness");
				if(GetProcessDpiAwareness != NULL) {
					HANDLE hProcess = OpenProcess(PROCESS_ALL_ACCESS, false, GetCurrentProcessId());
					if(GetProcessDpiAwareness(hProcess, &m_Awareness) == S_OK) {
					}
				}
			}
		}

		return m_Awareness;
	}

	BOOL CDPI::SetDPIAwareness(PROCESS_DPI_AWARENESS Awareness)
	{
		BOOL bRet = FALSE;
		if (IsWindows8Point1OrGreater()) {
			HMODULE hModule =::LoadLibrary(_T("Shcore.dll"));
			if(hModule != NULL) {
				LPSetProcessDpiAwareness SetProcessDpiAwareness = (LPSetProcessDpiAwareness)GetProcAddress(hModule, "SetProcessDpiAwareness");
				if (SetProcessDpiAwareness != NULL && SetProcessDpiAwareness(Awareness) == S_OK) {
					m_Awareness = Awareness;
					bRet = TRUE;
				}
			}
		}
		else {
			m_Awareness = Awareness;
			bRet = TRUE;
		}
		return bRet;
	}

	UINT DuiLib::CDPI::GetDPI()
	{
		if (m_Awareness == PROCESS_DPI_UNAWARE) {
			return 96;
		}

		if (m_Awareness == PROCESS_SYSTEM_DPI_AWARE) {
			return MulDiv(m_nScaleFactorSDA, 96, 100);
		}

		return MulDiv(m_nScaleFactor, 96, 100);
	}

	UINT CDPI::GetScale()
	{
		if (m_Awareness == PROCESS_DPI_UNAWARE) {
			return 100;
		}
		if (m_Awareness == PROCESS_SYSTEM_DPI_AWARE) {
			return m_nScaleFactorSDA;
		}
		return m_nScaleFactor;
	}


	void CDPI::SetScale(UINT uDPI)
	{
		m_nScaleFactor = MulDiv(uDPI, 100, 96);
		if (m_nScaleFactorSDA == 0) {
			m_nScaleFactorSDA = m_nScaleFactor;
		}
	}

	int  CDPI::Scale(int iValue)
	{
		int iResult = iValue;
		if (m_Awareness == PROCESS_DPI_UNAWARE) {
			return iValue;
		}
		else if (m_Awareness == PROCESS_SYSTEM_DPI_AWARE) {
			iResult = MulDiv(iValue, m_nScaleFactorSDA, 100);
		}
		else{
			iResult = MulDiv(iValue, m_nScaleFactor, 100);
		}
		if(iValue > 0 && iResult == 0) iResult = 1;
		if(iValue < 0 && iResult == 0) iResult = -1;
		return iResult;
	}

	int  CDPI::ScaleBack(int iValue) {

		if (m_Awareness == PROCESS_DPI_UNAWARE) {
			return iValue;
		}
		if (m_Awareness == PROCESS_SYSTEM_DPI_AWARE) {
			return MulDiv(iValue,  100, m_nScaleFactorSDA);
		}
		return MulDiv(iValue, 100, m_nScaleFactor);
	}

	RECT CDPI::Scale(RECT rcRect)
	{
		RECT rcScale = rcRect;
		int sw = Scale(rcRect.right - rcRect.left);
		int sh = Scale(rcRect.bottom - rcRect.top);
		rcScale.left = Scale(rcRect.left);
		rcScale.top = Scale(rcRect.top);
		rcScale.right = rcScale.left + sw;
		rcScale.bottom = rcScale.top + sh;
		return rcScale;
	}

	void CDPI::Scale(RECT *pRect)
	{
		int sw = Scale(pRect->right - pRect->left);
		int sh = Scale(pRect->bottom - pRect->top);
		pRect->left = Scale(pRect->left);
		pRect->top = Scale(pRect->top);
		pRect->right = pRect->left + sw;
		pRect->bottom = pRect->top + sh;
	}

	void CDPI::Scale(CDuiBox *pBox)
	{
		if( pBox == NULL ) return;
		pBox->top = Scale(pBox->top);
		pBox->right = Scale(pBox->right);
		pBox->bottom = Scale(pBox->bottom);
		pBox->left = Scale(pBox->left);
	}

	void CDPI::ScaleBack(RECT *pRect)
	{
		int sw = ScaleBack(pRect->right - pRect->left);
		int sh = ScaleBack(pRect->bottom - pRect->top);
		pRect->left = ScaleBack(pRect->left);
		pRect->top = ScaleBack(pRect->top);
		pRect->right = pRect->left + sw;
		pRect->bottom = pRect->top + sh;
	}

	void CDPI::Scale(POINT *pPoint)
	{
		pPoint->x = Scale(pPoint->x);
		pPoint->y = Scale(pPoint->y);
	}

	POINT CDPI::Scale(POINT ptPoint)
	{
		POINT ptScale = ptPoint;
		ptScale.x = Scale(ptPoint.x);
		ptScale.y = Scale(ptPoint.y);
		return ptScale;
	}

	void CDPI::Scale(SIZE *pSize)
	{
		pSize->cx = Scale(pSize->cx);
		pSize->cy = Scale(pSize->cy);
	}

	SIZE CDPI::Scale(SIZE szSize)
	{
		SIZE szScale = szSize;
		szScale.cx = Scale(szSize.cx);
		szScale.cy = Scale(szSize.cy);
		return szScale;
	}
}