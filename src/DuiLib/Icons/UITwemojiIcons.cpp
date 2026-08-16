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

	int TwemojiIcons::GetIconCount() { return g_twemojiIconCount; }
	const wchar_t* TwemojiIcons::GetNameByIndex(int iIndex)
	{ return (iIndex >= 0 && iIndex < g_twemojiIconCount) ? g_twemojiIcons[iIndex].name : NULL; }
	const char* TwemojiIcons::GetDataByIndex(int iIndex)
		{ return (iIndex >= 0 && iIndex < g_twemojiIconCount) ? g_twemojiIcons[iIndex].data : NULL; }

	int TwemojiIcons::GetIndexByName(LPCTSTR name)
	{
		if( name == NULL || *name == _T('\0') ) return -1;
		for( int i = 0; i < g_twemojiIconCount; ++i ) {
			if( ::_tcscmp(name, g_twemojiIcons[i].name) == 0 )
				return i;
		}
		return -1;
	}
}
