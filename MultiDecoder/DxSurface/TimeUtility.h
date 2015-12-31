
#pragma once

#include <TCHAR.H>
#include <windows.h>
#include <time.h>
#ifdef _UNICODE
#define GetDateTime			GetDateTimeW
#define	_DateTime			DateTimeW
#define UTC2DateTimeString	UTC2DateTimeStringW
#else
#define GetDateTime			GetDateTimeA
#define	_DateTime			DateTimeA
#define UTC2DateTimeString	UTC2DateTimeStringA
#endif


int		GetDateTimeA(CHAR *szDateTime, int nSize);
int		GetDateTimeW(WCHAR *szDateTime, int nSize);

bool	IsLeapYear(UINT nYear);
UINT64	DateTimeString2UTC(TCHAR *szTime, UINT64 &nTime);
void	UTC2DateTimeStringA(UINT64 nTime, CHAR *szTime, int nSize);
void	UTC2DateTimeStringW(UINT64 nTime, WCHAR *szTime, int nSize);
BOOL	SystemTime2UTC(SYSTEMTIME *pSystemTime, UINT64 *pTime);
BOOL	UTC2SystemTime(UINT64 *pTime, SYSTEMTIME *pSystemTime);

// NTPУʱ��
struct   NTP_Packet
{
	int			Control_Word;
	int			root_delay;
	int			root_dispersion;
	int			reference_identifier;
	__int64		reference_timestamp;
	__int64		originate_timestamp;
	__int64		receive_timestamp;
	int			transmit_timestamp_seconds;
	int			transmit_timestamp_fractions;
};

/************************************************************************/
/* ����˵��:�Զ���ʱ�������ͬ������
/* ����˵��:��
/* �� �� ֵ:�ɹ�����TRUE��ʧ�ܷ���FALSE
/************************************************************************/
BOOL NTPTiming(const char* szTimeServer);

#define TimeSpan(t)		(time(NULL) - (time_t)t)
#define TimeSpanf(t,etb) (GetExactTime(&etb) - t)
typedef struct __ExactTimeBase
{
	LONGLONG	dfFreq;
	LONGLONG	dfCounter;
	time_t		nBaseClock;
	double		dfMilliseconds;
	__ExactTimeBase()
	{
		ZeroMemory(this, sizeof(ETB));
		SYSTEMTIME systime;
		GetSystemTime(&systime);
		SystemTime2UTC(&systime, (UINT64 *)&nBaseClock);
		dfMilliseconds = (double)(systime.wMilliseconds / 1000);

#ifdef _DEBUG
		TCHAR szText[64] = { 0 };
		_stprintf_s(szText, _T("BaseClock of ETB = %I64d.\n"), nBaseClock);
		OutputDebugString(szText);
#endif
		LARGE_INTEGER LarInt;
		QueryPerformanceFrequency(&LarInt);
		dfFreq = LarInt.QuadPart;
		QueryPerformanceCounter(&LarInt);
		dfCounter = LarInt.QuadPart;
	}

	~__ExactTimeBase()
	{
	}

}ETB;
extern ETB* g_petb;
#define	InitPerformanceClock	InitHighPerformanceClock
void	InitHighPerformanceClock(ETB *petb = NULL);
double  GetExactTime(ETB *petb = NULL);
