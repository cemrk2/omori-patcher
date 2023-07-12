#include "rpc.h"
#include "utils.h"
#include "js.h"
#include <string>

using std::string;

namespace rpc
{
    void ParseMessage(const char* msg)
    {
        printf("rpc: %s\n", msg);
    }
}