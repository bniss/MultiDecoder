
// MultiDecoderDlg.cpp : 实现文件
//

#include "stdafx.h"
#include "MultiDecoder.h"
#include "MultiDecoderDlg.h"
#include "afxdialogex.h"
#include "DlgPlayConfig.h"
#include "AdjustDecoders.h"

#include "./DxSurface/AutoLock.h"
#include "./dxva/dxva2dec.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// 用于应用程序“关于”菜单项的 CAboutDlg 对话框

class CAboutDlg : public CDialog
{
public:
	CAboutDlg();

// 对话框数据
	enum { IDD = IDD_ABOUTBOX };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

// 实现
protected:
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialog(CAboutDlg::IDD)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialog)
END_MESSAGE_MAP()


// CMultiDecoderDlg 对话框



CMultiDecoderDlg::CMultiDecoderDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(CMultiDecoderDlg::IDD, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CMultiDecoderDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CMultiDecoderDlg, CDialogEx)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_COMMAND(ID_FILE_START, &CMultiDecoderDlg::OnFileStart)
	ON_COMMAND(ID_FILE_STOP, &CMultiDecoderDlg::OnFileStop)
	ON_COMMAND(ID_FILE_DECODECONFIG, &CMultiDecoderDlg::OnFileDecodeconfig)
	ON_MESSAGE(WM_RENDERFRAME, &CMultiDecoderDlg::OnRenderFrame)
	ON_MESSAGE(WM_INITDXSURFACE, &CMultiDecoderDlg::OnInitDxSurface)
	ON_WM_SIZE()
	ON_WM_DESTROY()
	ON_COMMAND(ID_FILE_SWITCHVIDEO, &CMultiDecoderDlg::OnFileSwitchvideo)
	ON_WM_TIMER()
	ON_COMMAND(ID_DECODER_SETTING, &CMultiDecoderDlg::OnDecoderSetting)
END_MESSAGE_MAP()

#define GetDlgItemRect(nID,Rt) { GetDlgItem(nID)->GetWindowRect(&Rt); ScreenToClient(&Rt);}
// CMultiDecoderDlg 消息处理程序

BOOL CMultiDecoderDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// 将“关于...”菜单项添加到系统菜单中。

	// IDM_ABOUTBOX 必须在系统命令范围内。
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != NULL)
	{
		BOOL bNameValid;
		CString strAboutMenu;
		bNameValid = strAboutMenu.LoadString(IDS_ABOUTBOX);
		ASSERT(bNameValid);
		if (!strAboutMenu.IsEmpty())
		{
			pSysMenu->AppendMenu(MF_SEPARATOR);
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		}
	}

	// 设置此对话框的图标。  当应用程序主窗口不是对话框时，框架将自动
	//  执行此操作
	SetIcon(m_hIcon, TRUE);			// 设置大图标
	SetIcon(m_hIcon, FALSE);		// 设置小图标

	av_register_all();
	CRect rtClient;
	GetClientRect(&rtClient);
// 	GetDlgItem(IDC_STATIC_VIDEO)->GetWindowRect(&rtClient);
// 	GetDlgItemRect(IDC_STATIC_VIDEO, rtClient);
	m_pVideoWndFrame = new CVideoFrame;	
	m_pVideoWndFrame->Create(1024, rtClient,1,1, this);
	
	return TRUE;  // 除非将焦点设置到控件，否则返回 TRUE
}

void CMultiDecoderDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else
	{
		CDialogEx::OnSysCommand(nID, lParam);
	}
}

// 如果向对话框添加最小化按钮，则需要下面的代码
//  来绘制该图标。  对于使用文档/视图模型的 MFC 应用程序，
//  这将由框架自动完成。

void CMultiDecoderDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // 用于绘制的设备上下文

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// 使图标在工作区矩形中居中
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// 绘制图标
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialogEx::OnPaint();
	}
}

//当用户拖动最小化窗口时系统调用此函数取得光标
//显示。
HCURSOR CMultiDecoderDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

void CMultiDecoderDlg::OnFileStart()
{
	TCHAR szText[MAX_PATH] = { 0 };
	if (!PathFileExists((LPCTSTR)m_strFilePath))
	{
		_stprintf_s(szText, MAX_PATH, _T("找不到\"%s\"文件."), m_strFilePath);
		AfxMessageBox(szText, MB_OK | MB_ICONSTOP);
		return;
	}
	if (m_nDecodeCount < 1 || m_nRenderCount < 1)
	{
		AfxMessageBox(_T("请至少设置一路解码."), MB_OK | MB_ICONSTOP);
		return;
	}
	if (m_nRenderCount > m_pVideoWndFrame->GetPanelCount())
	{
		m_pVideoWndFrame->AdjustPanels(m_nRenderCount);
	}
	m_hThreadArray = new HANDLE[m_nDecodeCount + 1];
	m_bInputThreadRun = true;
	
	m_hThreadArray[0] = (HANDLE)_beginthreadex(nullptr, 0, InputThread, this, 0, nullptr);

	for (int i = 0; i < m_nDecodeCount; i++)
	{
		ThreadParamPtr pTP = make_shared<ThreadParam>();
		pTP->nThreadIndex = i;
		pTP->bThreadRun = true;
		pTP->hRenderWnd = m_pVideoWndFrame->GetPanelWnd(i);
		m_pVideoWndFrame->SetPanelParam(i,pTP.get());
		pTP->pThis = this;
		if (m_bEnableHaccel)
			m_hThreadArray[i + 1] = (HANDLE)_beginthreadex(nullptr, 0, DXVADecodeThread, pTP.get(), CREATE_SUSPENDED, nullptr);
		else 
			m_hThreadArray[i + 1] = (HANDLE)_beginthreadex(nullptr, 0, DecodeThread, pTP.get(), CREATE_SUSPENDED, nullptr);
		m_vecTP.push_back(pTP);
	}
	m_nCurRender1st = 0;
	m_nCurRenderlast = m_nRenderCount - 1;
	DxTraceMsg("%s RenderRange(%d,%d).\n",__FUNCTION__, m_nCurRender1st, m_nCurRenderlast);
}

void CMultiDecoderDlg::OnFileSwitchvideo()
{
	KillTimer(ID_INVLIAD_PANEL);
	m_pVideoWndFrame->Invalidate();	
	DxTraceMsg("%s Old RenderRange(%d,%d).\n",__FUNCTION__, m_nCurRender1st, m_nCurRenderlast);
	for (int i = m_nCurRender1st; i <= m_nCurRenderlast; i++)
	{
		m_vecTP[i]->hRenderWnd = nullptr;
		m_pVideoWndFrame->SetPanelParam(i - m_nRenderCount, nullptr);
	}

	int nIndex = 0;
	if (m_nCurRenderlast == m_nDecodeCount - 1)
	{
		DxTraceMsg("%s Reach max Decoder Count,Now loop back.\n", __FUNCTION__);
		m_nCurRender1st = 0;
	}
	else
		m_nCurRender1st = m_nCurRenderlast + 1;
	
	for (int i = m_nCurRender1st; i < m_nDecodeCount; i++)
	{
		m_vecTP[i]->hRenderWnd = m_pVideoWndFrame->GetPanelWnd(nIndex ++);
		if (!m_vecTP[i]->hRenderWnd)
		{
			if (i >= m_nRenderCount)
				m_pVideoWndFrame->SetPanelParam(i - m_nRenderCount, nullptr);
			else
				m_pVideoWndFrame->SetPanelParam(i, nullptr);
			break;
		}
		else
		{
			if (i >= m_nRenderCount)
				m_pVideoWndFrame->SetPanelParam(i - m_nRenderCount, m_vecTP[i].get());
			else
				m_pVideoWndFrame->SetPanelParam(i, m_vecTP[i].get());
		}
		m_nCurRenderlast = i;
	}
	DxTraceMsg("%s New RenderRange(%d,%d).\n", __FUNCTION__, m_nCurRender1st, m_nCurRenderlast);
	SetTimer(ID_INVLIAD_PANEL,1000,nullptr);
}

void CMultiDecoderDlg::OnFileStop()
{
	CWaitCursor Wait;
	m_bInputThreadRun = false;
	//m_bDecodeThreadRun = false;
	if (m_hThreadArray)
	{
		for (int i = 0; i < m_nDecodeCount; i++)
		{
			ResumeThread(m_hThreadArray[i + 1]);
			m_vecTP[i]->bThreadRun = false;
		}
		int nWaits = (m_nDecodeCount + 1) / 64;
		if ((m_nDecodeCount + 1) % 64 != 0)
			nWaits++;
		int i = 0;
		for (i = 0; i < nWaits - 1; i++)
		{
			WaitForMultipleObjects(64, (HANDLE *)&m_hThreadArray[i], TRUE, INFINITE);
		}
		WaitForMultipleObjects((m_nDecodeCount + 1) - 64 * i, (HANDLE *)&m_hThreadArray[i * 64], TRUE, INFINITE);
	}

	m_pVideoWndFrame->Invalidate(TRUE);
	delete []m_hThreadArray;
	m_hThreadArray = nullptr;
	m_InputQueue.clear();
}

void CMultiDecoderDlg::OnFileDecodeconfig()
{
	CDlgPlayConfig dlg;
#ifdef _DEBUG
	dlg.m_strFilePath	 = _T("D:/DVORecord/CH00_192.168.24.211_ch00_20151210_095601_H.264.mp4");
	dlg.m_nDecodeCount	 = 1;
	dlg.m_bRender		 = TRUE;
	dlg.m_nRenderCount	 = 1;
	dlg.m_bEnableHaccel	 = TRUE;	
#else
	if (PathFileExists(m_strFilePath))
	{
		dlg.m_strFilePath	 = m_strFilePath;
		dlg.m_nDecodeCount	 = m_nDecodeCount;
		dlg.m_bRender		 = m_bRender;
		dlg.m_nRenderCount	 = m_nRenderCount;
		dlg.m_bEnableHaccel	 = m_bEnableHaccel;
	}
#endif
	if (dlg.DoModal() == IDOK)
	{
		m_strFilePath	 = dlg.m_strFilePath;
		m_nDecodeCount	 = dlg.m_nDecodeCount;
		m_bRender		 = dlg.m_bRender;
		m_nRenderCount	 = dlg.m_nRenderCount;
		m_bEnableHaccel	 = dlg.m_bEnableHaccel;
	}
}

UINT CMultiDecoderDlg::InputThread(void *p)
{
	CMultiDecoderDlg *pThis = (CMultiDecoderDlg *)p;
			
	char szFilePath[MAX_PATH] = { 0 };
	WideCharToMultiByte(CP_ACP, 0, (LPCTSTR)pThis->m_strFilePath, -1, szFilePath, MAX_PATH, NULL, NULL);	
	AVFormatContext *pFormatCtx = avformat_alloc_context();
	char szAvError[1024] = { 0 };
	int nAvError = 0;
	if ((nAvError = avformat_open_input(&pFormatCtx, szFilePath, NULL, NULL)))
	{
		av_strerror(nAvError, szAvError, 1024);
		DxTraceMsg("打开视频文件失败:%s.\r\n", szAvError);
		return 0;
	}
	if ((nAvError = avformat_find_stream_info(pFormatCtx, NULL)) < 0)
	{
		av_strerror(nAvError, szAvError, 1024);
		DxTraceMsg("找不到视频流信息:%s.\r\n", szAvError);
		return 0;
	}
	int videoindex = 0;
	int i = 0;
	for (i = 0; i < pFormatCtx->nb_streams; i++)
		if (pFormatCtx->streams[i]->codec->codec_type == AVMEDIA_TYPE_VIDEO)
		{
			videoindex = i;
			break;
		}
	if (videoindex == -1)
	{
		DxTraceMsg("目标文件中找不到视频流.\r\n");
		return 0;
	}

	AVCodecContext *pCodecCtx = pFormatCtx->streams[videoindex]->codec;
	AVCodec *pCodec = avcodec_find_decoder(pCodecCtx->codec_id);

	if ((nAvError = avcodec_open2(pCodecCtx, pCodec, NULL)) < 0)
	{
		av_strerror(nAvError, szAvError, 1024);
		DxTraceMsg("打开解码器失败:%s.\n", szAvError);
		return 0;
	}
	bool bResumed = false;
	AVPacket *packet = (AVPacket *)av_malloc(sizeof(AVPacket));
	while (pThis->m_bInputThreadRun)
	{
		if (nAvError = av_read_frame(pFormatCtx, packet) < 0)
		{
			av_strerror(nAvError, szAvError, 1024);
			DxTraceMsg("读取视频帧失败:%s.\n", szAvError);
			goto Resume; 
		}
		FramePtr pFrame = make_shared<Frame>((byte *)packet->data, packet->size);
		pThis->m_InputQueue.push_back(pFrame);		
		av_packet_unref(packet);
	}
Resume:
	av_free(packet);
	avcodec_close(pCodecCtx);
	avformat_close_input(&pFormatCtx);
	if (!bResumed && pThis->m_InputQueue.size() > 1)
	{
		for (int i = 0; i < pThis->m_nDecodeCount; i++)
		{
			::ResumeThread(pThis->m_hThreadArray[i + 1]);
		}
		bResumed = true;
	}
	return 0;
}

struct AvQueue
{
	CMultiDecoderDlg *pThis;
	list<FramePtr>::iterator ItLoop;
	uint8_t *pAvBuffer;
	uint8_t *pOriBuffer;
	int nOffset;
};
int ReadAvData(void *opaque, uint8_t *buf, int buf_size)
{
	AvQueue *pAvQueue = (AvQueue *)opaque;	
	if (pAvQueue->ItLoop == pAvQueue->pThis->m_InputQueue.end())
		return 0;
 	
	int nReturnVal = buf_size;
	pAvQueue->pAvBuffer = buf;
	int nRemainedLength = (*(pAvQueue->ItLoop))->nLength - pAvQueue->nOffset;
	if (nRemainedLength > buf_size)
	{
		memcpy(buf, &(*(pAvQueue->ItLoop))->pData[pAvQueue->nOffset], buf_size);
		pAvQueue->nOffset += buf_size;
	}
	else
	{
		memcpy(buf, &(*(pAvQueue->ItLoop))->pData[pAvQueue->nOffset], nRemainedLength);
		pAvQueue->nOffset = 0;
		nReturnVal = nRemainedLength;
		pAvQueue->ItLoop++;
	}
	if (pAvQueue->pOriBuffer != pAvQueue->pAvBuffer)
	{
		DxTraceMsg("%s Ori Buffer = %08X\t Buffer = %08X.", __FUNCTION__, pAvQueue->pOriBuffer, pAvQueue->pAvBuffer);
		pAvQueue->pOriBuffer = pAvQueue->pAvBuffer;
	}
// 	memcpy(buf, (*(pAvQueue->ItLoop))->pData, (*(pAvQueue->ItLoop))->nLength);	
// 	pAvQueue->ItLoop++;	
		
	return nReturnVal;// (*(pAvQueue->ItLoop))->nLength;
}

UINT CMultiDecoderDlg::DecodeThread(void *p)
{
	ThreadParam *TPPtr = (ThreadParam *)p;	
	CMultiDecoderDlg *pThis = TPPtr->pThis;
	int nAvError = 0;
	AvQueue *pAvQueue = new AvQueue;
	pAvQueue->pThis = pThis;
	pAvQueue->ItLoop = pThis->m_InputQueue.begin();
	pAvQueue->nOffset = 0;
	char szAvError[1024] = { 0 };
	
	int nAvBufferSize = 1024 * 32;
	uint8_t *pAvBuffer = (uint8_t *)av_malloc(nAvBufferSize);
	//av_free(pAvBuffer);
	int nWriteable = 0;
	pAvQueue->pAvBuffer = pAvBuffer;
	pAvQueue->pOriBuffer = pAvBuffer;

	AVIOContext *pIoContext = NULL;	
	pIoContext = avio_alloc_context(pAvBuffer, nAvBufferSize, nWriteable, pAvQueue, ReadAvData, nullptr, nullptr);
	if (!pIoContext)
	{
		DxTraceMsg("%s avio_alloc_context Failed.\n", __FUNCTION__);
		return -1;
	}
	AVInputFormat *pInputFormatCtx = NULL;
	if (nAvError = av_probe_input_buffer(pIoContext, &pInputFormatCtx, "", NULL, 0, 0) < 0)
	{
		av_strerror(nAvError, szAvError, 1024);
		DxTraceMsg("%s av_probe_input_buffer Failed:%s\n", __FUNCTION__, szAvError);
		return -1;
	}
	
	AVFormatContext *pFormatCtx = avformat_alloc_context();
	if (!pFormatCtx)
	{
		DxTraceMsg("%s avformat_alloc_context Failed:%s\n", __FUNCTION__);
		return -1;
	}
	pFormatCtx->pb = pIoContext;
	if (nAvError = avformat_open_input(&pFormatCtx, "", pInputFormatCtx, nullptr) < 0)
	{
		av_strerror(nAvError, szAvError, 1024);
		DxTraceMsg("%s avformat_open_input Failed:%s\n", __FUNCTION__, szAvError);
		return -1;
	}

	if (avformat_find_stream_info(pFormatCtx,nullptr) < 0) 
	{
		DxTraceMsg("%s avformat_find_stream_info Failed.\n", __FUNCTION__);
		return -1;
	}

	av_dump_format(pFormatCtx, 0, "", 0);

	int videoindex = -1;
	int audioindex = -1;
	for (int i = 0; i < pFormatCtx->nb_streams; i++) 
	{
		if ((pFormatCtx->streams[i]->codec->codec_type == AVMEDIA_TYPE_VIDEO) &&
			(videoindex < 0)) 
		{
			videoindex = i;
		}
		if ((pFormatCtx->streams[i]->codec->codec_type == AVMEDIA_TYPE_AUDIO) &&
			(audioindex < 0)) 
		{
			audioindex = i;
		}
	}

	if (videoindex < 0  && audioindex < 0) 
	{
		DxTraceMsg("%s can't found any video stream or audio stream.\n", __FUNCTION__);
		return -1;		
	}
	AVCodecContext* pAvCodecCtx = pFormatCtx->streams[videoindex]->codec;
	auto pAvCodec = avcodec_find_decoder(pAvCodecCtx->codec_id);
	if (pAvCodec == NULL)
	{
		DxTraceMsg("%s avcodec_find_decoder Failed.\n", __FUNCTION__);
		return -1;
	}
	if ((nAvError = avcodec_open2(pAvCodecCtx, pAvCodec, NULL)) < 0)
	{
		av_strerror(nAvError, szAvError, 1024);
		DxTraceMsg("%s avcodec_open2 Failed:%s.\n", __FUNCTION__, szAvError);
		return 0;
	}
	double dfT1 = 0.0f;
	double dfT2 = 0.0f;
	double dfTimeSpan = 0.0f;
	AVPacket *pAvPacket = (AVPacket *)av_malloc(sizeof(AVPacket));
	av_init_packet(pAvPacket);
	
	int nGot_picture = 0;
	AVFrame *pAvFrame = av_frame_alloc();
	DWORD nResult = 0;
	int nTimeSpan = 0;
	int nFrameInterval = 40;
	pAvQueue->ItLoop = pThis->m_InputQueue.begin();
	//av_free(pAvBuffer);
	while (TPPtr->bThreadRun)
	{
		if (av_read_frame(pFormatCtx, pAvPacket) >= 0)
		//if (ItLoop != pThis->m_InputQueue.end())
		{
			dfT1 = GetExactTime();					
			nAvError = avcodec_decode_video2(pAvCodecCtx, pAvFrame, &nGot_picture, pAvPacket);
			if (nAvError < 0)
			{
				av_strerror(nAvError, szAvError, 1024);
				DxTraceMsg("%s Decode error:%s.\n", __FUNCTION__,szAvError);
				continue;
			}
			av_packet_unref(pAvPacket);
			if (nGot_picture)
			{
				if (TPPtr->hRenderWnd)
				{
					// 使用线程内CDxSurface对象显示图象
					if (!TPPtr->pDxSurface->IsInited())		// D3D设备尚未创建,说明未初始化
					{
						DxSurfaceInitInfo InitInfo;
						InitInfo.nFrameWidth = pAvFrame->width;
						InitInfo.nFrameHeight = pAvFrame->height;
						InitInfo.nD3DFormat = (D3DFORMAT)MAKEFOURCC('Y', 'V', '1', '2');
						InitInfo.bWindowed = TRUE;
						InitInfo.hPresentWnd = TPPtr->hRenderWnd;
					
							if (!TPPtr->pDxSurface->InitD3D(InitInfo.hPresentWnd,
								InitInfo.nFrameWidth,
								InitInfo.nFrameHeight,
								InitInfo.bWindowed,
								InitInfo.nD3DFormat))
							{
								assert(false);
								return 0;
							}
						//::SendMessageTimeout(pThis->m_hWnd, WM_INITDXSURFACE, (WPARAM)TPPtr->pDxSurface, (LPARAM)&InitInfo, SMTO_BLOCK, 500, (PDWORD_PTR)&nResult);
					}
					//::SendMessageTimeout(pThis->m_hWnd, WM_RENDERFRAME, (WPARAM)TPPtr->pDxSurface, (LPARAM)pAvFrame, SMTO_BLOCK, 500, (PDWORD_PTR)&nResult);
					TPPtr->pDxSurface->Render(pAvFrame);
				}
				av_frame_unref(pAvFrame);
			}
			else
			{
				DxTraceMsg("解码失败.\n");
			}
			nTimeSpan = (int)(1000 *(GetExactTime() - dfT1));
			int nSleepTime = nFrameInterval - nTimeSpan;
// 			if (nSleepTime > 0)
// 				Sleep(nSleepTime);
			
		}
		else
			break;
	}
	
	av_frame_free(&pAvFrame);
	avcodec_close(pAvCodecCtx);
	avformat_close_input(&pFormatCtx);
	avformat_free_context(pFormatCtx);	
	av_free(pAvPacket);
	av_free(pIoContext);	
 	av_free(pAvQueue->pAvBuffer);
	delete pAvQueue;

	return 0;
}

/// @brief 把NV12图像转换为YV12图像
/// @remark 若要转换成YUV420P格式，把U和V分量调换即可
void CopyNV12ToYV12(byte *pYV12, byte *pNV12[2], int src_pitch[2], unsigned width, unsigned height)
{
	byte* dstV = pYV12 + width*height;
	byte* dstU = pYV12 + width*height / 4;
	UINT heithtUV = height / 2;
	UINT widthUV = width / 2;
	byte *pSrcUV = pNV12[1];
	byte *pSrcY = pNV12[0];
	int &nYpitch = src_pitch[0];
	int &nUVpitch = src_pitch[1];

	// 复制Y分量
	for (int i = 0; i < height; i++)
		memcpy(pYV12 + i*width, pSrcY + i*nYpitch, width);

	// 复制VU分量
	for (int i = 0; i < heithtUV; i++)
	{
		for (int j = 0; j < width; j++)
		{
			dstU[i*widthUV + j] = pSrcUV[i*nUVpitch + 2 * j];
			dstV[i*widthUV + j] = pSrcUV[i*nUVpitch + 2 * j + 1];
		}
	}
}

int dxva2_retrieve_data(AVFrame **pDstFrame, AVFrame *frame)
{
	LPDIRECT3DSURFACE9 surface = (LPDIRECT3DSURFACE9)frame->data[3];	
	D3DSURFACE_DESC    surfaceDesc;
	D3DLOCKED_RECT     LockedRect;
	HRESULT            hr;
	int                ret;

	IDirect3DSurface9_GetDesc(surface, &surfaceDesc);

	if (*pDstFrame == NULL)
	{
		*pDstFrame = av_frame_alloc();
		(*pDstFrame)->width = frame->width;
		(*pDstFrame)->height = frame->height;
		(*pDstFrame)->format = AV_PIX_FMT_NV12;

		ret = av_frame_get_buffer(*pDstFrame, 16);
		if (ret < 0)
			return ret;
	}
	

	hr = IDirect3DSurface9_LockRect(surface, &LockedRect, NULL, D3DLOCK_READONLY);
	if (FAILED(hr))
	{
		av_log(NULL, AV_LOG_ERROR, "Unable to lock DXVA2 surface\n");
		return AVERROR_UNKNOWN;
	}

	av_image_copy_plane((*pDstFrame)->data[0], (*pDstFrame)->linesize[0],
		(uint8_t*)LockedRect.pBits,
		LockedRect.Pitch, frame->width, frame->height);

	av_image_copy_plane((*pDstFrame)->data[1], (*pDstFrame)->linesize[1],
		(uint8_t*)LockedRect.pBits + LockedRect.Pitch * surfaceDesc.Height,
		LockedRect.Pitch, frame->width, frame->height / 2);

	IDirect3DSurface9_UnlockRect(surface);

	ret = av_frame_copy_props((*pDstFrame), frame);
	if (ret < 0)
		goto fail;

	return 0;
fail:
	av_frame_unref((*pDstFrame));
	return ret;
}

/// @brief 把Dxva硬解码帧转换成NV12图像
void CopyFrame(AVFrame *pFrameYUV420P, AVFrame *pAvFrameDXVA)
{
	if (pAvFrameDXVA->format != AV_PIX_FMT_DXVA2_VLD)
		return;

	IDirect3DSurface9* pSurface = (IDirect3DSurface9 *)pAvFrameDXVA->data[3];
	D3DLOCKED_RECT lRect;
	D3DSURFACE_DESC SurfaceDesc;
	pSurface->GetDesc(&SurfaceDesc);
	HRESULT hr = pSurface->LockRect(&lRect, nullptr, D3DLOCK_READONLY);
	if (FAILED(hr))
	{
		DxTraceMsg("%s IDirect3DSurface9::LockRect failed:hr = %08.\n", __FUNCTION__, hr);
		return;
	}

	//CopyFrameNV12((BYTE *)lRect.pBits, pFrameNV12->data[0], pFrameNV12->data[1], SurfaceDesc.Height, pFrameNV12->height, lRect.Pitch);
	
	// Y分量图像
	byte *pSrcY = (byte *)lRect.pBits;
	// UV分量图像
	//byte *pSrcUV = (byte *)lRect.pBits + lRect.Pitch * SurfaceDesc.Height;
	byte *pSrcUV = (byte *)lRect.pBits + lRect.Pitch * pFrameYUV420P->height;
		
	byte* dstY = pFrameYUV420P->data[0];
	byte* dstU = pFrameYUV420P->data[1];
	byte* dstV = pFrameYUV420P->data[2];
	
	UINT heithtUV = pFrameYUV420P->height / 2;
	UINT widthUV = pFrameYUV420P->width / 2;

	// 复制Y分量
 	for (int i = 0; i < pFrameYUV420P->height; i++)
 		memcpy(&dstY[i*pFrameYUV420P->width], &pSrcY[i*lRect.Pitch], pFrameYUV420P->width);

	// 复制VU分量
	for (int i = 0; i < heithtUV; i++)
	{
		for (int j = 0; j < widthUV; j++)
		{
			dstU[i*widthUV + j] = pSrcUV[i*lRect.Pitch + 2 * j];
			dstV[i*widthUV + j] = pSrcUV[i*lRect.Pitch + 2 * j + 1];
		}
	}
	
	pSurface->UnlockRect();
}

UINT CMultiDecoderDlg::DXVADecodeThread(void *p)
{
	ThreadParam *TPPtr = (ThreadParam *)p;
	CMultiDecoderDlg *pThis = TPPtr->pThis;
	int nAvError = 0;
	char szAvError[1024] = { 0 };
	shared_ptr<CDXVA2Decode>pDecodec = make_shared<CDXVA2Decode>();

	pDecodec->InitDecoder(1920,1080,AV_CODEC_ID_H264);
	double dfT1 = 0.0f;
	double dfT2 = 0.0f;
	double dfTimeSpan = 0.0f;
	AVPacket *pAvPacket = (AVPacket *)av_malloc(sizeof(AVPacket));
	auto ItLoop = pThis->m_InputQueue.begin();
	FramePtr pFrame;
	int nGot_picture = 0;
	AVFrame *pAvFrame = av_frame_alloc();
	DWORD nResult = 0;
	int nTimeSpan = 0;
	int nFrameInterval = 40;
	
	AVFrame *pFrame420 = av_frame_alloc();
	AVFrame *pFrameNV12 = av_frame_alloc();
	int nWidth = pDecodec->GetAlignedDimension(1920);
	int nHeight = pDecodec->GetAlignedDimension(1080);
	int nImage420Size = av_image_get_buffer_size(AV_PIX_FMT_YUV420P, nWidth, nHeight, 16);
	int nImageNV12Size = av_image_get_buffer_size(AV_PIX_FMT_NV12, nWidth, nHeight, 16);
	
	if (nImage420Size < 0)
	{
		char szAvError[256] = { 0 };
		av_strerror(nImage420Size, szAvError, 1024);
		DxTraceMsg("%s av_image_get_buffer_size failed:%s.\n", __FUNCTION__, szAvError);
		assert(false);
	}
	if ( nImageNV12Size < 0)
	{
		char szAvError[256] = {0};
		av_strerror(nImageNV12Size, szAvError, 1024);
		DxTraceMsg("%s av_image_get_buffer_size failed:%s.\n", __FUNCTION__, szAvError);
		assert(false);
	}
	byte *pImage420 = (byte *)av_malloc(nImage420Size);
	byte *pImageNV12 = (byte *)av_malloc(nImageNV12Size);

	ZeroMemory(pImage420, nImage420Size);
	ZeroMemory(pImageNV12, nImageNV12Size);
	
	// 把显示图像与YUV帧关联
	av_image_fill_arrays(pFrame420->data, pFrame420->linesize, pImage420, AV_PIX_FMT_YUV420P, 1920, 1080, 16);
	av_image_fill_arrays(pFrameNV12->data, pFrameNV12->linesize, pImageNV12, AV_PIX_FMT_NV12, 1920, 1080, 16);

	pFrame420->width = 1920;
	pFrame420->height = 1080;
	pFrame420->format = AV_PIX_FMT_YUV420P;

	pFrameNV12->width = 1920;
	pFrameNV12->height = 1080;
	pFrameNV12->format = AV_PIX_FMT_NV12;
	PixelConvert *pc = nullptr;

	while (TPPtr->bThreadRun)
	{
		if (ItLoop != pThis->m_InputQueue.end())
		{
			dfT1 = GetExactTime();
			av_init_packet(pAvPacket);
			pFrame = *ItLoop;
			pAvPacket->data = (byte *)pFrame->pData;
			pAvPacket->size = pFrame->nLength;
			pAvPacket->flags = AV_PKT_FLAG_KEY;

			nAvError = pDecodec->Decode(pAvFrame, nGot_picture, pAvPacket);			
			if (nAvError < 0)
			{
				av_strerror(nAvError, szAvError, 1024);
				DxTraceMsg("%s Decode error:%s.\n", __FUNCTION__, szAvError);
				continue;
			}
			if (nGot_picture)
			{
				if (TPPtr->hRenderWnd)
				{
					// 使用线程内CDxSurface对象显示图象
					if (!TPPtr->pDxSurface->IsInited())		// D3D设备尚未创建,说明未初始化
					{
						DxSurfaceInitInfo InitInfo;
						InitInfo.nFrameWidth = pDecodec->GetAlignedDimension(pAvFrame->width);
						InitInfo.nFrameHeight = pDecodec->GetAlignedDimension(pAvFrame->height);
						InitInfo.nD3DFormat = (D3DFORMAT)MAKEFOURCC('Y', 'V', '1', '2');
						InitInfo.bWindowed = TRUE;
						InitInfo.hPresentWnd = TPPtr->hRenderWnd;

						if (!TPPtr->pDxSurface->InitD3D(InitInfo.hPresentWnd,
							InitInfo.nFrameWidth,
							InitInfo.nFrameHeight,
							InitInfo.bWindowed,
							InitInfo.nD3DFormat))
						{
							assert(false);
							return 0;
						}
						//::SendMessageTimeout(pThis->m_hWnd, WM_INITDXSURFACE, (WPARAM)TPPtr->pDxSurface, (LPARAM)&InitInfo, SMTO_BLOCK, 500, (PDWORD_PTR)&nResult);
					}
					ZeroMemory(pImageNV12, nImageNV12Size);
					
					//dxva2_retrieve_data(&pFrameNV12, pAvFrame);
					CopyFrame(pFrame420, pAvFrame);

					//::SendMessageTimeout(pThis->m_hWnd, WM_RENDERFRAME, (WPARAM)TPPtr->pDxSurface, (LPARAM)pAvFrame, SMTO_BLOCK, 500, (PDWORD_PTR)&nResult);
					TPPtr->pDxSurface->Render(pFrame420);
				}
			}
			else
			{
				DxTraceMsg("解码失败.\n");
			}
			nTimeSpan = (int)(1000 * (GetExactTime() - dfT1));
			int nSleepTime = nFrameInterval - nTimeSpan;
			if (nSleepTime > 0)
				Sleep(nSleepTime);
			ItLoop++;
		}
		else
			ItLoop = pThis->m_InputQueue.begin();
	}

	av_frame_free(&pAvFrame);
	
	av_free(pAvPacket);
	av_free(pImageNV12);
	av_free(pImage420);
	av_frame_free(&pFrameNV12);
	av_frame_free(&pFrame420);
	return 0;
}
void CMultiDecoderDlg::OnSize(UINT nType, int cx, int cy)
{
	CDialogEx::OnSize(nType, cx, cy);
	if (m_pVideoWndFrame)
		m_pVideoWndFrame->MoveWindow(0, 0, cx, cy);
}

LRESULT CMultiDecoderDlg::OnInitDxSurface(WPARAM w, LPARAM l)
{
	if (!w || !l)
		return -1;
	CDxSurface *pDxSurface = (CDxSurface *)w;
	DxSurfaceInitInfo *pInitInfo = (DxSurfaceInitInfo *)l;
	
	if (!pDxSurface->InitD3D(pInitInfo->hPresentWnd,
		pInitInfo->nFrameWidth,
		pInitInfo->nFrameHeight,
		pInitInfo->bWindowed,
		pInitInfo->nD3DFormat))
	{
		assert(false);
		return 0;
	}
	m_nDxInitCount++;
	DxTraceMsg("%s m_nDxInitCount = %d.\n", __FUNCTION__, m_nDxInitCount);
	return 0;
}

LRESULT CMultiDecoderDlg::OnRenderFrame(WPARAM w, LPARAM l)
{
	if (!w || !l)
		return -1;

	CDxSurface *pDxSurface = (CDxSurface *)w;
	AVFrame *pAvFrame = (AVFrame *)l;

// 	if (!pDxSurface->IsInited())		// D3D设备尚未创建,说明未初始化
// 	{
// 		assert(false);
// 		return 0;
// 	}
// 	RECT rtWnd;
// 	GetDlgItem(IDC_STATIC_VIDEO)->GetWindowRect(&rtWnd);
// 	ScreenToClient(&rtWnd);
// 	int nWidth = rtWnd.right - rtWnd.left;
// 	int nHeight = rtWnd.bottom - rtWnd.top;
// 	float dfScaleWnd = (float)nWidth / nHeight;
// 	float dfScaleVideo = (float)pAvFrame->width / pAvFrame->height;
// 	int nNewHeight = 0, nNewWidth = 0;
// 	int nCenterX = rtWnd.left + nWidth / 2;
// 	int nCenterY = rtWnd.top + nHeight / 2;
// 	RECT rtVideo;
// 	if (dfScaleWnd > dfScaleVideo)
// 	{
// 		nNewHeight = nHeight;
// 		nNewWidth = nNewHeight*pAvFrame->width / pAvFrame->height;
// 	}
// 	else if (dfScaleWnd < dfScaleVideo)
// 	{
// 		nNewWidth = nWidth;
// 		nNewHeight = nNewWidth*pAvFrame->height / pAvFrame->width;
// 	}
// 	else
// 	{
// 		nNewWidth = nWidth;
// 		nNewHeight = nHeight;
// 	}
// 
// 	rtVideo.left = nCenterX - nNewWidth / 2;
// 	rtVideo.right = rtVideo.left + nNewWidth;
// 	rtVideo.top = nCenterY - nNewHeight / 2;
// 	rtVideo.bottom = rtVideo.top + nNewHeight;
//	pDxSurface->Render(pAvFrame, m_hWnd, &rtVideo);
	pDxSurface->Render(pAvFrame);
	return 0;
}

void CMultiDecoderDlg::OnDestroy()
{
	CDialogEx::OnDestroy();
	OnFileStop();
	m_pVideoWndFrame->DestroyWindow();
}


void CMultiDecoderDlg::OnTimer(UINT_PTR nIDEvent)
{
	if (nIDEvent == ID_INVLIAD_PANEL)
	{
		KillTimer(nIDEvent);
		if (m_pVideoWndFrame)
		{
			for (int i = 0; i < m_pVideoWndFrame->GetPanelCount(); i++)
			{
				if (!m_pVideoWndFrame->GetPanelParam(i))
					::InvalidateRect(m_pVideoWndFrame->GetPanelWnd(i), NULL, TRUE);
			}
		}
	}

	CDialogEx::OnTimer(nIDEvent);
}

void CMultiDecoderDlg::OnDecoderSetting()
{
	CAdjustDecoders dlg;
	dlg.m_nCurDecoders = m_nDecodeCount;
	if (dlg.DoModal() == IDOK)
	{
		if (dlg.m_nNewDecoders == m_nDecodeCount)
			return;
		else if (dlg.m_nNewDecoders > m_nDecodeCount)
		{
			HANDLE *pNewArray = new HANDLE[dlg.m_nNewDecoders + 1];
			memcpy(pNewArray, m_hThreadArray, sizeof(HANDLE)*(m_nDecodeCount + 1));
			delete[]m_hThreadArray;
			m_hThreadArray = pNewArray;
			for (int i = m_nDecodeCount; i < dlg.m_nNewDecoders; i++)
			{
				ThreadParamPtr pTP = make_shared<ThreadParam>();
				pTP->bThreadRun = true;
				pTP->nThreadIndex = i;
				pTP->hRenderWnd = NULL;				
				pTP->pThis = this;
				if (dlg.m_bEnableHaccel)
					m_hThreadArray[i + 1] = (HANDLE)_beginthreadex(nullptr, 0, DXVADecodeThread, pTP.get(), 0, nullptr);
				else
					m_hThreadArray[i + 1] = (HANDLE)_beginthreadex(nullptr, 0, DecodeThread, pTP.get(), 0, nullptr);
				m_vecTP.push_back(pTP);
			}
		}
		else
		{
			int nSubCount = m_nDecodeCount - dlg.m_nNewDecoders;
			for (auto it = m_vecTP.rbegin(); it != m_vecTP.rend();)
			{
				if (nSubCount > 0)
				{
					(*it)->bThreadRun = false;
					nSubCount--;
					it++;
				}
				else
					break;
			}
			nSubCount = m_nDecodeCount - dlg.m_nNewDecoders;
			WaitForMultipleObjects(nSubCount, &m_hThreadArray[1 + dlg.m_nNewDecoders], TRUE, INFINITE);			
			auto itbegin = m_vecTP.begin() + dlg.m_nNewDecoders;
			auto itEnd = m_vecTP.begin() + m_nDecodeCount;
			m_vecTP.erase(itbegin, itEnd);
			for (int i = m_nDecodeCount; i < dlg.m_nNewDecoders; i++)
				m_hThreadArray[1 + i] = nullptr;
		}
		m_nDecodeCount = dlg.m_nNewDecoders;
	}
}
