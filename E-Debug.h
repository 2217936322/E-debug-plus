// E-Debug.h : E-Debug DLL ����ͷ�ļ�
//
#include "stdafx.h"

#pragma once

#define StaticMode 0  //��̬����ģʽ
#define NormalMode 1  //������������ģʽ
#define CMode      2  //���±��룬����C����ģʽ

#ifndef __AFXWIN_H__
	#error "�ڰ������ļ�֮ǰ������stdafx.h�������� PCH �ļ�"
#endif

#include "resource.h"		// ������

extern char DIRECTORY[MAX_PATH];  //����ȫ�ֱ���,���Ŀ¼
extern UINT AnalysisMode;
// CEDebugApp
// �йش���ʵ�ֵ���Ϣ������� E-Debug.cpp
//

static HANDLE GethProcess() {           //���������Խ��̵ľ��
	return (HANDLE)*(DWORD*)0x4D5A68;
}

class CEDebugApp : public CWinApp
{
public:
	CEDebugApp();

// ��д
public:
	virtual BOOL InitInstance();
	
	DECLARE_MESSAGE_MAP()
};
