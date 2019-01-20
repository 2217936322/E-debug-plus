#pragma once
#include "E-Debug.h"

typedef struct LIBMAP
{
	vector<string>  Command_name;
	vector<DWORD>   Command_addr;
}*LibMap;


#define MAX_ESIZE 256


// CPage1 �Ի���

class CPage1 : public CDialog
{
	DECLARE_DYNAMIC(CPage1)

public:
	CPage1(CWnd* pParent = NULL);   // ��׼���캯��
	virtual ~CPage1();

// �Ի�������
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_PAGE1 };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV ֧��

	DECLARE_MESSAGE_MAP()
public:
	CDialog* GetHwnd();
	virtual BOOL OnInitDialog();
	BOOL MatchCode_FAST(UCHAR* FuncSrc, UCHAR* BinCode, int length);  //����һ��������Ա�,������Ϊ�Աȳ���
	bool MatchCode_UnEx(unsigned char* pSrc1, unsigned char* pSrc2, int nLen);//
	CListCtrl m_lib;
	CListCtrl m_command;
	map<int, LIBMAP> m_map;
	afx_msg void OnNMClickListlib(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnLvnColumnclickListcommand(NMHDR *pNMHDR, LRESULT *pResult);
	
	afx_msg void OnNMDblclkListcommand(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnNMRClickListcommand(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void On32771();

	UINT Currentindex = 0;

	map<string, string> m_subFunc;
	map<string, string> m_Func;

	map<ULONG, string> m_RFunc;  //R����Runtime

	BOOL IsValidAddr(ULONG addr);

	BOOL MatchCode(UCHAR* FuncAddr, string& FuncTxt);//������ַ�뺯���ı���ƥ��

};
