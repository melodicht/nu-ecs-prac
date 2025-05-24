#pragma once

#if SKL_LOGGING_ENABLED
#include <iostream>

#define LOG(x) std::cout << (x) << std::endl;
#else
#define LOG(x) (void)(0)
#endif