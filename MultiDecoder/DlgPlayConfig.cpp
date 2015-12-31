// DlgPlayConfig.cpp : 实现文件
//

#include "stdafx.h"
#include "MultiDecoder.h"
#include "DlgPlayConfig.h"
#include "afxdialogex.h"


// CDlgPlayConfig 对话框

IMPLEMENT_DYNAMIC(CDlgPlayConfig, CDialogEx)

CDlgPlayConfig::CDlgPlayConfig(CWnd* pParent /*=NULL*/)
	: CDialogEx(CDlgPlayConfig::IDD, pParent)
	, m_strFilePath(_T(""))
	, m_nDecodeCount(1)
	, m_bRender(TRUE)
	, m_nRenderCount(1)
	, m_bEnableHaccel(FALSE)
{

}

CDlgPlayConfig::~CDlgPlayConfig()
{
}

void CDlgPlayConfig::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_EDIT_FILEPATH, m_strFilePath);
	DDX_Text(pDX, IDC_EDIT_DECODECOUNT, m_nDecodeCount);
	DDX_Check(pDX, IDC_CHECK_RENDER, m_bRender);
	DDX_Text(pDX, IDC_EDIT_RENDERCOUNT, m_nRenderCount);
	DDX_Check(pDX, IDC_CHECK_HACCEL, m_bEnableHaccel);
}


BEGIN_MESSAGE_MAP(CDlgPlayConfig, CDialogEx)
	ON_BN_CLICKED(IDC_BUTTON_BROWSE, &CDlgPlayConfig::OnBnClickedButtonBrowse)
END_MESSAGE_MAP()


// CDlgPlayConfig 消息处理程序


void CDlgPlayConfig::OnBnClickedButtonBrowse()
{
	CFileDialog dlg(TRUE);
	if (dlg.DoModal() == IDOK)
	{
		m_strFilePath = dlg.GetPathName();
		UpdateData(FALSE);
	}
}


void CDlgPlayConfig::OnOK()
{
	UpdateData();

	CDialogEx::OnOK();
}


BOOL CDlgPlayConfig::OnInitDialog()
{
	CDialogEx::OnInitDialog();


	return TRUE;  // return TRUE unless you set the focus to a control
	// 异常:  OCX 属性页应返回 FALSE
}
