#pragma once
#include <windows.h>
#include <tchar.h>
#include <d3d9.h>
#include <dxva2api.h>

#include "moreuuids.h"

#pragma warning(push)
#pragma warning(disable:4244)
#ifdef __cplusplus
extern "C" {
#endif
#define __STDC_CONSTANT_MACROS
#define FF_API_PIX_FMT 0
#include "libavformat/avformat.h"
#include "libavcodec/avcodec.h"
#include "libswscale/swscale.h"
#include "libavcodec/dxva2.h"
#include "libavutil/pixdesc.h"
#include "libavutil/pixfmt.h"	
// #include "libavutil/imgutils.h"	
// #include "libavutil/opt.h"

	//#define _ENABLE_FFMPEG_STAITC_LIB

#ifdef _ENABLE_FFMPEG_STAITC_LIB
#pragma comment(lib,"libgcc.a")
#pragma comment(lib,"libmingwex.a")
#pragma comment(lib,"libcoldname.a")
#pragma comment(lib,"libavcodec.a")
#pragma comment(lib,"libavformat.a")
#pragma comment(lib,"libavutil.a")
#pragma comment(lib,"libswscale.a")
#pragma comment(lib,"libswresample.a")
#pragma comment(lib,"WS2_32.lib")	
#else
#pragma comment(lib,"avcodec.lib")
	//#pragma comment(lib,"avfilter.lib")
#pragma comment(lib,"avformat.lib")
#pragma comment(lib,"avutil.lib")
#pragma comment(lib,"swscale.lib")	
#endif	
#ifdef __cplusplus
}
#endif
#pragma warning(pop)

#include "../DxSurface/DxTrace.h"
#include <string>
using namespace std;

#define DXVA2_MAX_SURFACES 64
#define DXVA2_QUEUE_SURFACES 4
#define DXVA2_SURFACE_BASE_ALIGN 16

#ifndef SAFE_ARRAY_DELETE
#define SAFE_ARRAY_DELETE(x) if (x) { delete [] x; x = nullptr; }
#endif

#ifndef SafeDelete
#define SafeDelete(p)       { if(p) { delete (p);     (p)=NULL; } }
#endif

#ifndef SafeDeleteArray
#define SafeDeleteArray(p)  { if(p) { delete[] (p);   (p)=NULL; } }
#endif

#ifndef SafeRelease
#define SafeRelease(p)      { if(p) { (p)->Release(); (p)=NULL; } }
#endif

// some common macros
#define SAFE_DELETE(pPtr) { delete pPtr; pPtr = nullptr; }
#define SAFE_CO_FREE(pPtr) { CoTaskMemFree(pPtr); pPtr = nullptr; }
#define CHECK_HR(hr) if (FAILED(hr)) { goto done; }
#define QI(i) (riid == __uuidof(i)) ? GetInterface((i*)this, ppv) :
#define QI2(i) (riid == IID_##i) ? GetInterface((i*)this, ppv) :

std::wstring WStringFromGUID(const GUID& guid);
#define CheckPointer(p,ret) {if((p)==NULL) return (ret);}

typedef HRESULT WINAPI pCreateDeviceManager9(UINT *pResetToken, IDirect3DDeviceManager9 **);

typedef struct 
{
	int index;
	bool used;
	LPDIRECT3DSURFACE9 d3d;
	uint64_t age;
} d3d_surface_t;


#define AVCODEC_MAX_THREADS 16

// SafeRelease Template, for type safety
// template <class T> 
// void SafeRelease(T **ppT)
// {
// 	if (*ppT)
// 	{
// 		(*ppT)->Release();
// 		*ppT = nullptr;
// 	}
// }


typedef void(*CopyFrameProc)(const BYTE *pSourceData, BYTE *pY, BYTE *pUV, size_t surfaceHeight, size_t imageHeight, size_t pitch);

extern CopyFrameProc CopyFrameNV12;
extern CopyFrameProc CopyFrameYUV420P;

class CDXVA2Decode
{
public:
	CDXVA2Decode(void);
	virtual ~CDXVA2Decode(void);
	// 初始解码器
	// 注意：不可与LoadFile函数同时调用，二者只能选一
	STDMETHODIMP InitDecoder(int nWidth,int Height,AVCodecID nCodecID = AV_CODEC_ID_H264,bool bEnalbeHaccel = false)
	{
		UINT nAdapter = D3DADAPTER_DEFAULT;
		HRESULT hr = InitD3D(nAdapter);
		if (FAILED(hr))
		{
			DxTraceMsg("-> D3D Initialization failed with hr: %X\n", hr);
			return hr;
		}		
		av_register_all();	
		DestroyDecoder();

		int nAvError = 0;
		char szAvError[1024] = { 0 };
		m_pAVCodec = avcodec_find_decoder(nCodecID);
		if (m_pAVCodec == NULL)
		{
			DxTraceMsg("%s avcodec_find_decoder Failed.\n", __FUNCTION__);
			return -1;
		}

		m_pAVCtx = avcodec_alloc_context3(m_pAVCodec);
		if (!m_pAVCtx)
		{
			DxTraceMsg("%s avcodec_alloc_context3 Failed.\n", __FUNCTION__);
		}

		m_pAVCtx->flags = 0;
		m_pAVCtx->time_base.num = 1;
		m_pAVCtx->time_base.den = 25;		//fps

		m_pAVCtx->bit_rate = 0;
		m_pAVCtx->frame_number = 1; //每包一个视频帧
		m_pAVCtx->codec_type = AVMEDIA_TYPE_VIDEO;
		m_pAVCtx->width = nWidth;
		m_pAVCtx->height = Height;

		// Setup threading
		// Thread Count. 0 = auto detect
		int thread_count = av_cpu_count() * 3 / 2;
		m_pAVCtx->thread_count = max(1, min(thread_count, AVCODEC_MAX_THREADS));
		if (m_pAVCtx->codec_id == AV_CODEC_ID_MPEG4)
		{
			m_pAVCtx->thread_count = 1;
		}

		m_pFrame = av_frame_alloc();

		if (FAILED(AdditionaDecoderInit()))
		{
			return E_FAIL;
		}
		m_bInInit = TRUE;
		nAvError = avcodec_open2(m_pAVCtx, m_pAVCodec, nullptr);
		m_bInInit = FALSE;
		if (nAvError >= 0)
		{
			m_nCodecId = m_pAVCodec->id;
		}
		else
		{
			av_strerror(nAvError, szAvError, 1024);
			DxTraceMsg("%s codec failed to avcodec_open2:\n", __FUNCTION__, szAvError);
			DestroyDecoder();
			return E_FAIL;
		}

		return S_OK;
	}
	STDMETHODIMP_(long) GetBufferCount()
	{
		long buffers = 0;

		// Native decoding should use 16 buffers to enable seamless codec changes
		// Buffers based on max ref frames
		if (m_nCodecId == AV_CODEC_ID_H264)
			buffers = 8;
		else if (m_nCodecId == AV_CODEC_ID_HEVC)
			buffers = 16;
		else
			buffers = 2;

		// 4 extra buffers for handling and safety
		// buffers += 4;
		// buffers += m_DisplayDelay;

		return buffers;
	}

	// 加载要解码的文件，同时初始化解码器
	// 注意：不可与InitDecoder同时调用，二者只能选一
	STDMETHODIMP LoadDecodingFile(char *szFilePath,bool bEnableHaccel = false)
	{
		UINT nAdapter = D3DADAPTER_DEFAULT;
		HRESULT hr = InitD3D(nAdapter);
		if (FAILED(hr))
		{
			DxTraceMsg("-> D3D Initialization failed with hr: %X\n", hr);
			return hr;
		}
		av_register_all();
		
		DestroyDecoder();
		//m_pAVCtx = avcodec_alloc_context3(m_pAVCodec);	
		char szAvError[1024] = { 0 };
		int nAvError = 0;
		m_pFormatCtx = avformat_alloc_context();
		if ((nAvError = avformat_open_input(&m_pFormatCtx, szFilePath, NULL, NULL)))
		{
			av_strerror(nAvError, szAvError, 1024);
			DxTraceMsg("%s avformat_open_input failed:%s.\n",__FUNCTION__,szAvError);
			return 0;
		}
		if ((nAvError = avformat_find_stream_info(m_pFormatCtx, NULL)) < 0)
		{
			av_strerror(nAvError, szAvError, 1024);
			DxTraceMsg("%s avformat_find_stream_info failed:%s.\n",__FUNCTION__,szAvError);
			return 0;
		}
		m_nVideoIndex = -1;
		int i = 0;
		for (i = 0; i < m_pFormatCtx->nb_streams; i++)
			if (m_pFormatCtx->streams[i]->codec->codec_type == AVMEDIA_TYPE_VIDEO)
			{
				m_nVideoIndex = i;
				break;
			}

		m_pAVCtx = m_pFormatCtx->streams[m_nVideoIndex]->codec;
		m_pAVCodec = avcodec_find_decoder(m_pAVCtx->codec_id);

		// Setup threading
		// Thread Count. 0 = auto detect
		int thread_count = av_cpu_count() * 3 / 2;
		m_pAVCtx->thread_count = max(1, min(thread_count, AVCODEC_MAX_THREADS));
		if (m_pAVCtx->codec_id == AV_CODEC_ID_MPEG4)
		{
			m_pAVCtx->thread_count = 1;
		}

		m_pFrame = av_frame_alloc();

		if (FAILED(AdditionaDecoderInit()))
		{
			return E_FAIL;
		}
		m_bInInit = TRUE;
		nAvError= avcodec_open2(m_pAVCtx, m_pAVCodec, nullptr);
		m_bInInit = FALSE;
		if (nAvError >= 0)
		{
			m_nCodecId = m_pAVCodec->id;
		}
		else
		{
			av_strerror(nAvError, szAvError, 1024);
			DxTraceMsg("%s codec failed to avcodec_open2:\n", __FUNCTION__,szAvError);
			DestroyDecoder();
			return E_FAIL;
		}

		return S_OK;
	}

	STDMETHODIMP DestroyDecoder()
	{
		DxTraceMsg("%s Shutting down ffmpeg...\n",__FUNCTION__);
		
		m_pAVCodec = nullptr;
		if (m_pAVCtx) 
		{
			avcodec_close(m_pAVCtx);
			if (m_pAVCtx->hwaccel_context)
			{
				av_free(m_pAVCtx->hwaccel_context);
				m_pAVCtx->hwaccel_context = nullptr;
			}
			av_freep(&m_pAVCtx->extradata);
			av_freep(&m_pAVCtx);
		}
		av_frame_free(&m_pFrame);
		m_pFrame = nullptr;
		m_nCodecId = AV_CODEC_ID_NONE;
		return S_OK;
	}
	DWORD GetAlignedDimension(DWORD dim);

public:
	HRESULT InitD3D(UINT &nAdapter /*= D3DADAPTER_DEFAULT*/);
	HRESULT AdditionaDecoderInit();
	STDMETHODIMP CodecIsSupported(AVCodecID codec);
	STDMETHODIMP DestroyDXVADecoder(bool bFull, bool bNoAVCodec = false);
	STDMETHODIMP FreeD3DResources();
	STDMETHODIMP LoadDXVA2Functions();
	HRESULT CreateD3DDeviceManager(IDirect3DDevice9Ex *pDevice, UINT *pReset, IDirect3DDeviceManager9 **ppManager);
	HRESULT CreateDXVAVideoService(IDirect3DDeviceManager9 *pManager, IDirectXVideoDecoderService **ppService);
	HRESULT FindVideoServiceConversion(AVCodecID codec, bool bHighBitdepth, GUID *input, D3DFORMAT *output);
	HRESULT FindDecoderConfiguration(const GUID &input, const DXVA2_VideoDesc *pDesc, DXVA2_ConfigPictureDecode *pConfig);
	HRESULT CreateDXVA2Decoder(int nSurfaces = 0, IDirect3DSurface9 **ppSurfaces = nullptr);
	HRESULT SetD3DDeviceManager(IDirect3DDeviceManager9 *pDevManager);
	HRESULT RetrieveVendorId(IDirect3DDeviceManager9 *pDevManager);
	HRESULT CheckHWCompatConditions(GUID decoderGuid);
	HRESULT FillHWContext(dxva_context *ctx);
	HRESULT ReInitDXVA2Decoder(AVCodecContext *c);
	static enum AVPixelFormat get_dxva2_format(struct AVCodecContext *s, const enum AVPixelFormat * pix_fmts);
	static int get_dxva2_buffer(struct AVCodecContext *c, AVFrame *pic, int flags);
	static void free_dxva2_buffer(void *opaque, uint8_t *data);	
	inline IDirect3DDevice9 *GetD3DDevice()
	{
		return m_pD3DDev;
	}

	inline IDirect3D9 *GetD3D9()
	{
		return m_pD3D;
	}

	inline int Decode(AVFrame *pFrame, int &got_picture,AVPacket *pPacket)
	{
		return avcodec_decode_video2(m_pAVCtx, pFrame, &got_picture, pPacket);
	}

	inline int SeekFrame(int64_t timestamp, int flags)
	{
		if (!m_pFormatCtx)
			return -1;
		 return av_seek_frame(m_pFormatCtx,m_nVideoIndex,timestamp,flags);
	}

	inline int ReadFrame(AVPacket *pkt)
	{
		if (!m_pFormatCtx)
			return -1;
		return av_read_frame(m_pFormatCtx, pkt);
	}
	
public:
	long					m_nVtableAddr;		// 虚函数表地址，该变量地址位置虚函数表之后，仅用于类初始化，请匆移动该变量的位置
	struct 
	{
		HMODULE dxva2lib;
		pCreateDeviceManager9 *createDeviceManager;
	} m_dxva;

	IDirect3D9Ex            *m_pD3D /*= nullptr*/;
	IDirect3DDevice9Ex      *m_pD3DDev/* = nullptr*/;
	IDirect3DDeviceManager9 *m_pD3DDevMngr/* = nullptr*/;
	UINT                    m_pD3DResetToken/* = 0*/;
	HANDLE                  m_hDevice/* = INVALID_HANDLE_VALUE*/;
	IDirectXVideoDecoderService *m_pDXVADecoderService/* = nullptr*/;
	IDirectXVideoDecoder        *m_pDecoder/* = nullptr*/;
	DXVA2_ConfigPictureDecode   m_DXVAVideoDecoderConfig;
	int					m_NumSurfaces/* = 0*/;
	d3d_surface_t		m_pSurfaces[DXVA2_MAX_SURFACES];
	uint64_t			m_CurrentSurfaceAge/* = 1*/;
	LPDIRECT3DSURFACE9	m_pRawSurface[DXVA2_MAX_SURFACES];

	AVPixelFormat		m_DecoderPixelFormat/* = AV_PIX_FMT_NONE*/;
	DWORD				m_dwSurfaceWidth/* = 0*/;
	DWORD				m_dwSurfaceHeight/* = 0*/;
	D3DFORMAT			m_eSurfaceFormat/* = D3DFMT_UNKNOWN*/;
	DWORD				m_dwVendorId/* = 0*/;
	DWORD				m_dwDeviceId/* = 0*/;
	GUID				m_guidDecoderDevice/* = GUID_NULL*/;
	int					m_DisplayDelay/* = DXVA2_QUEUE_SURFACES*/;
	AVCodecContext      *m_pAVCtx/* = nullptr*/;
	AVFormatContext		*m_pFormatCtx/* = nullptr*/;
	AVFrame             *m_pFrame/* = nullptr*/;
	AVCodecID           m_nCodecId/* = AV_CODEC_ID_NONE*/;
	BOOL                m_bInInit/* = FALSE*/;
	AVCodec             *m_pAVCodec/* = nullptr*/;
	SwsContext          *m_pSwsContext/* = nullptr*/;
	int					m_nVideoIndex;
	D3DFORMAT			m_nD3DFormat;
	UINT				m_nWidth;
	UINT				m_nHeight;
	HWND				m_hPresentWnd; 
	LPDIRECT3DSURFACE9	m_pDirect3DSurfaceRender;
	D3DPRESENT_PARAMETERS m_d3dpp;
};

CDXVA2Decode *CreateDecoderDXVA2();