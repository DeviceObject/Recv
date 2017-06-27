// PasServer.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "Initialize.h"
#include "control.h"

int _tmain(int argc, _TCHAR* argv[])
{
	SOCKET ListenSocket;
	SetBlackGreen();
	ShowWelcome();
	InitializeGlobal();
	ListenSocket = InitializeSocket();
	if (ListenSocket)
	{
		StartRecv(ListenSocket);
	}
	UnInitializeSocket(ListenSocket);
	return 0;
}

