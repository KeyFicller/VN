#include "server.h"

int main()
{
    ::VN::server_instance::instance().init();

    ::VN::server_instance::instance().exec();
}