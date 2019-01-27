#include "stdafx.h"
#include "MainWindow.h"
#include "TrieTree.h"

extern  CMainWindow *pMaindlg;

map<string, LIBMAP> m_LibMap;	//������ʾ���

TrieTreeNode::TrieTreeNode() {
	ChildNodes = new TrieTreeNode*[256];
	for (int i = 0;i < 256;i++) {
		ChildNodes[i] = NULL;
	}
	EsigText = NULL;
	FuncName = NULL;
	IsMatched = false;
}



BOOL TrieTree::LoadSig(const char* lpMapPath) {
	HANDLE hFile = CreateFileA(lpMapPath, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);

	if (hFile == INVALID_HANDLE_VALUE)
	{
		return false;
	}

	DWORD	dwHitSize = 0;
	DWORD	dwSize = GetFileSize(hFile, &dwHitSize);
	DWORD	dwReadSize = 0;
	char* pMap = (char*)malloc(dwSize);

	ReadFile(hFile, pMap, dwSize, &dwReadSize, NULL);	//��ȡ�ı����ڴ�

	string Sig = pMap;

	string Config = GetMidString(Sig, "******Config******\r\n", "******Config_End******", 0);

	if (Config.find("IsAligned=true") != -1) {
		IsAligned = true;
	}
	else
	{
		IsAligned = false;
	}

	string SubFunc = GetMidString(Sig, "*****SubFunc*****\r\n", "*****SubFunc_End*****", 0);

	int pos = SubFunc.find("\r\n");     //�Ӻ���

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


	string Func = GetMidString(Sig, "***Func***\r\n", "***Func_End***", 0);

	pos = Func.find("\r\n");

	while (pos != -1) {
		string temp = Func.substr(0, pos);    //�����Ӻ���
		int tempos = temp.find(':');
		if (tempos == -1) {
			break;
		}
		if (!Insert(temp.substr(tempos + 1), temp.substr(0, tempos))) {
			MessageBoxA(NULL, "���뺯��ʧ��", "12", 0);
		}
		Func = Func.substr(pos + 2);
		pos = Func.find("\r\n");
	}


	if (pMap) {
		free(pMap);
	}

	CloseHandle(hFile);
	return TRUE;
}

char* TrieTree::MatchSig(UCHAR* CodeSrc) {
	TrieTreeNode* p = root;		//��ǰָ��ָ��root
	return Match(p, (UCHAR*)CodeSrc);
}

void TrieTree::ScanSig(UCHAR* CodeSrc, ULONG SrcLen, string& LibName) {

	TrieTreeNode* p = root;		//��ǰָ��ָ��root

	if (IsAligned) {
		for (ULONG offset = 0;offset < SrcLen-16;offset = offset + 16) {		//��һ������û���������ɨ�赽�߽�Ĵ���,��ʱ-16����
			char* FuncName = Match(p, (UCHAR*)CodeSrc + offset);
			if (FuncName) {
				DWORD dwAddr = pEAnalysisEngine->V2O((DWORD)CodeSrc + offset, 0);
				m_LibMap[LibName].Command_addr.push_back(dwAddr);
				m_LibMap[LibName].Command_name.push_back(FuncName);
				Insertname(dwAddr, NM_LABEL, FuncName);
			}
		}
	}
	else {
		for (ULONG offset = 0;offset < SrcLen - 16;offset++) {			
			char* FuncName = Match(p, (UCHAR*)CodeSrc + offset);
			if (FuncName) {
				DWORD dwAddr = pEAnalysisEngine->V2O((DWORD)CodeSrc + offset, 0);
				m_LibMap[LibName].Command_addr.push_back(dwAddr);
				m_LibMap[LibName].Command_name.push_back(FuncName);
				Insertname(dwAddr, NM_LABEL, FuncName);
			}
		}
	}
	return;
}

BOOL TrieTree::Insert(string& FuncTxt, const string& FuncName) {		//����һΪ�������ı���ʽ,������Ϊ����������
	TrieTreeNode *p = root;		//����ǰ�ڵ�ָ��ָ��ROOT�ڵ�

	string BasicTxt;
	string SpecialTxt;

	for (UINT n = 0;n < FuncTxt.length();n++) {
		switch (FuncTxt[n])
		{
		case '-':	//Check 1��
			if (FuncTxt[n + 1] == '-' && FuncTxt[n + 2] == '>')
			{
				p = AddSpecialNode(p, TYPE_LONGJMP, "");
				n = n + 2;
				continue;		//��continue�����ⲿѭ��
			}
			return false;
		case '<':
			if (FuncTxt[n + 1] == '[') {						//CALLAPI
				int post = FuncTxt.find("]>", n);
				if (post == -1) {
					return false;
				}
				SpecialTxt = FuncTxt.substr(n + 2, post - n - 2);   //�õ��ı��е�IAT����
				p = AddSpecialNode(p, TYPE_CALLAPI, SpecialTxt);
				n = post + 1;
				continue;
			}
			else {
				int post = FuncTxt.find('>', n);
				if (post == -1) {
					return false;
				}
				SpecialTxt = FuncTxt.substr(n + 1, post - n - 1);
				p = AddSpecialNode(p, TYPE_CALL, SpecialTxt);
				n = post;
				continue;
			}
		case '[':
			if (FuncTxt[n + 1] == ']' && FuncTxt[n + 2] == '>') {
				//To Do
			}
			else
			{
				int post = FuncTxt.find(']', n);
				if (post == -1) {
					return false;
				}
				SpecialTxt = FuncTxt.substr(n, post - n + 1);
				p = AddSpecialNode(p, TYPE_JMPAPI, SpecialTxt);
				n = post;
				continue;
			}
		case '?':
			if (FuncTxt[n + 1] == '?') {
				p = AddSpecialNode(p, TYPE_ALLPASS, FuncTxt.substr(n, 2));	//ȫͨ���
				n = n + 1;
				continue;
			}
			else
			{
				p = AddSpecialNode(p, TYPE_LEFTPASS, FuncTxt.substr(n, 2));	//��ͨ���
				n = n + 1;
				continue;
			}
		case '!':	//��֤1��
		{
			int post = FuncTxt.find('!', n + 1);	//��!����һ���ַ���ʼѰ��!
			if (post == -1) {
				return false;
			}
			SpecialTxt = FuncTxt.substr(n + 1, post - n - 1);
			p = AddSpecialNode(p, TYPE_CONSTANT, SpecialTxt);
			n = post;	//����ǰָ��ָ���ұߵ�!��
			continue;
		}
		default:
			if (FuncTxt[n + 1] == '?') {
				p = AddSpecialNode(p, TYPE_RIGHTPASS, FuncTxt.substr(n, 2));	//��ͨ���
				n = n + 1;
				continue;
			}
			else {
				BasicTxt = FuncTxt.substr(n, 2);
				p = AddNode(p, BasicTxt);
				n = n + 1;
				continue;
			}
		}
	}

	if (p->FuncName) {		//ȷ����������Ψһ�ԣ�����
		MessageBoxA(NULL, p->FuncName, "������ͬ����", 0);
		return false;
	}
	
	p->FuncName = new char[FuncName.length() + 1];strcpy_s(p->FuncName, FuncName.length() + 1, FuncName.c_str());
	return true;
}

TrieTreeNode* TrieTree::AddNode(TrieTreeNode* p, string& Txt) {
	UCHAR index = 0;
	HexToBin(Txt, &index);
	if (p->ChildNodes[index]) {
		return p->ChildNodes[index];
	}

	TrieTreeNode* NewNode = new TrieTreeNode(); //������еĽڵ��ж�û��,�򴴽�һ���½ڵ�
	p->ChildNodes[index] = NewNode;      //��ǰ�ڵ�������ӽڵ�

	NewNode->EsigText = new char[Txt.length() + 1];strcpy_s(NewNode->EsigText, Txt.length() + 1, Txt.c_str());//��ֵEsigTxt
	return NewNode;
}

TrieTreeNode* TrieTree::AddSpecialNode(TrieTreeNode* p, UINT type, string Txt) {		//����ڵ�����
	for (int i = 0;i < p->SpecialNodes.size();i++) {		//������ǰ�ӽڵ�
		if (p->SpecialNodes[i]->SpecialType == type && Txt.compare(p->SpecialNodes[i]->EsigText) == 0) {
			return p->SpecialNodes[i];
		}
	}

	TrieTreeNode* NewNode = new TrieTreeNode(); //������еĽڵ��ж�û��,�򴴽�һ���½ڵ�
	p->SpecialNodes.push_back(NewNode);      //��ǰ�ڵ�������ӽڵ�
	NewNode->EsigText = new char[Txt.length() + 1];strcpy_s(NewNode->EsigText, Txt.length() + 1, Txt.c_str());//��ֵEsigTxt
	NewNode->SpecialType = type;
	return NewNode;
}

BOOL TrieTree::CmpCode(UCHAR* FuncSrc, string& FuncTxt) {
	USES_CONVERSION;
	UCHAR* pSrc = FuncSrc;  //��ʼ����������ָ��

	for (UINT n = 0;n < FuncTxt.length();n++) {
		switch (FuncTxt[n])
		{
		case '-':
			if (FuncTxt[n + 1] == '-' && FuncTxt[n + 2] == '>') {		//����ת
				if (*pSrc != 0xE9) {
					return false;
				}
				DWORD offset = *(DWORD*)(pSrc + 1);
				pSrc = pSrc + offset + 5;
				if (pEAnalysisEngine->FindVirutalSection((ULONG)pSrc) == -1) {	//��ת����������
					return false;	//��ʱ����ʧ��
				}
				n = n + 2;
				continue;
			}
			break;
		case '<':
			if (FuncTxt[n + 1] == '[') {						//CALLAPI
				if (*pSrc != 0xFF || *(pSrc + 1) != 0x15) {
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
				//FindIndex..Todo
				if (Findname(addr, NM_IMPORT, buffer) != 0 && IATCom.CompareNoCase(A2W(buffer)) == 0) {  //����IATƥ��
					pSrc = pSrc + 6;
					n = post + 1;
					continue;
				}
				if (Findname(*(ULONG*)pEAnalysisEngine->O2V(addr, 0), NM_EXPORT, buffer) != 0 && EATCom.CompareNoCase(A2W(buffer)) == 0) {      //EATƥ��
					pSrc = pSrc + 6;
					n = post + 1;
					continue;
				}
				return false;
			}
			else					  //NORMALCALL
			{
				if (*pSrc != 0xE8) {
					return false;
				}
				int post = FuncTxt.find('>', n);
				if (post == -1) {
					return false;
				}
				string SubFunc = FuncTxt.substr(n + 1, post - n - 1);   //�Ӻ�������
				DWORD addr = (DWORD)pSrc + *(DWORD*)(pSrc + 1) + 5;
				if (m_RFunc[addr] == SubFunc) {       //Ϊʲô���ȼ�麯����ַ�Ϸ���?��Ϊ���󲿷ֺ������ǺϷ���
					pSrc = pSrc + 5;
					n = post;
					continue;
				}
				if (pEAnalysisEngine->FindVirutalSection(addr) == -1) {		//���ַ���������һ���Ĳ��ȶ���
					return false;	//��ʱ��֧��CALL�������δ���
				}
				if (CmpCode((UCHAR*)addr, m_subFunc[SubFunc])) {
					pSrc = pSrc + 5;
					n = post;
					m_RFunc[addr] = SubFunc;
					continue;
				}
				return false;
			}
			break;
		case '[':
			if (*pSrc != 0xFF || *(pSrc + 1) != 0x25) {
				return false;
			}
			if (FuncTxt[n + 1] == ']' && FuncTxt[n + 2] == '>') {		//FF25��ת
																		//To Do
			}
			else {								//FF25 IATEAT
				int post = FuncTxt.find(']', n);
				if (post == -1) {
					return false;
				}
				string IATEAT = FuncTxt.substr(n + 1, post - n - 1);
				if (pEAnalysisEngine->FindVirutalSection((ULONG)pSrc + 2) == -1) {
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
				if (Findname(*(ULONG*)pEAnalysisEngine->O2V(addr, 0), NM_EXPORT, buffer) != 0 && EATCom.CompareNoCase(A2W(buffer)) == 0) {      //EATƥ��
					pSrc = pSrc + 6;
					n = post;
					continue;
				}
				return false;
			}
			break;
		case '?':
			if (FuncTxt[n + 1] == '?') {	//ȫͨ���
				n++;
				pSrc++;
				continue;
			}
			else {							//��ͨ���
											//To Do
			}
		default:
			if (FuncTxt[n + 1] == '?') {	//��ͨ���
											//To Do
			}
			else {							//��ͨ�Ĵ���
				UCHAR ByteCode = 0;
				HexToBin(FuncTxt.substr(n, 2), &ByteCode);
				if (*pSrc == ByteCode) {
					n++;
					pSrc++;
					continue;
				}
				return false;
			}
		}
	}
	return true;
}

char* TrieTree::MatchSpecial(const TrieTreeNode* p, UCHAR* FuncSrc) {
	
	for (UINT i = 0;i < p->SpecialNodes.size();i++) {
		switch (p->SpecialNodes[i]->SpecialType)
		{
		case TYPE_LONGJMP:	//Check 1��
		{
			if (*FuncSrc != 0xE9) {
				continue;
			}
			ULONG JmpSrc = (ULONG)FuncSrc + *(DWORD*)(FuncSrc + 1) + 5;	//��ȡ��jmp�������ַ

			INT BaseIndex = pEAnalysisEngine->FindVirutalSection((ULONG)FuncSrc);	//�Ե�ǰ����Ϊ�ο�ϵ

			ULONG oaddr = pEAnalysisEngine->V2O(JmpSrc, BaseIndex);	//ת��Ϊʵ�ʴ����еĵ�ַ

			INT index = pEAnalysisEngine->FindOriginSection(oaddr);	//�ж���ת�Ƿ����
			if (index == -1) {
				index = pEAnalysisEngine->AddSection(oaddr);
				if (index == -1) {
					pMaindlg->outputInfo("Jmp��ַ���󣡣���");
					continue;
				}
			}
			return Match(p->SpecialNodes[i], (UCHAR*)pEAnalysisEngine->O2V(oaddr, index));
		}
		case TYPE_CALL:		//Check 2��
		{
			if (*FuncSrc != 0xE8) {
				continue;
			}
			ULONG CallSrc = (ULONG)FuncSrc + *(DWORD*)(FuncSrc + 1) + 5;	//�õ������ַ

			INT BaseIndex = pEAnalysisEngine->FindVirutalSection((ULONG)FuncSrc);	//�Ե�ǰ����Ϊ�ο�ϵ

			ULONG oaddr = pEAnalysisEngine->V2O(CallSrc, BaseIndex);	//ת��Ϊʵ�ʴ����еĵ�ַ

			if (m_RFunc[oaddr] == p->SpecialNodes[i]->EsigText) {		//�˺����Ѿ�ƥ���һ��
				return Match(p->SpecialNodes[i], FuncSrc + 5);
			}

			INT index = pEAnalysisEngine->FindOriginSection(oaddr);
			if (index == -1) {		//CALL������������...
				index = pEAnalysisEngine->AddSection(oaddr);
				if (index == -1) {
					//pMaindlg->outputInfo("CALL��ַ���󣡣���");
					continue;
				}
			}
			if (CmpCode((UCHAR*)pEAnalysisEngine->O2V(oaddr, index), m_subFunc[p->SpecialNodes[i]->EsigText])) {
				return Match(p->SpecialNodes[i], FuncSrc + 5);
			}
			continue;
		}
		case TYPE_JMPAPI:	//Check 1��
		{
			if (*FuncSrc != 0xFF || *(FuncSrc + 1) != 0x25) {
				continue;
			}

			string IATEAT = p->SpecialNodes[i]->EsigText;
			string IATCom;
			string EATCom;
			char buffer[256] = { 0 };

			int EATpos = IATEAT.find("||");
			if (EATpos != -1) {            //�����Զ���EAT
				IATCom = IATEAT.substr(0, EATpos);
				EATCom = IATEAT.substr(EATpos + 2);
			}
			else
			{
				IATCom = IATEAT;
				EATCom = IATEAT.substr(IATEAT.find('.') + 1);
			}
			DWORD addr = *(FuncSrc + 2);
			if (Findname(addr, NM_IMPORT, buffer) != 0 && stricmp(IATCom.c_str(), buffer) == 0) {
				return Match(p->SpecialNodes[i], FuncSrc + 6);
			}
			int index = pEAnalysisEngine->FindOriginSection(addr);
			if (index == -1) {
				index = pEAnalysisEngine->AddSection(addr);
				if (index == -1) {
					continue;
				}
			}
			if (Findname(*(ULONG*)pEAnalysisEngine->O2V(addr, index), NM_EXPORT, buffer) != 0 && stricmp(EATCom.c_str(), buffer) == 0) {      //EATƥ��
				return Match(p->SpecialNodes[i], FuncSrc + 6);
			}
			continue;
		}
		case TYPE_CALLAPI:		//Check 1��
		{
			if (*FuncSrc != 0xFF || *(FuncSrc + 1) != 0x15) {
				continue;
			}
			string IATEAT = p->SpecialNodes[i]->EsigText;

			string IATCom;
			string EATCom;
			char buffer[256] = { 0 };

			int EATpos = IATEAT.find("||");
			if (EATpos != -1) {					//�����Զ���EAT
				IATCom = IATEAT.substr(0, EATpos);
				EATCom = IATEAT.substr(EATpos + 2);
			}
			else
			{
				IATCom = IATEAT;
				EATCom = IATEAT.substr(IATEAT.find('.') + 1);
			}

			ULONG oaddr = *(ULONG*)(FuncSrc + 2);			//�õ�IAT��������ʵ��ַ
			if (Findname(oaddr, NM_IMPORT, buffer) != 0 && stricmp(IATCom.c_str(), buffer) == 0) {		//����IATƥ��
				return Match(p->SpecialNodes[i], FuncSrc + 6);
			}

			INT index = pEAnalysisEngine->FindOriginSection(oaddr);	//Ѱ�ҵ�ַ����index
			if (index == -1) {
				index = pEAnalysisEngine->AddSection(oaddr);
				if (index == -1) {
					continue;
				}
			}

			if (Findname(*(ULONG*)pEAnalysisEngine->O2V(oaddr, index), NM_EXPORT, buffer) != 0 && stricmp(EATCom.c_str(), buffer) == 0) {      //EATƥ��
				return Match(p->SpecialNodes[i], FuncSrc + 6);
			}
			continue;
		}
		case TYPE_CONSTANT:		//Check 2��
		{
			ULONG ConSrc = *(ULONG*)FuncSrc;	//ȡ��ʵ�ʵ�ַ

			INT ConIndex = pEAnalysisEngine->FindOriginSection(ConSrc);	//Ѱ�ҵ�ַ����index
			if (ConIndex == -1) {		//�����������������
				ConIndex = pEAnalysisEngine->AddSection(ConSrc);
				if (ConIndex == -1) {
					pMaindlg->outputInfo("Type_Constant�����ڴ�ʧ��");
					continue;
				}
			}
			if (m_RFunc[ConSrc] == p->SpecialNodes[i]->EsigText || CmpCode((UCHAR*)pEAnalysisEngine->O2V(ConSrc, ConIndex), m_subFunc[p->SpecialNodes[i]->EsigText])) {
				return Match(p->SpecialNodes[i], FuncSrc + 4);
			}
			continue;
		}
		case TYPE_ALLPASS:
		{
			return Match(p->SpecialNodes[i], FuncSrc + 1);
		}
		}
	}

	//if ((p->SpecialNodes[i]->EsigText[0] == '?' || HByteToBin(p->SpecialNodes[i]->EsigText[0])==*(FuncSrc)>>4) &&(p->SpecialNodes[i]->EsigText[1] == '?'|| HByteToBin(p->SpecialNodes[i]->EsigText[1]) == *(FuncSrc)& 0x0F)) {		//��һ����ͨ���
	//	return Match(p->SpecialNodes[i], FuncSrc + 1);		//����ƥ����һ��
	//}
	return NULL;
}

char* TrieTree::Match(const TrieTreeNode* p, UCHAR* FuncSrc) {
	if (p->FuncName) {		//����ɹ�Ѱ�ҵ�����,��δ��ƥ����򷵻ؽ��   --������ͬ��ַ�Ĳ�ͬ����,�⽫���µ�һ������ƥ��ɹ�,�ڶ�������ƥ��ʧ��
		return p->FuncName;
	}

	if (p->ChildNodes[*FuncSrc]) {
		return Match(p->ChildNodes[*FuncSrc], FuncSrc + 1);
	}
	else {
		return MatchSpecial(p, FuncSrc);
	}
	return NULL;
}

void TrieTree::Destroy(TrieTreeNode* p) {
	if (!p) {
		return;
	}

	for (int i = 0;i < p->SpecialNodes.size();i++) {
		Destroy(p->SpecialNodes[i]);
	}

	for (int i = 0;i < 256;i++) {
		Destroy(p->ChildNodes[i]);
	}

	if (p->EsigText) {
		delete[] p->EsigText;
		p->EsigText = NULL;
	}

	if (p->FuncName) {
		delete[] p->FuncName;
		p->FuncName = NULL;
	}

	delete p;
	p = NULL;
}

TrieTree::TrieTree()
{
	root = new TrieTreeNode();
}