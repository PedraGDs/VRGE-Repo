
#include "TimeUtil.h"
#include "util/detect.h"

#if (OPERATING_SYSTEM == OS_LINUX) || (OPERATING_SYSTEM == OS_DARWIN) || (OPERATING_SYSTEM == OS_SOLARIS)

    #include <sys/time.h>
    #include <unistd.h>
    
#elif (OPERATING_SYSTEM == OS_WINDOWS)
    #include <windows.h>
#elif (OPERATING_SYSTEM == OS_QNX)
    #include <time.h>
#endif

void sleep ( double seconds ) {
#if (OPERATING_SYSTEM == OS_LINUX) || (OPERATING_SYSTEM == OS_DARWIN) || (OPERATING_SYSTEM == OS_SOLARIS)

    struct timespec ts;
    ts.tv_sec = static_cast<long> (seconds);
    ts.tv_nsec = static_cast<long> ( (seconds-ts.tv_sec) * 1000000000 );
    nanosleep(&ts, NULL);

#elif (OPERATING_SYSTEM == OS_WINDOWS)
    // A waitable timer seems to be better than the Windows Sleep().
    HANDLE WaitTimer;
    LARGE_INTEGER dueTime;

    seconds *= -10000000.0;

    dueTime.QuadPart = static_cast<LONGLONG>(seconds); //dueTime is in 100ns

    WaitTimer = CreateWaitableTimer(NULL, true, NULL);
    SetWaitableTimer(WaitTimer, &dueTime, 0, NULL, NULL, 0);
    WaitForSingleObject(WaitTimer, INFINITE);
    CloseHandle(WaitTimer);

#elif (OPERATING_SYSTEM == OS_QNX)
    struct timespec ts;
    _uint64 nsec = (_uint64) (seconds * 1000000000.0);
    nsec2timespec(&ts, nsec);
    nanosleep(&ts, NULL);
#endif
}

void sleepInSecs ( uint32_t secs ) {
    sleep ( secs );
}

void sleepInMs ( uint32_t ms ) {
    sleep ( ms / 1000.0 );
}

void sleepInUs ( uint32_t us ) {
    sleep ( us / 1000000.0 );
}
