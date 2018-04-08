// IAccessibleAply.cpp : 定义控制台应用程序的入口点。
//

#include "stdafx.h"
#include<tchar.h>
#include <windows.h>
#include <oleacc.h>
#include <stdio.h>
#include <comutil.h>
#include <Shlwapi.h>
#include <string>
#include <commctrl.h>  
//#include<initguid.h>///As with any COM interface, the system file initguid.h should be included in any source code that requires Active Accessibility.
#pragma comment(lib, "user32.lib")
#pragma comment(lib, "oleacc.lib")
#pragma comment(lib, "comsuppw.lib")
#pragma comment(lib, "Shlwapi.lib")
#pragma comment(lib,"kernel32.lib")
//////////////////////////////////
#include "comutil.h"  
#pragma comment(lib, "comsuppw.lib")  
#pragma comment(lib,"comsuppwd.lib")  

/////////////////////////////////////////
UINT GetObjectState(IAccessible* pacc, VARIANT* pvarChild, LPTSTR lpszState, UINT cchState)
{
	HRESULT hr;
	VARIANT varRetVal;
	*lpszState = 0;
	VariantInit(&varRetVal);
	hr = pacc->get_accState(*pvarChild, &varRetVal);
	if (!SUCCEEDED(hr))
		return(0);
	if (varRetVal.vt == VT_I4)
	{
		// 根据返回的状态值生成以逗号连接的字符串。
		GetStateText(varRetVal.lVal, lpszState, cchState);
	}
	else if (varRetVal.vt == VT_BSTR)
	{
		if (varRetVal.bstrVal)_tcscpy_s(lpszState, cchState, _com_util::ConvertBSTRToString(varRetVal.bstrVal));
	}
	VariantClear(&varRetVal);
	return _tcslen(lpszState);
}

UINT GetObjectName(IAccessible* pacc, VARIANT* pvarChild, LPTSTR lpszName, UINT cchName)
{
	HRESULT hr;
	BSTR pszName;
	hr = pacc->get_accName(*pvarChild, &pszName);
	if (pszName)_tcscpy_s(lpszName, cchName, _com_util::ConvertBSTRToString(pszName));
	return _tcslen(lpszName);
}

UINT GetObjectValue(IAccessible* pacc, VARIANT* pvarChild, TCHAR* pValue, UINT cchValue)
{
	HRESULT hr;
	BSTR pszValue;
	hr = pacc->get_accValue(*pvarChild, &pszValue);
	if (pszValue)_tcscpy_s(pValue, cchValue, _com_util::ConvertBSTRToString( pszValue));
	return _tcslen(pValue);
}

UINT GetObjectClass(IAccessible* paccChild, LPTSTR lpszClass, UINT cchClass)
{
	HWND hWnd;
	::WindowFromAccessibleObject(paccChild, &hWnd);
	if (hWnd)::GetClassName(hWnd, lpszClass, cchClass);
	return _tcslen(lpszClass);
}

UINT GetObjectRole(IAccessible* pacc, VARIANT* pvarChild, LPTSTR lpszRole, UINT cchRole)
{
	HRESULT hr;
	VARIANT varRetVal;
	*lpszRole = 0;
	VariantInit(&varRetVal);
	hr = pacc->get_accRole(*pvarChild, &varRetVal);
	if (!SUCCEEDED(hr))
		return(0);
	if (varRetVal.vt == VT_I4)
	{
		// 根据返回的状态值生成以逗号连接的字符串。
		GetRoleText(varRetVal.lVal, lpszRole, cchRole);
	}
	else if (varRetVal.vt == VT_BSTR)
	{
		if (varRetVal.bstrVal)_tcscpy_s(lpszRole, cchRole, _com_util::ConvertBSTRToString(varRetVal.bstrVal));
	}
	VariantClear(&varRetVal);
	return(_tcslen(lpszRole));
}

/////////////////////////
BOOL FindAccessible(
	IAccessible* paccParent,
	LPSTR szName,
	LPSTR szRole,
	LPSTR szClass,
	IAccessible** paccChild,
	VARIANT* pvarChild)
{

	HRESULT hr;
	long numChildren;
	unsigned long numFetched;
	VARIANT varChild;
	int indexCount;
	IAccessible* pCAcc = NULL;
	IEnumVARIANT* pEnum = NULL;
	IDispatch* pDisp = NULL;
	BOOL found = false;
	char szObjName[MAX_PATH] = { 0 }, szObjRole[MAX_PATH] = { 0 }, szObjClass[MAX_PATH] = { 0 }, szObjState[MAX_PATH] = { 0 };

	//Get the IEnumVARIANT interface
	hr = paccParent->QueryInterface(IID_IEnumVARIANT, (PVOID*)& pEnum);

	if (pEnum)
		pEnum->Reset();

	// Get child count
	paccParent->get_accChildCount(&numChildren);

	for (indexCount = 1; indexCount <= numChildren && !found; indexCount++)
	{
		pCAcc = NULL;

		// Get next child
		if (pEnum)
			hr = pEnum->Next(1, &varChild, &numFetched);
		else
		{
			varChild.vt = VT_I4;
			varChild.lVal = indexCount;
		}

		// Get IDispatch interface for the child
		if (varChild.vt == VT_I4)
		{
			pDisp = NULL;
			hr = paccParent->get_accChild(varChild, &pDisp);
		}
		else
			pDisp = varChild.pdispVal;

		// Get IAccessible interface for the child
		if (pDisp)
		{
			hr = pDisp->QueryInterface(IID_IAccessible, (void**)&pCAcc);
			hr = pDisp->Release();
		}

		// Get information about the child
		if (pCAcc)
		{
			VariantInit(&varChild);
			varChild.vt = VT_I4;
			varChild.lVal = CHILDID_SELF;

			*paccChild = pCAcc;
		}
		else
			*paccChild = paccParent;

		// Skip invisible and unavailable objects and their children
		GetObjectState(*paccChild, &varChild, szObjState, sizeof(szObjState));
		if (NULL != strstr(szObjState, "不可见"))
		{
			if (pCAcc)
				pCAcc->Release();
			continue;
		}

		GetObjectName(*paccChild, &varChild, szObjName, sizeof(szObjName));
		GetObjectRole(*paccChild, &varChild, szObjRole, sizeof(szObjRole));
		GetObjectClass(*paccChild, szObjClass, sizeof(szObjClass));

		if ((!szName || !strcmp(szName, szObjName)) && (!szRole || !strcmp(szRole, szObjRole)) && (!szClass || !strcmp(szClass, szObjClass)))
		{
			found = true;
			*pvarChild = varChild;
			break;

		}
		if (!found && pCAcc)
		{
			// Go deeper
			found = FindAccessible(pCAcc, szName, szRole, szClass, paccChild, pvarChild);
			if (*paccChild != pCAcc)
				pCAcc->Release();
		}
	}

	// Clean up
	if (pEnum)
		pEnum->Release();

	return found;
}
/////////////////////
//
//  函数: GetWindowHWndByParentHWndAndClassName(HWND hParentHWnd, LPTSTR lpszClassName)
//  目的: 通过父句柄和类名称获取窗口句柄。
//  FindWindowEx
HWND GetWindowHWndByParentHWndAndClassName(HWND hParentHWnd, LPTSTR lpszClassName)
{
	HWND hWnd = NULL;
	HWND hTempParentHWnd = NULL;
	TCHAR szClassText[MAX_PATH] = { 0 };


	//hWnd = GetWindow(GetDesktopWindow(), GW_CHILD);
	hWnd = GetWindow(hParentHWnd, GW_CHILD);
	while (hWnd) {
		GetClassName(hWnd, szClassText, MAX_PATH);
		if (_tcsstr(szClassText, lpszClassName) != NULL) {

			hTempParentHWnd = GetParent(hWnd);

			if (hTempParentHWnd == hParentHWnd) {
				return hWnd;
			}
		}
		hWnd = GetNextWindow(hWnd, GW_HWNDNEXT);
	}
	return NULL;
}
/////////////////////////////

int _tmain(int argc, _TCHAR* argv[])
{
	system("title WOW&color 2e");

	
	::CoInitialize(NULL);
	//ShellExecuteA(NULL, "open", "rundll32.exe", "shell32.dll,Control_RunDLL ncpa.cpl", NULL, SW_NORMAL);
	//HWND hWnd = ::FindWindow("CabinetWClass", "控制面板\\网络和 Internet\\网络连接");
	//hWnd = ::FindWindowEx(hWnd, NULL, "ShellTabWindowClass", "控制面板\\网络和 Internet\\网络连接");
	//hWnd = ::FindWindowEx(hWnd, NULL, "DUIViewWndClassName", NULL);
	HWND hWnd = ::FindWindow("#32770", "来自网页的消息");
	hWnd = ::FindWindowEx(hWnd, NULL, "DirectUIHWND", NULL);
	//hWnd = ::FindWindowEx(hWnd, NULL, "CtrlNotifySink", NULL);
	
	//hWnd = GetNextWindow(hWnd, GW_HWNDNEXT);
	//hWnd = GetNextWindow(hWnd, GW_HWNDNEXT);
	//hWnd = ::FindWindowEx(hWnd, NULL, "SHELLDLL_DefView", "ShellView");
	//hWnd = ::FindWindowEx(hWnd, NULL, "DirectUIHWND", NULL);





	if (!::IsWindow(hWnd))
		return FALSE;
	IAccessible *pIAcc = NULL;
	HRESULT hr = AccessibleObjectFromWindow(hWnd, OBJID_WINDOW, IID_IAccessible, (void**)&pIAcc);
	if (SUCCEEDED(hr) && pIAcc)
	{
		VARIANT varChild;
		VariantInit(&varChild);
		IAccessible *pIAccChild = NULL;
		char g_sSubPrefix[MAX_PATH] = "";
		if (FindAccessible(pIAcc, "本地连接", "列表项目","DirectUIHWND", &pIAccChild, &varChild))
		{
			//find
			printf("find\n");
			pIAccChild->accSelect(SELFLAG_TAKESELECTION | SELFLAG_TAKEFOCUS, varChild);

			Sleep(2000);

			pIAccChild->accDoDefaultAction(varChild);
			
		}
		else
		{
			//not find
			printf("not find\n");
		}
	}
	::CoUninitialize();
	system("pause");
	return 0;
}

///////////////////

//void main1()
//{
//	HWND hwnd = ::FindWindow(NULL, "运行");
//
//	IAccessible *paccMainWindow = NULL;
//	AccessibleObjectFromWindow(hwnd, OBJID_WINDOW, IID_IAccessible, (void**)&paccMainWindow);
//
//	long count = 0;
//	paccMainWindow->get_accChildCount(&count);
//
//	IAccessible*    paccControl = NULL;
//	VARIANT         varControl;
//
//	if (FindAccessible(paccMainWindow, "打开(O):", "可编辑文本", "Edit", &paccControl, &varControl))
//	{
//		paccControl->put_accValue(varControl, _com_util::ConvertStringToBSTR("123"));
//	}
//
//}