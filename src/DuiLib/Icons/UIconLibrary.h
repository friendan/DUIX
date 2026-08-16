#ifndef __UIICONLIBRARY_H__
#define __UIICONLIBRARY_H__

#pragma once

namespace DuiLib
{
	/// 统一入口：按图标库名枚举全部内置图标（给选择器/展示用），不必分别记 7 个库类。
	/// lib 用 XML 属性名：bsicon / iconpark / lucide / remixicon / tabler-outline / tabler-filled / twicon。
	class UILIB_API CIconLibrary
	{
	public:
		/// 库的图标总数；未知库返回 0。
		static int GetIconCount(LPCTSTR lib);
		/// 按库名下标的图标名（wchar*，属库者数组，勿 free）；未知库或越界返回 NULL。
		static const wchar_t* GetNameByIndex(LPCTSTR lib, int iIndex);
		/// 按库名下标的图标 SVG（UTF-8 char*）；未知库或越界返回 NULL。
		static const char* GetDataByIndex(LPCTSTR lib, int iIndex);
		/// 按图标名取图标下标（与 GetNameByIndex 互逆）；未知库或找不到返回 -1。
		static int GetIndexByName(LPCTSTR lib, LPCTSTR name);
		/// 按图标名取图标 SVG（UTF-8 char*）；未知库或找不到返回 NULL。（存储通常只存图标名，据此取回 SVG 显示）
		static const char* GetDataByName(LPCTSTR lib, LPCTSTR name);
	};
}

#endif // __UIICONLIBRARY_H__
