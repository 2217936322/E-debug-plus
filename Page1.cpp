/*��������������������������������������������������������
E-debug   ����ʶ��
��������������������������������������������������������*/

#include "stdafx.h"
#include "E-Debug.h"
#include "Page1.h"
#include "EAnalyEngine.h"
#include "Elib.h"
#include "MainWindow.h"
#include "TrieTree.h"

extern  EAnalysis	*pEAnalysisEngine;
extern 	CMainWindow *pMaindlg;

char DIRECTORY[MAX_PATH];
static int addrtype;
static int nametype;

IMPLEMENT_DYNAMIC(CPage1, CDialog)

CPage1::CPage1(CWnd* pParent /*=NULL*/)
	: CDialog(IDD_PAGE1, pParent)
{
	
}

CPage1::~CPage1()
{
	m_map.clear();
}

void CPage1::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_LISTLib, m_lib);
	DDX_Control(pDX, IDC_LISTCommand, m_command);
}


BEGIN_MESSAGE_MAP(CPage1, CDialog)
	ON_NOTIFY(NM_CLICK, IDC_LISTLib, &CPage1::OnNMClickListlib)
	ON_NOTIFY(LVN_COLUMNCLICK, IDC_LISTCommand, &CPage1::OnLvnColumnclickListcommand)
	ON_NOTIFY(NM_DBLCLK, IDC_LISTCommand, &CPage1::OnNMDblclkListcommand)
	ON_NOTIFY(NM_RCLICK, IDC_LISTCommand, &CPage1::OnNMRClickListcommand)
	ON_COMMAND(ID_32771, &CPage1::On32771)
END_MESSAGE_MAP()


// CPage1 ��Ϣ�������
BOOL CPage1::OnInitDialog() {
	CDialog::OnInitDialog();

	LONG lStyle;

	lStyle = GetWindowLong(m_lib.m_hWnd, GWL_STYLE);//��ȡ��ǰ����style
	lStyle &= ~LVS_TYPEMASK; //�����ʾ��ʽλ
	lStyle |= LVS_REPORT; //����style
	SetWindowLong(m_lib.m_hWnd, GWL_STYLE, lStyle);//����style

	lStyle = GetWindowLong(m_command.m_hWnd, GWL_STYLE);//��ȡ��ǰ����style
	lStyle &= ~LVS_TYPEMASK; //�����ʾ��ʽλ
	lStyle |= LVS_REPORT; //����style
	SetWindowLong(m_command.m_hWnd, GWL_STYLE, lStyle);//����style

	DWORD dwStyle = m_lib.GetExtendedStyle();
	dwStyle |= LVS_EX_FULLROWSELECT;//ѡ��ĳ��ʹ���и�����ֻ������report����listctrl��
									//dwStyle |= LVS_EX_GRIDLINES;//�����ߣ�ֻ������report����listctrl��
	m_lib.SetExtendedStyle(dwStyle); //������չ���

	dwStyle = m_command.GetExtendedStyle();
	dwStyle |= LVS_EX_FULLROWSELECT;//ѡ��ĳ��ʹ���и�����ֻ������report����listctrl��
	dwStyle |= LVS_EX_GRIDLINES;//�����ߣ�ֻ������report����listctrl��
	m_command.SetExtendedStyle(dwStyle); //������չ���


	m_lib.InsertColumn(0, _T("֧�ֿ���Ϣ"), LVCFMT_LEFT, 250);

	m_command.InsertColumn(0, L"��ַ", LVCFMT_LEFT, 65);
	m_command.InsertColumn(1, L"��������", LVCFMT_CENTER, 205);

	int			nPos = 0;
	DWORD		pFirst = pEAnalysisEngine->pEnteyInfo->pLibEntey;

	PLIB_INFO	pLibInfo = NULL;
	CString		strLib, strGuid;
	CString		str;

	UINT r_index = pEAnalysisEngine->FindSection(pFirst);
	if (r_index == -1) {
		r_index = pEAnalysisEngine->AddSection(pFirst);
	}

	Currentindex = r_index;
	INT ProgressAdd = 500 / pEAnalysisEngine->pEnteyInfo->dwLibNum;
	for (UINT i = 0; i < pEAnalysisEngine->pEnteyInfo->dwLibNum; i++)  //���ڽ���������ÿ��֧�ֿ�
	{
		pLibInfo = (PLIB_INFO)pEAnalysisEngine->O2V(pEAnalysisEngine->GetOriginPoint(pFirst, r_index), r_index);
		strLib.Format(L"---->%s (Ver:%1d.%1d)",
			(CString)(char*)pEAnalysisEngine->O2V((DWORD)pLibInfo->m_szName, r_index),
			pLibInfo->m_nMajorVersion,
			pLibInfo->m_nMinorVersion);
		strGuid.Format(L"        %s", (CString)(char*)pEAnalysisEngine->O2V((DWORD)pLibInfo->m_szGuid, r_index));

		m_lib.InsertItem(nPos, strLib); nPos++;   //��ʾLib����(Ver:�汾��)
		m_lib.InsertItem(nPos, strGuid); nPos++;  //��ʾLib��GUID

		str.Empty();
		str.Format(L"   -> �������� (����:%d)", pLibInfo->m_nCmdCount); //��ʾ������������

		DWORD		pFunc = pEAnalysisEngine->O2V((DWORD)pLibInfo->m_pCmdsFunc, r_index);
		DWORD		dwAddress;

		char szLibVer[12] = { 0 };
		wsprintfA(szLibVer, "\\%1d.%1d", pLibInfo->m_nMajorVersion, pLibInfo->m_nMinorVersion);

		char szDirectory[MAX_PATH] = {};
		StrCpyA(szDirectory, DIRECTORY);

		strcat_s(szDirectory, "\\Plugin\\Esig\\");strcat_s(szDirectory, (char*)pEAnalysisEngine->O2V((DWORD)pLibInfo->m_szGuid, r_index));
		strcat_s(szDirectory, szLibVer);strcat_s(szDirectory, ".Esig");


		BOOL Sret = LoadSig(szDirectory,m_subFunc,m_Func);    //��ȡESig�ļ�
		LIBMAP m_Libmap;

		m_Libmap.Command_addr.clear();
		m_Libmap.Command_name.clear();
		if (Sret == false) {    //�����ȡ����Sig�ļ�
			for (int n = 0;n < pLibInfo->m_nCmdCount;n++) {
				dwAddress = pEAnalysisEngine->GetPoint(pFunc);
				m_Libmap.Command_addr.push_back(dwAddress);
				m_Libmap.Command_name.push_back("Esig Not Founded");
				pFunc += sizeof(int);
			}
		}
		else {
			for (int n = 0;n < pLibInfo->m_nCmdCount;n++) {     //���ڳ����е�ÿ������,����һ�ξ�ȷƥ��
				dwAddress = pEAnalysisEngine->GetPoint(pFunc);
				m_Libmap.Command_addr.push_back(dwAddress);
				BOOL bMatchCom = false;
				map<string, string>::iterator it;
				for(it=m_Func.begin();it!=m_Func.end();it++){
					if (MatchCode((UCHAR*)pEAnalysisEngine->O2V(dwAddress, 0), it->second)) {
						m_Libmap.Command_name.push_back(it->first);
						Insertname(dwAddress, NM_LABEL,(char*)it->first.c_str());
						bMatchCom = true;
						m_Func.erase(it);
						Progress(pMaindlg->promile = pMaindlg->promile + 1, "����ʶ��֧�ֿ�����...");
						break;
					}

				}

				if (!bMatchCom)
				{
					m_Libmap.Command_name.push_back("Error");
					Insertname(dwAddress, NM_LABEL, "δ֪����");
				}

				pFunc += sizeof(int);
			}
		}

		m_map[nPos] = m_Libmap;
		m_lib.InsertItem(nPos, str);nPos++;
		m_lib.InsertItem(nPos, L"�D�D�D�D�D�D�D�D�D�D�D�D�D�D�D�D�D�D�D�D�D�D�D�D"); nPos++;

		Progress(pMaindlg->promile = pMaindlg->promile + ProgressAdd, "����ʶ��֧�ֿ�...");
		pFirst += sizeof(DWORD);
	}
	
	//������ɨ����������롪����
	TrieTree Tree;

	char szDirectory[MAX_PATH] = {};
	StrCpyA(szDirectory, DIRECTORY);
	strcat_s(szDirectory, "\\Plugin\\Esig\\Emain.Esig");
	map<string, string> m_temp;
	map<string, string> m_basic;
	LoadSig(szDirectory, m_temp, m_basic);//���Emain.Esig��������
	ProgressAdd = 300 / m_basic.size();
	
	map<string, string>::iterator it;

	for (it = m_basic.begin();it != m_basic.end();it++) {
		Tree.Insert(it->second,it->first);
	}

	Tree.MatchSig((UCHAR*)pEAnalysisEngine->O2V(pEAnalysisEngine->dwUsercodeStart, 0), pEAnalysisEngine->dwUsercodeEnd - pEAnalysisEngine->dwUsercodeStart);

	
	Progress(1000, "����ɨ���������,��ȴ�......");
	Progress(0, "");
	Infoline("ʶ���������...");
	
	pMaindlg->outputInfo("->  ����������<KrnlLibCmd>&&<LibCmd>���...");
	return true;
}

BOOL CPage1::IsValidAddr(ULONG addr) {		//����
	if (addr > pEAnalysisEngine->SectionMap[0].dwBase + pEAnalysisEngine->SectionMap[0].dwSize || addr < pEAnalysisEngine->SectionMap[0].dwBase) {
		return false;
	}
	return true;
}

BOOL CPage1::MatchCode_FAST(UCHAR* FuncSrc, UCHAR* BinCode, int nLen)  //����һ��������Ա�,������Ϊ�Աȳ���
{
	if (nLen == 0)
	{
		return FALSE;
	}
	for (int i = 0; i < nLen; i++)
	{
		if (FuncSrc[i] != BinCode[i])
			return FALSE;
	}
	return TRUE;
}

BOOL CPage1::MatchCode(UCHAR* FuncSrc,string& FuncTxt)  //����һΪ�����ַ,������Ϊ�����ı�
{
	USES_CONVERSION;
	UCHAR* pSrc = FuncSrc;  //��ʼ����������ָ��

	for (UINT n = 0;n < FuncTxt.length();n++) {
		if (FuncTxt[n] == '-' && FuncTxt[n + 1] == '-' && FuncTxt[n + 2] == '>') {    //--> ����ת,��δͨ��ʵ����֤
			if (*pSrc != 0xE9) {
				return false;
			}

			DWORD offset = *(DWORD*)(pSrc + 1);
			pSrc = pSrc + offset + 5;
			if (IsBadReadPtr(pSrc, 4) != 0) {
				return false;
			}

			n = n + 2;
			continue;
		}
		else if (FuncTxt[n] == '<' && FuncTxt[n + 1] == '[')  //FF15 CALL
		{
			if (*pSrc != 0xFF || *(pSrc + 1) != 0x15 || IsBadReadPtr((ULONG*)(pSrc + 2), 4) != 0) {      //����VMP��SE����
				return false;
			}

			int post = FuncTxt.find("]>", n);
			if (post == -1) {
				return false;
			}

			string IATEAT = FuncTxt.substr(n + 2, post - n - 2);   //�õ��ı��е�IAT����

			CString IATCom;
			CString EATCom;
			char buffer[256] = { 0 };
			int EATpos = IATEAT.find("||");
			if (EATpos != -1) {            //�����Զ���EAT
				IATCom = IATEAT.substr(0, EATpos).c_str();
				EATCom = IATEAT.substr(EATpos + 2).c_str();
			}
			else
			{
				IATCom = IATEAT.c_str();
				EATCom = IATEAT.substr(IATEAT.find('.') + 1).c_str();
			}

			ULONG addr = *(ULONG*)(pSrc + 2);
			if (Findname(addr, NM_IMPORT, buffer) != 0 && IATCom.CompareNoCase(A2W(buffer)) == 0) {  //����IATƥ��
				pSrc = pSrc + 6;
				n = post + 1;
				continue;
			}
			if (Findname(*(ULONG*)pEAnalysisEngine->O2V(addr, Currentindex), NM_EXPORT, buffer) != 0 && EATCom.CompareNoCase(A2W(buffer)) == 0) {      //EATƥ��
				pSrc = pSrc + 6;
				n = post + 1;
				continue;
			}
			return false;
		}
		else if (FuncTxt[n] == '<')    //��ͨCALL
		{
			if (*pSrc != 0xE8) {   //�����ж�
				return false;
			}

			int post = FuncTxt.find('>', n);
			if (post == -1) {
				return false;
			}
			string SubFunc = FuncTxt.substr(n + 1, post - n - 1);   //�Ӻ�������
			DWORD addr = (DWORD)pSrc + *(DWORD*)(pSrc + 1) + 5;

			if (m_RFunc[addr] == SubFunc) {       //���Ȳ�ѯ�˺����Ƿ���ƥ���
				pSrc = pSrc + 5;
				n = post;
				continue;
			}
			if (IsBadReadPtr((ULONG*)addr, 4) != 0) {
				return false;
			}
			if (MatchCode((UCHAR*)addr, m_subFunc[SubFunc])) {
				pSrc = pSrc + 5;
				n = post;
				m_RFunc[addr] = SubFunc;
				continue;
			}
			return false;
		}
		else if (FuncTxt[n] == '[' && FuncTxt[n + 1] == ']' && FuncTxt[n + 2] == '>')   //FF25������ת
		{
			if (*pSrc != 0xFF || *(pSrc + 1) != 0x25) {
				return false;
			}

			ULONG jmpaddr = *(ULONG*)(pSrc + 2);  //�õ���ź�����ַ��ָ��

			UINT r_index = pEAnalysisEngine->FindSection(jmpaddr);
			if (r_index == -1) {
				r_index = pEAnalysisEngine->AddSection(jmpaddr);
			}

			jmpaddr = *(ULONG*)pEAnalysisEngine->O2V(jmpaddr, r_index);   //�õ�������ַ
			if (jmpaddr == 0) {
				return false;
			}

			r_index = pEAnalysisEngine->FindSection(jmpaddr);
			if (r_index == -1) {
				r_index = pEAnalysisEngine->AddSection(jmpaddr);
			}
			Currentindex = r_index;

			pSrc = (UCHAR*)pEAnalysisEngine->O2V(jmpaddr, Currentindex); //������ʵ��ַת��Ϊ�����ַ

			n = n + 2;
			continue;
		}
		else if (FuncTxt[n] == '[')  //FF25 IAT��ת
		{
			if (*pSrc != 0xFF || *(pSrc + 1) != 0x25) {
				return false;
			}

			int post = FuncTxt.find(']', n);
			if (post == -1) {
				return false;
			}
			string IATEAT = FuncTxt.substr(n + 1, post - n - 1);

			if (IsBadReadPtr((ULONG*)(pSrc + 2), 4) != 0) {   //���ָ��Ϸ���
				return false;
			}

			CString IATCom;
			CString EATCom;
			char buffer[256] = { 0 };
			int EATpos = IATEAT.find("||");
			if (EATpos != -1) {            //�����Զ���EAT
				IATCom = IATEAT.substr(0, EATpos).c_str();
				EATCom = IATEAT.substr(EATpos + 2).c_str();
			}
			else
			{
				IATCom = IATEAT.c_str();
				EATCom = IATEAT.substr(IATEAT.find('.') + 1).c_str();
			}

			ULONG addr = *(ULONG*)(pSrc + 2);
			if (Findname(addr, NM_IMPORT, buffer) != 0 && IATCom.CompareNoCase(A2W(buffer)) == 0) {  //����IATƥ��
				pSrc = pSrc + 6;
				n = post;
				continue;
			}
			if (Findname(*(ULONG*)pEAnalysisEngine->O2V(addr, Currentindex), NM_EXPORT, buffer) != 0 && EATCom.CompareNoCase(A2W(buffer)) == 0) {      //EATƥ��
				pSrc = pSrc + 6;
				n = post;
				continue;
			}
			return false;
		}
		else if (FuncTxt[n] == '?')   //ģ��ƥ��
		{
			n++;
			pSrc++;
			continue;
		}
		else     //��ͨ����
		{
			UCHAR ByteCode=0;
			HexToBin(FuncTxt.substr(n, 2), &ByteCode);
			if (*pSrc == ByteCode) {
				n++;
				pSrc++;
				continue;
			}
			//pMaindlg->outputInfo("ʧ�ܵ�ַ:%X", pEAnalysisEngine->V2O((DWORD)pSrc, 0));

			//pMaindlg->outputInfo("ʧ����:%X", FuncSrc);
			return false;
		}
	}

	return TRUE;
}

bool CPage1::MatchCode_UnEx(unsigned char* pSrc1, unsigned char* pSrc2, int nLen)
{
	float Count = 0;
	if (nLen == 0)
	{
		return FALSE;
	}
	for (int i = 0; i < nLen; i++)
	{
		if (pSrc2[i] == 0x90)//ģ��ƥ��
		{
			Count++;
			continue;
		}
		if (pSrc1[i] != pSrc2[i])
		{
			continue;
		}
		Count++;
	}
	if (Count / (float)nLen > 0.75f) {   
		return TRUE;
	}
	else {
		return false;
	}
}



void CPage1::OnNMClickListlib(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMITEMACTIVATE pNMItemActivate = reinterpret_cast<LPNMITEMACTIVATE>(pNMHDR);
	USES_CONVERSION;
	CString		strAddress;
	int	nPos= m_lib.GetSelectionMark();
	int nCmdCount = m_map[nPos].Command_addr.size();

	if (nCmdCount) {
		m_command.DeleteAllItems();
		for (int n = 0;n < nCmdCount;n++) {
			strAddress.Format(L"%08X", m_map[nPos].Command_addr[n]);
			m_command.InsertItem(n, strAddress);
			m_command.SetItemData(n, m_map[nPos].Command_addr[n]);
			m_command.SetItemText(n, 1,A2W(m_map[nPos].Command_name[n].c_str()));
		}
	}

	addrtype = 0;
	nametype = 0;
	*pResult = 0;
}

static int CALLBACK CompareAddr(LPARAM lParam1, LPARAM lParam2, LPARAM lParamSort) {
	if (lParamSort == 0) {
		return lParam1 > lParam2;
	}
	else if (lParamSort == 1) {
		return lParam2 > lParam1;
	}
	return 0;
}

static int CALLBACK CompareName(LPARAM lParam1, LPARAM lParam2, LPARAM lParamSort) {
	if (lParamSort == 0) {          //�����ı�ASCII�Ƚ�
		UCHAR x1 = *(UCHAR*)pMaindlg->m_page1.m_command.GetItemText(static_cast<int>(lParam1), 1).GetBuffer();
		UCHAR x2 = *(UCHAR*)pMaindlg->m_page1.m_command.GetItemText(static_cast<int>(lParam2), 1).GetBuffer();
		return x1 > x2;
	}
	else if (lParamSort == 1) {     //�����ı����ȱȽ�
		return pMaindlg->m_page1.m_command.GetItemText(static_cast<int>(lParam1), 1).GetLength() > pMaindlg->m_page1.m_command.GetItemText(static_cast<int>(lParam2), 1).GetLength();
	}
	return 0;
}

void CPage1::OnLvnColumnclickListcommand(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMLISTVIEW pNMLV = reinterpret_cast<LPNMLISTVIEW>(pNMHDR);
	if (pNMLV->iSubItem == 0) {
		m_command.SortItems(CompareAddr, (DWORD_PTR)addrtype);
		if (addrtype == 0) {  //��������
			addrtype = 1;
		}
		else {                //��������
			addrtype = 0;
		}
	}
	else if (pNMLV->iSubItem == 1) {
		m_command.SortItemsEx(CompareName, (DWORD_PTR)nametype);
		if (nametype == 0) {       //����ASCII����
			nametype = 1;
		}
		else {                    //���ճ�������
			nametype = 0;
		}
	}
	*pResult = 0;
}



void CPage1::OnNMDblclkListcommand(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMITEMACTIVATE pNMItemActivate = reinterpret_cast<LPNMITEMACTIVATE>(pNMHDR);
	
	if (m_command.GetSelectedCount() <= 0) {    //˵��δѡȡ������
		return;
	}
	int nPos = m_command.GetSelectionMark();
	if (nPos == -1) {
		return;
	}
	int dwData = m_command.GetItemData(nPos);
	if (!dwData)
		return;

	Setcpu(0, dwData, 0, 0, CPU_NOFOCUS);

	*pResult = 0;
}


void CPage1::OnNMRClickListcommand(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMITEMACTIVATE pNMItemActivate = reinterpret_cast<LPNMITEMACTIVATE>(pNMHDR);
	
	CMenu Menu;
	CMenu *pSubMenu;
	if (pNMItemActivate->iItem != -1) {
		DWORD dwPos = GetMessagePos();
		CPoint point(LOWORD(dwPos), HIWORD(dwPos));
		Menu.LoadMenu(IDR_MENU1);
		pSubMenu = Menu.GetSubMenu(0);
		pSubMenu->TrackPopupMenu(TPM_LEFTALIGN | TPM_RIGHTBUTTON, point.x, point.y, this);
	}
	*pResult = 0;
}

void CPage1::On32771()   //�������ð�ť
{
	USES_CONVERSION;
	int nPos = m_command.GetSelectionMark();
	if (nPos == -1) {
		return;
	}
	DWORD dwData = m_command.GetItemData(nPos);
	if (!dwData) {
		return;
	}

	CString	strCom= m_command.GetItemText(nPos, 1);

	pMaindlg->m_output.ResetContent();
	pMaindlg->outputInfo("-> ִ������   --==��������==--");
	byte ComCall[5] = { 0xBB, 0, 0, 0, 0 };
	memcpy(&ComCall[1], &dwData, sizeof(DWORD));
 	byte *pTmp = (byte*)pEAnalysisEngine->O2V(pEAnalysisEngine->dwUsercodeStart,0);

	DWORD	dwSecSize = pEAnalysisEngine->dwUsercodeEnd - pEAnalysisEngine->dwUsercodeStart;
	DWORD	dwResult = pEAnalysisEngine->dwUsercodeStart;    //��Ѱ�����ַ
	DWORD   dwCount = 0;   //�������ֽ���
	while (true) {
		DWORD offset = pEAnalysisEngine->Search_BinEx(pTmp, ComCall, dwSecSize, sizeof(ComCall));//�õ�ƫ�Ƶ�ַ
		if (offset == 0)
			break;
		dwResult += offset;
		int index= pMaindlg->outputInfo("%08X    mov ebx,%08X    //%s", dwResult, dwData, W2A(strCom)); //��ʾ�������ַ
		pMaindlg->m_output.SetItemData(index, dwResult);
		dwResult += sizeof(ComCall);
		pTmp += offset+sizeof(ComCall);		
		dwSecSize -= offset + sizeof(ComCall);
	}
}
