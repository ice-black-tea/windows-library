#if !defined(STRING_H__02F3F393_12C6_4024_8829_7B6D7A1FB1F4__INCLUDED_)
#define STRING_H__02F3F393_12C6_4024_8829_7B6D7A1FB1F4__INCLUDED_

#include "Common/StringA.h"
#include "Common/StringW.h"

char* stristr(const char * str1, const char * str2);
wchar_t* wcsistr(const wchar_t * str1, const wchar_t * str2);

#if !defined(UNICODE) && !defined(_UNICODE)

typedef char TCHAR;
typedef CMyStringA CMyString;
#define _tcsistr stristr

#else

typedef wchar_t TCHAR;
typedef CMyStringW CMyString;
#define _tcsistr wcsistr

#endif // !UNICODE && !_UNICODE

#if !defined(__AFX_H__)

typedef CMyString CString;

#endif // !__AFX_H__

#endif // !defined(STRING_H__02F3F393_12C6_4024_8829_7B6D7A1FB1F4__INCLUDED_)