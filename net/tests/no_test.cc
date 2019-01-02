#include <iostream>

#include "../callbacks.h"
#include "../endian.h"
#include "../timer_id.h"
#include "../timer.h"

using namespace ccnet;
using namespace std;

int main(int argc, char *argv[]) {
    cout << "====\n";
    cout << net::sockets::hostToNetwork64(89);

    return 0;
}
