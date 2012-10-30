#ifndef TIMER_H
#define TIMER_H

#include <sys/time.h>
#include <stdlib.h>


class Timer
{
public:
    Timer()
    {
        startCount.tv_sec = startCount.tv_usec = 0;
        endCount.tv_sec = endCount.tv_usec = 0;
        stopped = 0;
        startTimeInMicroSec = 0;
        endTimeInMicroSec = 0;
    }

    ~Timer()
    {
    }

    void start()
    {
        stopped = 0;
        gettimeofday(&startCount, NULL);
    }

    void   stop()
    {
        stopped = 1;
        gettimeofday(&endCount, NULL);
    }

    double getElapsedTime()
    {
        return this->getElapsedTimeInSec();
    }

    double getElapsedTimeInSec()
    {
        return this->getElapsedTimeInMicroSec() * 0.000001;
    }

    double getElapsedTimeInMilliSec()
    {
        return this->getElapsedTimeInMicroSec() * 0.001;
    }

    double getElapsedTimeInMicroSec()
    {
        if(!stopped)
        {
            gettimeofday(&endCount, NULL);
        }

        startTimeInMicroSec = (startCount.tv_sec * 1000000.0) + startCount.tv_usec;
        endTimeInMicroSec = (endCount.tv_sec * 1000000.0) + endCount.tv_usec;

        return endTimeInMicroSec - startTimeInMicroSec;
    }

private:
    double startTimeInMicroSec;
    double endTimeInMicroSec;
    int    stopped;
    timeval startCount;
    timeval endCount;

};

#endif // TIMER_H
