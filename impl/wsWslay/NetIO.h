#pragma once

namespace Nakama {

// Result of async operation
enum class NetIOAsyncResult {
    ERR = -1,  // error, no further progress possible
    AGAIN = 0,   // to be called again
    DONE = 1,    // successful completion
};
}