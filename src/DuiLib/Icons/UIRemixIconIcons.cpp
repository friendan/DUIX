#include "StdAfx.h"
#include "UIRemixIconIcons.h"
#include "RemixIconIconsData.h"

namespace DuiLib
{
	const char* RemixIconIcons::GetIcon(LPCTSTR name)
	{
		if( name == NULL || *name == _T('\0') ) return NULL;
		for( int i = 0; i < g_remixIconIconCount; ++i ) {
			if( ::_tcscmp(name, g_remixIconIcons[i].name) == 0 )
				return g_remixIconIcons[i].data;
		}
		return NULL;
	}

	int RemixIconIcons::GetIconCount() { return g_remixIconIconCount; }
	const wchar_t* RemixIconIcons::GetNameByIndex(int iIndex)
	{ return (iIndex >= 0 && iIndex < g_remixIconIconCount) ? g_remixIconIcons[iIndex].name : NULL; }
	const char* RemixIconIcons::GetDataByIndex(int iIndex)
		{ return (iIndex >= 0 && iIndex < g_remixIconIconCount) ? g_remixIconIcons[iIndex].data : NULL; }

	int RemixIconIcons::GetIndexByName(LPCTSTR name)
	{
		if( name == NULL || *name == _T('\0') ) return -1;
		for( int i = 0; i < g_remixIconIconCount; ++i ) {
			if( ::_tcscmp(name, g_remixIconIcons[i].name) == 0 )
				return i;
		}
		return -1;
	}
}
