// Server.cpp : 定义控制台应用程序的入口点。
//

#include "stdafx.h"

#include <openssl/bio.h>
#include <openssl/err.h>
#include <openssl/evp.h>
#include <openssl/rsa.h>
#pragma comment(lib,"libeay32.lib")
#pragma comment(lib,"ssleay32.lib")

#include "Config.h"

SERVER_CONFIG g_ServerConfig;

char *my_encrypt(char *str,char *path_key)
{ 
	char *p_en; 
	RSA *p_rsa; 
	FILE *file; 
	int flen,rsa_len; 
	if((file=fopen(path_key,"r")) == NULL)
	{ 
		perror("open key file error"); 
		return NULL; 
	}
	if((p_rsa = PEM_read_RSA_PUBKEY(file,NULL,NULL,NULL)) == NULL)
	{ 
		//if((p_rsa=PEM_read_RSAPublicKey(file,NULL,NULL,NULL))==NULL){   换成这句死活通不过，无论是否将公钥分离源文件 
		ERR_print_errors_fp(stdout); 
		return NULL; 
	} 
	flen = strlen(str); 
	rsa_len = RSA_size(p_rsa); 
	p_en = (char *)malloc(rsa_len+1); 
	memset(p_en,0,rsa_len + 1); 
	if(RSA_public_encrypt(rsa_len,(unsigned char *)str,(unsigned char*)p_en,p_rsa,RSA_NO_PADDING)<0)
	{ 
		return NULL; 
	}
	SA_free(p_rsa); 
	fclose(file); 
	return p_en; 
} 
char *my_decrypt(char *str,char *path_key)
{ 
	char *p_de; 
	RSA *p_rsa; 
	FILE *file; 
	int rsa_len; 
	if((file=fopen(path_key,"r"))==NULL)
	{ 
		perror("open key file error"); 
		return NULL; 
	} 
	if((p_rsa = PEM_read_RSAPrivateKey(file,NULL,NULL,NULL)) == NULL)
	{ 
		ERR_print_errors_fp(stdout); 
		return NULL; 
	} 
	rsa_len = RSA_size(p_rsa); 
	p_de = (char *)malloc(rsa_len + 1); 
	memset(p_de,0,rsa_len + 1); 
	if(RSA_private_decrypt(rsa_len,(unsigned char *)str,(unsigned char*)p_de,p_rsa,RSA_NO_PADDING)<0)
	{ 
		return NULL; 
	} 
	RSA_free(p_rsa); 
	fclose(file); 
	return p_de; 
}
int _tmain(int argc, _TCHAR* argv[])
{
	BIO *pbio = NULL, *cbio = NULL;
	int pfd;

	RtlZeroMemory(&g_ServerConfig,sizeof(SERVER_CONFIG));
	GetConfigString(VERSION_FILE,&g_ServerConfig.szVersion);
	if (!g_ServerConfig.szVersion)
	{
		g_ServerConfig.szVersion = strdup(VERSION_DEFAULT);
	}
	GetConfigString(COOKIE_FILE,&g_ServerConfig.szCookie);
	if (!g_ServerConfig.szCookie)
	{
		return -1;
	}
	GetConfigString(KEY_FILE,&g_ServerConfig.szKey);
	if (!g_ServerConfig.szKey)
	{
		return -1;
	}
	GetConfigString(PORT_FILE,&g_ServerConfig.szPort);
	if (!g_ServerConfig.szPort || \
		atoi(g_ServerConfig.szPort) <= 0 || \
		atoi(g_ServerConfig.szPort) > 65535)
	{
		if (g_ServerConfig.szPort)
		{
			free(g_ServerConfig.szPort);
		}
		g_ServerConfig.szPort = strdup(PORT_DEFAULT);
	}
	OpenSSL_add_all_ciphers();
	pbio = BIO_new_accept(g_ServerConfig.szPort);
	BIO_set_bind_mode(pbio,BIO_BIND_REUSEADDR);
	pfd = BIO_get_fd(pbio,NULL);
	return 0;
}

