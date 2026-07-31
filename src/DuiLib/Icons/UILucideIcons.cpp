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
}
