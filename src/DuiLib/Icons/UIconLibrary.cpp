#include "StdAfx.h"
#include "UIconLibrary.h"
#include "UIBootstrapIcons.h"
#include "UIIconParkIcons.h"
#include "UILucideIcons.h"
#include "UIRemixIconIcons.h"
#include "UITablerIcons.h"
#include "UITwemojiIcons.h"

namespace DuiLib
{
	// 映射库名 → (CountFn, NameFn, DataFn, IndexFn, GetFn)
	typedef int (*FCount)();
	typedef const wchar_t* (*FName)(int);
	typedef const char* (*FData)(int);
	typedef int (*FIndex)(LPCTSTR);
	typedef const char* (*FGet)(LPCTSTR);

	static bool ResolveLib(LPCTSTR lib, FCount& fCount, FName& fName, FData& fData, FIndex& fIndex, FGet& fGet)
	{
		if( lib == NULL || *lib == _T('\0') ) return false;
		if( _tcsicmp(lib, _T("bsicon")) == 0 ) {
			fCount = BootstrapIcons::GetIconCount; fName = BootstrapIcons::GetNameByIndex; fData = BootstrapIcons::GetDataByIndex;
			fIndex = BootstrapIcons::GetIndexByName; fGet = BootstrapIcons::GetIcon;
			return true;
		}
		if( _tcsicmp(lib, _T("iconpark")) == 0 ) {
			fCount = IconParkIcons::GetIconCount; fName = IconParkIcons::GetNameByIndex; fData = IconParkIcons::GetDataByIndex;
			fIndex = IconParkIcons::GetIndexByName; fGet = IconParkIcons::GetIcon;
			return true;
		}
		if( _tcsicmp(lib, _T("lucide")) == 0 ) {
			fCount = LucideIcons::GetIconCount; fName = LucideIcons::GetNameByIndex; fData = LucideIcons::GetDataByIndex;
			fIndex = LucideIcons::GetIndexByName; fGet = LucideIcons::GetIcon;
			return true;
		}
		if( _tcsicmp(lib, _T("remixicon")) == 0 ) {
			fCount = RemixIconIcons::GetIconCount; fName = RemixIconIcons::GetNameByIndex; fData = RemixIconIcons::GetDataByIndex;
			fIndex = RemixIconIcons::GetIndexByName; fGet = RemixIconIcons::GetIcon;
			return true;
		}
		if( _tcsicmp(lib, _T("tabler-outline")) == 0 ) {
			fCount = TablerOutlineIcons::GetIconCount; fName = TablerOutlineIcons::GetNameByIndex; fData = TablerOutlineIcons::GetDataByIndex;
			fIndex = TablerOutlineIcons::GetIndexByName; fGet = TablerOutlineIcons::GetIcon;
			return true;
		}
		if( _tcsicmp(lib, _T("tabler-filled")) == 0 ) {
			fCount = TablerFilledIcons::GetIconCount; fName = TablerFilledIcons::GetNameByIndex; fData = TablerFilledIcons::GetDataByIndex;
			fIndex = TablerFilledIcons::GetIndexByName; fGet = TablerFilledIcons::GetIcon;
			return true;
		}
		if( _tcsicmp(lib, _T("twicon")) == 0 ) {
			fCount = TwemojiIcons::GetIconCount; fName = TwemojiIcons::GetNameByIndex; fData = TwemojiIcons::GetDataByIndex;
			fIndex = TwemojiIcons::GetIndexByName; fGet = TwemojiIcons::GetIcon;
			return true;
		}
		return false;
	}

	int CIconLibrary::GetIconCount(LPCTSTR lib)
	{
		FCount fCount = NULL; FName fName = NULL; FData fData = NULL; FIndex fIndex = NULL; FGet fGet = NULL;
		if( !ResolveLib(lib, fCount, fName, fData, fIndex, fGet) ) return 0;
		return fCount();
	}

	const wchar_t* CIconLibrary::GetNameByIndex(LPCTSTR lib, int iIndex)
	{
		FCount fCount = NULL; FName fName = NULL; FData fData = NULL; FIndex fIndex = NULL; FGet fGet = NULL;
		if( !ResolveLib(lib, fCount, fName, fData, fIndex, fGet) ) return NULL;
		return fName(iIndex);
	}

	const char* CIconLibrary::GetDataByIndex(LPCTSTR lib, int iIndex)
	{
		FCount fCount = NULL; FName fName = NULL; FData fData = NULL; FIndex fIndex = NULL; FGet fGet = NULL;
		if( !ResolveLib(lib, fCount, fName, fData, fIndex, fGet) ) return NULL;
		return fData(iIndex);
	}

	int CIconLibrary::GetIndexByName(LPCTSTR lib, LPCTSTR name)
	{
		FCount fCount = NULL; FName fName = NULL; FData fData = NULL; FIndex fIndex = NULL; FGet fGet = NULL;
		if( !ResolveLib(lib, fCount, fName, fData, fIndex, fGet) ) return -1;
		return fIndex(name);
	}

	const char* CIconLibrary::GetDataByName(LPCTSTR lib, LPCTSTR name)
	{
		FCount fCount = NULL; FName fName = NULL; FData fData = NULL; FIndex fIndex = NULL; FGet fGet = NULL;
		if( !ResolveLib(lib, fCount, fName, fData, fIndex, fGet) ) return NULL;
		return fGet(name);
	}
}
