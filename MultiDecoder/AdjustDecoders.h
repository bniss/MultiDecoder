#pragma once


// CAdjustDecoders dialog

class CAdjustDecoders : public CDialogEx
{
	DECLARE_DYNAMIC(CAdjustDecoders)

public:
	CAdjustDecoders(CWnd* pParent = NULL);   // standard constructor
	virtual ~CAdjustDecoders();

// Dialog Data
	enum { IDD = IDD_DIALOG_SETTINGDECODER };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
	int m_nCurDecoders;
	int m_nNewDecoders;
	BOOL m_bEnableHaccel;
};
