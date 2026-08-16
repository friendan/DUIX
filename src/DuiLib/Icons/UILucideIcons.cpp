#include "StdAfx.h"
#include "UILucideIcons.h"
#include "LucideIconsIconsData.h"

namespace DuiLib
{
	const char* LucideIcons::GetIcon(LPCTSTR name)
	{
		if( name == NULL || *name == _T('\0') ) return NULL;
		for( int i = 0; i < g_lucideIconsIconCount; ++i ) {
			if( ::_tcscmp(name, g_lucideIconsIcons[i].name) == 0 )
				return g_lucideIconsIcons[i].data;
		}
		return NULL;
	}

	int LucideIcons::GetIconCount() { return g_lucideIconsIconCount; }
	const wchar_t* LucideIcons::GetNameByIndex(int iIndex)
	{ return (iIndex >= 0 && iIndex < g_lucideIconsIconCount) ? g_lucideIconsIcons[iIndex].name : NULL; }
	const char* LucideIcons::GetDataByIndex(int iIndex)
		{ return (iIndex >= 0 && iIndex < g_lucideIconsIconCount) ? g_lucideIconsIcons[iIndex].data : NULL; }

	int LucideIcons::GetIndexByName(LPCTSTR name)
	{
		if( name == NULL || *name == _T('\0') ) return -1;
		for( int i = 0; i < g_lucideIconsIconCount; ++i ) {
			if( ::_tcscmp(name, g_lucideIconsIcons[i].name) == 0 )
				return i;
		}
		return -1;
	}
}
