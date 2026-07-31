#include "StdAfx.h"
#include "UITablerIcons.h"
#include "TablerOutlineIconsData.h"
#include "TablerFilledIconsData.h"

namespace DuiLib
{
	const char* TablerOutlineIcons::GetIcon(LPCTSTR name)
	{
		if( name == NULL || *name == _T('\0') ) return NULL;
		for( int i = 0; i < g_tablerOutlineIconCount; ++i ) {
			if( ::_tcscmp(name, g_tablerOutlineIcons[i].name) == 0 )
				return g_tablerOutlineIcons[i].data;
		}
		return NULL;
	}

	const char* TablerFilledIcons::GetIcon(LPCTSTR name)
	{
		if( name == NULL || *name == _T('\0') ) return NULL;
		for( int i = 0; i < g_tablerFilledIconCount; ++i ) {
			if( ::_tcscmp(name, g_tablerFilledIcons[i].name) == 0 )
				return g_tablerFilledIcons[i].data;
		}
		return NULL;
	}
}
