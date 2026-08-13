#include "StdAfx.h"
#include "VersionHelpers.h"

namespace DuiLib
{
	BOOL IsWindowsVersionOrGreater(WORD wMajorVersion, WORD wMinorVersion, WORD wServicePackMajor)
	{
		OSVERSIONINFOEXW osvi = { sizeof(osvi), 0, 0, 0, 0, {0}, 0, 0 };
		DWORDLONG const dwlConditionMask = VerSetConditionMask(
			VerSetConditionMask(
				VerSetConditionMask(0, VER_MAJORVERSION, VER_GREATER_EQUAL),
				VER_MINORVERSION, VER_GREATER_EQUAL),
			VER_SERVICEPACKMAJOR, VER_GREATER_EQUAL);
		osvi.dwMajorVersion = wMajorVersion;
		osvi.dwMinorVersion = wMinorVersion;
		osvi.wServicePackMajor = wServicePackMajor;
		return VerifyVersionInfoW(&osvi, VER_MAJORVERSION | VER_MINORVERSION | VER_SERVICEPACKMAJOR, dwlConditionMask) != FALSE;
	}

	BOOL IsWindowsXPOrGreater()
	{
		return IsWindowsVersionOrGreater(HIBYTE(_WIN32_WINNT_WINXP), LOBYTE(_WIN32_WINNT_WINXP), 0);
	}

	BOOL IsWindowsXPSP1OrGreater()
	{
		return IsWindowsVersionOrGreater(HIBYTE(_WIN32_WINNT_WINXP), LOBYTE(_WIN32_WINNT_WINXP), 1);
	}

	BOOL IsWindowsXPSP2OrGreater()
	{
		return IsWindowsVersionOrGreater(HIBYTE(_WIN32_WINNT_WINXP), LOBYTE(_WIN32_WINNT_WINXP), 2);
	}

	BOOL IsWindowsXPSP3OrGreater()
	{
		return IsWindowsVersionOrGreater(HIBYTE(_WIN32_WINNT_WINXP), LOBYTE(_WIN32_WINNT_WINXP), 3);
	}

	BOOL IsWindowsVistaOrGreater()
	{
		return IsWindowsVersionOrGreater(HIBYTE(_WIN32_WINNT_VISTA), LOBYTE(_WIN32_WINNT_VISTA), 0);
	}

	BOOL IsWindowsVistaSP1OrGreater()
	{
		return IsWindowsVersionOrGreater(HIBYTE(_WIN32_WINNT_VISTA), LOBYTE(_WIN32_WINNT_VISTA), 1);
	}

	BOOL IsWindowsVistaSP2OrGreater()
	{
		return IsWindowsVersionOrGreater(HIBYTE(_WIN32_WINNT_VISTA), LOBYTE(_WIN32_WINNT_VISTA), 2);
	}

	BOOL IsWindows7OrGreater()
	{
		return IsWindowsVersionOrGreater(HIBYTE(_WIN32_WINNT_WIN7), LOBYTE(_WIN32_WINNT_WIN7), 0);
	}

	BOOL IsWindows7SP1OrGreater()
	{
		return IsWindowsVersionOrGreater(HIBYTE(_WIN32_WINNT_WIN7), LOBYTE(_WIN32_WINNT_WIN7), 1);
	}

	BOOL IsWindows8OrGreater()
	{
		return IsWindowsVersionOrGreater(HIBYTE(_WIN32_WINNT_WIN8), LOBYTE(_WIN32_WINNT_WIN8), 0);
	}

	BOOL IsWindows8Point1OrGreater()
	{
		return IsWindowsVersionOrGreater(HIBYTE(_WIN32_WINNT_WINBLUE), LOBYTE(_WIN32_WINNT_WINBLUE), 0);
	}

	BOOL IsWindowsThresholdOrGreater()
	{
		return IsWindowsVersionOrGreater(HIBYTE(_WIN32_WINNT_WINTHRESHOLD), LOBYTE(_WIN32_WINNT_WINTHRESHOLD), 0);
	}

	BOOL IsWindows10OrGreater()
	{
		return IsWindowsVersionOrGreater(HIBYTE(_WIN32_WINNT_WINTHRESHOLD), LOBYTE(_WIN32_WINNT_WINTHRESHOLD), 0);
	}

	BOOL IsWindowsServer()
	{
		OSVERSIONINFOEXW osvi = { sizeof(osvi), 0, 0, 0, 0, {0}, 0, 0, 0, VER_NT_WORKSTATION };
		DWORDLONG const dwlConditionMask = VerSetConditionMask(0, VER_PRODUCT_TYPE, VER_EQUAL);
		return !VerifyVersionInfoW(&osvi, VER_PRODUCT_TYPE, dwlConditionMask);
	}
}
