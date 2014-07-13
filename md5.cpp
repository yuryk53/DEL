#pragma once
#include "md5.h"
#include <stdio.h>
#include <windows.h>
#include <Wincrypt.h>
#include <string>
#define BUFSIZE 1024
#define MD5LEN  16
using namespace std;
//source: http://msdn.microsoft.com/en-us/library/aa382380%28VS.85%29.aspx
string md5(char *filename)
{

	DWORD dwStatus = 0;
	BOOL bResult = FALSE;
	HCRYPTPROV hProv = 0;
	HCRYPTHASH hHash = 0;
	HANDLE hFile = NULL;
	BYTE rgbFile[BUFSIZE];
	DWORD cbRead = 0;
	BYTE rgbHash[MD5LEN];
	DWORD cbHash = 0;
	CHAR rgbDigits[] = "0123456789abcdef";
	//LPCWSTR filename=L"filename.txt";
	// Logic to check usage goes here.

	hFile = CreateFile(filename,
		GENERIC_READ,
		FILE_SHARE_READ,
		NULL,
		OPEN_EXISTING,
		FILE_FLAG_SEQUENTIAL_SCAN,
		NULL);

	if (INVALID_HANDLE_VALUE == hFile)
	{
		dwStatus = GetLastError();
		return "";
	}

	// Get handle to the crypto provider
	if (!CryptAcquireContext(&hProv,
		NULL,
		NULL,
		PROV_RSA_FULL,
		CRYPT_VERIFYCONTEXT))
	{
		dwStatus = GetLastError();
		CloseHandle(hFile);
		return "";
	}

	if (!CryptCreateHash(hProv, CALG_MD5, 0, 0, &hHash))
	{
		dwStatus = GetLastError();
		CloseHandle(hFile);
		CryptReleaseContext(hProv, 0);
		return "";
	}

	while (bResult = ReadFile(hFile, rgbFile, BUFSIZE, 
		&cbRead, NULL))
	{
		if (0 == cbRead)
		{
			break;
		}

		if (!CryptHashData(hHash, rgbFile, cbRead, 0))
		{
			dwStatus = GetLastError();
			CryptReleaseContext(hProv, 0);
			CryptDestroyHash(hHash);
			CloseHandle(hFile);
			return "";
		}
	}

	if (!bResult)
	{
		dwStatus = GetLastError();
		CryptReleaseContext(hProv, 0);
		CryptDestroyHash(hHash);
		CloseHandle(hFile);
		return "";
	}

	cbHash = MD5LEN;
	char MD5Hash[33] = ""; // MD5 is 16 bytes, or 32 hex digits
	if (CryptGetHashParam(hHash, HP_HASHVAL, rgbHash, &cbHash, 0))
	{
		if(cbHash>16) return "";
		for (DWORD i = 0; i < cbHash; i++)
        {
           MD5Hash[2*i] = rgbDigits[rgbHash[i] >> 4];
            MD5Hash[2*i + 1] = rgbDigits[rgbHash[i] & 0xf];
        }
        MD5Hash[cbHash * 2] = 0;
	}
	else
	{
		dwStatus = GetLastError();
	}

	CryptDestroyHash(hHash);
	CryptReleaseContext(hProv, 0);
	CloseHandle(hFile); 
	return string(MD5Hash);
}