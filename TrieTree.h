#pragma once
#include "E-Debug.h"
#include "Page1.h"
#include "EAnalyEngine.h"

extern CMainWindow *pMaindlg;
extern  EAnalysis	*pEAnalysisEngine;

class TrieTreeNode {
public:
	TrieTreeNode() {
		ChildNodes = new TrieTreeNode*[256];
		for (int i = 0;i < 256;i++) {
			ChildNodes[i] = NULL;
		}
		EsigText = NULL;
		FuncName = NULL;
		IsMatched = false;
	}
public:
	vector<TrieTreeNode*> SpecialNodes;
	TrieTreeNode **ChildNodes;

	BOOL IsMatched;     //�Ƿ��Ѿ�ƥ���
	char* FuncName;		//��������
	char* EsigText;		//�ڵ���������Esig����
};

//������������������������������������������������//

class TrieTree
{
public:
	TrieTree();
	~TrieTree() { Destroy(root); };
	BOOL Insert(string& FuncTxt,const string& FuncName);	//����ڵ�
	void MatchSig(UCHAR* CodeSrc, ULONG SrcLen);    //����һ�Ǵ�����ʼ�ڵ�,������Ϊ������С,ɨ��汾
	char* MatchSig(UCHAR* CodeSrc);					//����ƥ��
private:
	TrieTreeNode* root;
	TrieTreeNode* AddNode(TrieTreeNode* p, string& Txt);		//������ͨ�ڵ�
	TrieTreeNode* AddSpecialNode(TrieTreeNode*p, string& Txt);	//��������ڵ�
	void Destroy(TrieTreeNode* p);
	char* Match(TrieTreeNode*p, UCHAR* FuncSrc);		//����һΪƥ��ڵ�,������Ϊƥ�������,����ƥ��ɹ��ĺ����ı�
};

TrieTree::TrieTree()
{
	root = new TrieTreeNode();
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

char* TrieTree::Match(TrieTreeNode* p, UCHAR* FuncSrc) {
	if (p->FuncName && !p->IsMatched) {		//���ɾ��IsMatched�ж�,��Esig��ƥ�����޴�
		p->IsMatched = true;
		return p->FuncName;
	}
	for (UINT i = 0;i < p->SpecialNodes.size();i++) {		//�ȴ�������ڵ�
		/*if (p->SpecialNodes[i]->EsigText[0] == '-' && p->SpecialNodes[i]->EsigText[1] == '-' && p->SpecialNodes[i]->EsigText[2] == '>') {

		}
		else if (p->SpecialNodes[i]->EsigText[0]=='<' && p->SpecialNodes[i]->EsigText[1]=='[') {	//FF15 CALL���ж�
			if (*FuncSrc != 0xFF || *(FuncSrc + 1) != 0x15 || IsBadReadPtr((ULONG*)(FuncSrc + 2), 4) != 0) {      //����VMP��SE����
				return NULL;
			}
			

		}
		else if (p->SpecialNodes[i]->EsigText[0] == '<' && *FuncSrc == 0xE8) {	//����һ��CALL�Ļ����ж�
			
		}
		else {*/
			if ((p->SpecialNodes[i]->EsigText[0] == '?' || HByteToBin(p->SpecialNodes[i]->EsigText[0])==*(FuncSrc)>>4) &&(p->SpecialNodes[i]->EsigText[1] == '?'|| HByteToBin(p->SpecialNodes[i]->EsigText[1]) == *(FuncSrc)& 0x0F)) {		//��һ����ͨ���
				return Match(p->SpecialNodes[i], FuncSrc + 1);		//����ƥ����һ��
			}
		//}
	}

	if (!p->ChildNodes[*FuncSrc]) {
		return NULL;
	}
	return Match(p->ChildNodes[*FuncSrc], FuncSrc + 1);
}

char* TrieTree::MatchSig(UCHAR* CodeSrc) {
	TrieTreeNode* p = root;		//��ǰָ��ָ��root

	return Match(p, (UCHAR*)CodeSrc);
}

void TrieTree::MatchSig(UCHAR* CodeSrc, ULONG SrcLen) {
	
	TrieTreeNode* p = root;		//��ǰָ��ָ��root

	for (ULONG offset = 0;offset < SrcLen;offset++) {
		char* FuncName = Match(p, (UCHAR*)CodeSrc + offset);
		if (FuncName) {
			DWORD dwAddr = pEAnalysisEngine->V2O((DWORD)CodeSrc + offset, 0);
			//pMaindlg->m_page1.m_map[2].Command_addr.push_back(dwAddr);
			//pMaindlg->m_page1.m_map[2].Command_name.push_back(FuncName);
			Insertname(dwAddr, NM_LABEL, FuncName);
		}
	}
	return ;
}

BOOL TrieTree::Insert(string& FuncTxt,const string& FuncName) {
	TrieTreeNode *p = root;//��ǰ�ڵ�ָ��ָ��ROOT�ڵ�

	string BasicTxt;
	string SpecialTxt;

	for (UINT n = 0;n < FuncTxt.length();n++) {
		if (FuncTxt[n] == '-' && FuncTxt[n + 1] == '-' && FuncTxt[n + 2] == '>') {		//-->����ת
			SpecialTxt = "-->";
			p = AddSpecialNode(p, SpecialTxt);
			n = n + 2;
			continue;
		}
		else if (FuncTxt[n] == '<' && FuncTxt[n + 1] == '[')	//FF15 CALL
		{
			int post = FuncTxt.find("]>", n);
			if (post == -1) {
				return false;
			}

			SpecialTxt = FuncTxt.substr(n, post - n + 2);   //�õ��ı��е�IAT����
			p = AddSpecialNode(p, SpecialTxt);
			n = post + 1;
			continue;
		}
		else if (FuncTxt[n] == '<')	//����CALL
		{
			int post = FuncTxt.find('>', n);
			if (post == -1) {
				return false;
			}
			SpecialTxt = FuncTxt.substr(n, post - n + 1);
			p = AddSpecialNode(p, SpecialTxt);
			n = post;
			continue;
		}
		else if (FuncTxt[n] == '[' && FuncTxt[n + 1] == ']' && FuncTxt[n + 2] == '>'){	//FF25��ת
			continue;
		}
		else if (FuncTxt[n]=='['){
			int post = FuncTxt.find(']', n);
			if (post == -1) {
				return false;
			}
			SpecialTxt = FuncTxt.substr(n, post - n + 1);
			n = post;
			continue;
		}
		else if(FuncTxt[n]=='?' || FuncTxt[n+1]=='?')	//����ͨ���
		{
			SpecialTxt= FuncTxt.substr(n, 2);
			p = AddSpecialNode(p, SpecialTxt);
			n = n + 1;
			continue;
		}
		else {
			BasicTxt = FuncTxt.substr(n, 2);
			p = AddNode(p, BasicTxt);
			n = n + 1;
		}
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

TrieTreeNode* TrieTree::AddSpecialNode(TrieTreeNode* p, string& Txt) {		//����ڵ�����
	for (int i = 0;i < p->SpecialNodes.size();i++) {		//������ǰ�ӽڵ�
		if (Txt.compare(p->SpecialNodes[i]->EsigText) == 0) {
			return p->SpecialNodes[i];
		}
	}

	TrieTreeNode* NewNode = new TrieTreeNode(); //������еĽڵ��ж�û��,�򴴽�һ���½ڵ�
	p->SpecialNodes.push_back(NewNode);      //��ǰ�ڵ�������ӽڵ�
	NewNode->EsigText = new char[Txt.length() + 1];strcpy_s(NewNode->EsigText, Txt.length() + 1, Txt.c_str());//��ֵEsigTxt
	return NewNode;
}