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
		struct CssNamedColor { LPCTSTR name; DWORD argb; };

		// CSS Color Module Level 3 命名色（含 grey 别名）；大小写不敏感匹配
		static const CssNamedColor kCssNamedColors[] = {
			{ _T("aliceblue"), 0xFFF0F8FF }, { _T("antiquewhite"), 0xFFFAEBD7 },
			{ _T("aqua"), 0xFF00FFFF }, { _T("aquamarine"), 0xFF7FFFD4 },
			{ _T("azure"), 0xFFF0FFFF }, { _T("beige"), 0xFFF5F5DC },
			{ _T("bisque"), 0xFFFFE4C4 }, { _T("black"), 0xFF000000 },
			{ _T("blanchedalmond"), 0xFFFFEBCD }, { _T("blue"), 0xFF0000FF },
			{ _T("blueviolet"), 0xFF8A2BE2 }, { _T("brown"), 0xFFA52A2A },
			{ _T("burlywood"), 0xFFDEB887 }, { _T("cadetblue"), 0xFF5F9EA0 },
			{ _T("chartreuse"), 0xFF7FFF00 }, { _T("chocolate"), 0xFFD2691E },
			{ _T("coral"), 0xFFFF7F50 }, { _T("cornflowerblue"), 0xFF6495ED },
			{ _T("cornsilk"), 0xFFFFF8DC }, { _T("crimson"), 0xFFDC143C },
			{ _T("cyan"), 0xFF00FFFF }, { _T("darkblue"), 0xFF00008B },
			{ _T("darkcyan"), 0xFF008B8B }, { _T("darkgoldenrod"), 0xFFB8860B },
			{ _T("darkgray"), 0xFFA9A9A9 }, { _T("darkgreen"), 0xFF006400 },
			{ _T("darkgrey"), 0xFFA9A9A9 }, { _T("darkkhaki"), 0xFFBDB76B },
			{ _T("darkmagenta"), 0xFF8B008B }, { _T("darkolivegreen"), 0xFF556B2F },
			{ _T("darkorange"), 0xFFFF8C00 }, { _T("darkorchid"), 0xFF9932CC },
			{ _T("darkred"), 0xFF8B0000 }, { _T("darksalmon"), 0xFFE9967A },
			{ _T("darkseagreen"), 0xFF8FBC8F }, { _T("darkslateblue"), 0xFF483D8B },
			{ _T("darkslategray"), 0xFF2F4F4F }, { _T("darkslategrey"), 0xFF2F4F4F },
			{ _T("darkturquoise"), 0xFF00CED1 }, { _T("darkviolet"), 0xFF9400D3 },
			{ _T("deeppink"), 0xFFFF1493 }, { _T("deepskyblue"), 0xFF00BFFF },
			{ _T("dimgray"), 0xFF696969 }, { _T("dimgrey"), 0xFF696969 },
			{ _T("dodgerblue"), 0xFF1E90FF }, { _T("firebrick"), 0xFFB22222 },
			{ _T("floralwhite"), 0xFFFFFAF0 }, { _T("forestgreen"), 0xFF228B22 },
			{ _T("fuchsia"), 0xFFFF00FF }, { _T("gainsboro"), 0xFFDCDCDC },
			{ _T("ghostwhite"), 0xFFF8F8FF }, { _T("gold"), 0xFFFFD700 },
			{ _T("goldenrod"), 0xFFDAA520 }, { _T("gray"), 0xFF808080 },
			{ _T("green"), 0xFF008000 }, { _T("greenyellow"), 0xFFADFF2F },
			{ _T("grey"), 0xFF808080 }, { _T("honeydew"), 0xFFF0FFF0 },
			{ _T("hotpink"), 0xFFFF69B4 }, { _T("indianred"), 0xFFCD5C5C },
			{ _T("indigo"), 0xFF4B0082 }, { _T("ivory"), 0xFFFFFFF0 },
			{ _T("khaki"), 0xFFF0E68C }, { _T("lavender"), 0xFFE6E6FA },
			{ _T("lavenderblush"), 0xFFFFF0F5 }, { _T("lawngreen"), 0xFF7CFC00 },
			{ _T("lemonchiffon"), 0xFFFFFACD }, { _T("lightblue"), 0xFFADD8E6 },
			{ _T("lightcoral"), 0xFFF08080 }, { _T("lightcyan"), 0xFFE0FFFF },
			{ _T("lightgoldenrodyellow"), 0xFFFAFAD2 }, { _T("lightgray"), 0xFFD3D3D3 },
			{ _T("lightgreen"), 0xFF90EE90 }, { _T("lightgrey"), 0xFFD3D3D3 },
			{ _T("lightpink"), 0xFFFFB6C1 }, { _T("lightsalmon"), 0xFFFFA07A },
			{ _T("lightseagreen"), 0xFF20B2AA }, { _T("lightskyblue"), 0xFF87CEFA },
			{ _T("lightslategray"), 0xFF778899 }, { _T("lightslategrey"), 0xFF778899 },
			{ _T("lightsteelblue"), 0xFFB0C4DE }, { _T("lightyellow"), 0xFFFFFFE0 },
			{ _T("lime"), 0xFF00FF00 }, { _T("limegreen"), 0xFF32CD32 },
			{ _T("linen"), 0xFFFAF0E6 }, { _T("magenta"), 0xFFFF00FF },
			{ _T("maroon"), 0xFF800000 }, { _T("mediumaquamarine"), 0xFF66CDAA },
			{ _T("mediumblue"), 0xFF0000CD }, { _T("mediumorchid"), 0xFFBA55D3 },
			{ _T("mediumpurple"), 0xFF9370DB }, { _T("mediumseagreen"), 0xFF3CB371 },
			{ _T("mediumslateblue"), 0xFF7B68EE }, { _T("mediumspringgreen"), 0xFF00FA9A },
			{ _T("mediumturquoise"), 0xFF48D1CC }, { _T("mediumvioletred"), 0xFFC71585 },
			{ _T("midnightblue"), 0xFF191970 }, { _T("mintcream"), 0xFFF5FFFA },
			{ _T("mistyrose"), 0xFFFFE4E1 }, { _T("moccasin"), 0xFFFFE4B5 },
			{ _T("navajowhite"), 0xFFFFDEAD }, { _T("navy"), 0xFF000080 },
			{ _T("oldlace"), 0xFFFDF5E6 }, { _T("olive"), 0xFF808000 },
			{ _T("olivedrab"), 0xFF6B8E23 }, { _T("orange"), 0xFFFFA500 },
			{ _T("orangered"), 0xFFFF4500 }, { _T("orchid"), 0xFFDA70D6 },
			{ _T("palegoldenrod"), 0xFFEEE8AA }, { _T("palegreen"), 0xFF98FB98 },
			{ _T("paleturquoise"), 0xFFAFEEEE }, { _T("palevioletred"), 0xFFDB7093 },
			{ _T("papayawhip"), 0xFFFFEFD5 }, { _T("peachpuff"), 0xFFFFDAB9 },
			{ _T("peru"), 0xFFCD853F }, { _T("pink"), 0xFFFFC0CB },
			{ _T("plum"), 0xFFDDA0DD }, { _T("powderblue"), 0xFFB0E0E6 },
			{ _T("purple"), 0xFF800080 }, { _T("rebeccapurple"), 0xFF663399 },
			{ _T("red"), 0xFFFF0000 }, { _T("rosybrown"), 0xFFBC8F8F },
			{ _T("royalblue"), 0xFF4169E1 }, { _T("saddlebrown"), 0xFF8B4513 },
			{ _T("salmon"), 0xFFFA8072 }, { _T("sandybrown"), 0xFFF4A460 },
			{ _T("seagreen"), 0xFF2E8B57 }, { _T("seashell"), 0xFFFFF5EE },
			{ _T("sienna"), 0xFFA0522D }, { _T("silver"), 0xFFC0C0C0 },
			{ _T("skyblue"), 0xFF87CEEB }, { _T("slateblue"), 0xFF6A5ACD },
			{ _T("slategray"), 0xFF708090 }, { _T("slategrey"), 0xFF708090 },
			{ _T("snow"), 0xFFFFFAFA }, { _T("springgreen"), 0xFF00FF7F },
			{ _T("steelblue"), 0xFF4682B4 }, { _T("tan"), 0xFFD2B48C },
			{ _T("teal"), 0xFF008080 }, { _T("thistle"), 0xFFD8BFD8 },
			{ _T("tomato"), 0xFFFF6347 }, { _T("transparent"), 0x00000000 },
			{ _T("turquoise"), 0xFF40E0D0 }, { _T("violet"), 0xFFEE82EE },
			{ _T("wheat"), 0xFFF5DEB3 }, { _T("white"), 0xFFFFFFFF },
			{ _T("whitesmoke"), 0xFFF5F5F5 }, { _T("yellow"), 0xFFFFFF00 },
			{ _T("yellowgreen"), 0xFF9ACD32 },
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
				dwColor = 0xFF000000u | (r * 17u) << 16 | (g * 17u) << 8 | (b * 17u);
				return true;
			}
			if( len == 6 ) {
				LPTSTR pEnd = NULL;
				dwColor = 0xFF000000u | _tcstoul(tok, &pEnd, 16);
				return pEnd != tok;
			}
			if( len == 8 ) {
				LPTSTR pEnd = NULL;
				dwColor = _tcstoul(tok, &pEnd, 16);
				return pEnd != tok;
			}
			return false;
		}
	} // namespace

	bool ParseColorString(LPCTSTR pstrColor, DWORD& dwColor)
	{
		if( pstrColor == NULL ) return false;
		while( *pstrColor == _T(' ') || *pstrColor == _T('\t') ) ++pstrColor;
		if( *pstrColor == _T('\0') ) return false;

		// 先匹配命名色（避免 "red" 等被误解析）
		for( size_t i = 0; i < sizeof(kCssNamedColors) / sizeof(kCssNamedColors[0]); ++i ) {
			if( _tcsicmp(pstrColor, kCssNamedColors[i].name) == 0 ) {
				dwColor = kCssNamedColors[i].argb;
				return true;
			}
		}

		LPCTSTR tok = pstrColor;
		if( *tok == _T('#') ) ++tok;
		else if( _tcsnicmp(tok, _T("0x"), 2) == 0 ) tok += 2;

		return ParseHexColorDigits(tok, dwColor);
	}

} // namespace DuiLib