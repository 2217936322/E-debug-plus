#pragma once
#include "E-Debug.h"
#include "Page0.h"
#include "Page1.h"
#include "EAnalyEngine.h"

#define TYPE_NORMAL 0	
#define TYPE_LONGJMP 1	       //      -->
#define TYPE_CALL   2	       //      <>
#define TYPE_JMPAPI 3	       //      []
#define TYPE_CALLAPI 4	      //      <[]>

#define TYPE_PUSH   6	      //      !!
#define TYPE_LEFTPASS 11      //      ?
#define TYPE_RIGHTPASS 12     //       ?
#define TYPE_ALLPASS   13     //      ??


extern  EAnalysis	*pEAnalysisEngine;

typedef struct FuncMap
{
	vector<string>  Command_name;	//��������
	vector<DWORD>   Command_addr;	//������ַ
}*pFuncMap;




class TrieTreeNode {
public:
	TrieTreeNode();

	vector<TrieTreeNode*> SpecialNodes;
	TrieTreeNode **ChildNodes;

	UINT SpecialType;	//һ�����ִ�������

	char* EsigText;		//һ�����ִ�������
	char* FuncName;		//��������

	BOOL IsMatched;     //�Ƿ��Ѿ�ƥ���
};

//������������������������������������������������//

class TrieTree
{
public:
	TrieTree();
	~TrieTree() { Destroy(root); };
	BOOL LoadSig(const char* lpMapPath);
	
	void  ScanSig(UCHAR* CodeSrc, ULONG SrcLen,string& LibName);    //����һ�Ǵ�����ʼ�ڵ�,������Ϊ������С,������Ϊ��Ӧɨ�����
	char* MatchSig(UCHAR* CodeSrc);					//����ƥ��
	
	
	

protected:
	BOOL Insert(string& FuncTxt, const string& FuncName);		//���뺯��,�ڵ�ĺ���������Ψһ��
	BOOL CmpCall(UCHAR* FuncSrc, string& FuncTxt);				//�����ƥ�亯����,����ƥ���Ӻ���

private:
	TrieTreeNode* root;
	TrieTreeNode* AddNode(TrieTreeNode* p, string& Txt);		//������ͨ�ڵ�
	TrieTreeNode* AddSpecialNode(TrieTreeNode*p, UINT type, string Txt);	//��������ڵ�

	

	void Destroy(TrieTreeNode* p);
	char* Match(TrieTreeNode*p, UCHAR* FuncSrc);		//����һΪƥ��ڵ�,������Ϊƥ���ַ,����ƥ��ɹ��ĺ����ı�

	BOOL IsAligned=false;
	
	
	map<string, string> m_subFunc;	//�Ӻ���,�������ƺͺ����ı�һһӳ��
	map<ULONG,string> m_RFunc;  //R����Runtime,����ʱ��¼����,��Զ��ס ��ַ�ͺ���������һһӳ��ģ���Ҫ��ͼһ����ַ�����������
};









