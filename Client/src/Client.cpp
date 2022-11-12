#include "Core/ClientApplication.h"
#include "Log.h"


int main()
{
    Log::Init();

    ClientApplication* app = new ClientApplication;
    app->Run();
    delete app;

    return 0;
}