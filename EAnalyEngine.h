#pragma once

#include "E-Debug.h"

static BOOL ReadSig(const char *lpMapPath, map<string, string>& m_subFunc, map<string, string>& m_Func)		//����һΪ·��,������Ϊ���ص��Ӻ��������ı�,������Ϊ���صĺ��������ı�
{
	HANDLE hFile = CreateFileA(lpMapPath, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if (hFile == INVALID_HANDLE_VALUE)
	{
		return false;
	}
	m_subFunc.clear();
	m_Func.clear();
	DWORD	dwHitSize = 0;
	DWORD	dwSize = GetFileSize(hFile, &dwHitSize);
	DWORD	dwReadSize;

	char* pMap = (char*)malloc(dwSize);
	ReadFile(hFile, pMap, dwSize, &dwReadSize, NULL);
	string Sig = pMap;

	int delimiter = Sig.find("******");   //�ֽ��
	if (delimiter == -1) {
		return false;
	}
	string SubFunc = Sig.substr(0, delimiter);

	int pos = SubFunc.find_first_of("\r\n");     //�Ӻ���
	while (pos != -1) {
		string temp = SubFunc.substr(0, pos);  //�����Ӻ���
		int tempos = temp.find(':');
		if (tempos == -1) {
			break;
		}
		m_subFunc[temp.substr(0, tempos)] = temp.substr(tempos + 1);
		SubFunc = SubFunc.substr(pos + 2);
		pos = SubFunc.find("\r\n");
	}

	string Func = Sig.substr(delimiter + 8);    //ȫ�������ı�
	pos = Func.find("\r\n");

	while (pos != -1) {
		string temp = Func.substr(0, pos);    //�����Ӻ���
		int tempos = temp.find(':');
		if (tempos == -1) {
			break;
		}
		m_Func[temp.substr(0, tempos)] = temp.substr(tempos + 1);
		Func = Func.substr(pos + 2);
		pos = Func.find("\r\n");
	}

	if (pMap) {
		free(pMap);
	}

	CloseHandle(hFile);
	return TRUE;
}

typedef struct sectionAlloc		//�ڴ濽��
{
	BYTE* SectionAddr;  //������ڴ�ռ��ַ
	DWORD dwBase;       //ԭʼ�������εĻ�ַ
	DWORD dwSize;       //ԭʼ�������εĴ�С
}*psectionAlloc;

typedef struct _ENTRYINFO // �����������Ϣ
{
	DWORD	dwMagic;		//<- δ֪
	DWORD	dwUnkown1;		//+4 δ֪
	DWORD	dwUnkown2;		//+8 δ֪
	DWORD	dwUserCodeStart;//+c �û����뿪ʼ
	DWORD	dwEString;		//+10 �ַ��� ��Դ       ���û���ַ�����Դ,��Ϊ0
	DWORD	dwEStringSize;	//+14 �ַ��� ��Դ��С   ���û���ַ�����Դ,��Ϊ0
	DWORD	dwEWindow;		//+18 ���������Ϣ      �������ڡ���ǩ�ȿؼ�
	DWORD	dwEWindowSize;	//+1c ��С
	DWORD	dwLibNum;		//+20 ֧�ֿ�����
	DWORD	pLibEntey;		//+24 ֧�ֿ���Ϣ���
	DWORD	dwApiCount;     //+28 Api����
	DWORD	pLibName;		//+2C ָ�������
	DWORD	pApiName;		//+30 ָ��Api����
}*PEENTRYINFO;



class EAnalysis
{
public:
	EAnalysis(ULONG dwVBase);
	~EAnalysis();
	
	BOOL EStaticLibInit();     //��̬����--��ʼ��
	BOOL GetUserEntryPoint();  //��̬����--ȡ�û�������ַ

	UINT FindSection(DWORD addr); //Ѱ�ҵ�ַ�Ƿ������α���,����index
	UINT AddSection(DWORD addr);  //�ڴ濽��������������,�����µ�index

	DWORD   Search_BinEx(byte *pSrc, byte *pTrait, int nSrcLen, int nTraitLen);
	DWORD	O2V(DWORD dwVaddr, UINT index);		//ԭʼ��ַת��Ϊ�����ַ
	DWORD	V2O(DWORD dwOaddr, UINT index);		//�����ַת��Ϊԭʼ��ַ


	DWORD	GetPoint(DWORD dwAddr);
	DWORD	GetOriginPoint(DWORD dwAddr, UINT index);
	DWORD dwUsercodeStart; //�û��������ʼ��ַ
	DWORD dwUsercodeEnd;   //�û�����Ľ�����ַ

	vector<sectionAlloc> SectionMap;    //ά��һ���ڴ濽����

	ULONG DLLCALL=0;		//����DLL_CALL��ַ,�ڲ���DLL�����ʱ����õ�


	PEENTRYINFO pEnteyInfo; // entry info
private:
protected:
};