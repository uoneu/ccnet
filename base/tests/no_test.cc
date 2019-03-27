#include <iostream>

#include "../copyable.h"
#include "../noncopyable.h"
#include "../types.h"
#include "../bounded_blocking_queue.h"
#include "../threadlocal.h"
#include "../threadlocal_singleton.h"
#include "../mutex.h"
#include "../count_down_latch.h"

using namespace ccnet;
using namespace std;



int main(int argc, char *argv[]) {

  std::cout << "=======\n";
  BoundedBlockingQueue<int> bq(10);
  bq.put(8);
  cout << bq.take()<<endl;
  char p[] = "00000";
  cout << sizeof(p);
  
  return 0;
}
