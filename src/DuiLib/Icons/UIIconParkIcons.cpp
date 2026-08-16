#include "StdAfx.h"
#include "UIIconParkIcons.h"
#include "IconParkIconsData.h"

namespace DuiLib
{
	const char* IconParkIcons::GetIcon(LPCTSTR name)
	{
		if( name == NULL || *name == _T('\0') ) return NULL;
		for( int i = 0; i < g_iconParkIconCount; ++i ) {
			if( ::_tcscmp(name, g_iconParkIcons[i].name) == 0 )
				return g_iconParkIcons[i].data;
		}
		return NULL;
	}

	int IconParkIcons::GetIconCount() { return g_iconParkIconCount; }
	const wchar_t* IconParkIcons::GetNameByIndex(int iIndex)
	{ return (iIndex >= 0 && iIndex < g_iconParkIconCount) ? g_iconParkIcons[iIndex].name : NULL; }
	const char* IconParkIcons::GetDataByIndex(int iIndex)
		{ return (iIndex >= 0 && iIndex < g_iconParkIconCount) ? g_iconParkIcons[iIndex].data : NULL; }

	int IconParkIcons::GetIndexByName(LPCTSTR name)
	{
		if( name == NULL || *name == _T('\0') ) return -1;
		for( int i = 0; i < g_iconParkIconCount; ++i ) {
			if( ::_tcscmp(name, g_iconParkIcons[i].name) == 0 )
				return i;
		}
		return -1;
	}
}
