#include "stdafx.h"
#include "Config.h"

BOOLEAN ReadConfig(CHAR *szFileName,CHAR *szParam,int nLength)
{
	FILE *fp;

	if (nLength <= 0)
	{
		return FALSE;
	}
	if (NULL == (fp = fopen(szFileName,"r")))
	{
		return FALSE;
	}
	if (NULL == fgets(szParam,nLength,fp))
	{
		szParam[0] = '\0';
	}
	else
	{
		nLength = strlen(szParam);
		while((nLength > 0) && isspace(szParam[nLength - 1]))
		{
			nLength--;
		}
	}
	return TRUE;
}
BOOLEAN GetConfigString(CHAR *szFileName,CHAR **szParam)
{
	char szBuf[1024];

	*szParam = NULL;
	if (ReadConfig(szFileName,szBuf,sizeof(szBuf)) == FALSE)
	{
		return FALSE;
	}
	*szParam = strdup(szBuf);
	return TRUE;
}