
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <event2/event.h>
#include <event2/bufferevent.h>
#include<iostream>
#include<cassert>
#ifdef WIN32
#include"crash_dumper_w32.h"

#pragma comment (lib,"ws2_32.lib")
//#pragma comment (lib,"legacy_stdio_definitions.lib")
#include<ws2tcpip.h>
#include <WinSock2.h>
#else
#include <signal.h>
#endif

//extern "C" { FILE __iob_func[3] = { *stdin,*stdout,*stderr }; }

#ifdef WIN32
bool ctrlhandler(DWORD fdwctrltype)
{
	switch (fdwctrltype)
	{
		// handle the ctrl-c signal.
	case CTRL_C_EVENT:
		printf("ctrl-c event\n\n");
		return(true);
		// ctrl-close: confirm that the user wants to exit.
	case CTRL_CLOSE_EVENT:
		printf("ctrl-close event\n\n");
		return(true);
		// pass other signals to the next handler.
	case CTRL_BREAK_EVENT:
		printf("ctrl-break event\n\n");
		return false;
	case CTRL_LOGOFF_EVENT:
		printf("ctrl-logoff event\n\n");
		return false;
	case CTRL_SHUTDOWN_EVENT:
		printf("ctrl-shutdown event\n\n");
		return false;
	default:
		return false;
	}
}
#endif

#include "EventServer.h"
Linux_Win_Event wait_event;
CEventServer* g_server = NULL;
int main()
{
#ifdef WIN32

	CrashDumper::_PlaceHolder();

	WORD wVersionRequested;
	WSADATA wsaData;
	int err;

	/* Use the MAKEWORD(lowbyte, highbyte) macro declared in Windef.h */
	wVersionRequested = MAKEWORD(1, 1);
	err = WSAStartup(wVersionRequested, &wsaData);
	if (err != 0) {
		/* Tell the user that we could not find a usable */
		/* Winsock DLL.                                  */
		printf("WSAStartup failed with error: %d\n", err);
		return 1;
	}	
#else
	signal(SIGPIPE,SIG_IGN);
#endif

#if defined(_MSC_VER)
	printf("_MSC_VER is defined");
#endif

#ifdef WIN32

	if (SetConsoleCtrlHandler((PHANDLER_ROUTINE)ctrlhandler, true))
	{
		printf("\nthe control handler is installed.\n");
		printf("\n -- now try pressing ctrl+c or ctrl+break, or");
		printf("\n try logging off or closing the console...\n");
		printf("\n(...waiting in a loop for events...)\n\n");
	}
	else
		printf("\nerror: could not set control handler");
#endif
	
	wait_event.Initialize("gameserver");
	g_server = new CEventServer();
	g_server->Start("server.cfg");
	wait_event.Wait(-1);
	delete g_server;
	g_server = NULL;

#ifdef WIN32
	WSACleanup();
#endif


	return 0;
}
