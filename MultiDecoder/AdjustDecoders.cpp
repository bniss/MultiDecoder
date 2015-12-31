// AdjustDecoders.cpp : implementation file
//

#include "stdafx.h"
#include "MultiDecoder.h"
#include "AdjustDecoders.h"
#include "afxdialogex.h"


// CAdjustDecoders dialog

IMPLEMENT_DYNAMIC(CAdjustDecoders, CDialogEx)

CAdjustDecoders::CAdjustDecoders(CWnd* pParent /*=NULL*/)
	: CDialogEx(CAdjustDecoders::IDD, pParent)
	, m_nCurDecoders(0)
	, m_nNewDecoders(0)
	, m_bEnableHaccel(FALSE)
{

}

CAdjustDecoders::~CAdjustDecoders()
{
}

void CAdjustDecoders::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_EDIT_CURDECODERS, m_nCurDecoders);
	DDX_Text(pDX, IDC_EDIT_NEWDECODERS2, m_nNewDecoders);
	DDX_Check(pDX, IDC_CHECK_HACCEL, m_bEnableHaccel);
}


BEGIN_MESSAGE_MAP(CAdjustDecoders, CDialogEx)
END_MESSAGE_MAP()


// CAdjustDecoders message handlers
