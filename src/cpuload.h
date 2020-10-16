#pragma once

#define DEFAULT_CPU_LOAD_TM_MS 1000

namespace zutils {

    double cpu_load(unsigned long tm_us = DEFAULT_CPU_LOAD_TM_MS);

}//namespace zutils
