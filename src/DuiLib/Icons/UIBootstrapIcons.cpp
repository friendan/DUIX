#include "StdAfx.h"
#include "UIBootstrapIcons.h"
#include "BootstrapIconsData.h"

namespace DuiLib
{
	const char* BootstrapIcons::GetIcon(LPCTSTR name)
	{
		if( name == NULL || *name == _T('\0') ) return NULL;
		for( int i = 0; i < g_bootstrapIconCount; ++i ) {
			if( ::_tcscmp(name, g_bootstrapIcons[i].name) == 0 )
				return g_bootstrapIcons[i].data;
		}
		return NULL;
	}

	int BootstrapIcons::GetIconCount()
	{
		return g_bootstrapIconCount;
	}

	const wchar_t* BootstrapIcons::GetNameByIndex(int iIndex)
	{
		if( iIndex < 0 || iIndex >= g_bootstrapIconCount ) return NULL;
		return g_bootstrapIcons[iIndex].name;
	}

	const char* BootstrapIcons::GetDataByIndex(int iIndex)
	{
		if( iIndex < 0 || iIndex >= g_bootstrapIconCount ) return NULL;
		return g_bootstrapIcons[iIndex].data;
	}

	int BootstrapIcons::GetIndexByName(LPCTSTR name)
	{
		if( name == NULL || *name == _T('\0') ) return -1;
		for( int i = 0; i < g_bootstrapIconCount; ++i ) {
			if( ::_tcscmp(name, g_bootstrapIcons[i].name) == 0 )
				return i;
		}
		return -1;
	}
}
