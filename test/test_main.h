#pragma once

#include <chrono>
#include <thread>
#include <iostream>
#include "nakama-cpp/Nakama.h"

namespace Nakama {
namespace Test {

    void setWorkingClientParameters(DefaultClientParameters& parameters);

} // namespace Test
} // namespace Nakama
