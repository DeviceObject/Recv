#include "stdafx.h"
#include "control.h"

void SetBlackGreen()
{
	HANDLE hOut;
	hOut = GetStdHandle(STD_OUTPUT_HANDLE);  
	SetConsoleTextAttribute(hOut,FOREGROUND_GREEN | FOREGROUND_INTENSITY);
	return;
}
void ShowWelcome()
{
	printf("\n\n\n");
	printf("================================================================================\n");
	printf("=                           PasSend encrypt file transfer                      =\n");
	printf("=                                   Version:1.0                                =\n");
	printf("================================================================================\n");
	//system("pause");
}