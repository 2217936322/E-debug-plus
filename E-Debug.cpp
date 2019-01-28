/*��������������������������������������������������������
E-debug   DLL��OD�Ľ����ӿ�
��������������������������������������������������������*/

#include "stdafx.h"
#include "E-Debug.h"
#include "MainWindow.h"

HINSTANCE g_hInstace;		//һ��E-debug��Ӧһ��hInstace
vector<EsigInfo> EsigList;  //һ��E-debug��Ӧһ��EsigList


BOOL LoadSig(char* lpMapPath,char* Category) {		//��ȡ��Ϣ��EsigList
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
	string Name = GetMidString(Sig, "Name=", "\r\n", 0);
	string Description = GetMidString(Sig, "Description=", "\r\n", 0);

	if (!Name.size()) {
		return false;
	}
	
	EsigInfo saveinfo;
	saveinfo.Category = Category;
	saveinfo.Name = Name;
	saveinfo.Description = Description;
	saveinfo.Path = lpMapPath;
	
	EsigList.push_back(saveinfo);
	
	if (pMap) {
		free(pMap);
	}

	CloseHandle(hFile);
	return TRUE;
}

BOOL EnumEsig() {
	EsigList.clear();
	/*if (EsigList.size()) {		//ֻ��ȡһ�ξ͹���
		return true;
	}*/

	char path[256] = {};
	strcpy_s(path, DIRECTORY);strcat_s(path, "\\plugin\\esig\\*.*");
	WIN32_FIND_DATAA FindFileData;
	HANDLE hFind = FindFirstFileA(path, &FindFileData);
	if (hFind == INVALID_HANDLE_VALUE) {
		MessageBoxA(NULL, "ö��esig�ļ�ʧ��", "����", MB_ICONINFORMATION);
		return false;
	}

	do {
		if (FindFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
			string strname = FindFileData.cFileName;
			if (strname != "." && strname != "..") {
				char path2[256] = {};
				strcpy_s(path2, DIRECTORY);strcat_s(path2, "\\plugin\\esig\\");strcat_s(path2, FindFileData.cFileName);strcat_s(path2, "\\*.esig");
				WIN32_FIND_DATAA FindFileData2;
				HANDLE hFind2 = FindFirstFileA(path2, &FindFileData2);
				if (hFind2) {
					do {
						char lpMapPath[256] = {};
						strcpy_s(lpMapPath, DIRECTORY);strcat_s(lpMapPath, "\\plugin\\esig\\");strcat_s(lpMapPath, FindFileData.cFileName);strcat_s(lpMapPath, "\\");
						strcat_s(lpMapPath,FindFileData2.cFileName);
						LoadSig(lpMapPath,FindFileData.cFileName);	
					} while (FindNextFileA(hFind2, &FindFileData2));
					CloseHandle(hFind2);
				}
			}
		}
	} while (FindNextFileA(hFind, &FindFileData));
	CloseHandle(hFind);
	
	return true;
}


extc int _export cdecl ODBG_Plugindata(char shortname[32])
{
	strcpy_s(shortname, 13, "E-Debug Plus");
	return PLUGIN_VERSION;
}

extc int _export cdecl ODBG_Plugininit(int ollydbgversion, HWND hw, DWORD *features)
{
	char tmpbuffer[MAX_PATH] = {};
	GetCurrentDirectoryA(MAX_PATH, tmpbuffer);
	StrCpyA((char*)DIRECTORY, tmpbuffer);
	
	if (ollydbgversion < PLUGIN_VERSION)
	{
		Addtolist(0, 0, "��ʾ: ����汾��OD��ƥ��!");
		return -1;
	}
	Addtolist(0, 0, "%s", "E-Debug Plus 1.2");
	Addtolist(0, -1, "%s","  by:fjqisba");

	//�������޸�һ��OD ����ѡ��BUG
	int* Fix = (int*)34152;
	if (*(DWORD*)4647851 == 99688) {
		WriteProcessMemory(GetCurrentProcess(), (LPVOID)4647851, &Fix, 4, NULL);
	}
	return 0;
}

extc int _export cdecl ODBG_Pluginmenu(int origin, CHAR data[4096], VOID *item)
{
	if (origin == PM_MAIN)
	{
		strcpy(data, "0&��ʼ����|1&�������");
		return 1;
	}
	if (origin == PM_DISASM || origin == PM_CPUDUMP) {
		strcpy(data, "0 ������{0 ȡʮ������|1 ���ֽڼ�}");
		return 1;
	}
	return 1;
}

extc void _export cdecl ODBG_Pluginaction(int origin, int action, VOID *item)
{
	if (origin == PM_MAIN) {
		if (action == 1) {
			CString szInfo;
			szInfo += "Plugin:E-Debug Plus\r\n";
			szInfo += "Verion:1.2\r\n";
			szInfo += "Author:fjqisba\r\n";
			szInfo += " Thanks to Xjun";
			MessageBox(NULL, szInfo, L"About", MB_ICONINFORMATION);
			return;
		}

		if (Getcputhreadid() == 0) {
			AfxMessageBox(L"δ�������������������ת�������������!");
			Flash("δ�������!");
			return;
		}

		EnumEsig();
		
		CMainWindow* pWindow = new CMainWindow;	//������ʱ�Ĵ���ָ��
		pWindow->Create(IDD_MainWindow, NULL);
		pWindow->ShowWindow(SW_SHOW);
		return;
	}

	//����������������
	HGLOBAL hClip;
	t_dump* DumpData;
	if (origin == PM_DISASM) {
		DumpData = (t_dump*)Plugingetvalue(VAL_CPUDASM);
	}
	else if(origin==PM_CPUDUMP)
	{
		DumpData = (t_dump*)Plugingetvalue(VAL_CPUDDUMP);
	}

	int len = DumpData->sel1 - DumpData->sel0;
	byte *buffer = new byte[len];
	memset(buffer, 0, len);
	Readmemory(buffer, DumpData->sel0, len, MM_RESILENT);
	if (!OpenClipboard(NULL)) {
		return;
	}

	EmptyClipboard();

	if (action == 0) {    //ȡʮ������
		hClip = GlobalAlloc(GMEM_MOVEABLE, len * 2 + 1);  //��������ı��ռ�
		char *buff;
		buff = (char*)GlobalLock(hClip);
		for (int n = 0;n < len;n++) {					//ʮ������ת��
			sprintf(buff + n * 2, "%02X", buffer[n]);
		}
		GlobalUnlock(hClip);
		SetClipboardData(CF_TEXT, hClip);
		CloseClipboard();
		delete[] buffer;
		return;
	}
	if (action == 1) {	  //���ֽڼ�
		hClip = GlobalAlloc(GMEM_MOVEABLE, len * 4 + 1);  //��������ı��ռ�
		char *buff = (char*)GlobalLock(hClip);
		memset(buff, 0, len * 4 + 1);
		string byteset;//�ֽڼ�
		byteset.append("{");
		for (int n = 0;n < len;n++) {					//ʮ������ת��
			sprintf(buff, "%d,", buffer[n]);
			byteset.append(buff);
		}
		byteset[byteset.length() - 1] = '}';
		strcpy(buff,byteset.c_str());
		GlobalUnlock(hClip);
		SetClipboardData(CF_TEXT, hClip);
		CloseClipboard();   //����Ҫʹ��GlobalFree,��Ϊ�ڴ��Ѿ��ɼ��а����й�
		delete[] buffer;
		return;
	}
}


