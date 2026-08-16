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

	const char* TablerOutlineIcons::GetDataByIndex(int iIndex)
		{ return (iIndex >= 0 && iIndex < g_tablerOutlineIconCount) ? g_tablerOutlineIcons[iIndex].data : NULL; }

	int TablerOutlineIcons::GetIconCount() { return g_tablerOutlineIconCount; }
	const wchar_t* TablerOutlineIcons::GetNameByIndex(int iIndex)
		{ return (iIndex >= 0 && iIndex < g_tablerOutlineIconCount) ? g_tablerOutlineIcons[iIndex].name : NULL; }

	int TablerOutlineIcons::GetIndexByName(LPCTSTR name)
	{
		if( name == NULL || *name == _T('\0') ) return -1;
		for( int i = 0; i < g_tablerOutlineIconCount; ++i ) {
			if( ::_tcscmp(name, g_tablerOutlineIcons[i].name) == 0 )
				return i;
		}
		return -1;
	}

	int TablerFilledIcons::GetIconCount() { return g_tablerFilledIconCount; }
	const wchar_t* TablerFilledIcons::GetNameByIndex(int iIndex)
		{ return (iIndex >= 0 && iIndex < g_tablerFilledIconCount) ? g_tablerFilledIcons[iIndex].name : NULL; }
	const char* TablerFilledIcons::GetDataByIndex(int iIndex)
		{ return (iIndex >= 0 && iIndex < g_tablerFilledIconCount) ? g_tablerFilledIcons[iIndex].data : NULL; }

	int TablerFilledIcons::GetIndexByName(LPCTSTR name)
	{
		if( name == NULL || *name == _T('\0') ) return -1;
		for( int i = 0; i < g_tablerFilledIconCount; ++i ) {
			if( ::_tcscmp(name, g_tablerFilledIcons[i].name) == 0 )
				return i;
		}
		return -1;
	}
}
