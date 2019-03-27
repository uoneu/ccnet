#include "../thread.h"

#include <iostream>
#include <pthread.h>

using namespace std;
using namespace ccnet;


void foo() {
    cout << current_thread::t_tidString << endl;
    cout << current_thread::t_cachedTid << endl;
    mysleep(1);

}


int main(void) {
    cout << current_thread::t_cachedTid << endl;
    cout << current_thread::tidString() << endl;
    cout << current_thread::name() << endl;
    
    Thread t1(foo);
    cout << t1.name() << endl;
    cout << t1.tid() << endl;
    t1.start();
    t1.join();

    Thread t2(foo, "foo2");
    cout << t2.name() << endl;
    cout << t2.tid() << endl;
    t2.start();
    t2.join();


    return 0;
}
