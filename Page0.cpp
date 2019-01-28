// Page0.cpp : ʵ���ļ�
//

#include "stdafx.h"
#include "E-Debug.h"
#include "Page0.h"
#include "MainWindow.h"
#include "TrieTree.h"

extern map<string, LIBMAP> m_LibMap;
extern vector<CDialog*> Tab_HWND;
extern CMainWindow *pMaindlg;

// CPage0 �Ի���

IMPLEMENT_DYNAMIC(CPage0, CDialog)

CPage0::CPage0(CWnd* pParent /*=NULL*/)
	: CDialog(IDD_PAGE0, pParent)
{

}

CPage0::~CPage0()
{

}

void CPage0::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_LIST1, m_List);
}


BEGIN_MESSAGE_MAP(CPage0, CDialog)
	ON_NOTIFY(NM_RCLICK, IDC_LIST1, &CPage0::OnNMRClickList1)
	ON_COMMAND(ID_32772, &CPage0::On32772)
END_MESSAGE_MAP()

CDialog* CPage0::GetHwnd() {
	return this;
}

// CPage0 ��Ϣ�������
BOOL CPage0::OnInitDialog() {

	CDialog::OnInitDialog();

	LONG lStyle;

	lStyle = GetWindowLong(m_List.m_hWnd, GWL_STYLE);//��ȡ��ǰ����style
	lStyle &= ~LVS_TYPEMASK; //�����ʾ��ʽλ
	lStyle |= LVS_REPORT; //����style
	SetWindowLong(m_List.m_hWnd, GWL_STYLE, lStyle);//����style

	DWORD dwStyle = m_List.GetExtendedStyle();
	dwStyle |= LVS_EX_FULLROWSELECT;//ѡ��ĳ��ʹ���и�����ֻ������report����listctrl��
	dwStyle |= LVS_EX_GRIDLINES;//�����ߣ�ֻ������report����listctrl��
	m_List.SetExtendedStyle(dwStyle); //������չ���

	m_List.InsertColumn(0, L"�ļ�", LVCFMT_LEFT, 175);
	m_List.InsertColumn(1, L"ѡ��", LVCFMT_CENTER, 80);
	m_List.InsertColumn(2, L"����", LVCFMT_CENTER, 270);

	USES_CONVERSION;

	for (UINT n = 0;n < EsigList.size();n++) {
		m_List.InsertItem(n, A2W(EsigList[n].Name.c_str()));		
		m_List.SetItemText(n, 2, A2W(EsigList[n].Description.c_str()));
	}

	return true;
}

void CPage0::OnNMRClickList1(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMITEMACTIVATE pNMItemActivate = reinterpret_cast<LPNMITEMACTIVATE>(pNMHDR);
	// TODO: �ڴ���ӿؼ�֪ͨ����������
	static CMenu Menu;
	static CMenu *pSubMenu;
	if (Menu == NULL) {
		Menu.LoadMenu(IDR_MENU2);
		pSubMenu = Menu.GetSubMenu(0);
	}
	if (pNMItemActivate->iItem != -1) {
		DWORD dwPos = GetMessagePos();
		CPoint point(LOWORD(dwPos), HIWORD(dwPos));
		pSubMenu->TrackPopupMenu(TPM_LEFTALIGN | TPM_RIGHTBUTTON, point.x, point.y, this);
	}
	*pResult = 0;
}


void CPage0::On32772()	//Ӧ������
{
	USES_CONVERSION;
	int nPos = m_List.GetSelectionMark();
	if (nPos == -1) {
		return;
	}

	nPos = m_List.GetNextItem(-1, LVNI_SELECTED);
	while (nPos != -1) {
		CString	strCom = m_List.GetItemText(nPos, 0);

		EsigInfo info;
		for (UINT n = 0;n < EsigList.size();n++) {
			if (strCom.Compare(A2W(EsigList[n].Name.c_str())) == 0) {
				info = EsigList[n];
				break;
			}
		}

		int flag = m_List.GetItemText(nPos, 1).Compare(L"��Ӧ��");	//�Ƿ���Ӧ�ù�����

		m_LibMap[info.Name].Command_addr.clear();
		m_LibMap[info.Name].Command_name.clear();

		if (flag == 0) {		//����Ѿ�Ӧ�ù���ɾ��ԭ������
			LVFINDINFO findinfo;
			findinfo.flags = LVFI_PARTIAL | LVFI_STRING;
			findinfo.psz = A2W(info.Name.c_str());
			for (UINT n = 0;n < pMaindlg->m_page1.m_lib.GetItemCount();n++) {
				if (pMaindlg->m_page1.m_lib.GetItemText(n, 0).Find(A2W(info.Name.c_str())) != -1) {		
					pMaindlg->m_page1.m_lib.DeleteItem(n);
					pMaindlg->m_page1.m_lib.DeleteItem(n);
					pMaindlg->m_page1.m_lib.DeleteItem(n);
					break;
				}
			}		
		}

		TrieTree Tree;
		Tree.LoadSig(info.Path.c_str());
		
		Tree.ScanSig(pEAnalysisEngine->SectionMap[0].SectionAddr, pEAnalysisEngine->SectionMap[0].dwSize, info.Name);


		UINT Pos = 0;

		CString LibStr;

		if (m_LibMap[info.Name].Command_addr.size() != 0) {		
			LibStr.Format(L"%s(��������:%d)", A2W(info.Name.c_str()), m_LibMap[info.Name].Command_addr.size());
			pMaindlg->m_page1.m_lib.InsertItem(Pos, LibStr);Pos++;

			LibStr.Format(L"   %s", A2W(info.Description.c_str()));
			pMaindlg->m_page1.m_lib.InsertItem(Pos, LibStr);Pos++;
			pMaindlg->m_page1.m_lib.InsertItem(Pos, L"�D�D�D�D�D�D�D�D�D�D�D�D�D�D�D�D�D�D�D�D�D�D�D�D");Pos++;
		}

		m_List.SetItemText(nPos, 1, L"��Ӧ��");
		nPos = m_List.GetNextItem(nPos, LVNI_SELECTED);	
	}
	// TODO: �ڴ���������������
}
