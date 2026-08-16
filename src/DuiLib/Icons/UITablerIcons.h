#ifndef __UITABLERICONS_H__
#define __UITABLERICONS_H__

#pragma once

namespace DuiLib
{
	class UILIB_API TablerOutlineIcons
	{
	public:
		static const char* GetIcon(LPCTSTR name);
		static int GetIconCount();
		static const wchar_t* GetNameByIndex(int iIndex);
		static const char* GetDataByIndex(int iIndex);
		/// 按图标名取其下标（与 GetNameByIndex 互逆）；找不到返回 -1
		static int GetIndexByName(LPCTSTR name);
	};

	class UILIB_API TablerFilledIcons
	{
	public:
		static const char* GetIcon(LPCTSTR name);
		static int GetIconCount();
		static const wchar_t* GetNameByIndex(int iIndex);
		static const char* GetDataByIndex(int iIndex);
		/// 按图标名取其下标（与 GetNameByIndex 互逆）；找不到返回 -1
		static int GetIndexByName(LPCTSTR name);
	};
}

#endif
