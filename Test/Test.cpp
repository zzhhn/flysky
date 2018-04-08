// Test.cpp : 定义控制台应用程序的入口点。
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
{//显示SysListView32类的内容
	setlocale(LC_ALL, "chs");
	int count = (int)SendMessage(listview, LVM_GETITEMCOUNT, 0, 0);//向listview发消息，返回item的个数
	printf("%d:ListView Item Count=%d\n", listview, count);//输出item的个数
	int i;
	LVITEM lvi, *_lvi;
	TCHAR item[512], subitem[512], ssubitem[512];
	TCHAR *_item, *_subitem, *_ssubitem;
	unsigned long pid;
	HANDLE process;
	GetWindowThreadProcessId(listview, &pid);//获得listview的线程句柄
	process = OpenProcess(PROCESS_VM_OPERATION | PROCESS_VM_READ |
		PROCESS_VM_WRITE | PROCESS_QUERY_INFORMATION, FALSE, pid);

	//开辟空间用于存储返回的信息
	_lvi = (LVITEM*)VirtualAllocEx(process, NULL, sizeof(LVITEM),
		MEM_COMMIT, PAGE_READWRITE);
	_item = (TCHAR*)VirtualAllocEx(process, NULL, 512, MEM_COMMIT,
		PAGE_READWRITE);
	_subitem = (TCHAR*)VirtualAllocEx(process, NULL, 512, MEM_COMMIT,
		PAGE_READWRITE);
	_ssubitem = (TCHAR*)VirtualAllocEx(process, NULL, 512, MEM_COMMIT,
		PAGE_READWRITE);
	lvi.cchTextMax = 512;
	for (i = 0; i < count; i++) {//枚举每一个item
		lvi.iSubItem = 0;
		lvi.pszText = _item;
		WriteProcessMemory(process, _lvi, &lvi, sizeof(LVITEM), NULL);
		SendMessage(listview, LVM_GETITEMTEXT, (WPARAM)i, (LPARAM)_lvi);//发消息获得item分项1的text值
		lvi.iSubItem = 1;
		lvi.pszText = _subitem;
		WriteProcessMemory(process, _lvi, &lvi, sizeof(LVITEM), NULL);
		SendMessage(listview, LVM_GETITEMTEXT, (WPARAM)i, (LPARAM)_lvi);//发消息获得item分项2的text值
		lvi.iSubItem = 2;
		lvi.pszText = _ssubitem;
		WriteProcessMemory(process, _lvi, &lvi, sizeof(LVITEM), NULL);
		SendMessage(listview, LVM_GETITEMTEXT, (WPARAM)i, (LPARAM)_lvi);//发消息获得item分项3的text值
		ReadProcessMemory(process, _item, item, 512, NULL);
		ReadProcessMemory(process, _subitem, subitem, 512, NULL);
		ReadProcessMemory(process, _ssubitem, ssubitem, 512, NULL);
		if (!_wcsicmp(item, L"GOLD"))//如果item的text是GOLD，表明是黄金价格，则输出
			wprintf(_T("%ws=%ws=%ws\n"), item, subitem, ssubitem);
	}

	//释放空间
	VirtualFreeEx(process, _lvi, 0, MEM_RELEASE);
	VirtualFreeEx(process, _item, 0, MEM_RELEASE);
	VirtualFreeEx(process, _subitem, 0, MEM_RELEASE);
	VirtualFreeEx(process, _ssubitem, 0, MEM_RELEASE);
}
BOOL CALLBACK EnumChildProc(HWND hwnd, LPARAM lParam)
{//枚举子窗口的回调函数
	setlocale(LC_ALL, "chs");
	count++;
	TCHAR lpWinTitle[256], lpClassName[256];
	::GetWindowTextW(hwnd, (LPWSTR)lpWinTitle, 256 - 1); //获得窗口caption
	::GetClassNameW(hwnd, (LPWSTR)lpClassName, 256 - 1); //获得窗口类名
	if (_wcsicmp((LPWSTR)lpWinTitle, L"") != NULL && _wcsicmp((LPWSTR)lpClassName, L"") != NULL) //过滤掉没有名子和类名的窗口
	{
		fwprintf(fp, _T("\n"));
		for (int i = 1; i <= count; i++) fprintf(fp, " ");
		fwprintf(fp, _T("-0xX:'%ws':'%ws'"), hwnd, lpWinTitle, lpClassName);
		if (_wcsicmp(lpClassName, L"SysListView32") == NULL)
			ShowListView(hwnd);

		EnumChildWindows(hwnd, EnumChildProc, 0); //递归枚举子窗口
	}
	count--;
	return TRUE;
}
BOOL CALLBACK EnumWindowsProc(HWND hwnd, LPARAM lparam)
{//枚举窗口的回调函数
	setlocale(LC_ALL, "chs");
	count = 1;
	TCHAR lpWinTitle[256], lpClassName[256];
	::GetWindowTextW(hwnd, lpWinTitle, 256 - 1); //获得窗口caption
	::GetClassNameW(hwnd, lpClassName, 256 - 1); //获得窗口类名
	if (_wcsicmp(lpWinTitle, L"") != NULL && _wcsicmp(lpClassName, L"") != NULL) //过滤掉没有名子和类名的窗口
	{
		printf("%d\n", GetLastError());
		fwprintf(fp, _T("\n*-0xX:'%ws':'%ws'"), hwnd, lpWinTitle, lpClassName);

		EnumChildWindows(hwnd, EnumChildProc, 0); //继续枚举子窗口，传递给回调函数该子窗口的句柄
	}
	return TRUE;
}
int main()
{
	setlocale(LC_ALL, "chs");
	::EnumWindows(EnumWindowsProc, 0); //枚举桌面的窗口，每找到一个窗口，调用一次回调函数，传递给回调函数该窗口的句柄
	system("pause");
	return 0;
}