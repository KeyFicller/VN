#include "client.h"

int main()
{
    ::VN::vn_client_instance::instance().init();

    ::VN::vn_client_instance::instance().exec();
}
