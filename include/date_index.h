#ifndef DATE_INDEX_H
#define DATE_INDEX_H

#include "constants.h"
#include "numerictypes.h"
#include <iostream>
#include <string>


// DateIndex
//      Used to transform a date in a line of a Yahoo Finance CSV file
//      into an index into an array to be sent to a CUDA processor.
//
class DateIndex
{
public:

    // IndexType
    //      use this to index an array... probably won't use
    //      more than 32768 elements in an array.
    //
    typedef int IndexType;

    // from_string
    //      Parse a simple date string (eg. "2010-10-07")
    //      into a boost::gregorian::date and compute the difference
    //      from that date to a start date configured in constants.h.
    //
    inline static IndexType from_string(const string& in_date) throw()
    {
        try
        {
            return (int)(boost::gregorian::from_simple_string(in_date).julian_day() - s_zero_date);
        }
        catch(...) { return -1; }
    }
    
    // to_string
    //      Create a simple date string (eg. "2010-Oct-07")
    //      from a date index relative to the start date
    //      configured in constants.h.
    //
    inline static string to_string(const IndexType& in_date) throw()
    {
        using namespace boost::gregorian;
        try
        {
            return (to_iso_extended_string(
                        constants::start_date + date_duration(in_date)) );
        }
        catch(...) { return "INVALID DATE"; }
    }
    
    // valid
    //      Double check that the date was parsed correctly
    //      and represents some value after the start_date
    //      and before the end_date.
    //
    inline static bool valid(int index)
    {
        return (0 <= index < s_max_index);
    }
    
    // interval
    //      Compute the number of days in the valid range.
    //
    inline static IndexType interval()
    {
        return s_max_index + 1;
    }

    inline static IndexType first() { return 0;           }
    inline static IndexType last()  { return s_max_index; }

protected:
    // beginning Julian Date and max delta.
    static const long s_zero_date;
    static const int s_max_index;

private:
    // static only! do not construct!
    inline DateIndex() { }

    // Do Not Copy!
    inline DateIndex(const DateIndex& di) {}
};
const long DateIndex::s_zero_date(constants::start_date.julian_day());
const int DateIndex::s_max_index(
    (DateIndex::IndexType)(constants::end_date.julian_day() - s_zero_date) );


//  DateIndexedType
//      Aggregate a DateIndex::IndexType with a container.
//      Note that Container needs to have default and
//      copy constructors and to be serializable.
//
template<class Container>
struct DateIndexedType
{
    //  Size
    //      Used to determine offsets into data files.
    //      Returns the size of the data structure.
    //
    inline const static int Size() 
    {
        return( sizeof(DateIndex::IndexType) + Container::Size() );
    }

    DateIndex::IndexType    index;
    Container               value;
    
    //  Default Constructor
    //      default construction of container, index to first date.
    //
    inline DateIndexedType() : index(0), value() { }
    
    //  Construct by Parts
    //      copy construction of container.
    //
    inline DateIndexedType(const DateIndex::IndexType& i, const Container& c) :
        index(i), value(c) { }
        
    //  Copy Constructor
    //      copy construction of container and date index.
    //
    inline DateIndexedType(const DateIndexedType& dit) : 
        index(dit.index), value(dit.value) { }
};

//  Comparison Operators
//      Order the date indexed type by index.
//
// operator==
//      a, b - indexed values to compare.
//      returns - true if indexed value have the same date index.
//
template<class C>
inline const bool operator==(const DateIndexedType<C>& a, const DateIndexedType<C>& b)
{
    return (a.index == b.index);
}

// operator<
//      a, b - indexed values to compare.
//      returns - true if a.index < b.index
//
template<class C>
inline const bool operator<(const DateIndexedType<C>& a, const DateIndexedType<C>& b)
{
    return (a.index < b.index);
}

//  Stream I/O operators.
//  operator<<
//      Requires a valid C.operator<< or ostream& operator<<(ostream&, const C&)
//      out - output text stream.
//      a   - indexed data item to stream.
//      returns - stream for continued use.
//
template<class C>
ostream& operator<< (ostream& out, const DateIndexedType<C>& a)
{
    if (constants::save_as_binary)
    {
        out.write((char *)(&(a.index)), sizeof(DateIndex::IndexType));
        out << a.value;
    }
    else
        out << a.index << " " << a.value << endl;
    
    return out;
}

//  operator>>
//      Requires a valid C.operator>> or istream& operator>>(istream&, C&)
//      in - input stream.
//      a  - indexed data item to stream.
//      returns - stream for continued use.
//
template<class C>
istream& operator>> (istream& in, DateIndexedType<C>& a)
{
    if (constants::save_as_binary)
    {
        in.read((char *)(&(a.index)), sizeof(DateIndex::IndexType));
        in >> a.value;
    }
    else
        in >> a.index >> a.value;
    
    return in;
}

#endif // DATE_INDEX_H

