#include <openssl/rsa.h>
#include <openssl/pem.h>
#include <windows.h>
#include "rsa.h"
#include "base64.h"

#pragma comment(lib,"libeay32.lib")
#pragma comment(lib,"ssleay32.lib")

extern "C"
{
#include <openssl/applink.c>
}


ULONG_PTR MyFileEncrypt(WCHAR *wFileName)
{
	HANDLE hFile;
	ULONG_PTR ulSize,ulRetBytes,ulRet;
	char *szSrcFileDat,*szEncryptedFileDat,*szBase64EnCode;

	hFile = CreateFile(wFileName, \
		GENERIC_ALL, \
		FILE_SHARE_READ | FILE_SHARE_WRITE, \
		NULL, \
		OPEN_EXISTING, \
		FILE_ATTRIBUTE_NORMAL, \
		NULL);
	if (INVALID_HANDLE_VALUE == hFile)
	{
		return 0;
	}
	ulSize = GetFileSize(hFile,NULL);
	if (0 >= ulSize)
	{
		CloseHandle(hFile);
		return 0;
	}
	do 
	{
		szSrcFileDat = (char *)LocalAlloc(LPTR,ulSize);
	} while (NULL == szSrcFileDat);
	if (!ReadFile(hFile,szSrcFileDat,ulSize,&ulRetBytes,NULL))
	{
		if (szSrcFileDat)
		{
			LocalFree(szSrcFileDat);
		}
		CloseHandle(hFile);
		return 0;
	}
	szBase64EnCode = base64_encode((unsigned char *)szSrcFileDat,ulSize);
	ulRetBytes = strlen(szBase64EnCode);
	ulRet = MyRSAEncrypt(szBase64EnCode,&szEncryptedFileDat);
	if (ulRet)
	{
		CloseHandle(hFile);
		LocalFree(szSrcFileDat);
	}
	return ulRet;
}
RSA* createRSA(unsigned char* key,ULONG_PTR flag)
{
	RSA *rsa;
	BIO *keybio;

	rsa = NULL;

	keybio = BIO_new_mem_buf(key,-1);
	if (keybio == NULL)
	{
		printf("Failed to create key BIO");
		return 0;
	}
	if(flag)
	{
		rsa = PEM_read_bio_RSA_PUBKEY(keybio,&rsa,NULL,NULL);
	}
	else
	{
		rsa = PEM_read_bio_RSAPrivateKey(keybio,&rsa,NULL,NULL);
	}
	if(rsa == NULL)
	{
		printf("Failed to create RSA");
	}
	return rsa;
}
ULONG_PTR MyRSAEncrypt(char *szSrcDat,char **szOutEncrypted)
{
	RSA* rsa;
	int rsa_len;

	rsa = createRSA((unsigned char*)szPublicKey,1);
	rsa_len = RSA_size(rsa);
	do 
	{
		*szOutEncrypted = (char *)LocalAlloc(LPTR,rsa_len + 1);
	} while (NULL == *szOutEncrypted);
	return RSA_public_encrypt(rsa_len,(unsigned char*)szSrcDat,(unsigned char*)*szOutEncrypted,rsa,RSA_NO_PADDING);  
}
ULONG_PTR MyRSADecrypt(char *szEncrypted,char **szSrcDat)
{
	RSA* rsa;
	int rsa_len;

	rsa = createRSA((unsigned char*)szPrivateKey,0);
	rsa_len = RSA_size(rsa);
	do 
	{
		*szSrcDat = (char *)LocalAlloc(LPTR,rsa_len + 1);
	} while (NULL == *szSrcDat);
	return RSA_private_decrypt(rsa_len,(unsigned char*)szEncrypted,(unsigned char*)*szSrcDat,rsa,RSA_NO_PADDING);  
}