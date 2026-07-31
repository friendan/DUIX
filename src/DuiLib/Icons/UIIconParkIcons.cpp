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
}
