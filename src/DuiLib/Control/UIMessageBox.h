#ifndef __UIMESSAGEBOX_H__
#define __UIMESSAGEBOX_H__

#pragma once

#include "UIModal.h"

namespace DuiLib {

	enum {
		MESSAGEBOX_OK = 1,
		MESSAGEBOX_CANCEL = 0,
	};

	/// 同步 MessageBox：阻塞直到关闭。
	/// 默认走 Modal 纯代码 UI（无需皮肤）；也可 ShowSkin 用自定义 html/xml。
	class UILIB_API CMessageBox
	{
	public:
		/// 确定 + 取消；返回 MESSAGEBOX_OK / MESSAGEBOX_CANCEL
		static int Show(HWND hOwner, LPCTSTR title, LPCTSTR text);

		/// 同 Show，可链式配置 kind / 按钮文案 / 尺寸等（OnResult 仍会在关闭时调用）
		static int Show(HWND hOwner, LPCTSTR title, LPCTSTR text, const CModalOptions& opts);

		/// 仅确定按钮
		static int ShowInfo(HWND hOwner, LPCTSTR title, LPCTSTR text);

		/// 自定义皮肤：资源 id（如 XML_MSG）、相对路径（msg.html）、或内联 XML（以 '<' 开头）
		/// skin 为空时使用内置默认 XML（仍不依赖工程皮肤目录）
		static int ShowSkin(HWND hOwner, LPCTSTR title, LPCTSTR text,
			LPCTSTR skin = NULL, LPCTSTR skinType = NULL);
	};

} // namespace DuiLib

#endif // __UIMESSAGEBOX_H__
