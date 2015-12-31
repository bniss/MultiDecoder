
// MultiDecoderDlg.h : 头文件
//

#pragma once
#include <list>
#include <vector>
#include <memory>
#include "./DxSurface/DxSurface.h"
#include "./DxSurface/TimeUtility.h"
#include "VideoFrame.h"
using namespace std;
using namespace std::tr1;

struct Frame
{
	Frame(byte *pInput,int nInputLen)
	{
		pData = new byte[nInputLen];
		nLength = nInputLen;
		memcpy(pData, pInput, nInputLen);
	}
	~Frame()
	{
		if (pData)
			delete[]pData;
		//DxTraceMsg("%s Free memory length:%d.\n", __FUNCTION__, nLength);
		nLength = 0;
	}
	byte	*pData;
	UINT	nLength;
};

class CMultiDecoderDlg;
struct ThreadParam
{
	ThreadParam()
	{
		ZeroMemory(this, sizeof(ThreadParam));
		pDxSurface = new CDxSurface();
	}
	~ThreadParam()
	{
		if (pDxSurface)
			delete pDxSurface;
	}
	bool			bThreadRun;
	CMultiDecoderDlg *pThis;
	UINT			 nThreadIndex;
	HWND			 hRenderWnd;
	CDxSurface		*pDxSurface;
};

typedef shared_ptr<Frame> FramePtr;
typedef shared_ptr<ThreadParam> ThreadParamPtr;
// CMultiDecoderDlg 对话框
class CMultiDecoderDlg : public CDialogEx
{
// 构造
public:
	CMultiDecoderDlg(CWnd* pParent = NULL);	// 标准构造函数
	~CMultiDecoderDlg()
	{
		if (m_pVideoWndFrame)
			delete m_pVideoWndFrame;
	}

// 对话框数据
	enum { IDD = IDD_MULTIDECODER_DIALOG };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV 支持
	
// 实现
protected:
	HICON m_hIcon;

	// 生成的消息映射函数
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnFileStart();
	afx_msg void OnFileStop();
	afx_msg void OnFileDecodeconfig();
	volatile bool m_bInputThreadRun = false;
	//volatile bool m_bDecodeThreadRun = false;
	static UINT __stdcall InputThread(void *);
	static UINT __stdcall DecodeThread(void *);
	static UINT __stdcall DXVADecodeThread(void *);
	CString		m_strFilePath = _T("");
	UINT		m_nDecodeCount = 1;
	UINT		m_nRenderCount = 1;
	UINT		m_nDxInitCount = 0;
	UINT		m_nCurRender1st = 1;		// 第1个渲染的解码路数
	UINT		m_nCurRenderlast = 1;		// 最后一个渲染的解码路数
	BOOL		m_bEnableHaccel = FALSE;
	BOOL		m_bRender = true;
	HANDLE		*m_hThreadArray = NULL;
	UINT		m_nVideoWndID = 1024;		// 第一个视频窗口ID
	CVideoFrame *m_pVideoWndFrame = nullptr;
	list<FramePtr>	m_InputQueue;
	LPCTSTR		m_szWndClass = NULL;
	afx_msg void OnSize(UINT nType, int cx, int cy);
	LRESULT OnInitDxSurface(WPARAM w, LPARAM l);	
	LRESULT OnRenderFrame(WPARAM w, LPARAM l);
	vector<ThreadParamPtr>m_vecTP;
	afx_msg void OnDestroy();
	afx_msg void OnFileSwitchvideo();
	afx_msg void OnTimer(UINT_PTR nIDEvent);
	afx_msg void OnDecoderSetting();
};