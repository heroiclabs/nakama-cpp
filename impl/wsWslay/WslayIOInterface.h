#pragma once

namespace Nakama {

class WslayIOInterface {
public:
    virtual ssize_t recv(void* buf, size_t len, int* wouldBlock) = 0;
    virtual ssize_t send(const void* data, size_t len, int* wouldBlock) = 0;
    virtual void genmask(uint8_t* buf, size_t len) = 0;
    virtual void close() = 0;
    virtual NetIOAsyncResult connect_init(const URLParts& urlParts) = 0;
    virtual NetIOAsyncResult connect_tick() = 0;
};

// Result of async operation
enum class NetIOAsyncResult {
    ERR = -1,  // error, no further progress possible
    AGAIN = 0,   // to be called again
    DONE = 1,    // successful completion
};
}