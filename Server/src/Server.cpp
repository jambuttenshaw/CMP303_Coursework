#include "ServerApplication.h"
#include "Log.h"

int main()
{
	Log::Init();

	ServerApplication* serverApp = new ServerApplication;
	serverApp->Run();
	delete serverApp;

	return 0;
}