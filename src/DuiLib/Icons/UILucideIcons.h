#ifndef __UILUCIDEICONS_H__
#define __UILUCIDEICONS_H__

#pragma once

namespace DuiLib
{
	class UILIB_API LucideIcons
	{
	public:
		static const char* GetIcon(LPCTSTR name);
		/// 图标总数（枚举用）
		static int GetIconCount();
		/// 按下标取图标名（wchar*，属所有者数组，勿 free）；越界返回 NULL
		static const wchar_t* GetNameByIndex(int iIndex);
		/// 按下标取图标 SVG（UTF-8）；越界返回 NULL
		static const char* GetDataByIndex(int iIndex);
		/// 按图标名取其下标（与 GetNameByIndex 互逆）；找不到返回 -1
		static int GetIndexByName(LPCTSTR name);
	};
}

#endif
