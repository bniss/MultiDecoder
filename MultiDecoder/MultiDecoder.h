
// MultiDecoder.h : PROJECT_NAME Ӧ�ó������ͷ�ļ�
//

#pragma once

#ifndef __AFXWIN_H__
	#error "�ڰ������ļ�֮ǰ������stdafx.h�������� PCH �ļ�"
#endif

#include "resource.h"		// ������


// CMultiDecoderApp: 
// �йش����ʵ�֣������ MultiDecoder.cpp
//

class CMultiDecoderApp : public CWinApp
{
public:
	CMultiDecoderApp();

// ��д
public:
	virtual BOOL InitInstance();

// ʵ��

	DECLARE_MESSAGE_MAP()
};

extern CMultiDecoderApp theApp;