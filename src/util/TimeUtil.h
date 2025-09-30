
#pragma once

#include "stdint.h"

#ifdef __cplusplus
#include <chrono>

extern "C" {
#endif

void sleepInUs ( uint32_t us );
void sleepInMs ( uint32_t ms );
void sleepInSecs ( uint32_t secs );

#ifdef __cplusplus
} // extern C

inline void sleepFor ( std::chrono::high_resolution_clock::duration duration ) {

    uint64_t duration64 = std::chrono::duration_cast<std::chrono::microseconds>(duration).count();

    if ( duration64 > 0 ) {
        sleepInUs(duration64);
    }

}

inline void sleepUntil ( std::chrono::high_resolution_clock::time_point timepoint ) {
    sleepFor ( timepoint - std::chrono::high_resolution_clock::now() );
}

#endif