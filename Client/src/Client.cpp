#include "Core/ClientApplication.h"


int main()
{
    ClientApplication* app = new ClientApplication;
    app->Run();
    delete app;

    return 0;
}