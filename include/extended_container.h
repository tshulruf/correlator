#ifndef EXTENDED_CONTAINER_H
#define EXTENDED_CONTAINER_H

#include <list>
#include <deque>
#include <vector>
#include <boost/foreach.hpp>

using namespace std;

// ExtendedContainer
//   Add a BOOST foreach and reverse_foreach to a std::Container<T>
//
template< class T, class Container >
class ExtendedContainer : public Container
{
public:
    // A Couple of Easy Loops
    //
    // foreach
    //      Do something with each element in the list
    //      starting at the front and working toward the back.
    //      a - function to execute on each element
    //
    template <class Action>
    void foreach(Action& a)
    {
        if(!this->empty())
        {
            BOOST_FOREACH(T& t, *this) { a(t); }
        }
    }
    
    // reverse_foreach
    //      Do something with each element in the list
    //      starting at the back and working toward the front.
    //      tda - function to execute on each element
    //
    template <class Action>
    void reverse_foreach(Action& a)
    {
        if(!this->empty())
        {
            BOOST_REVERSE_FOREACH(T& t, *this) { a(t); }
        }
    }

};


#endif // EXTENDED_CONTAINER_H
