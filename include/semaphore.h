#ifndef SEMAPHORE_H
#define SEMAPHORE_H

#include <boost/thread.hpp>

//  Semaphore
//      This is an up- and down-counting semaphore. A thread-safe counter.
//      Implementing this as a template enables more than one counter.
//      All parameterized actions will be taken when locked - use wisely.
//      R() - do something with the count on reset.
//      I(int) - do something with the count on increment.
//      D(int) - do something with the count on decrement.
//
template< class R, class I, class D >
class Semaphore
{
public:
    //  reset
    //      Set the counter back to zero and call R().
    //
    inline static void reset(R& do_r)
    {
        boost::lock_guard<boost::mutex> lock(_count_mutex);
        _count = 0;
        do_r();
    }
    
    //  increment
    //      Increase the count by one and call I(count).
    //
    inline static void increment(I& do_i)
    {
        boost::lock_guard<boost::mutex> lock(_count_mutex);
        ++_count;
        do_i(_count);
    }

    //  decrement
    //      Decrease the count by one and call D(count).
    //
    inline static void decrement(D& do_d)
    {
        boost::lock_guard<boost::mutex> lock(_count_mutex);
        --_count;
        do_d(_count);
    }
    
    //  count
    //      Expose the current count.
    //
    inline static int count() { return _count; }

protected:
    //  _count
    //      The current count.
    //
    static int          _count;

    //  _count_mutex
    //      A mutex to lock access to the count.
    //
    static boost::mutex _count_mutex;
};

template< class R, class I, class D >
int Semaphore< R, I, D >::_count(0);

template< class R, class I, class D >
boost::mutex Semaphore< R, I, D >::_count_mutex;


//  DoNothing
//      Use this as a dummy class to cause Semaphore to do nothing
//      after a particular operation.
//
class DoNothing
{
public:
    //  operator()()
    //      Do nothing on reset.
    //
    void operator()() { }

    //  operator()(int)
    //      Do nothing on increment and/or decrement.
    //
    void operator()(int i) { }
};

#endif // SEMAPHORE_H

