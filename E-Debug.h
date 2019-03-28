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

HANDLE GethProcess();	 //���������Խ��̵ľ��

UINT ByteToDec(char* HexCode);	//ʮ����ʮ,���ٰ棬ֻ֧��һ���ֽ�

void HexToBin(string& HexCode, UCHAR* BinCode);

char* DecToByte(UINT DecCode);

UCHAR HByteToBin(char HByte);	//����ֽڵ�HEXת��Ϊ����

INT findstr(const char* Func, const char* find);

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