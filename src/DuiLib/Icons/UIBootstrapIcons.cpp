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
}
