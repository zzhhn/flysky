// Test.cpp : �������̨Ӧ�ó������ڵ㡣
//

#include "stdafx.h"
#define WIN32_LEAN_AND_MEAN
#define _AFXDLL
#include <stdio.h>
#include <stdlib.h>
#include <locale.h>
#include <string.h>
//#include <afxwin.h>
#include <windows.h>
#include <commctrl.h>
int count;
FILE *fp = _wfopen(_T("D:/1.txt"), _T("w"));
void ShowListView(HWND listview)
{//��ʾSysListView32�������
	setlocale(LC_ALL, "chs");
	int count = (int)SendMessage(listview, LVM_GETITEMCOUNT, 0, 0);//��listview����Ϣ������item�ĸ���
	printf("%d:ListView Item Count=%d\n", listview, count);//���item�ĸ���
	int i;
	LVITEM lvi, *_lvi;
	TCHAR item[512], subitem[512], ssubitem[512];
	TCHAR *_item, *_subitem, *_ssubitem;
	unsigned long pid;
	HANDLE process;
	GetWindowThreadProcessId(listview, &pid);//���listview���߳̾��
	process = OpenProcess(PROCESS_VM_OPERATION | PROCESS_VM_READ |
		PROCESS_VM_WRITE | PROCESS_QUERY_INFORMATION, FALSE, pid);

	//���ٿռ����ڴ洢���ص���Ϣ
	_lvi = (LVITEM*)VirtualAllocEx(process, NULL, sizeof(LVITEM),
		MEM_COMMIT, PAGE_READWRITE);
	_item = (TCHAR*)VirtualAllocEx(process, NULL, 512, MEM_COMMIT,
		PAGE_READWRITE);
	_subitem = (TCHAR*)VirtualAllocEx(process, NULL, 512, MEM_COMMIT,
		PAGE_READWRITE);
	_ssubitem = (TCHAR*)VirtualAllocEx(process, NULL, 512, MEM_COMMIT,
		PAGE_READWRITE);
	lvi.cchTextMax = 512;
	for (i = 0; i < count; i++) {//ö��ÿһ��item
		lvi.iSubItem = 0;
		lvi.pszText = _item;
		WriteProcessMemory(process, _lvi, &lvi, sizeof(LVITEM), NULL);
		SendMessage(listview, LVM_GETITEMTEXT, (WPARAM)i, (LPARAM)_lvi);//����Ϣ���item����1��textֵ
		lvi.iSubItem = 1;
		lvi.pszText = _subitem;
		WriteProcessMemory(process, _lvi, &lvi, sizeof(LVITEM), NULL);
		SendMessage(listview, LVM_GETITEMTEXT, (WPARAM)i, (LPARAM)_lvi);//����Ϣ���item����2��textֵ
		lvi.iSubItem = 2;
		lvi.pszText = _ssubitem;
		WriteProcessMemory(process, _lvi, &lvi, sizeof(LVITEM), NULL);
		SendMessage(listview, LVM_GETITEMTEXT, (WPARAM)i, (LPARAM)_lvi);//����Ϣ���item����3��textֵ
		ReadProcessMemory(process, _item, item, 512, NULL);
		ReadProcessMemory(process, _subitem, subitem, 512, NULL);
		ReadProcessMemory(process, _ssubitem, ssubitem, 512, NULL);
		if (!_wcsicmp(item, L"GOLD"))//���item��text��GOLD�������ǻƽ�۸������
			wprintf(_T("%ws=%ws=%ws\n"), item, subitem, ssubitem);
	}

	//�ͷſռ�
	VirtualFreeEx(process, _lvi, 0, MEM_RELEASE);
	VirtualFreeEx(process, _item, 0, MEM_RELEASE);
	VirtualFreeEx(process, _subitem, 0, MEM_RELEASE);
	VirtualFreeEx(process, _ssubitem, 0, MEM_RELEASE);
}
BOOL CALLBACK EnumChildProc(HWND hwnd, LPARAM lParam)
{//ö���Ӵ��ڵĻص�����
	setlocale(LC_ALL, "chs");
	count++;
	TCHAR lpWinTitle[256], lpClassName[256];
	::GetWindowTextW(hwnd, (LPWSTR)lpWinTitle, 256 - 1); //��ô���caption
	::GetClassNameW(hwnd, (LPWSTR)lpClassName, 256 - 1); //��ô�������
	if (_wcsicmp((LPWSTR)lpWinTitle, L"") != NULL && _wcsicmp((LPWSTR)lpClassName, L"") != NULL) //���˵�û�����Ӻ������Ĵ���
	{
		fwprintf(fp, _T("\n"));
		for (int i = 1; i <= count; i++) fprintf(fp, " ");
		fwprintf(fp, _T("-0xX:'%ws':'%ws'"), hwnd, lpWinTitle, lpClassName);
		if (_wcsicmp(lpClassName, L"SysListView32") == NULL)
			ShowListView(hwnd);

		EnumChildWindows(hwnd, EnumChildProc, 0); //�ݹ�ö���Ӵ���
	}
	count--;
	return TRUE;
}
BOOL CALLBACK EnumWindowsProc(HWND hwnd, LPARAM lparam)
{//ö�ٴ��ڵĻص�����
	setlocale(LC_ALL, "chs");
	count = 1;
	TCHAR lpWinTitle[256], lpClassName[256];
	::GetWindowTextW(hwnd, lpWinTitle, 256 - 1); //��ô���caption
	::GetClassNameW(hwnd, lpClassName, 256 - 1); //��ô�������
	if (_wcsicmp(lpWinTitle, L"") != NULL && _wcsicmp(lpClassName, L"") != NULL) //���˵�û�����Ӻ������Ĵ���
	{
		printf("%d\n", GetLastError());
		fwprintf(fp, _T("\n*-0xX:'%ws':'%ws'"), hwnd, lpWinTitle, lpClassName);

		EnumChildWindows(hwnd, EnumChildProc, 0); //����ö���Ӵ��ڣ����ݸ��ص��������Ӵ��ڵľ��
	}
	return TRUE;
}
int main()
{
	setlocale(LC_ALL, "chs");
	::EnumWindows(EnumWindowsProc, 0); //ö������Ĵ��ڣ�ÿ�ҵ�һ�����ڣ�����һ�λص����������ݸ��ص������ô��ڵľ��
	system("pause");
	return 0;
}