#include <iostream>
#include "../logging.h"

using namespace ccnet;

FILE *g_file = nullptr;

void logOutput(const char* msg, int len)
{
  size_t n = fwrite(msg, 1, len, g_file);
  //FIXME check n
  (void)n;
}

void logFlush()
{
  fflush(g_file);
}




int main(void) {
    printf("%d\n", getpid());
    g_file = fopen("a.txt", "ae");
    Logger::setOutput(logOutput);
    Logger::setFlush(logFlush);


    LOG_TRACE << "xxxxxxxx";
    LOG_INFO << "xxxxxxxx";
    LOG_FATAL << "siji";

    fclose(g_file);
    return 0;
}