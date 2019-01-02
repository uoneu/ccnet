#include <iostream>
#include <assert.h>
#include <string>
#include "../string_piece.h"


int main(void) {
    using namespace std;

    string s {"123456"};

    ccnet::StringArg sa(s);
    cout << sa.c_str() << endl;

    ccnet::StringPiece sp(s);
    cout << sp.size() << endl;
    cout << sp[1] << endl;

    return 0;
}