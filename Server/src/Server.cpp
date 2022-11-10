#include "ServerApplication.h"

int main()
{
	ServerApplication* serverApp = new ServerApplication;
	serverApp->Run();
	delete serverApp;

	return 0;
}