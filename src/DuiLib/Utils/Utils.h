#ifndef __UTILS_H__
#define __UTILS_H__

#pragma once
#include "OAIdl.h"
#include <vector>

namespace DuiLib
{
	/////////////////////////////////////////////////////////////////////////////////////
	//

	class UILIB_API STRINGorID
	{
	public:
		STRINGorID(LPCTSTR lpString) : m_lpstr(lpString)
		{ }
		STRINGorID(UINT nID) : m_lpstr(MAKEINTRESOURCE(nID))
		{ }
		LPCTSTR m_lpstr;
	};

	/////////////////////////////////////////////////////////////////////////////////////
	//

	class UILIB_API CDuiPoint : public tagPOINT
	{
	public:
		CDuiPoint();
		CDuiPoint(const POINT& src);
		CDuiPoint(int x, int y);
		CDuiPoint(LPARAM lParam);
	};


	/////////////////////////////////////////////////////////////////////////////////////
	//

	class UILIB_API CDuiSize : public tagSIZE
	{
	public:
		CDuiSize();
		CDuiSize(const SIZE& src);
		CDuiSize(const RECT rc);
		CDuiSize(int cx, int cy);
	};


	/////////////////////////////////////////////////////////////////////////////////////
	//

	class UILIB_API CDuiRect : public tagRECT
	{
	public:
		CDuiRect();
		CDuiRect(const RECT& src);
		CDuiRect(int iLeft, int iTop, int iRight, int iBottom);

		int GetWidth() const;
		int GetHeight() const;
		void Empty();
		bool IsNull() const;
		void Join(const RECT& rc);
		void ResetOffset();
		void Normalize();
		void Offset(int cx, int cy);
		void Inflate(int cx, int cy);
		void Deflate(int cx, int cy);
		void Union(CDuiRect& rc);
	};

	/// margin / padding 等 inset：字段与四参构造均为 CSS 顺序 top,right,bottom,left
	class UILIB_API CDuiBox
	{
	public:
		int top;
		int right;
		int bottom;
		int left;

		CDuiBox();
		explicit CDuiBox(int iAll);
		CDuiBox(int iTop, int iRight, int iBottom, int iLeft);
		/// 按字段名映射（src.left→left …），不是按聚合初始化顺序重排
		CDuiBox(const RECT& src);

		void Empty();
		bool IsNull() const;
		RECT ToRect() const;
		operator RECT() const { return ToRect(); }
	};

	/////////////////////////////////////////////////////////////////////////////////////
	//

	class UILIB_API CStdPtrArray
	{
	public:
		CStdPtrArray(int iPreallocSize = 0);
		CStdPtrArray(const CStdPtrArray& src);
		~CStdPtrArray();

		void Empty();
		void Resize(int iSize);
		bool IsEmpty() const;
		int Find(LPVOID iIndex) const;
		bool Add(LPVOID pData);
		bool SetAt(int iIndex, LPVOID pData);
		bool InsertAt(int iIndex, LPVOID pData);
		bool Remove(int iIndex);
		int GetSize() const;
		LPVOID* GetData();

		LPVOID GetAt(int iIndex) const;
		LPVOID operator[] (int nIndex) const;

	protected:
		LPVOID* m_ppVoid;
		int m_nCount;
		int m_nAllocated;
	};


	/////////////////////////////////////////////////////////////////////////////////////
	//

	class UILIB_API CStdValArray
	{
	public:
		CStdValArray(int iElementSize, int iPreallocSize = 0);
		~CStdValArray();

		void Empty();
		bool IsEmpty() const;
		bool Add(LPCVOID pData);
		bool Remove(int iIndex);
		int GetSize() const;
		LPVOID GetData();

		LPVOID GetAt(int iIndex) const;
		LPVOID operator[] (int nIndex) const;

	protected:
		LPBYTE m_pVoid;
		int m_iElementSize;
		int m_nCount;
		int m_nAllocated;
	};


	/////////////////////////////////////////////////////////////////////////////////////
	//

	class UILIB_API CDuiString
	{
	public:
		enum { MAX_LOCAL_STRING_LEN = 63 };

		CDuiString();
		CDuiString(const TCHAR ch);
		CDuiString(const CDuiString& src);
		CDuiString(LPCTSTR lpsz, int nLen = -1);
		~CDuiString();

		void Empty();
		int GetLength() const;
		bool IsEmpty() const;
		TCHAR GetAt(int nIndex) const;
		void Append(LPCTSTR pstr);
		void Assign(LPCTSTR pstr, int nLength = -1);
		LPCTSTR GetData() const;

		void SetAt(int nIndex, TCHAR ch);

		TCHAR operator[] (int nIndex) const;
		const CDuiString& operator=(const CDuiString& src);
		const CDuiString& operator=(const TCHAR ch);
		const CDuiString& operator=(LPCTSTR pstr);
#ifdef _UNICODE
		const CDuiString& operator=(LPCSTR lpStr);
		const CDuiString& operator+=(LPCSTR lpStr);
#else
		const CDuiString& operator=(LPCWSTR lpwStr);
		const CDuiString& operator+=(LPCWSTR lpwStr);
#endif
		CDuiString operator+(const CDuiString& src) const;
		CDuiString operator+(LPCTSTR pstr) const;
		const CDuiString& operator+=(const CDuiString& src);
		const CDuiString& operator+=(LPCTSTR pstr);
		const CDuiString& operator+=(const TCHAR ch);

		bool operator == (LPCTSTR str) const;
		bool operator != (LPCTSTR str) const;
		bool operator <= (LPCTSTR str) const;
		bool operator <  (LPCTSTR str) const;
		bool operator >= (LPCTSTR str) const;
		bool operator >  (LPCTSTR str) const;
		bool operator == (const CDuiString& str) const { return Compare(str.GetData()) == 0; }
		bool operator != (const CDuiString& str) const { return Compare(str.GetData()) != 0; }
		bool operator <= (const CDuiString& str) const { return Compare(str.GetData()) <= 0; }
		bool operator <  (const CDuiString& str) const { return Compare(str.GetData()) <  0; }
		bool operator >= (const CDuiString& str) const { return Compare(str.GetData()) >= 0; }
		bool operator >  (const CDuiString& str) const { return Compare(str.GetData()) >  0; }

		int Compare(LPCTSTR pstr) const;
		int CompareNoCase(LPCTSTR pstr) const;

		void MakeUpper();
		void MakeLower();

		CDuiString Left(int nLength) const;
		CDuiString Mid(int iPos, int nLength = -1) const;
		CDuiString Right(int nLength) const;
		CDuiString& TrimLeft();
		CDuiString& TrimRight();
		CDuiString& Trim();

		int Find(TCHAR ch, int iPos = 0) const;
		int Find(LPCTSTR pstr, int iPos = 0) const;
		int Find(const CDuiString& str, int iPos = 0) const { return Find(str.GetData(), iPos); }
		int ReverseFind(TCHAR ch) const;
		int Replace(LPCTSTR pstrFrom, LPCTSTR pstrTo);

		int __cdecl Format(LPCTSTR pstrFormat, ...);
		int __cdecl SmallFormat(LPCTSTR pstrFormat, ...);

	protected:
		int __cdecl InnerFormat(LPCTSTR pstrFormat, va_list Args);

	protected:
		LPTSTR m_pstr;
		TCHAR m_szBuffer[MAX_LOCAL_STRING_LEN + 1];
	};

	UILIB_API std::vector<CDuiString> StrSplit(CDuiString text, CDuiString sp);

	/////////////////////////////////////////////////////////////////////////////////////
	//

	struct TITEM
	{
		CDuiString Key;
		LPVOID Data;
		struct TITEM* pPrev;
		struct TITEM* pNext;
	};

	UILIB_API UINT HashKey(LPCTSTR Key);
	UILIB_API UINT HashKey(const CDuiString& Key);

	class UILIB_API CStdStringPtrMap
	{
	public:
		CStdStringPtrMap(int nSize = 83);
		~CStdStringPtrMap();

		void Resize(int nSize = 83);
		LPVOID Find(LPCTSTR key, bool optimize = true) const;
		LPVOID Find(const CDuiString& key, bool optimize = true) const { return Find(key.GetData(), optimize); }
		bool Insert(LPCTSTR key, LPVOID pData);
		bool Insert(const CDuiString& key, LPVOID pData) { return Insert(key.GetData(), pData); }
		LPVOID Set(LPCTSTR key, LPVOID pData);
		LPVOID Set(const CDuiString& key, LPVOID pData) { return Set(key.GetData(), pData); }
		bool Remove(LPCTSTR key);
		bool Remove(const CDuiString& key) { return Remove(key.GetData()); }
		void RemoveAll();
		int GetSize() const;
		LPCTSTR GetAt(int iIndex) const;
		LPCTSTR operator[] (int nIndex) const;

	protected:
		TITEM** m_aT;
		int m_nBuckets;
		int m_nCount;
	};

	/////////////////////////////////////////////////////////////////////////////////////
	//

	class UILIB_API CWaitCursor
	{
	public:
		CWaitCursor();
		~CWaitCursor();

	protected:
		HCURSOR m_hOrigCursor;
	};

	/////////////////////////////////////////////////////////////////////////////////////
	//

	class CDuiVariant : public VARIANT
	{
	public:
		CDuiVariant() 
		{ 
			VariantInit(this); 
		}
		CDuiVariant(int i)
		{
			VariantInit(this);
			this->vt = VT_I4;
			this->intVal = i;
		}
		CDuiVariant(float f)
		{
			VariantInit(this);
			this->vt = VT_R4;
			this->fltVal = f;
		}
		CDuiVariant(LPOLESTR s)
		{
			VariantInit(this);
			this->vt = VT_BSTR;
			this->bstrVal = s;
		}
		CDuiVariant(IDispatch *disp)
		{
			VariantInit(this);
			this->vt = VT_DISPATCH;
			this->pdispVal = disp;
		}

		~CDuiVariant() 
		{ 
			VariantClear(this); 
		}
	};

	//////////////////////////////////////////////////////////////////////////////////////
	//
	/// 返回 new[] 缓冲区，调用方 delete[]
	UILIB_API char* w2a(wchar_t* lpszSrc, UINT CodePage = CP_ACP);
	UILIB_API wchar_t* a2w(char* lpszSrc, UINT CodePage = CP_ACP);

	///////////////////////////////////////////////////////////////////////////////////////
	////
	//struct TImageInfo;
	//class CPaintManagerUI;
	//class UILIB_API CImageString
	//{
	//public:
	//	CImageString();
	//	CImageString(const CImageString&);
	//	const CImageString& operator=(const CImageString&);
	//	virtual ~CImageString();

	//	const CDuiString& GetAttributeString() const;
	//	void SetAttributeString(LPCTSTR pStrImageAttri);
	//	void ModifyAttribute(LPCTSTR pStrModify);
	//	bool LoadImage(CPaintManagerUI* pManager);
	//	bool IsLoadSuccess();

	//	RECT GetDest() const;
	//	void SetDest(const RECT &rcDest);
	//	const TImageInfo* GetImageInfo() const;

	//private:
	//	void Clone(const CImageString&);
	//	void Clear();
	//	void ParseAttribute(LPCTSTR pStrImageAttri);

	//protected:
	//	friend class CRenderEngine;
	//	CDuiString	m_sImageAttribute;

	//	CDuiString	m_sImage;
	//	CDuiString	m_sResType;
	//	TImageInfo	*m_imageInfo;
	//	bool		m_bLoadSuccess;

	//	RECT	m_rcDest;
	//	RECT	m_rcSource;
	//	RECT	m_rcCorner;
	//	BYTE	m_bFade;
	//	DWORD	m_dwMask;
	//	bool	m_bHole;
	//	bool	m_bTiledX;
	//	bool	m_bTiledY;
	//};
	/////////////////////////////////////////////////////////////////////////////////////
	/////////////////////////////////////////////////////////////////////////////////////
	// 颜色 DWORD = CSS RRGGBBAA（与 #RRGGBBAA / 0xRRGGBBAA 字面量一致）
	// C++：0x1677FFFF；属性串同序。像素缓冲里的 ARGB 内存布局不在此列。
	inline BYTE DuiColorR(DWORD c) { return (BYTE)((c >> 24) & 0xFF); }
	inline BYTE DuiColorG(DWORD c) { return (BYTE)((c >> 16) & 0xFF); }
	inline BYTE DuiColorB(DWORD c) { return (BYTE)((c >> 8) & 0xFF); }
	inline BYTE DuiColorA(DWORD c) { return (BYTE)(c & 0xFF); }
	inline COLORREF DuiColorToCOLORREF(DWORD c) { return RGB(DuiColorR(c), DuiColorG(c), DuiColorB(c)); }
	inline DWORD DuiColorFromRGB(BYTE r, BYTE g, BYTE b, BYTE a = 0xFF)
	{
		return ((DWORD)r << 24) | ((DWORD)g << 16) | ((DWORD)b << 8) | (DWORD)a;
	}
	inline DWORD DuiColorSetA(DWORD c, BYTE a) { return (c & 0xFFFFFF00u) | (DWORD)a; }
	inline bool DuiColorIsOpaque(DWORD c) { return DuiColorA(c) >= 0xFFu; }

	// 通用“纯色”（RGBA 不透明，即 0xRRGGBBAA），任意控件/皮肤逻辑可按需复用；前缀 DuiColor_*
	inline constexpr DWORD DuiColor_Black  = 0x000000FF;   // 纯黑
	inline constexpr DWORD DuiColor_White  = 0xFFFFFFFF;   // 纯白
	inline constexpr DWORD DuiColor_Red    = 0xFF0000FF;   // 纯红
	inline constexpr DWORD DuiColor_Green  = 0x00FF00FF;   // 纯绿
	inline constexpr DWORD DuiColor_Blue   = 0x0000FFFF;   // 纯蓝
	inline constexpr DWORD DuiColor_Cyan   = 0x00FFFFFF;   // 纯青
	inline constexpr DWORD DuiColor_Magenta= 0xFF00FFFF;   // 纯品红/紫（R=FF，仅每字节高位同 0xFF 才显黄，此色为红+蓝）
	inline constexpr DWORD DuiColor_Yellow = 0xFFFF00FF;   // 纯黄
	inline constexpr DWORD DuiColor_Orange = 0xFF8000FF;   // 纯橙

	// 颜色：#RGB / #RRGGBB / #RRGGBBAA / #RGBA；0x 同序；rgb()/rgba()/hsl()/hsla()；命名色
	UILIB_API bool ParseColorString(LPCTSTR pstrColor, DWORD& dwColor);
	/// 从流中解析一个色值 token 并前进指针（供 showhtml `<c …>` 等）
	UILIB_API bool ParseColorStringToken(LPCTSTR& pstrInOut, DWORD& dwColor);
	/// CSS opacity：`0.5` / `50%` / `128`（0–255 字节）→ 0–255
	UILIB_API bool ParseCssOpacity(LPCTSTR pstrValue, BYTE& nOpacity);
	/// 属性布尔：true/false、1/0、yes/no、on/off（大小写不敏感）
	UILIB_API bool ParseAttrBool(LPCTSTR pstrValue, bool& bValue);
	/// CSS font-weight → 是否粗体（bold/700+ / normal/400…）
	UILIB_API bool ParseCssFontWeightBold(LPCTSTR pstrValue, bool& bBold);
	/// CSS font-style → 是否斜体（italic/oblique / normal）
	UILIB_API bool ParseCssFontStyleItalic(LPCTSTR pstrValue, bool& bItalic);
	/// CSS text-decoration：可含 underline / line-through；none 清零。未识别返回 false
	UILIB_API bool ParseCssTextDecoration(LPCTSTR pstrValue, bool& bUnderline, bool& bStrikeout);
	/// CSS pointer-events：none→false；auto→true
	UILIB_API bool ParseCssPointerEventsEnabled(LPCTSTR pstrValue, bool& bEnabled);
	/// CSS box shorthand top[,right[,bottom[,left]]] → CDuiBox
	UILIB_API bool ParseCssBox(LPCTSTR pstrValue, CDuiBox& box);
	/// 兼容：解析为 CDuiBox 再写入 RECT 字段（.left=left …）
	UILIB_API bool ParseCssBoxToRect(LPCTSTR pstrValue, RECT& rc);
	/// border-radius：CSS 半径 `12` / `12px` → cx=cy；`12,8` / `12px 8px` → 椭圆两轴半径
	UILIB_API bool ParseBorderRadiusValue(LPCTSTR pstrValue, SIZE& szRound);
	/// CSS 半径 → GDI RoundRect / CreateRoundRectRgn 椭圆直径
	inline SIZE CssRadiusToEllipse(SIZE szRadius)
	{
		SIZE sz = { szRadius.cx * 2, szRadius.cy * 2 };
		return sz;
	}
	inline void CssRadiusToEllipse(int radiusX, int radiusY, int& ellipseW, int& ellipseH)
	{
		ellipseW = radiusX * 2;
		ellipseH = radiusY * 2;
	}
	/// `url(path)` / `url('path')` / `url("path")` → 裸路径；非 url 原样返回 false
	UILIB_API bool ParseCssUrlImage(LPCTSTR pstrValue, CDuiString& sPath);
	/// CSS overflow 值：`auto`/`scroll`/`true` → 启用滚动条；`hidden`/`clip`/`visible`/`false` → 关闭
	UILIB_API bool ParseCssOverflowEnablesScroll(LPCTSTR pstrValue, bool& bEnable);
	/// `overflow` 简写：1 值双轴；2 值按 CSS 为 overflow-x overflow-y
	UILIB_API bool ParseCssOverflowShorthand(LPCTSTR pstrValue, bool& bEnableX, bool& bEnableY);

	/////////////////////////////////////////////////////////////////////////////////////
	//
	// 极简文本日志（默认关）。供库内排障/使用者定位事件用：
	//   关闭时零开销，不在分发路径产生任何调用。

	class UILIB_API CDuiLog
	{
	public:
		static void SetEnabled(bool bEnable);
		static bool IsEnabled();
		/// 设置日志文件路径（工程固定 UNICODE，写 UTF-16(LE)）。传 NULL/空则恢复默认：
		/// 有 D 盘用 D:\\DUIX.log，否则 C:\\DUIX.log。
		static void SetLogFile(LPCTSTR pszPath);
		/// 写一行（自动加时间戳与换行）；未启用时为空操作。
		static void Write(LPCTSTR pstrFormat, ...);
		/// 带来源（文件+行号）写一行。（不建议直接调用，用 DUILOG 宏）
		static void WriteAt(LPCTSTR pszFile, int nLine, LPCTSTR pstrFormat, ...);
	};

	/// 带文件与行号写日志（默认关，未启用时零开销）。推荐用这个宏而非直接 Write。
#define DUILOG(...) DuiLib::CDuiLog::WriteAt(__FILEW__, __LINE__, __VA_ARGS__)

}// namespace DuiLib

#endif // __UTILS_H__