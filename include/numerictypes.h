#ifndef NUMERICTYPES_H
#define NUMERICTYPES_H

#include <math.h>
#include <boost/operators.hpp>
#include <boost/numeric/conversion/converter.hpp>
#include <limits>
#include "constants.h"

using namespace std;

//  RealType
//      Define a floating point type with an invalid value and 
//      custom stream I/O to handle NaN's.
//
//  RealType
//      Default template.
//
template< class T >
struct RealType : public boost::additive< RealType< T >,
                         boost::multiplicative< RealType< T > > >
{
    //  Limits
    //      Expose some information about the type.
    //
    typedef numeric_limits< T > Limits;
    
    //  Invalid Value handling
    //
    //  invalid_value
    //      NaN.
    //
    const static T invalid_value;

    //  is_invalid
    //      Test if a number is invalid. Might only be used in operator>>...
    //      Works because of operator T() below...
    //      t - number to test.
    //      returns true if number matches the invalid value (NaN).
    //
    static bool is_invalid(const T& t) { return isnan(t); }

    //  is_valid
    //      Test if a number is valid. Might only be used in operator>>...
    //      Works because of operator T() below...
    //      t - number to test.
    //      returns true if number does not match the invalid value (NaN).
    //
    static bool is_valid(const T& t) { return !isnan(t); }

    //  value
    //      Container.
    //
    T value;
    
    //  Constructors
    //
    //  Default - value = invalid_value.
    //
    inline RealType() : value(invalid_value) { }

    //  Construct with a specific value.
    //      v - initial value of container.
    //
    inline RealType(const T& v) : value(v) { }

    //  Copy constructor.
    //
    inline RealType(const RealType& v) : value(v.value) { }

    //  Operators
    //
    //  operator T()
    //      Cast this to a T.
    //      Enables all of the default numeric operators.
    //      returns the container stripped of encasing type.
    //
    inline operator T() const { return value; }
    
    //  operator=()
    //      Copy Constructor.
    //
    inline RealType& operator=(const RealType& r)
    {
        if (&r != this)
            value = r.value;
        return *this; 
    }
    
    //  IEEE NaN is supposed to "infect" operations.
    //  Don't want that in + or -, want it in * or /, etc.
    //  Treat invalid_value as an additive identity.
    //
    //  operator+=
    //      Addition. boost::additive<RealType<T>> turns this into +.
    //      x - value to add to this.
    //      returns sum of this and x.
    //
    inline RealType& operator+=(const RealType& x)
    {
        // Treat invalid_value as an additive identity.
        if (!is_invalid(x.value))
            if (is_invalid(value)) value = x.value;
            else                   value += x.value;
        return *this;
    }

    //  operator-=
    //      Subtraction. boost::additive<RealType<T>> turns this into -.
    //      x - value to subtract from this.
    //      returns the difference between this and x.
    //
    inline RealType& operator-=(const RealType& x)
    {
        // Treat invalid_value as an additive identity.
        if (!is_invalid(x.value))
            if (is_invalid(value)) value = 0.0 - x.value;
            else                   value -= x.value;
        return *this;
    }

    //  operator*=
    //      Multiplication. boost::multiplicative<RealType<T>> turns this into *
    //      x - value to divide from this.
    //      returns the ratio between this and x.
    //
    inline RealType& operator*=(const RealType& x)
    {
        // Use default multiplication! NaN will corrupt results.
        value *= x.value;
        return *this;
    }

    //  operator/=
    //      Division. boost::multiplicative<RealType<T>> turns this into /
    //      x - value to divide from this.
    //      returns the ratio between this and x.
    //
    inline RealType& operator/=(const RealType& x)
    {
        // Use default division! NaN will corrupt results.
        value /= x.value;
        return *this;
    }
};

//  invalid_value
//      NaN.
//
template< class T >
const T RealType<T>::invalid_value(NAN);

template< class T >
ostream& operator<< (ostream& out, const RealType<T>& t)
{
    if (constants::save_as_binary)
        out.write((char *)(&(t.value)), sizeof(T));
    else
        out << t.value << " ";
        
    return out;
}

//  operator>>
//      Streaming input. "nan" == cout << NaN but NaN != "nan" >> cin.
//      Invalid value breaks streaming I/O of floating point types.
//      Catch the broken value and move on...
//      in - Input stream.
//      t  - Target RealType<T>
//      returns sane input stream.
//
template< class T >
istream& operator>> (istream& in, RealType<T>& t)
{
    // Try it.
    if(constants::save_as_binary)
        in.read((char *)(&(t.value)), sizeof(T));
    else        
    {
        in >> t.value;
        if (in.fail())
        {
            // load an invalid value for a failure to cast.
            t = RealType<T>::invalid_value;
            
            // clear the fail state and skip past the invalid string.
            in.clear();
            string dropped;
            in >> dropped;
        }
    }
        
    return in;
}

//  Default TypeDefs for float and double (not used).
//
typedef RealType<float> FloatType;
typedef RealType<double> DoubleType;


//
//
// Now do it all over again for long - 
// partial template instantiation breaks the defaults.
//
//  IntegerType
//      Define an integer type with an invalid value.
//
//  IntegerType
//      Default template.
//
template< class T >
struct IntegerType : public boost::additive< IntegerType< T > >
{
    //  Limits
    //      Expose some information about the type.
    //
    typedef numeric_limits< T > Limits;
    
    //  Invalid Value handling
    //
    //  invalid_value
    //      NaN.
    //
    const static T invalid_value;

    //  is_invalid
    //      Test if a number is invalid. Might only be used in operator>>...
    //      Works because of operator T() below...
    //      t - number to test.
    //      returns true if number matches the invalid value (-1).
    //
    static bool is_invalid(const T& t) { return (invalid_value == t); }

    //  is_valid
    //      Test if a number is valid. Might only be used in operator>>...
    //      Works because of operator T() below...
    //      t - number to test.
    //      returns true if number doesn't match the invalid value (-1).
    //
    static bool is_valid(const T& t) { return (invalid_value != t); }

    //  value
    //      Container.
    //
    T value;
    
    //  Constructors
    //
    //  Default - value = invalid_value.
    //
    IntegerType() : value(invalid_value) { }

    //  Construct with a specific value.
    //      v - initial value of container.
    //
    IntegerType(const T& v) : value(v) { }

    //  Copy constructor.
    //
    IntegerType(const IntegerType& v) : value(v.value) { }

    //  Operators
    //
    //  operator T()
    //      Cast this to a T.
    //      Enables all of the default numeric operators.
    //      returns the container stripped of encasing type.
    //
    operator T() const { return value; }
    
    //  operator=()
    //      Copy Constructor.
    //
    inline IntegerType& operator=(const IntegerType& r)
    {
        if (&r != this)
            value = r.value;
        return *this; 
    }
    
    //  Treat invalid_value as an additive identity.
    //
    //  operator+=
    //      Addition. boost::additive<IntegerType<T>> turns this into +.
    //      x - value to add to this.
    //      returns sum of this and x.
    //
    IntegerType& operator+=(const IntegerType& x)
    {
        // Treat invalid_value as an additive identity.
        if (!is_invalid(x.value))
            if (is_invalid(value)) value = x.value;
            else                   value += x.value;
        return *this;
    }

    //  operator-=
    //      Subtraction. boost::additive<IntegerType<T>> turns this into -.
    //      x - value to subtract from this.
    //      returns the difference between this and x.
    //
    IntegerType& operator-=(const IntegerType& x)
    {
        // Treat invalid_value as an additive identity.
        if (!is_invalid(x.value))
            if (is_invalid(value)) value = 0 - x.value;
            else                   value -= x.value;
        return *this;
    }
};

//  invalid_value
//      -1.
//
template< class T >
const T IntegerType<T>::invalid_value(-1);

template< class T >
ostream& operator<< (ostream& out, const IntegerType<T>& t)
{
    if (constants::save_as_binary)
        out.write((char *)(&(t.value)), sizeof(T));
    else
        out << t.value << " ";
        
    return out;
}

//  operator>>
//      Invalid value breaks streaming I/O of integer types.
//      Catch the broken value and move on...
//      in - Input stream.
//      t  - Target RealType<T>
//      returns sane input stream.
//
template< class T >
istream& operator>> (istream& in, IntegerType<T>& t)
{
    if(constants::save_as_binary)
        in.read((char *)(&(t.value)), sizeof(T));
    else        
    {
        // Try it.
        in >> t.value;
        if (in.fail())
        {
            // load an invalid value for a failure to cast.
            t = IntegerType<T>::invalid_value;
            
            // clear the fail state and skip past the invalid string.
            in.clear();
            string dropped;
            in >> dropped;
        }
    }    
    return in;
}

//  Default TypeDef for long and int.
//
typedef IntegerType<int>  IntType;
typedef IntegerType<long> LongType;

//  Type Conversion
//
//  float_type_to_int_type
//      Convert a floating point number to an integer, rounding.
//      NaN's will be converted to -1's.
//      fin - input floating point number.
//      returns rounded integer value or -1 for invalid input.
//
template< class F, class I >
I float_type_to_int_type(F fin)
{
    if (RealType<F>::is_invalid(fin))    
        return IntegerType<I>::invalid_value;
    
    using namespace boost::numeric;
    typedef converter< I,
                       F,
                       conversion_traits< I, F >,
                       def_overflow_handler,
                       RoundEven< F >,
                       raw_converter< conversion_traits< I, F > >,
                       UseInternalRangeChecker > FtoI;

    FtoI convert;
    I result = IntegerType< I >::invalid_value;

    try
    {
        result = convert(fin);
    }
    catch(exception&)
    {
        result = IntegerType< I >::invalid_value;
    }
    
    return result;
}


//  Random Helper Functions
//
//  sum_first_n_numbers
//      Add up the numbers from 1 to n.
//      Overflow might be an issue here - use it wisely.
//      Max N is < sqrt(numeric_limits<unsigned int>::max())
//      or Max N < about 65536.
//      Here we're typically working with n ~ 8000. No prob.
//      PS: Gauss rocked.
//      Formula courtesy CRC Math Handbook 29th Ed.
//      n - last number in the series.
//      returns the sum of all of the numbers from 1 to n.
//
inline const unsigned int sum_first_n_numbers(const unsigned int n)
{
    // return ((n+1) * n) / 2
    unsigned int temp = n;
    temp += 1;
    temp *= n;
    temp >>= 1; // right shift one == divide by 2.
    return temp;
}

#endif // NUMERICTYPES_H

