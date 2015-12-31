#pragma once


// CDlgPlayConfig �Ի���

class CDlgPlayConfig : public CDialogEx
{
	DECLARE_DYNAMIC(CDlgPlayConfig)

public:
	CDlgPlayConfig(CWnd* pParent = NULL);   // ��׼���캯��
	virtual ~CDlgPlayConfig();


// �Ի�������
	enum { IDD = IDD_DIALOG_PLAYCONFIG };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV ֧��

	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedButtonBrowse();
	virtual void OnOK();
	virtual BOOL OnInitDialog();
	CString m_strFilePath;
	UINT m_nDecodeCount;
	BOOL m_bRender;
	int m_nRenderCount;
	BOOL m_bEnableHaccel;
};
