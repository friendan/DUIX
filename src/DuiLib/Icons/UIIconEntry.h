#ifndef __UIICONENTRY_H__
#define __UIICONENTRY_H__

#pragma once

namespace DuiLib
{
	struct IconEntry
	{
		const wchar_t* name;
		const char* data; // UTF-8 SVG
	};
}

#endif // __UIICONENTRY_H__
