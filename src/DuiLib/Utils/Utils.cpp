#include "StdAfx.h"
#include "Utils.h"

namespace DuiLib
{

	/////////////////////////////////////////////////////////////////////////////////////
	//
	//

	CDuiPoint::CDuiPoint()
	{
		x = y = 0;
	}

	CDuiPoint::CDuiPoint(const POINT& src)
	{
		x = src.x;
		y = src.y;
	}

	CDuiPoint::CDuiPoint(int _x, int _y)
	{
		x = _x;
		y = _y;
	}

	CDuiPoint::CDuiPoint(LPARAM lParam)
	{
		x = GET_X_LPARAM(lParam);
		y = GET_Y_LPARAM(lParam);
	}


	/////////////////////////////////////////////////////////////////////////////////////
	//
	//

	CDuiSize::CDuiSize()
	{
		cx = cy = 0;
	}

	CDuiSize::CDuiSize(const SIZE& src)
	{
		cx = src.cx;
		cy = src.cy;
	}

	CDuiSize::CDuiSize(const RECT rc)
	{
		cx = rc.right - rc.left;
		cy = rc.bottom - rc.top;
	}

	CDuiSize::CDuiSize(int _cx, int _cy)
	{
		cx = _cx;
		cy = _cy;
	}


	/////////////////////////////////////////////////////////////////////////////////////
	//
	//

	CDuiBox::CDuiBox()
		: top(0), right(0), bottom(0), left(0)
	{
	}

	CDuiBox::CDuiBox(int iAll)
		: top(iAll), right(iAll), bottom(iAll), left(iAll)
	{
	}

	CDuiBox::CDuiBox(int iTop, int iRight, int iBottom, int iLeft)
		: top(iTop), right(iRight), bottom(iBottom), left(iLeft)
	{
	}

	CDuiBox::CDuiBox(const RECT& src)
		: top(src.top), right(src.right), bottom(src.bottom), left(src.left)
	{
	}

	void CDuiBox::Empty()
	{
		top = right = bottom = left = 0;
	}

	bool CDuiBox::IsNull() const
	{
		return (top == 0 && right == 0 && bottom == 0 && left == 0);
	}

	RECT CDuiBox::ToRect() const
	{
		RECT rc = { left, top, right, bottom };
		return rc;
	}

	CDuiRect::CDuiRect()
	{
		left = top = right = bottom = 0;
	}

	CDuiRect::CDuiRect(const RECT& src)
	{
		left = src.left;
		top = src.top;
		right = src.right;
		bottom = src.bottom;
	}

	CDuiRect::CDuiRect(int iLeft, int iTop, int iRight, int iBottom)
	{
		left = iLeft;
		top = iTop;
		right = iRight;
		bottom = iBottom;
	}

	int CDuiRect::GetWidth() const
	{
		return right - left;
	}

	int CDuiRect::GetHeight() const
	{
		return bottom - top;
	}

	void CDuiRect::Empty()
	{
		left = top = right = bottom = 0;
	}

	bool CDuiRect::IsNull() const
	{
		return (left == 0 && right == 0 && top == 0 && bottom == 0); 
	}

	void CDuiRect::Join(const RECT& rc)
	{
		if( rc.left < left ) left = rc.left;
		if( rc.top < top ) top = rc.top;
		if( rc.right > right ) right = rc.right;
		if( rc.bottom > bottom ) bottom = rc.bottom;
	}

	void CDuiRect::ResetOffset()
	{
		::OffsetRect(this, -left, -top);
	}

	void CDuiRect::Normalize()
	{
		if( left > right ) { int iTemp = left; left = right; right = iTemp; }
		if( top > bottom ) { int iTemp = top; top = bottom; bottom = iTemp; }
	}

	void CDuiRect::Offset(int cx, int cy)
	{
		::OffsetRect(this, cx, cy);
	}

	void CDuiRect::Inflate(int cx, int cy)
	{
		::InflateRect(this, cx, cy);
	}

	void CDuiRect::Deflate(int cx, int cy)
	{
		::InflateRect(this, -cx, -cy);
	}

	void CDuiRect::Union(CDuiRect& rc)
	{
		::UnionRect(this, this, &rc);
	}


	/////////////////////////////////////////////////////////////////////////////////////
	//
	//

	CStdPtrArray::CStdPtrArray(int iPreallocSize) : m_ppVoid(NULL), m_nCount(0), m_nAllocated(iPreallocSize)
	{
		ASSERT(iPreallocSize>=0);
		if( iPreallocSize > 0 ) m_ppVoid = static_cast<LPVOID*>(malloc(iPreallocSize * sizeof(LPVOID)));
	}

	CStdPtrArray::CStdPtrArray(const CStdPtrArray& src) : m_ppVoid(NULL), m_nCount(0), m_nAllocated(0)
	{
		for(int i=0; i<src.GetSize(); i++)
			Add(src.GetAt(i));
	}

	CStdPtrArray::~CStdPtrArray()
	{
		if( m_ppVoid != NULL ) free(m_ppVoid);
	}

	void CStdPtrArray::Empty()
	{
		if( m_ppVoid != NULL ) free(m_ppVoid);
		m_ppVoid = NULL;
		m_nCount = m_nAllocated = 0;
	}

	void CStdPtrArray::Resize(int iSize)
	{
		Empty();
		m_ppVoid = static_cast<LPVOID*>(malloc(iSize * sizeof(LPVOID)));
		::ZeroMemory(m_ppVoid, iSize * sizeof(LPVOID));
		m_nAllocated = iSize;
		m_nCount = iSize;
	}

	bool CStdPtrArray::IsEmpty() const
	{
		return m_nCount == 0;
	}

	bool CStdPtrArray::Add(LPVOID pData)
	{
		if( ++m_nCount >= m_nAllocated) {
			int nAllocated = m_nAllocated * 2;
			if( nAllocated == 0 ) nAllocated = 11;
			LPVOID* ppVoid = static_cast<LPVOID*>(realloc(m_ppVoid, nAllocated * sizeof(LPVOID)));
			if( ppVoid != NULL ) {
				m_nAllocated = nAllocated;
				m_ppVoid = ppVoid;
			}
			else {
				--m_nCount;
				return false;
			}
		}
		m_ppVoid[m_nCount - 1] = pData;
		return true;
	}

	bool CStdPtrArray::InsertAt(int iIndex, LPVOID pData)
	{
		if( iIndex == m_nCount ) return Add(pData);
		if( iIndex < 0 || iIndex > m_nCount ) return false;
		if( ++m_nCount >= m_nAllocated) {
			int nAllocated = m_nAllocated * 2;
			if( nAllocated == 0 ) nAllocated = 11;
			LPVOID* ppVoid = static_cast<LPVOID*>(realloc(m_ppVoid, nAllocated * sizeof(LPVOID)));
			if( ppVoid != NULL ) {
				m_nAllocated = nAllocated;
				m_ppVoid = ppVoid;
			}
			else {
				--m_nCount;
				return false;
			}
		}
		memmove(&m_ppVoid[iIndex + 1], &m_ppVoid[iIndex], (m_nCount - iIndex - 1) * sizeof(LPVOID));
		m_ppVoid[iIndex] = pData;
		return true;
	}

	bool CStdPtrArray::SetAt(int iIndex, LPVOID pData)
	{
		if( iIndex < 0 || iIndex >= m_nCount ) return false;
		m_ppVoid[iIndex] = pData;
		return true;
	}

	bool CStdPtrArray::Remove(int iIndex)
	{
		if( iIndex < 0 || iIndex >= m_nCount ) return false;
		if( iIndex < --m_nCount ) ::CopyMemory(m_ppVoid + iIndex, m_ppVoid + iIndex + 1, (m_nCount - iIndex) * sizeof(LPVOID));
		return true;
	}

	int CStdPtrArray::Find(LPVOID pData) const
	{
		for( int i = 0; i < m_nCount; i++ ) if( m_ppVoid[i] == pData ) return i;
		return -1;
	}

	int CStdPtrArray::GetSize() const
	{
		return m_nCount;
	}

	LPVOID* CStdPtrArray::GetData()
	{
		return m_ppVoid;
	}

	LPVOID CStdPtrArray::GetAt(int iIndex) const
	{
		if( iIndex < 0 || iIndex >= m_nCount ) return NULL;
		return m_ppVoid[iIndex];
	}

	LPVOID CStdPtrArray::operator[] (int iIndex) const
	{
		ASSERT(iIndex>=0 && iIndex<m_nCount);
		return m_ppVoid[iIndex];
	}


	/////////////////////////////////////////////////////////////////////////////////////
	//
	//

	CStdValArray::CStdValArray(int iElementSize, int iPreallocSize /*= 0*/) : 
	m_pVoid(NULL), 
		m_nCount(0), 
		m_iElementSize(iElementSize), 
		m_nAllocated(iPreallocSize)
	{
		ASSERT(iElementSize>0);
		ASSERT(iPreallocSize>=0);
		if( iPreallocSize > 0 ) m_pVoid = static_cast<LPBYTE>(malloc(iPreallocSize * m_iElementSize));
	}

	CStdValArray::~CStdValArray()
	{
		if( m_pVoid != NULL ) free(m_pVoid);
	}

	void CStdValArray::Empty()
	{   
		m_nCount = 0;  // NOTE: We keep the memory in place
	}

	bool CStdValArray::IsEmpty() const
	{
		return m_nCount == 0;
	}

	bool CStdValArray::Add(LPCVOID pData)
	{
		if( ++m_nCount >= m_nAllocated) {
			int nAllocated = m_nAllocated * 2;
			if( nAllocated == 0 ) nAllocated = 11;
			LPBYTE pVoid = static_cast<LPBYTE>(realloc(m_pVoid, nAllocated * m_iElementSize));
			if( pVoid != NULL ) {
				m_nAllocated = nAllocated;
				m_pVoid = pVoid;
			}
			else {
				--m_nCount;
				return false;
			}
		}
		::CopyMemory(m_pVoid + ((m_nCount - 1) * m_iElementSize), pData, m_iElementSize);
		return true;
	}

	bool CStdValArray::Remove(int iIndex)
	{
		if( iIndex < 0 || iIndex >= m_nCount ) return false;
		if( iIndex < --m_nCount ) ::CopyMemory(m_pVoid + (iIndex * m_iElementSize), m_pVoid + ((iIndex + 1) * m_iElementSize), (m_nCount - iIndex) * m_iElementSize);
		return true;
	}

	int CStdValArray::GetSize() const
	{
		return m_nCount;
	}

	LPVOID CStdValArray::GetData()
	{
		return static_cast<LPVOID>(m_pVoid);
	}

	LPVOID CStdValArray::GetAt(int iIndex) const
	{
		if( iIndex < 0 || iIndex >= m_nCount ) return NULL;
		return m_pVoid + (iIndex * m_iElementSize);
	}

	LPVOID CStdValArray::operator[] (int iIndex) const
	{
		ASSERT(iIndex>=0 && iIndex<m_nCount);
		return m_pVoid + (iIndex * m_iElementSize);
	}


	/////////////////////////////////////////////////////////////////////////////////////
	//
	//

	CDuiString::CDuiString() : m_pstr(m_szBuffer)
	{
		m_szBuffer[0] = '\0';
	}

	CDuiString::CDuiString(const TCHAR ch) : m_pstr(m_szBuffer)
	{
		m_szBuffer[0] = ch;
		m_szBuffer[1] = '\0';
	}

	CDuiString::CDuiString(LPCTSTR lpsz, int nLen) : m_pstr(m_szBuffer)
	{      
		ASSERT(!::IsBadStringPtr(lpsz,-1) || lpsz==NULL);
		m_szBuffer[0] = '\0';
		Assign(lpsz, nLen);
	}

	CDuiString::CDuiString(const CDuiString& src) : m_pstr(m_szBuffer)
	{
		m_szBuffer[0] = '\0';
		Assign(src.m_pstr);
	}

	CDuiString::~CDuiString()
	{
		if( m_pstr != m_szBuffer ) free(m_pstr);
	}

	int CDuiString::GetLength() const
	{ 
		return (int) _tcslen(m_pstr); 
	}

	CDuiString::operator LPCTSTR() const 
	{ 
		return m_pstr; 
	}

	void CDuiString::Append(LPCTSTR pstr)
	{
		int nNewLength = GetLength() + (int) _tcslen(pstr);
		if( nNewLength >= MAX_LOCAL_STRING_LEN ) {
			if( m_pstr == m_szBuffer ) {
				m_pstr = static_cast<LPTSTR>(malloc((nNewLength + 1) * sizeof(TCHAR)));
				_tcscpy(m_pstr, m_szBuffer);
				_tcscat(m_pstr, pstr);
			}
			else {
				m_pstr = static_cast<LPTSTR>(realloc(m_pstr, (nNewLength + 1) * sizeof(TCHAR)));
				_tcscat(m_pstr, pstr);
			}
		}
		else {
			if( m_pstr != m_szBuffer ) {
				free(m_pstr);
				m_pstr = m_szBuffer;
			}
			_tcscat(m_szBuffer, pstr);
		}
	}

	void CDuiString::Assign(LPCTSTR pstr, int cchMax)
	{
		if( pstr == NULL ) pstr = _T("");
		cchMax = (cchMax < 0 ? (int) _tcslen(pstr) : cchMax);
		if( cchMax < MAX_LOCAL_STRING_LEN ) {
			if( m_pstr != m_szBuffer ) {
				free(m_pstr);
				m_pstr = m_szBuffer;
			}
		}
		else if( cchMax > GetLength() || m_pstr == m_szBuffer ) {
			if( m_pstr == m_szBuffer ) m_pstr = NULL;
			m_pstr = static_cast<LPTSTR>(realloc(m_pstr, (cchMax + 1) * sizeof(TCHAR)));
		}
		_tcsncpy(m_pstr, pstr, cchMax);
		m_pstr[cchMax] = '\0';
	}

	bool CDuiString::IsEmpty() const 
	{ 
		return m_pstr[0] == '\0'; 
	}

	void CDuiString::Empty() 
	{ 
		if( m_pstr != m_szBuffer ) free(m_pstr);
		m_pstr = m_szBuffer;
		m_szBuffer[0] = '\0'; 
	}

	LPCTSTR CDuiString::GetData() const
	{
		return m_pstr;
	}

	TCHAR CDuiString::GetAt(int nIndex) const
	{
		return m_pstr[nIndex];
	}

	TCHAR CDuiString::operator[] (int nIndex) const
	{ 
		return m_pstr[nIndex];
	}   

	const CDuiString& CDuiString::operator=(const CDuiString& src)
	{      
		Assign(src);
		return *this;
	}

	const CDuiString& CDuiString::operator=(LPCTSTR lpStr)
	{      
		if ( lpStr )
		{
			ASSERT(!::IsBadStringPtr(lpStr,-1));
			Assign(lpStr);
		}
		else
		{
			Empty();
		}
		return *this;
	}

#ifdef _UNICODE

	const CDuiString& CDuiString::operator=(LPCSTR lpStr)
	{
		if ( lpStr )
		{
			ASSERT(!::IsBadStringPtrA(lpStr,-1));
			int cchStr = (int) strlen(lpStr) + 1;
			LPWSTR pwstr = (LPWSTR) _alloca(cchStr);
			if( pwstr != NULL ) ::MultiByteToWideChar(::GetACP(), 0, lpStr, -1, pwstr, cchStr) ;
			Assign(pwstr);
		}
		else
		{
			Empty();
		}
		return *this;
	}

	const CDuiString& CDuiString::operator+=(LPCSTR lpStr)
	{
		if ( lpStr )
		{
			ASSERT(!::IsBadStringPtrA(lpStr,-1));
			int cchStr = (int) strlen(lpStr) + 1;
			LPWSTR pwstr = (LPWSTR) _alloca(cchStr);
			if( pwstr != NULL ) ::MultiByteToWideChar(::GetACP(), 0, lpStr, -1, pwstr, cchStr) ;
			Append(pwstr);
		}
		
		return *this;
	}

#else

	const CDuiString& CDuiString::operator=(LPCWSTR lpwStr)
	{      
		if ( lpwStr )
		{
			ASSERT(!::IsBadStringPtrW(lpwStr,-1));
			int cchStr = ((int) wcslen(lpwStr) * 2) + 1;
			LPSTR pstr = (LPSTR) _alloca(cchStr);
			if( pstr != NULL ) ::WideCharToMultiByte(::GetACP(), 0, lpwStr, -1, pstr, cchStr, NULL, NULL);
			Assign(pstr);
		}
		else
		{
			Empty();
		}
		
		return *this;
	}

	const CDuiString& CDuiString::operator+=(LPCWSTR lpwStr)
	{
		if ( lpwStr )
		{
			ASSERT(!::IsBadStringPtrW(lpwStr,-1));
			int cchStr = ((int) wcslen(lpwStr) * 2) + 1;
			LPSTR pstr = (LPSTR) _alloca(cchStr);
			if( pstr != NULL ) ::WideCharToMultiByte(::GetACP(), 0, lpwStr, -1, pstr, cchStr, NULL, NULL);
			Append(pstr);
		}
		
		return *this;
	}

#endif // _UNICODE

	const CDuiString& CDuiString::operator=(const TCHAR ch)
	{
		Empty();
		m_szBuffer[0] = ch;
		m_szBuffer[1] = '\0';
		return *this;
	}

	CDuiString CDuiString::operator+(const CDuiString& src) const
	{
		CDuiString sTemp = *this;
		sTemp.Append(src);
		return sTemp;
	}

	CDuiString CDuiString::operator+(LPCTSTR lpStr) const
	{
		if ( lpStr )
		{
			ASSERT(!::IsBadStringPtr(lpStr,-1));
			CDuiString sTemp = *this;
			sTemp.Append(lpStr);
			return sTemp;
		}

		return *this;
	}

	const CDuiString& CDuiString::operator+=(const CDuiString& src)
	{      
		Append(src);
		return *this;
	}

	const CDuiString& CDuiString::operator+=(LPCTSTR lpStr)
	{      
		if ( lpStr )
		{
			ASSERT(!::IsBadStringPtr(lpStr,-1));
			Append(lpStr);
		}
		
		return *this;
	}

	const CDuiString& CDuiString::operator+=(const TCHAR ch)
	{      
		TCHAR str[] = { ch, '\0' };
		Append(str);
		return *this;
	}

	bool CDuiString::operator == (LPCTSTR str) const { return (Compare(str) == 0); };
	bool CDuiString::operator != (LPCTSTR str) const { return (Compare(str) != 0); };
	bool CDuiString::operator <= (LPCTSTR str) const { return (Compare(str) <= 0); };
	bool CDuiString::operator <  (LPCTSTR str) const { return (Compare(str) <  0); };
	bool CDuiString::operator >= (LPCTSTR str) const { return (Compare(str) >= 0); };
	bool CDuiString::operator >  (LPCTSTR str) const { return (Compare(str) >  0); };

	void CDuiString::SetAt(int nIndex, TCHAR ch)
	{
		ASSERT(nIndex>=0 && nIndex<GetLength());
		m_pstr[nIndex] = ch;
	}

	int CDuiString::Compare(LPCTSTR lpsz) const 
	{ 
		return _tcscmp(m_pstr, lpsz); 
	}

	int CDuiString::CompareNoCase(LPCTSTR lpsz) const 
	{ 
		return _tcsicmp(m_pstr, lpsz); 
	}

	void CDuiString::MakeUpper() 
	{ 
		_tcsupr(m_pstr); 
	}

	void CDuiString::MakeLower() 
	{ 
		_tcslwr(m_pstr); 
	}

	CDuiString CDuiString::Left(int iLength) const
	{
		if( iLength < 0 ) iLength = 0;
		if( iLength > GetLength() ) iLength = GetLength();
		return CDuiString(m_pstr, iLength);
	}

	CDuiString CDuiString::Mid(int iPos, int iLength) const
	{
		if( iLength < 0 ) iLength = GetLength() - iPos;
		if( iPos + iLength > GetLength() ) iLength = GetLength() - iPos;
		if( iLength <= 0 ) return CDuiString();
		return CDuiString(m_pstr + iPos, iLength);
	}

	CDuiString CDuiString::Right(int iLength) const
	{
		int iPos = GetLength() - iLength;
		if( iPos < 0 ) {
			iPos = 0;
			iLength = GetLength();
		}
		return CDuiString(m_pstr + iPos, iLength);
	}

	CDuiString& CDuiString::TrimLeft()
    {
        // find first non-space character

        LPTSTR psz = this->m_pstr;

        while (::_istspace(*psz))
        {
            psz = ::CharNext(psz);
        }

        if (psz != this->m_pstr)
        {
            int iFirst = int(psz - this->m_pstr);
			Assign(psz, this->GetLength() - iFirst);
        }

        return(*this);
    }

    CDuiString& CDuiString::TrimRight()
    {
        LPTSTR psz = this->m_pstr;
		LPTSTR pszLast = NULL;

        while (*psz != 0)
        {
            if (::_istspace(*psz))
            {
                if (pszLast == NULL)
                    pszLast = psz;
            }
            else
            {
                pszLast = NULL;
            }
            psz = ::CharNext(psz);
        }

        if (pszLast != NULL)
        {
            // truncate at trailing space start
            int iLast = int(pszLast - this->GetData());

            this->SetAt(iLast, 0);
        }

        return(*this);
    }

	CDuiString& CDuiString::Trim()
    {
		TrimLeft();
		TrimRight();
		return(*this);
    }

    int CDuiString::Find(TCHAR ch, int iPos /*= 0*/) const
	{
		ASSERT(iPos>=0 && iPos<=GetLength());
		if( iPos != 0 && (iPos < 0 || iPos >= GetLength()) ) return -1;
		LPCTSTR p = _tcschr(m_pstr + iPos, ch);
		if( p == NULL ) return -1;
		return (int)(p - m_pstr);
	}

	int CDuiString::Find(LPCTSTR pstrSub, int iPos /*= 0*/) const
	{
		ASSERT(!::IsBadStringPtr(pstrSub,-1));
		ASSERT(iPos>=0 && iPos<=GetLength());
		if( iPos != 0 && (iPos < 0 || iPos > GetLength()) ) return -1;
		LPCTSTR p = _tcsstr(m_pstr + iPos, pstrSub);
		if( p == NULL ) return -1;
		return (int)(p - m_pstr);
	}

	int CDuiString::ReverseFind(TCHAR ch) const
	{
		LPCTSTR p = _tcsrchr(m_pstr, ch);
		if( p == NULL ) return -1;
		return (int)(p - m_pstr);
	}

	int CDuiString::Replace(LPCTSTR pstrFrom, LPCTSTR pstrTo)
	{
		CDuiString sTemp;
		int nCount = 0;
		int iPos = Find(pstrFrom);
		if( iPos < 0 ) return 0;
		int cchFrom = (int) _tcslen(pstrFrom);
		int cchTo = (int) _tcslen(pstrTo);
		while( iPos >= 0 ) {
			sTemp = Left(iPos);
			sTemp += pstrTo;
			sTemp += Mid(iPos + cchFrom);
			Assign(sTemp);
			iPos = Find(pstrFrom, iPos + cchTo);
			nCount++;
		}
		return nCount;
	}
    
    int CDuiString::Format(LPCTSTR pstrFormat, ...)
    {
        int nRet;
        va_list Args;

        va_start(Args, pstrFormat);
        nRet = InnerFormat(pstrFormat, Args);
        va_end(Args);

        return nRet;

    }

	int CDuiString::SmallFormat(LPCTSTR pstrFormat, ...)
	{
		CDuiString sFormat = pstrFormat;
		TCHAR szBuffer[64] = { 0 };
		va_list argList;
		va_start(argList, pstrFormat);
		int iRet = ::_vsntprintf(szBuffer, sizeof(szBuffer), sFormat, argList);
		va_end(argList);
		Assign(szBuffer);
		return iRet;
	}
	
    int CDuiString::InnerFormat(LPCTSTR pstrFormat, va_list Args)
    {
#if _MSC_VER <= 1400
        TCHAR *szBuffer = NULL;
        int size = 512, nLen, counts;
        szBuffer = (TCHAR*)malloc(size);
        ZeroMemory(szBuffer, size);
        while (TRUE){
            counts = size / sizeof(TCHAR);
            nLen = _vsntprintf (szBuffer, counts, pstrFormat, Args);
            if (nLen != -1 && nLen < counts){
                break;
            }
            if (nLen == -1){
                size *= 2;
            }else{
                size += 1 * sizeof(TCHAR);
            }

            if ((szBuffer = (TCHAR*)realloc(szBuffer, size)) != NULL){
                ZeroMemory(szBuffer, size);
            }else{
                break;
            }
        }

        Assign(szBuffer);
        free(szBuffer);
        return nLen;
#else
        int nLen, totalLen;
        TCHAR *szBuffer;
        nLen = _vsntprintf(NULL, 0, pstrFormat, Args);
        totalLen = (nLen + 1)*sizeof(TCHAR);
        szBuffer = (TCHAR*)malloc(totalLen);
        ZeroMemory(szBuffer, totalLen);
        nLen = _vsntprintf(szBuffer, nLen + 1, pstrFormat, Args);
        Assign(szBuffer);
        free(szBuffer);
        return nLen;

#endif
    }

	/////////////////////////////////////////////////////////////////////////////
	//
	//

	static UINT HashKey(LPCTSTR Key)
	{
		UINT i = 0;
		SIZE_T len = _tcslen(Key);
		while( len-- > 0 ) i = (i << 5) + i + Key[len];
		return i;
	}

	static UINT HashKey(const CDuiString& Key)
	{
		return HashKey((LPCTSTR)Key);
	};

	CStdStringPtrMap::CStdStringPtrMap(int nSize) : m_nCount(0)
	{
		if( nSize < 16 ) nSize = 16;
		m_nBuckets = nSize;
		m_aT = new TITEM*[nSize];
		memset(m_aT, 0, nSize * sizeof(TITEM*));
	}

	CStdStringPtrMap::~CStdStringPtrMap()
	{
		if( m_aT ) {
			int len = m_nBuckets;
			while( len-- ) {
				TITEM* pItem = m_aT[len];
				while( pItem ) {
					TITEM* pKill = pItem;
					pItem = pItem->pNext;
					delete pKill;
				}
			}
			delete [] m_aT;
			m_aT = NULL;
		}
	}

	void CStdStringPtrMap::RemoveAll()
	{
		this->Resize(m_nBuckets);
	}

	void CStdStringPtrMap::Resize(int nSize)
	{
		if( m_aT ) {
			int len = m_nBuckets;
			while( len-- ) {
				TITEM* pItem = m_aT[len];
				while( pItem ) {
					TITEM* pKill = pItem;
					pItem = pItem->pNext;
					delete pKill;
				}
			}
			delete [] m_aT;
			m_aT = NULL;
		}

		if( nSize < 0 ) nSize = 0;
		if( nSize > 0 ) {
			m_aT = new TITEM*[nSize];
			memset(m_aT, 0, nSize * sizeof(TITEM*));
		} 
		m_nBuckets = nSize;
		m_nCount = 0;
	}

	LPVOID CStdStringPtrMap::Find(LPCTSTR key, bool optimize) const
	{
		if( m_nBuckets == 0 || GetSize() == 0 ) return NULL;

		UINT slot = HashKey(key) % m_nBuckets;
		for( TITEM* pItem = m_aT[slot]; pItem; pItem = pItem->pNext ) {
			if( pItem->Key == key ) {
				if (optimize && pItem != m_aT[slot]) {
					if (pItem->pNext) {
						pItem->pNext->pPrev = pItem->pPrev;
					}
					pItem->pPrev->pNext = pItem->pNext;
					pItem->pPrev = NULL;
					pItem->pNext = m_aT[slot];
					pItem->pNext->pPrev = pItem;
					//将item移动至链条头部
					m_aT[slot] = pItem;
				}
				return pItem->Data;
			}        
		}

		return NULL;
	}

	bool CStdStringPtrMap::Insert(LPCTSTR key, LPVOID pData)
	{
		if( m_nBuckets == 0 ) return false;
		if( Find(key) ) return false;

		// Add first in bucket
		UINT slot = HashKey(key) % m_nBuckets;
		TITEM* pItem = new TITEM;
		pItem->Key = key;
		pItem->Data = pData;
		pItem->pPrev = NULL;
		pItem->pNext = m_aT[slot];
		if (pItem->pNext)
			pItem->pNext->pPrev = pItem;
		m_aT[slot] = pItem;
		m_nCount++;
		return true;
	}

	LPVOID CStdStringPtrMap::Set(LPCTSTR key, LPVOID pData)
	{
		if( m_nBuckets == 0 ) return pData;

		if (GetSize()>0) {
			UINT slot = HashKey(key) % m_nBuckets;
			// Modify existing item
			for( TITEM* pItem = m_aT[slot]; pItem; pItem = pItem->pNext ) {
				if( pItem->Key == key ) {
					LPVOID pOldData = pItem->Data;
					pItem->Data = pData;
					return pOldData;
				}
			}
		}

		Insert(key, pData);
		return NULL;
	}

	bool CStdStringPtrMap::Remove(LPCTSTR key)
	{
		if( m_nBuckets == 0 || GetSize() == 0 ) return false;

		UINT slot = HashKey(key) % m_nBuckets;
		TITEM** ppItem = &m_aT[slot];
		while( *ppItem ) {
			if( (*ppItem)->Key == key ) {
				TITEM* pKill = *ppItem;
				*ppItem = (*ppItem)->pNext;
				if (*ppItem)
					(*ppItem)->pPrev = pKill->pPrev;
				delete pKill;
				m_nCount--;
				return true;
			}
			ppItem = &((*ppItem)->pNext);
		}

		return false;
	}

	int CStdStringPtrMap::GetSize() const
	{
#if 0//def _DEBUG
		int nCount = 0;
		int len = m_nBuckets;
		while( len-- ) {
			for( const TITEM* pItem = m_aT[len]; pItem; pItem = pItem->pNext ) nCount++;
		}
		ASSERT(m_nCount==nCount);
#endif
		return m_nCount;
	}

	LPCTSTR CStdStringPtrMap::GetAt(int iIndex) const
	{
		if( m_nBuckets == 0 || GetSize() == 0 ) return false;

		int pos = 0;
		int len = m_nBuckets;
		while( len-- ) {
			for( TITEM* pItem = m_aT[len]; pItem; pItem = pItem->pNext ) {
				if( pos++ == iIndex ) {
					return pItem->Key.GetData();
				}
			}
		}

		return NULL;
	}

	LPCTSTR CStdStringPtrMap::operator[] (int nIndex) const
	{
		return GetAt(nIndex);
	}


	/////////////////////////////////////////////////////////////////////////////////////
	//
	//

	CWaitCursor::CWaitCursor()
	{
		m_hOrigCursor = ::SetCursor(::LoadCursor(NULL, IDC_WAIT));
	}

	CWaitCursor::~CWaitCursor()
	{
		::SetCursor(m_hOrigCursor);
	}


	/////////////////////////////////////////////////////////////////////////////////////
	//

	namespace {
		struct CssNamedColor { LPCTSTR name; DWORD color; };

		// CSS Color Module Level 3 命名色（含 grey 别名）；值为 RRGGBBAA
		static const CssNamedColor kCssNamedColors[] = {
			{ _T("aliceblue"), 0xF0F8FFFF }, { _T("antiquewhite"), 0xFAEBD7FF },
			{ _T("aqua"), 0x00FFFFFF }, { _T("aquamarine"), 0x7FFFD4FF },
			{ _T("azure"), 0xF0FFFFFF }, { _T("beige"), 0xF5F5DCFF },
			{ _T("bisque"), 0xFFE4C4FF }, { _T("black"), 0x000000FF },
			{ _T("blanchedalmond"), 0xFFEBCDFF }, { _T("blue"), 0x0000FFFF },
			{ _T("blueviolet"), 0x8A2BE2FF }, { _T("brown"), 0xA52A2AFF },
			{ _T("burlywood"), 0xDEB887FF }, { _T("cadetblue"), 0x5F9EA0FF },
			{ _T("chartreuse"), 0x7FFF00FF }, { _T("chocolate"), 0xD2691EFF },
			{ _T("coral"), 0xFF7F50FF }, { _T("cornflowerblue"), 0x6495EDFF },
			{ _T("cornsilk"), 0xFFF8DCFF }, { _T("crimson"), 0xDC143CFF },
			{ _T("cyan"), 0x00FFFFFF }, { _T("darkblue"), 0x00008BFF },
			{ _T("darkcyan"), 0x008B8BFF }, { _T("darkgoldenrod"), 0xB8860BFF },
			{ _T("darkgray"), 0xA9A9A9FF }, { _T("darkgreen"), 0x006400FF },
			{ _T("darkgrey"), 0xA9A9A9FF }, { _T("darkkhaki"), 0xBDB76BFF },
			{ _T("darkmagenta"), 0x8B008BFF }, { _T("darkolivegreen"), 0x556B2FFF },
			{ _T("darkorange"), 0xFF8C00FF }, { _T("darkorchid"), 0x9932CCFF },
			{ _T("darkred"), 0x8B0000FF }, { _T("darksalmon"), 0xE9967AFF },
			{ _T("darkseagreen"), 0x8FBC8FFF }, { _T("darkslateblue"), 0x483D8BFF },
			{ _T("darkslategray"), 0x2F4F4FFF }, { _T("darkslategrey"), 0x2F4F4FFF },
			{ _T("darkturquoise"), 0x00CED1FF }, { _T("darkviolet"), 0x9400D3FF },
			{ _T("deeppink"), 0xFF1493FF }, { _T("deepskyblue"), 0x00BFFFFF },
			{ _T("dimgray"), 0x696969FF }, { _T("dimgrey"), 0x696969FF },
			{ _T("dodgerblue"), 0x1E90FFFF }, { _T("firebrick"), 0xB22222FF },
			{ _T("floralwhite"), 0xFFFAF0FF }, { _T("forestgreen"), 0x228B22FF },
			{ _T("fuchsia"), 0xFF00FFFF }, { _T("gainsboro"), 0xDCDCDCFF },
			{ _T("ghostwhite"), 0xF8F8FFFF }, { _T("gold"), 0xFFD700FF },
			{ _T("goldenrod"), 0xDAA520FF }, { _T("gray"), 0x808080FF },
			{ _T("green"), 0x008000FF }, { _T("greenyellow"), 0xADFF2FFF },
			{ _T("grey"), 0x808080FF }, { _T("honeydew"), 0xF0FFF0FF },
			{ _T("hotpink"), 0xFF69B4FF }, { _T("indianred"), 0xCD5C5CFF },
			{ _T("indigo"), 0x4B0082FF }, { _T("ivory"), 0xFFFFF0FF },
			{ _T("khaki"), 0xF0E68CFF }, { _T("lavender"), 0xE6E6FAFF },
			{ _T("lavenderblush"), 0xFFF0F5FF }, { _T("lawngreen"), 0x7CFC00FF },
			{ _T("lemonchiffon"), 0xFFFACDFF }, { _T("lightblue"), 0xADD8E6FF },
			{ _T("lightcoral"), 0xF08080FF }, { _T("lightcyan"), 0xE0FFFFFF },
			{ _T("lightgoldenrodyellow"), 0xFAFAD2FF }, { _T("lightgray"), 0xD3D3D3FF },
			{ _T("lightgreen"), 0x90EE90FF }, { _T("lightgrey"), 0xD3D3D3FF },
			{ _T("lightpink"), 0xFFB6C1FF }, { _T("lightsalmon"), 0xFFA07AFF },
			{ _T("lightseagreen"), 0x20B2AAFF }, { _T("lightskyblue"), 0x87CEFAFF },
			{ _T("lightslategray"), 0x778899FF }, { _T("lightslategrey"), 0x778899FF },
			{ _T("lightsteelblue"), 0xB0C4DEFF }, { _T("lightyellow"), 0xFFFFE0FF },
			{ _T("lime"), 0x00FF00FF }, { _T("limegreen"), 0x32CD32FF },
			{ _T("linen"), 0xFAF0E6FF }, { _T("magenta"), 0xFF00FFFF },
			{ _T("maroon"), 0x800000FF }, { _T("mediumaquamarine"), 0x66CDAAFF },
			{ _T("mediumblue"), 0x0000CDFF }, { _T("mediumorchid"), 0xBA55D3FF },
			{ _T("mediumpurple"), 0x9370DBFF }, { _T("mediumseagreen"), 0x3CB371FF },
			{ _T("mediumslateblue"), 0x7B68EEFF }, { _T("mediumspringgreen"), 0x00FA9AFF },
			{ _T("mediumturquoise"), 0x48D1CCFF }, { _T("mediumvioletred"), 0xC71585FF },
			{ _T("midnightblue"), 0x191970FF }, { _T("mintcream"), 0xF5FFFAFF },
			{ _T("mistyrose"), 0xFFE4E1FF }, { _T("moccasin"), 0xFFE4B5FF },
			{ _T("navajowhite"), 0xFFDEADFF }, { _T("navy"), 0x000080FF },
			{ _T("oldlace"), 0xFDF5E6FF }, { _T("olive"), 0x808000FF },
			{ _T("olivedrab"), 0x6B8E23FF }, { _T("orange"), 0xFFA500FF },
			{ _T("orangered"), 0xFF4500FF }, { _T("orchid"), 0xDA70D6FF },
			{ _T("palegoldenrod"), 0xEEE8AAFF }, { _T("palegreen"), 0x98FB98FF },
			{ _T("paleturquoise"), 0xAFEEEEFF }, { _T("palevioletred"), 0xDB7093FF },
			{ _T("papayawhip"), 0xFFEFD5FF }, { _T("peachpuff"), 0xFFDAB9FF },
			{ _T("peru"), 0xCD853FFF }, { _T("pink"), 0xFFC0CBFF },
			{ _T("plum"), 0xDDA0DDFF }, { _T("powderblue"), 0xB0E0E6FF },
			{ _T("purple"), 0x800080FF }, { _T("rebeccapurple"), 0x663399FF },
			{ _T("red"), 0xFF0000FF }, { _T("rosybrown"), 0xBC8F8FFF },
			{ _T("royalblue"), 0x4169E1FF }, { _T("saddlebrown"), 0x8B4513FF },
			{ _T("salmon"), 0xFA8072FF }, { _T("sandybrown"), 0xF4A460FF },
			{ _T("seagreen"), 0x2E8B57FF }, { _T("seashell"), 0xFFF5EEFF },
			{ _T("sienna"), 0xA0522DFF }, { _T("silver"), 0xC0C0C0FF },
			{ _T("skyblue"), 0x87CEEBFF }, { _T("slateblue"), 0x6A5ACDFF },
			{ _T("slategray"), 0x708090FF }, { _T("slategrey"), 0x708090FF },
			{ _T("snow"), 0xFFFAFAFF }, { _T("springgreen"), 0x00FF7FFF },
			{ _T("steelblue"), 0x4682B4FF }, { _T("tan"), 0xD2B48CFF },
			{ _T("teal"), 0x008080FF }, { _T("thistle"), 0xD8BFD8FF },
			{ _T("tomato"), 0xFF6347FF }, { _T("transparent"), 0x00000000 },
			{ _T("turquoise"), 0x40E0D0FF }, { _T("violet"), 0xEE82EEFF },
			{ _T("wheat"), 0xF5DEB3FF }, { _T("white"), 0xFFFFFFFF },
			{ _T("whitesmoke"), 0xF5F5F5FF }, { _T("yellow"), 0xFFFF00FF },
			{ _T("yellowgreen"), 0x9ACD32FF },
		};

		bool ParseHexColorDigits(LPCTSTR tok, DWORD& dwColor)
		{
			if( tok == NULL || *tok == _T('\0') ) return false;
			size_t len = _tcslen(tok);
			for( size_t i = 0; i < len; ++i ) {
				TCHAR c = tok[i];
				bool hex = (c >= _T('0') && c <= _T('9'))
					|| (c >= _T('a') && c <= _T('f'))
					|| (c >= _T('A') && c <= _T('F'));
				if( !hex ) return false;
			}
			if( len == 3 ) {
				auto nib = [](TCHAR c) -> unsigned {
					if( c >= _T('0') && c <= _T('9') ) return (unsigned)(c - _T('0'));
					if( c >= _T('a') && c <= _T('f') ) return (unsigned)(c - _T('a') + 10);
					return (unsigned)(c - _T('A') + 10);
				};
				unsigned r = nib(tok[0]), g = nib(tok[1]), b = nib(tok[2]);
				dwColor = DuiColorFromRGB((BYTE)(r * 17u), (BYTE)(g * 17u), (BYTE)(b * 17u), 0xFF);
				return true;
			}
			if( len == 4 ) {
				// CSS #RGBA
				auto nib = [](TCHAR c) -> unsigned {
					if( c >= _T('0') && c <= _T('9') ) return (unsigned)(c - _T('0'));
					if( c >= _T('a') && c <= _T('f') ) return (unsigned)(c - _T('a') + 10);
					return (unsigned)(c - _T('A') + 10);
				};
				unsigned r = nib(tok[0]) * 17u, g = nib(tok[1]) * 17u;
				unsigned b = nib(tok[2]) * 17u, a = nib(tok[3]) * 17u;
				dwColor = DuiColorFromRGB((BYTE)r, (BYTE)g, (BYTE)b, (BYTE)a);
				return true;
			}
			if( len == 6 ) {
				LPTSTR pEnd = NULL;
				DWORD v = _tcstoul(tok, &pEnd, 16);
				if( pEnd == tok ) return false;
				dwColor = (v << 8) | 0xFFu; // RRGGBB → RRGGBBAA
				return true;
			}
			if( len == 8 ) {
				// CSS #RRGGBBAA / 0xRRGGBBAA（即内部格式）
				LPTSTR pEnd = NULL;
				dwColor = _tcstoul(tok, &pEnd, 16);
				return pEnd != tok;
			}
			return false;
		}
	} // namespace

	static int ClampByte(int v)
	{
		if( v < 0 ) return 0;
		if( v > 255 ) return 255;
		return v;
	}

	static bool ParseCssColorChannel(LPCTSTR& p, int& nOut)
	{
		while( *p == _T(' ') || *p == _T('\t') || *p == _T(',') || *p == _T('/') ) ++p;
		if( *p == _T('\0') || *p == _T(')') ) return false;
		LPTSTR pEnd = NULL;
		double v = _tcstod(p, &pEnd);
		if( pEnd == p ) return false;
		p = pEnd;
		while( *p == _T(' ') || *p == _T('\t') ) ++p;
		if( *p == _T('%') ) {
			++p;
			nOut = ClampByte((int)(v * 255.0 / 100.0 + (v >= 0 ? 0.5 : -0.5)));
			return true;
		}
		nOut = ClampByte((int)(v + (v >= 0 ? 0.5 : -0.5)));
		return true;
	}

	static bool ParseCssAlphaChannel(LPCTSTR& p, int& nOut)
	{
		while( *p == _T(' ') || *p == _T('\t') || *p == _T(',') || *p == _T('/') ) ++p;
		if( *p == _T('\0') || *p == _T(')') ) return false;
		LPTSTR pEnd = NULL;
		double v = _tcstod(p, &pEnd);
		if( pEnd == p ) return false;
		p = pEnd;
		while( *p == _T(' ') || *p == _T('\t') ) ++p;
		if( *p == _T('%') ) {
			++p;
			nOut = ClampByte((int)(v * 255.0 / 100.0 + (v >= 0 ? 0.5 : -0.5)));
			return true;
		}
		// CSS：通常 0–1；>1 则按 0–255 字节兼容
		if( v >= 0.0 && v <= 1.0 )
			nOut = ClampByte((int)(v * 255.0 + 0.5));
		else
			nOut = ClampByte((int)(v + (v >= 0 ? 0.5 : -0.5)));
		return true;
	}

	static bool ParseRgbColorString(LPCTSTR pstrColor, DWORD& dwColor)
	{
		if( pstrColor == NULL ) return false;
		bool bRgba = (_tcsnicmp(pstrColor, _T("rgba"), 4) == 0);
		bool bRgb = bRgba || (_tcsnicmp(pstrColor, _T("rgb"), 3) == 0);
		if( !bRgb ) return false;
		LPCTSTR p = pstrColor + (bRgba ? 4 : 3);
		while( *p == _T(' ') || *p == _T('\t') ) ++p;
		if( *p != _T('(') ) return false;
		++p;
		int r = 0, g = 0, b = 0, a = 255;
		if( !ParseCssColorChannel(p, r) ) return false;
		if( !ParseCssColorChannel(p, g) ) return false;
		if( !ParseCssColorChannel(p, b) ) return false;
		while( *p == _T(' ') || *p == _T('\t') ) ++p;
		if( *p != _T(')') ) {
			if( !ParseCssAlphaChannel(p, a) ) return false;
			while( *p == _T(' ') || *p == _T('\t') ) ++p;
		}
		if( *p != _T(')') ) return false;
		dwColor = DuiColorFromRGB((BYTE)r, (BYTE)g, (BYTE)b, (BYTE)a);
		return true;
	}

	static float CssHueToRgb(float p, float q, float t)
	{
		if( t < 0.0f ) t += 1.0f;
		if( t > 1.0f ) t -= 1.0f;
		if( t < 1.0f / 6.0f ) return p + (q - p) * 6.0f * t;
		if( t < 0.5f ) return q;
		if( t < 2.0f / 3.0f ) return p + (q - p) * (2.0f / 3.0f - t) * 6.0f;
		return p;
	}

	static bool ParseHslColorString(LPCTSTR pstrColor, DWORD& dwColor)
	{
		if( pstrColor == NULL ) return false;
		bool bHsla = (_tcsnicmp(pstrColor, _T("hsla"), 4) == 0);
		bool bHsl = bHsla || (_tcsnicmp(pstrColor, _T("hsl"), 3) == 0);
		if( !bHsl ) return false;
		LPCTSTR p = pstrColor + (bHsla ? 4 : 3);
		while( *p == _T(' ') || *p == _T('\t') ) ++p;
		if( *p != _T('(') ) return false;
		++p;
		while( *p == _T(' ') || *p == _T('\t') ) ++p;
		LPTSTR pEnd = NULL;
		double hDeg = _tcstod(p, &pEnd);
		if( pEnd == p ) return false;
		p = pEnd;
		while( *p == _T(' ') || *p == _T('\t') || *p == _T(',') ) ++p;
		int s = 0, l = 0, a = 255;
		if( !ParseCssColorChannel(p, s) ) return false; // 期望带 %
		if( !ParseCssColorChannel(p, l) ) return false;
		while( *p == _T(' ') || *p == _T('\t') ) ++p;
		if( *p != _T(')') ) {
			if( !ParseCssAlphaChannel(p, a) ) return false;
			while( *p == _T(' ') || *p == _T('\t') ) ++p;
		}
		if( *p != _T(')') ) return false;

		float H = (float)(hDeg / 360.0);
		while( H < 0.0f ) H += 1.0f;
		while( H >= 1.0f ) H -= 1.0f;
		float S = (float)s / 255.0f;
		float L = (float)l / 255.0f;
		BYTE r, g, b;
		if( S <= 0.0f ) {
			r = g = b = (BYTE)ClampByte((int)(L * 255.0f + 0.5f));
		}
		else {
			float q = L < 0.5f ? L * (1.0f + S) : (L + S - L * S);
			float pv = 2.0f * L - q;
			r = (BYTE)ClampByte((int)(CssHueToRgb(pv, q, H + 1.0f / 3.0f) * 255.0f + 0.5f));
			g = (BYTE)ClampByte((int)(CssHueToRgb(pv, q, H) * 255.0f + 0.5f));
			b = (BYTE)ClampByte((int)(CssHueToRgb(pv, q, H - 1.0f / 3.0f) * 255.0f + 0.5f));
		}
		dwColor = DuiColorFromRGB(r, g, b, (BYTE)a);
		return true;
	}

	bool ParseCssOpacity(LPCTSTR pstrValue, BYTE& nOpacity)
	{
		nOpacity = 255;
		if( pstrValue == NULL ) return false;
		while( *pstrValue == _T(' ') || *pstrValue == _T('\t') ) ++pstrValue;
		if( *pstrValue == _T('\0') ) return false;
		LPTSTR pEnd = NULL;
		double v = _tcstod(pstrValue, &pEnd);
		if( pEnd == pstrValue ) return false;
		while( *pEnd == _T(' ') || *pEnd == _T('\t') ) ++pEnd;
		if( *pEnd == _T('%') )
			nOpacity = (BYTE)ClampByte((int)(v * 255.0 / 100.0 + (v >= 0 ? 0.5 : -0.5)));
		else if( v >= 0.0 && v <= 1.0 )
			nOpacity = (BYTE)ClampByte((int)(v * 255.0 + 0.5));
		else
			nOpacity = (BYTE)ClampByte((int)(v + (v >= 0 ? 0.5 : -0.5)));
		return true;
	}

	bool ParseCssFontWeightBold(LPCTSTR pstrValue, bool& bBold)
	{
		bBold = false;
		if( pstrValue == NULL || *pstrValue == _T('\0') ) return false;
		if( _tcsicmp(pstrValue, _T("bold")) == 0 || _tcsicmp(pstrValue, _T("bolder")) == 0
			|| _tcsicmp(pstrValue, _T("true")) == 0 ) {
			bBold = true;
			return true;
		}
		if( _tcsicmp(pstrValue, _T("normal")) == 0 || _tcsicmp(pstrValue, _T("lighter")) == 0
			|| _tcsicmp(pstrValue, _T("false")) == 0 ) {
			bBold = false;
			return true;
		}
		LPTSTR pEnd = NULL;
		long v = _tcstol(pstrValue, &pEnd, 10);
		if( pEnd == pstrValue ) return false;
		bBold = (v >= 600);
		return true;
	}

	bool ParseCssFontStyleItalic(LPCTSTR pstrValue, bool& bItalic)
	{
		bItalic = false;
		if( pstrValue == NULL || *pstrValue == _T('\0') ) return false;
		if( _tcsicmp(pstrValue, _T("italic")) == 0 || _tcsicmp(pstrValue, _T("oblique")) == 0
			|| _tcsicmp(pstrValue, _T("true")) == 0 ) {
			bItalic = true;
			return true;
		}
		if( _tcsicmp(pstrValue, _T("normal")) == 0 || _tcsicmp(pstrValue, _T("false")) == 0 ) {
			bItalic = false;
			return true;
		}
		return false;
	}

	bool ParseCssTextDecoration(LPCTSTR pstrValue, bool& bUnderline, bool& bStrikeout)
	{
		if( pstrValue == NULL || *pstrValue == _T('\0') ) return false;
		if( _tcsicmp(pstrValue, _T("none")) == 0 ) {
			bUnderline = false;
			bStrikeout = false;
			return true;
		}
		bool any = false;
		bool u = bUnderline, s = bStrikeout;
		LPCTSTR p = pstrValue;
		while( *p != _T('\0') ) {
			while( *p == _T(' ') || *p == _T('\t') || *p == _T(',') ) ++p;
			if( *p == _T('\0') ) break;
			TCHAR tok[64] = { 0 };
			int i = 0;
			while( *p != _T('\0') && *p != _T(' ') && *p != _T('\t') && *p != _T(',') && i < 63 )
				tok[i++] = *p++;
			tok[i] = _T('\0');
			if( _tcsicmp(tok, _T("underline")) == 0 ) { u = true; any = true; }
			else if( _tcsicmp(tok, _T("line-through")) == 0 || _tcsicmp(tok, _T("strikeout")) == 0
				|| _tcsicmp(tok, _T("strikethrough")) == 0 ) { s = true; any = true; }
			else if( _tcsicmp(tok, _T("none")) == 0 ) { u = false; s = false; any = true; }
		}
		if( !any ) return false;
		bUnderline = u;
		bStrikeout = s;
		return true;
	}

	bool ParseCssPointerEventsEnabled(LPCTSTR pstrValue, bool& bEnabled)
	{
		bEnabled = true;
		if( pstrValue == NULL || *pstrValue == _T('\0') ) return false;
		if( _tcsicmp(pstrValue, _T("none")) == 0 || _tcsicmp(pstrValue, _T("false")) == 0
			|| _tcscmp(pstrValue, _T("0")) == 0 ) {
			bEnabled = false;
			return true;
		}
		if( _tcsicmp(pstrValue, _T("auto")) == 0 || _tcsicmp(pstrValue, _T("true")) == 0
			|| _tcsicmp(pstrValue, _T("visiblepainted")) == 0 || _tcsicmp(pstrValue, _T("visiblefill")) == 0
			|| _tcsicmp(pstrValue, _T("visiblestroke")) == 0 || _tcsicmp(pstrValue, _T("visible")) == 0
			|| _tcsicmp(pstrValue, _T("painted")) == 0 || _tcsicmp(pstrValue, _T("fill")) == 0
			|| _tcsicmp(pstrValue, _T("stroke")) == 0 || _tcsicmp(pstrValue, _T("all")) == 0
			|| _tcscmp(pstrValue, _T("1")) == 0 ) {
			bEnabled = true;
			return true;
		}
		return false;
	}

	bool ParseColorString(LPCTSTR pstrColor, DWORD& dwColor)
	{
		if( pstrColor == NULL ) return false;
		while( *pstrColor == _T(' ') || *pstrColor == _T('\t') ) ++pstrColor;
		if( *pstrColor == _T('\0') ) return false;

		if( ParseRgbColorString(pstrColor, dwColor) )
			return true;
		if( ParseHslColorString(pstrColor, dwColor) )
			return true;

		// 先匹配命名色（避免 "red" 等被误解析）
		for( size_t i = 0; i < sizeof(kCssNamedColors) / sizeof(kCssNamedColors[0]); ++i ) {
			if( _tcsicmp(pstrColor, kCssNamedColors[i].name) == 0 ) {
				dwColor = kCssNamedColors[i].color;
				return true;
			}
		}

		LPCTSTR tok = pstrColor;
		if( *tok == _T('#') ) ++tok;
		else if( _tcsnicmp(tok, _T("0x"), 2) == 0 ) tok += 2;
		// # / 0x / 纯 hex：与内部 DWORD 均为 CSS RRGGBBAA
		return ParseHexColorDigits(tok, dwColor);
	}

	bool ParseColorStringToken(LPCTSTR& pstrInOut, DWORD& dwColor)
	{
		if( pstrInOut == NULL ) return false;
		LPCTSTR p = pstrInOut;
		while( *p == _T(' ') || *p == _T('\t') ) ++p;
		if( *p == _T('\0') ) return false;

		CDuiString sTok;
		if( _tcsnicmp(p, _T("rgb"), 3) == 0 || _tcsnicmp(p, _T("hsl"), 3) == 0 ) {
			LPCTSTR pOpen = _tcschr(p, _T('('));
			if( pOpen == NULL ) return false;
			LPCTSTR pClose = _tcschr(pOpen, _T(')'));
			if( pClose == NULL ) return false;
			sTok.Assign(p, (int)(pClose - p + 1));
			p = pClose + 1;
		}
		else if( *p == _T('#') ) {
			LPCTSTR pStart = p++;
			while( (*p >= _T('0') && *p <= _T('9'))
				|| (*p >= _T('a') && *p <= _T('f'))
				|| (*p >= _T('A') && *p <= _T('F')) ) ++p;
			sTok.Assign(pStart, (int)(p - pStart));
		}
		else if( _tcsnicmp(p, _T("0x"), 2) == 0 ) {
			LPCTSTR pStart = p;
			p += 2;
			while( (*p >= _T('0') && *p <= _T('9'))
				|| (*p >= _T('a') && *p <= _T('f'))
				|| (*p >= _T('A') && *p <= _T('F')) ) ++p;
			sTok.Assign(pStart, (int)(p - pStart));
		}
		else if( (*p >= _T('a') && *p <= _T('z')) || (*p >= _T('A') && *p <= _T('Z')) ) {
			LPCTSTR pName = p;
			while( (*p >= _T('a') && *p <= _T('z')) || (*p >= _T('A') && *p <= _T('Z')) ) ++p;
			sTok.Assign(pName, (int)(p - pName));
		}
		else {
			return false;
		}

		if( !ParseColorString(sTok.GetData(), dwColor) ) return false;
		pstrInOut = p;
		return true;
	}

	static void SkipCssLengthSuffix(LPTSTR& p)
	{
		if( p == NULL ) return;
		while( *p == _T(' ') || *p == _T('\t') ) ++p;
		if( _tcsnicmp(p, _T("px"), 2) == 0 ) p += 2;
		while( *p == _T(' ') || *p == _T('\t') ) ++p;
	}

	bool ParseCssBox(LPCTSTR pstrValue, CDuiBox& box)
	{
		box.Empty();
		if( pstrValue == NULL ) return false;
		while( *pstrValue == _T(' ') || *pstrValue == _T('\t') ) ++pstrValue;
		if( *pstrValue == _T('\0') ) return false;

		int v[4] = { 0, 0, 0, 0 };
		LPTSTR pstr = NULL;
		v[0] = (int)_tcstol(pstrValue, &pstr, 10);
		if( pstr == pstrValue ) return false;
		SkipCssLengthSuffix(pstr);
		int n = 1;
		while( pstr && (*pstr == _T(',') || *pstr == _T(' ')) ) {
			++pstr;
			while( *pstr == _T(' ') ) ++pstr;
			if( *pstr == _T('\0') || n >= 4 ) break;
			LPTSTR pNext = NULL;
			v[n] = (int)_tcstol(pstr, &pNext, 10);
			if( pNext == pstr ) break;
			pstr = pNext;
			SkipCssLengthSuffix(pstr);
			++n;
		}

		if( n >= 4 ) { box.top = v[0]; box.right = v[1]; box.bottom = v[2]; box.left = v[3]; }
		else if( n == 3 ) { box.top = v[0]; box.right = v[1]; box.bottom = v[2]; box.left = v[1]; }
		else if( n == 2 ) { box.top = v[0]; box.right = v[1]; box.bottom = v[0]; box.left = v[1]; }
		else { box.top = box.right = box.bottom = box.left = v[0]; }
		return true;
	}

	bool ParseCssBoxToRect(LPCTSTR pstrValue, RECT& rc)
	{
		CDuiBox box;
		if( !ParseCssBox(pstrValue, box) ) {
			::ZeroMemory(&rc, sizeof(RECT));
			return false;
		}
		rc = box.ToRect();
		return true;
	}

	bool ParseBorderRadiusValue(LPCTSTR pstrValue, SIZE& szRound)
	{
		szRound.cx = szRound.cy = 0;
		if( pstrValue == NULL ) return false;
		while( *pstrValue == _T(' ') || *pstrValue == _T('\t') ) ++pstrValue;
		if( *pstrValue == _T('\0') ) return false;

		LPTSTR pstr = NULL;
		long v0 = _tcstol(pstrValue, &pstr, 10);
		if( pstr == pstrValue ) return false;
		SkipCssLengthSuffix(pstr);
		if( *pstr == _T('\0') ) {
			szRound.cx = szRound.cy = (int)v0;
			return true;
		}
		if( *pstr == _T(',') ) ++pstr;
		while( *pstr == _T(' ') || *pstr == _T('\t') ) ++pstr;
		LPTSTR pNext = NULL;
		long v1 = _tcstol(pstr, &pNext, 10);
		if( pNext == pstr ) return false;
		SkipCssLengthSuffix(pNext);
		szRound.cx = (int)v0;
		szRound.cy = (int)v1;
		return true;
	}

	bool ParseCssUrlImage(LPCTSTR pstrValue, CDuiString& sPath)
	{
		sPath.Empty();
		if( pstrValue == NULL ) return false;
		while( *pstrValue == _T(' ') || *pstrValue == _T('\t') ) ++pstrValue;
		if( _tcsnicmp(pstrValue, _T("url"), 3) != 0 ) return false;
		LPCTSTR p = pstrValue + 3;
		while( *p == _T(' ') || *p == _T('\t') ) ++p;
		if( *p != _T('(') ) return false;
		++p;
		while( *p == _T(' ') || *p == _T('\t') ) ++p;
		TCHAR q = 0;
		if( *p == _T('\'') || *p == _T('"') ) { q = *p; ++p; }
		CDuiString s;
		while( *p != _T('\0') ) {
			if( q != 0 ) {
				if( *p == q ) { ++p; break; }
			}
			else {
				if( *p == _T(')') ) break;
			}
			s += *p++;
		}
		while( *p == _T(' ') || *p == _T('\t') ) ++p;
		if( *p != _T(')') ) return false;
		// 去掉路径两侧空白
		LPCTSTR b = s.GetData();
		if( b == NULL ) return false;
		while( *b == _T(' ') || *b == _T('\t') ) ++b;
		int nLen = (int)_tcslen(b);
		while( nLen > 0 && (b[nLen - 1] == _T(' ') || b[nLen - 1] == _T('\t')) ) --nLen;
		if( nLen <= 0 ) return false;
		sPath.Assign(b, nLen);
		return true;
	}

	bool ParseCssOverflowEnablesScroll(LPCTSTR pstrValue, bool& bEnable)
	{
		bEnable = false;
		if( pstrValue == NULL ) return false;
		while( *pstrValue == _T(' ') || *pstrValue == _T('\t') ) ++pstrValue;
		if( *pstrValue == _T('\0') ) return false;
		if( _tcsicmp(pstrValue, _T("auto")) == 0
			|| _tcsicmp(pstrValue, _T("scroll")) == 0
			|| _tcsicmp(pstrValue, _T("overlay")) == 0
			|| _tcsicmp(pstrValue, _T("true")) == 0
			|| _tcscmp(pstrValue, _T("1")) == 0 ) {
			bEnable = true;
			return true;
		}
		if( _tcsicmp(pstrValue, _T("hidden")) == 0
			|| _tcsicmp(pstrValue, _T("clip")) == 0
			|| _tcsicmp(pstrValue, _T("visible")) == 0
			|| _tcsicmp(pstrValue, _T("false")) == 0
			|| _tcscmp(pstrValue, _T("0")) == 0 ) {
			bEnable = false;
			return true;
		}
		return false;
	}

	bool ParseCssOverflowShorthand(LPCTSTR pstrValue, bool& bEnableX, bool& bEnableY)
	{
		bEnableX = bEnableY = false;
		if( pstrValue == NULL ) return false;
		while( *pstrValue == _T(' ') || *pstrValue == _T('\t') ) ++pstrValue;
		if( *pstrValue == _T('\0') ) return false;

		TCHAR sz1[64] = { 0 };
		TCHAR sz2[64] = { 0 };
		int n = 0;
		LPCTSTR p = pstrValue;
		while( *p != _T('\0') && n < 2 ) {
			while( *p == _T(' ') || *p == _T('\t') || *p == _T(',') ) ++p;
			if( *p == _T('\0') ) break;
			LPTSTR dest = (n == 0) ? sz1 : sz2;
			int i = 0;
			while( *p != _T('\0') && *p != _T(' ') && *p != _T('\t') && *p != _T(',') && i < 63 )
				dest[i++] = *p++;
			dest[i] = _T('\0');
			++n;
		}
		if( n == 0 ) return false;
		bool en1 = false;
		if( !ParseCssOverflowEnablesScroll(sz1, en1) ) return false;
		if( n == 1 ) {
			bEnableX = bEnableY = en1;
			return true;
		}
		bool en2 = false;
		if( !ParseCssOverflowEnablesScroll(sz2, en2) ) return false;
		bEnableX = en1;
		bEnableY = en2;
		return true;
	}

} // namespace DuiLib