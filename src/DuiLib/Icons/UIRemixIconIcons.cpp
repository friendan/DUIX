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
}
