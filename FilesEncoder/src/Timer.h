#pragma once

#include <ratio>
#include <chrono>
#include <stdexcept>

// It's ok here.
using namespace std;

/** \brief Simple nanoseconds timer provider. */
class Timer
{

public:

    /** \brief Starts the timer. */
    void start()
    {
        triggered ? throw logic_error("The timer has already been started.") : triggered = true;
        m_start = chrono::high_resolution_clock::now();
    }

    /** \brief Stops the timer. */
    void stop()
    {
        !triggered ? throw logic_error("The timer has not been started yet.") : triggered = false;
        m_stop = chrono::high_resolution_clock::now();
        duration = chrono::duration_cast<chrono::nanoseconds>(m_stop - m_start);
    }

    /** \brief Gives result in nanoseconds. */
    long long result() const
    {
        return duration.count();
    }

private:

    // Стартовая точка таймера.
    chrono::time_point<chrono::high_resolution_clock> m_start;

    // Конечная точка таймера.
    chrono::time_point<chrono::high_resolution_clock> m_stop;

    // Флаг, показывающий запущен ли таймер.
    bool triggered = false;

    // Время в наносекундах между стартовой и конечной точками таймера.
    enable_if<integral_constant<bool, true>::value, chrono::duration<long long, ratio<1, 1000000000>>>::type duration;
};
