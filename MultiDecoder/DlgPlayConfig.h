#pragma once


// CDlgPlayConfig 对话框

class CDlgPlayConfig : public CDialogEx
{
	DECLARE_DYNAMIC(CDlgPlayConfig)

public:
	CDlgPlayConfig(CWnd* pParent = NULL);   // 标准构造函数
	virtual ~CDlgPlayConfig();


// 对话框数据
	enum { IDD = IDD_DIALOG_PLAYCONFIG };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

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
