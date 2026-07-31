#ifndef __UIBOOTSTRAPICONS_H__
#define __UIBOOTSTRAPICONS_H__

#pragma once

namespace DuiLib
{
	class UILIB_API BootstrapIcons
	{
	public:
		// 返回 UTF-8 SVG；找不到返回 NULL
		static const char* GetIcon(LPCTSTR name);
	};
}

#endif
