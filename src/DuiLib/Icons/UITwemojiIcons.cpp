#include "StdAfx.h"
#include "UITwemojiIcons.h"
#include "TwemojiIconsData.h"

namespace DuiLib
{
	const char* TwemojiIcons::GetIcon(LPCTSTR name)
	{
		if( name == NULL || *name == _T('\0') ) return NULL;
		for( int i = 0; i < g_twemojiIconCount; ++i ) {
			if( ::_tcscmp(name, g_twemojiIcons[i].name) == 0 )
				return g_twemojiIcons[i].data;
		}
		return NULL;
	}
}
