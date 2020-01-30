#ifndef PROGRESS_BAR
#define PROGRESS_BAR

#include "semaphore.h"
#include <stdio.h>

//  ProgressBar
//      Contain the logic for outputting a progress bar.
//      Progress bar is divided into sixtyfourths to 
//      fit on a terminal window (and for easy arithmetic.
//      Thread-safe.
//
class ProgressBar
{
public:
    //  Constructor
    //      Default constructor.
    //
    inline ProgressBar() : _sixtyfourth(0) {}

    //  Constructor
    //      message - What does the progress represent?
    //      length  - Count of things to progress through.
    //
    ProgressBar(const char * message, unsigned int length) : _banner(message), _sixtyfourth(length)
    {
        _sixtyfourth >>= 6;
        if (0 == _sixtyfourth) _sixtyfourth = 1;
        PBSem::reset(*this);
    }

    //  Destructor
    //      Spit out an extra newline to clean up the console...
    //
    ~ProgressBar()
    {
        if (0 != _sixtyfourth)
            ::printf("\n"); // sometimes two characters...
    }

    //  reset
    //      Start over with a new count.
    //      message - What does the progress represent?
    //      length  - Count of things to progress through.
    //
    void reset(const char * message, unsigned int length)
    {
        _banner = message;

        _sixtyfourth = length;
        _sixtyfourth >>= 6;
        if (0 == _sixtyfourth) _sixtyfourth = 1;
        PBSem::reset(*this);
    }        

    //  count
    //      Expose the current count.
    //
    inline const int count() const { return PBSem::count(); }

    //  increment
    //      Increment the internal counter.
    //      Output a '-' if the counter gets past a sixtyfourth of lentgh.    
    void increment()
    {
        PBSem::increment(*this);
    }

protected:
    //  _sixtyfourth
    //      An increment in the progress bar.
    //
    unsigned int _sixtyfourth;

    //  _banner
    //      Text to display over the progress bar.
    //
    string _banner;

    //  PBSem
    //      Semaphore class for actually updating the progress bar.
    //
    friend class Semaphore< ProgressBar, ProgressBar, ProgressBar >;
    typedef Semaphore< ProgressBar, ProgressBar, ProgressBar > PBSem;
    
    //  operator()()
    //      This is the action to take on reset.
    //
    void operator()()
    {
        ::printf("\n\n%s\n"\
//  64-character progress bar.
// 01234567890123456789012345678901234567890123456789012345678901234
  "|-------|-------|-------|-------|-------|-------|-------|--------|\n ",
                 _banner.c_str());
    }

    //  operator()(int)
    //      This is the action to take on increment (or decrement - not used).
    //      count - the current count.
    //
    void operator()(int count)
    {
        if (0 == _sixtyfourth) return;
        if (0 == (count % _sixtyfourth))
        {
            ::putchar('-');
            ::fflush(stdout);
        }
    }
};

#endif //PROGRESS_BAR
