#include "visual_nurb.h"

#include <iostream>
#include <winsock2.h>
#include <WS2tcpip.h>

#pragma comment(lib, "ws2_32.lib")

int main() {

    ::VN::VsNurbCurv crv;
    double knots[4] = { 0.0, 0.0, 1.0, 1.0 };
    crv.t.num_kt = 4;
    crv.t.knots = knots;
    crv.t.degree = 1;

    double cp[6] = { 0.0,0.0,0.0,100.0,100.0,100.0 };
    crv.cp.dim = 3;
    crv.cp.num_cp = 2;
    crv.cp.list = cp;

    while (true)
    {

        VN::client_instance::instance().plot_nurb_curve(&crv, {});

        Sleep(5000);
    }

    return 0;
}