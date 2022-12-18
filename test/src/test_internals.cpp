// unit tests, can use non public headers
#include <cstdlib>
#include <string>
#include <regex>
#include <sstream>
#include <iostream>
#include "nakama-cpp/log/NLogger.h"

// Copy of internal function from StrUtil so that we can test it
std::string encodeURIComponent(std::string decoded)
{
    std::ostringstream oss;
    std::regex r("[-.0-9A-Za-z_~]");

    for (char c : decoded)
    {
        if (std::regex_match(std::string(1, c), r))
        {
            oss << c;
        }
        else
        {
            oss <<'%' << std::uppercase << std::hex << static_cast<uint16_t>(0xff & c) ;
        }
    }
    return oss.str();
}

namespace Nakama {
namespace Test {

void test_uriencode() {
    std::string input = u8"βσκαταη3";
    std::string encoded = encodeURIComponent(input);
    std::string expected = "%CE%B2%CF%83%CE%BA%CE%B1%CF%84%CE%B1%CE%B73";

    if (encoded != expected) {
        NLOG_ERROR("Expected: " + expected);
        NLOG_ERROR("Encoded:  " + encoded);
        abort();
    }

    NLOG_INFO("test_uriencode passed");
}


void test_internals() {
    unsigned char c = char(120);
    test_uriencode();
}

}
}

