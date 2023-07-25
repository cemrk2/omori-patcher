#include "rpc.h"
#include "utils.h"
#include "js.h"
#include <string>

using std::string;
typedef void (__stdcall *RPC_t)(const char* data);
static RPC_t RPC = nullptr;

namespace rpc
{
    void ParseMessage(const char* msg)
    {
        if (RPC == nullptr) {
            auto modHandle = LoadLibraryA("ml.dll");
            RPC = (RPC_t) GetProcAddress(modHandle, "RPC");
            Infof("RPC: %p", RPC);
            if (RPC == nullptr || RPC == INVALID_HANDLE_VALUE) {
                Error("Failed to find RPC!");
                return;
            }
        }

        RPC(msg);
    }
}