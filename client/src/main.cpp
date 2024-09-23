#include "visual_nurb.h"

#include <iostream>
#include <winsock2.h>
#include <WS2tcpip.h>

#pragma comment(lib, "ws2_32.lib")

int main() {
    while (true)
    {
        VN::client_instance::instance().seed();
    }

    return 0;
}