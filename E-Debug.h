#pragma once
#include "stdafx.h"
#include "resource.h"		
#include "Plugin.h"

#pragma comment(lib,"Ollydbg.lib")

#define StaticMode 0  //��̬����ģʽ
#define NormalMode 1  //������������ģʽ
#define CMode      2  //���±��룬����C����ģʽ

/*һ��E-debugһ��EsigList*/
struct EsigInfo
{
	string Category;		//ֻ��һ��С���
	string Name;			//Esig������
	string Description;		//Esig������
	string Path;			//·��
};

/*��������������������������������������������������������
����ȫ�ֱ���
��������������������������������������������������������*/
extern char DIRECTORY[MAX_PATH];  //���Ŀ¼
extern vector<EsigInfo> EsigList;
extern HINSTANCE g_hInstace;

/*��������������������������������������������������������
������һЩ��������,�ܷ�������ĺ���������������
��������������������������������������������������������*/


static HANDLE GethProcess() {           //���������Խ��̵ľ��
	return (HANDLE)*(DWORD*)0x4D5A68;
}


static void HexToBin(string& HexCode, UCHAR* BinCode) {		//ʮ����ʮ
	static UCHAR BinMap[256] = {
		0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
		0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
		0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
		0,1,2,3,4,5,6,7,8,9,0,0,0,0,0,0,		//123456789
		0,10,11,12,13,14,15,0,0,0,0,0,0,0,0,0,	//ABCDEF
		0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
		0,10,11,12,13,14,15,0,0,0,0,0,0,0,0,0,	//abcdef
		0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
		0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
		0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
		0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
		0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
		0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
		0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
		0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
		0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 };
	for (UINT n = 0;n < HexCode.length() / 2;n++) {
		BinCode[n] = BinMap[HexCode[2 * n]] * 16 + BinMap[HexCode[2 * n + 1]];
	}
}

static UCHAR HByteToBin(char HByte) {		//����ֽڵ�HEXת��Ϊ����
	static UCHAR BinMap[256] = {
		0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
		0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
		0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
		0,1,2,3,4,5,6,7,8,9,0,0,0,0,0,0,		//123456789
		0,10,11,12,13,14,15,0,0,0,0,0,0,0,0,0,	//ABCDEF
		0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
		0,10,11,12,13,14,15,0,0,0,0,0,0,0,0,0,	//abcdef
		0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
		0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
		0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
		0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
		0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
		0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
		0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
		0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
		0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 };
	return BinMap[HByte];
}

static DWORD Search_Bin(byte *pSrc, byte *pTrait, int nSrcLen, int nTraitLen) //��������������,��0x90������ģ������,����ƫ�ƴ�С
{
	if (IsBadReadPtr(pSrc, 4) == TRUE)
	{
		return 0;
	}
	int i, j, k;
	for (i = 0; i <= (nSrcLen - nTraitLen); i++)
	{
		if (pSrc[i] == pTrait[0])
		{
			k = i;
			j = 0;
			while (j < nTraitLen)
			{
				k++; j++;
				if (pTrait[j] == 0x90)
				{
					continue;
				}
				if (pSrc[k] != pTrait[j])
				{
					break;
				}
			}

			if (j == nTraitLen)
			{
				return i;
			}

		}

	}
	return 0;
}

static string GetMidString(string& src,const char* left, const char* right, int offset) {		//����һΪԭ�ı�,������Ϊ����ı�,������Ϊ�ұ��ı�,������Ϊ��ʼƫ��
	int start = src.find(left, offset);
	if (start == -1) {
		return "";
	}

	int end = src.find(right, start);
	if (end == -1) {
		return "";
	}

	return src.substr(start + strlen(left), end - start - strlen(left));
}