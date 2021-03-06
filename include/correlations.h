#ifndef CORRELATIONS_H
#define CORRELATIONS_H

#include "source_data.h"
#include <boost/thread.hpp>

//  Correlations
//      Contain the correlations between two StatisticalData structs.
//      Note that there are some ugly double negatives to follow.
//      This is why statistics is a pain.
//
template<class Real>
struct Correlations
{
    //  *_day_min_r
    //      Minimum correlation value for at most 1%
    //      probability that correlation number was 
    //      generated by uncorrelated values.
    //      Ref.Table C. Page 249, 
    //      "An Introduction To Error Analysis",
    //      John R. Taylor, ISBN 0-935702-10-5
    //
    const static Real ten_day_min_r;
    const static Real fifty_day_min_r;
    
    //  ten_day
    //      Correlation between two 10-day moving averages.
    //
    Real ten_day;
    
    //  fifty_day
    //      Correlation between two 50-day moving averages.
    //
    Real fifty_day;
    
    //  Constructor
    //
    inline Correlations() { }
    
    //  Copy Constructor
    //
    inline Correlations(const Correlations& c) : 
        ten_day(c.ten_day), fifty_day(c.fifty_day)
    { }
    
    //  ten_day_correlated
    //      Return true if the 10-day correlation is unlikely
    //      to represent uncorrelated data.
    //
    inline bool ten_day_correlated() 
    {
        return nday_correlated(ten_day, ten_day_min_r); 
    }
    
    //  fifty_day_correlated
    //      Return true if the 50-day correlation is unlikely
    //      to represent uncorrelated data.
    //
    inline bool fifty_day_correlated() 
    {
        return nday_correlated(fifty_day, fifty_day_min_r); 
    }
    
    //  ten_day_transitive
    //      Return true if two 10-day correlations might be transitive.
    //
    inline bool ten_day_transitive(const Correlations& cors)
    {
        return nday_transitive(ten_day, cors.ten_day); 
    }
    
    //  fifty_day_transitive
    //      Return true if two 50-day correlations might be transitive.
    //
    inline bool fifty_day_transitive(const Correlations& cors)
    {
        return nday_transitive(fifty_day, cors.fifty_day); 
    }
    
protected:
    //  is_correlated
    //      Helper function to determine if a value represents a
    //      valid correlation.
    //      value     - Value to test.
    //      min_value - Minimum correlation value for low
    //                  probability of uncorrelated data.
    //      Return true if value is valid and abs(value) >= min.
    //
    inline bool nday_correlated(Real value, Real min_value)
    {
        return ((Real::is_valid(value)) && 
                (min_value <= abs(value))  );
    }
    
    //  is_transitive
    //      Helper function to see if a pair of correlation values
    //      are potentially transitively corellated (and therefore
    //      part of a network.
    //      If r^2(x, y) + r^2(y, z) > 1, then r(z, z) probably
    //      represents a corellation (not uncorellated).
    //      www.stat.auckland.ac.nz/~iase/serj/SERJ8(2)_Sotos.pdf
    //
    inline bool nday_transitive(Real Rxy, Real Ryz)
    {
        return( Real::is_valid(Rxy) && Real::is_valid(Ryz) &&
                (((Rxy * Rxy) + (Ryz * Ryz)) > 1.0)        );
    }
};

//  ten_day_min_r
//      For 10 samples, r = 0.8 gives P(uncorrelated) = 0.005
//      (Taylor).
//
template< class Real >
const Real Correlations<Real>::ten_day_min_r = Real(0.8);

//  fifty_day_min_r
//      For 50 samples, r = 0.4 gives P(uncorrelated) = 0.004
//      (Taylor).
//      www.stat.auckland.ac.nz/~iase/serj/SERJ8(2)_Sotos.pdf
//
template< class Real >
const Real Correlations<Real>::fifty_day_min_r = Real(0.4);

//  Stream I/O operators.
//  operator<<
//      out - output text stream.
//      c   - Correlations to stream.
//      returns - stream for continued use.
//
template<class Real>
ostream& operator<< (ostream& out, const Correlations<Real>& c)
{
    if (constants::save_as_binary)
        out.write((char *)(&c), sizeof(Correlations<Real>));
    else
        out << c.fifty_day << c.ten_day << endl;

    return out;
}

//  operator>>
//      in - input stream.
//      c  - Correlations to stream.
//      returns - stream for continued use.
//
template<class Real>
istream& operator>> (istream& in, Correlations<Real>& c)
{
    if (constants::save_as_binary)
        in.read((char *)(&c), sizeof(Correlations<Real>));
    else
        in >> c.fifty_day >> c.ten_day;

    return in;
}

typedef Correlations< FloatType  > FloatCorrelations;
typedef Correlations< DoubleType > DoubleCorrelations;


//  RowColPair
//      Store a set of correlated pairs.
//
struct RowColPair
{
    unsigned int row;
    unsigned int col;
    
    RowColPair() : row(0), col(0) { }
    RowColPair(const unsigned int r, const unsigned int c) : row(r), col(c) { }
    RowColPair(const RowColPair& rc) : row(rc.row), col(rc.col) { }
    RowColPair& operator=(const RowColPair& rc)
    {
        if(this != &rc)
        {
            row = rc.row;
            col = rc.col;
        }
        return *this;
    }
};

//  Stream I/O operators.
//  operator<<
//      out - output text stream.
//      rc  - RowColPair to stream.
//      returns - stream for continued use.
//
ostream& operator<< (ostream& out, const RowColPair& rc)
{
    if (constants::save_as_binary)
        out.write((char *)(&rc), sizeof(RowColPair));
    else
        out << rc.row << " " << rc.col << endl;

    return out;
}

//  operator>>
//      in - input stream.
//      rc - RowColPair to stream.
//      returns - stream for continued use.
//
istream& operator>> (istream& in, RowColPair& rc)
{
    if (constants::save_as_binary)
        in.read((char *)(&rc), sizeof(RowColPair));
    else
        in >> rc.row >> rc.col;

    return in;
}

typedef vector<RowColPair> RowColVector;


//  CrossCorrelation
//      This is a "slice." Contain the cross-correlations of a set of data.
//      It represents the bottom triangle of a symmetric matrix with 1's 
//      on the diagonal (skipping the diagonal).
//      Since every element will be assigned in the loop, probably don't need
//      to assign NaN's to all of the elements between cross-correlations.
//
template< class Real >
class CrossCorrelation
{
public:
    //  CorrelationsType
    //      Contain a pair of correlations.
    //
    typedef Correlations< Real > CorrelationsType;
    typedef CorrelationsType&    CorrelationsRef;

    //  Default Constructor - must resize!
    //
    inline CrossCorrelation() : _queued_element(1, 0, 0) { }
    
    //  Constructor
    //      Allocates room for the cross-correlations of "elements" elements.
    //      Max N < about 65536 elements.
    //      elements - number of elements to cross-correlate.
    //                 Probably the result of a SymbolVector.size() call.
    //
    inline CrossCorrelation(unsigned int elements) : 
        _queued_element(1, 0, 0),
        _slice(sum_first_n_numbers(elements - 1))
    { }

    //  size_for
    //      Allocates room for the cross-correlations of "elements" elements.
    //      Resets the queue to the first element.
    //      Max N < about 65536 elements.
    //      elements - number of elements to cross-correlate.
    //                 Probably the result of a SymbolVector.size() call.
    //
    inline void size_for(unsigned int elements)
    {
        _queued_element.rc.row = 1;
        _queued_element.rc.col = 0;
        _queued_element.index  = 0;

        _slice.resize(sum_first_n_numbers(elements - 1));
    }
    
    //  size
    //      Get the size of the slice. Used for scaling progress bar.
    //
    inline const int size() const { return _slice.size(); }
    
    //  at
    //      Access an element of the slice.
    //      row - One indexed Row in the slice.
    //      col - Zero indexed Column in the slice.
    //      returns a reference to an element in the slice.
    //      Throws out_of_range if row and/or col add up 
    //      to something outside of the lower triangle
    //      of a symmetric matrix (below the diagonal).
    //
    CorrelationsRef at(const RowColPair& rc) throw (out_of_range)
    {
        if(_slice.empty()) 
            throw out_of_range("Empty slice! Get a new can...");

        // sanity check row and col...
        if(rc.col >= rc.row) 
            throw out_of_range("Column greater than or equal to Row -"
                               " in upper half.");
        if(0 == rc.row) throw out_of_range("Row starts with one.");

        // compute index
        unsigned int index = sum_first_n_numbers(rc.row - 1) + rc.col;
        
        // make sure index is within the slice.
        if (_slice.size() <= index) 
            throw out_of_range("Element outside of the slice.");
        
        // return the indexed element within the slice.
        return _slice[index];
    }

    //  at
    //      Directly access an element of the slice.
    //      index - element of the slice to dereference.
    //      returns a reference to an element in the slice.
    //      Throws out_of_range if index is past the end of the slice. 
    //
    CorrelationsRef at(const unsigned int index) throw (out_of_range)
    {
        // make sure index is within the slice.
        if (_slice.size() <= index) 
            throw out_of_range("Element outside of the slice.");
        
        // return the indexed element within the slice.
        return _slice[index];
    }

    //  Element
    //      Represent an element in the matrix of cross-correlations.
    //      Sequential access is more efficient than random access 
    //      (by a smidgen).
    //
    struct Element
    {
        RowColPair   rc;
        unsigned int index;
        
        inline Element() : rc(0, 0), index(0) { }
        inline Element(const unsigned int r,
                       const unsigned int c,
                       const unsigned int i) : rc(r, c), index(i) { }
        inline Element(const Element& e) : rc(e.rc), index(e.index) { }
        
        Element& operator=(const Element& e)
        {
            if(this != &e)
            {
                rc = e.rc;
                index = e.index;
            }
            return *this;
        }
    };

    //  get_next_element
    //      Set the _queued_element to point to the next correlation.
    //      e - return a copy of the _queued_element by reference.
    //      return false if past the end of the set of correlations.
    //
    bool get_next_element(Element& e)
    {
        boost::lock_guard<boost::mutex> lock(_queue_mutex);

        e = _queued_element;

        _queued_element.rc.col += 1;

        _queued_element.rc.col %= _queued_element.rc.row;
        
        if(0 == _queued_element.rc.col)
            _queued_element.rc.row += 1;
        
        _queued_element.index += 1;

        return (_slice.size() > e.index);
    }

    //  visit_element
    //      Do something with an element in the slice.
    //      e - Specify an element.
    //      v - Function object with operator() signature:
    //          void operator()(const RowColPair& rc, 
    //                          CorrelationsRef   corrs)
    //
    template< class Visitor >
    void visit_element(const Element& e, Visitor& v)
    {
        if (_slice.size() > e.index) v(e.rc, _slice[e.index]);
    }

    //  save_to
    //      Save to a text file using STL fstreams.
    //      filename - target file.
    //
    void save_to(const char * filename)
    {
        ::save_to(_slice, filename);
    }

    //  load_from
    //      Load from a text file using STL fstreams.
    //      filename - source file.
    //
    void load_from(const char * filename)
    {
        ::load_from(_slice, filename);
    }

protected:
    //  _slice
    //      Defines the base container. This is the slice.
    //
    vector< Correlations< Real >  > _slice;

    //  _queued_element
    //      Current queued visitor - used to drive a thread pool.
    //  
    Element _queued_element;

    //  _queue_mutex
    //      Used to lock access to the _queued_element.
    //
    boost::mutex _queue_mutex;

private:
    //  Do Not Copy (this is likely a large chunk of memory)
    //
    inline CrossCorrelation(const CrossCorrelation& c) { }
};

typedef CrossCorrelation< FloatType  > FloatCrossCorrelation;
typedef CrossCorrelation< DoubleType > DoubleCrossCorrelation;


//  CorrelatorN
//      Correlate two N-Day moving averages.
//
template<class Real, int N>
class CorrelatorN
{
public:
    //  NDay
    //      The residuals of the last N values of a set relative to it's mean.
    //
    typedef NDayType<Real, N>  NDay;

    //  Constructor
    //
    inline CorrelatorN() { }
    
    //  compute
    //      Compute the correlation between two moving averages.
    //      Ref. (Taylor) p. 180, equation (9.15).
    //      nd_one - The first moving average.
    //      nd_two - The second moving average.
    //      Returns the correlation coefficient between them
    //      or Real::InvalidValue (likely NAN) if either
    //      moving average is invalid.
    //
    Real compute(const NDayType< Real, N >& nd_one, const NDayType< Real, N >& nd_two)
    {
        if ( (Real::is_invalid(nd_one.root_mean_square)) || 
             (Real::is_invalid(nd_two.root_mean_square)) )
            return Real::invalid_value;

        // compute the product of the two standard deviations
        // This is the divisor.
        _temp_divisor = (nd_one.root_mean_square * nd_two.root_mean_square);

        //  Check for divide by zero.
        if (Real::Limits::min() > abs(_temp_divisor))
            return Real::invalid_value;

        // Compute the covariance between the moving averages.
        // This is the numerator.
        _temp_numerator = Real::invalid_value;

        for(int i = 0; i < N; i++)
        {
            //  Each residual is value - mean.
            //
            _temp_numerator += (nd_one.residual[i] * nd_two.residual[i]);
        }
        
        // The correlation coefficient is the ratio between the covariance
        // and the product of the standard deviations.
        return (_temp_numerator / _temp_divisor);
    }

protected:
    //  _temp
    //      Buffer for the calculation.
    //
    Real _temp_numerator;
    Real _temp_divisor;
    
    //  Do Not Copy
    //
    inline CorrelatorN(const CorrelatorN& c) { }
};


//  Correlator
//      Contain the logic for computing the correlations between
//      a couple of StatisticalDataTypes.
//      This code will be part of a ThreadMain.
//
template<class Real>
class Correlator
{
public:
    //  Constructor
    //
    inline Correlator() { }

    //  compute
    //      Compute the correlation between two sets of
    //      moving averages.
    //
    inline void compute(Correlations< Real >&          cs,
                        const StatisticalData< Real >& sd_one,
                        const StatisticalData< Real >& sd_two)
    {
        cs.ten_day = 
            ten_day_correlator.compute(sd_one.ten_day,
                                       sd_two.ten_day);

        cs.fifty_day = 
            fifty_day_correlator.compute(sd_one.fifty_day,
                                         sd_two.fifty_day);
    }
                 

protected:
    //  *_day_correlator
    //      Compute the correlations for an *-Day moving average.
    //
    CorrelatorN<Real, 10> ten_day_correlator;
    CorrelatorN<Real, 50> fifty_day_correlator;
    
    //  Do Not Copy
    //
    inline Correlator(const Correlator& c) { }
};

typedef Correlator< FloatType  > FloatCorrelator;
typedef Correlator< DoubleType > DoubleCorrelator;




#endif // CORRELATIONS_H

