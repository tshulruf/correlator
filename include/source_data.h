#ifndef SOURCE_DATA_H
#define SOURCE_DATA_H

#include "constants.h"
#include "numerictypes.h"
#include "signals.h"

using namespace std;

//  NDayType
//      Contain the set of residuals and the root mean square
//      of the residuals.
//      Real    - some numeric type.
//      N       - the number of residuals.
//
template<class Real, int N>
struct NDayType
{
    //  Size
    //      Used to determine offsets into data files.
    //      Returns the size of the data structure.
    //
    inline const static int Size()
    {
        return( (N + 2) * sizeof(Real) );
    }

    //  mean
    //      N-Day moving average.
    Real mean;
    
    //  value
    //      The set of N residuals.
    //
    vector<Real> residual;
    
    //  root_mean_square
    //      The root mean square of the residuals.
    //
    Real root_mean_square;
    
    //  Default Constructor
    //      Construct with invalid values.
    //
    inline NDayType() : 
        mean(Real::invalid_value),
        residual(N, Real::invalid_value),
        root_mean_square(Real::invalid_value)
    { }
    
    //  Copy Constructor
    //      r - copied Residuals.
    //
    inline NDayType(const NDayType& r) : 
        mean(r.mean),
        residual(r.residual),
        root_mean_square(r.root_mean_square)
    { }

    //  clear
    //      Assign invalid values to the containers.
    //
    inline void clear()
    {
        residual.assign(N, Real::invalid_value);
        root_mean_square = mean = Real::invalid_value;
    }
};

//  Stream I/O operators.
//  operator<<
//      out - output text stream.
//      nd  - NDayType to stream.
//      returns - stream for continued use.
//
template<class Real, int N>
ostream& operator<< (ostream& out, const NDayType<Real, N>& nd)
{
    if (constants::save_as_binary)
    {
        out.write((char *)(&nd.mean), sizeof(Real));
        for(int i = 0; i < N; i++)
            out.write((char *)(&nd.residual[i]), sizeof(Real));
        out.write((char *)(&nd.root_mean_square), sizeof(Real));
    }
    else
    {
        out << nd.mean;
        for(int i = 0; i < N; i++)
            out << nd.residual[i];
        out << nd.root_mean_square << endl;
    }
    return out;
}

//  operator>>
//      in - input stream.
//      nd - NDayType to stream.
//      returns - stream for continued use.
//
template<class Real, int N>
istream& operator>> (istream& in, NDayType<Real, N>& nd)
{
    if (constants::save_as_binary)
    {
        in.read((char *)(&nd.mean), sizeof(Real));
        for(int i = 0; i < N; i++)
            in.read((char *)(&nd.residual[i]), sizeof(Real));
        in.read((char *)(&nd.root_mean_square), sizeof(Real));
    }
    else
    {
        in >> nd.mean;
        for(int i = 0; i < N; i++)
            in >> nd.residual[i];
        in >> nd.root_mean_square;
    }    
    return in;
}


//  StatisticalData
//      Contain the 10- and 50-day moving averages.
//      Real - some numeric type.
//
template<class Real>
struct StatisticalData
{
    typedef NDayType<Real, 10> TendMAType;
    typedef NDayType<Real, 50> FiftydMAType;

    //  Size
    //      Used to determine offsets into data files.
    //      Returns the size of the data structure.
    //
    inline const static int Size()
    {
        return( sizeof(Real) + TendMAType::Size() + FiftydMAType::Size() );
    }

    //  value
    //      Current value for comparison (among other things).
    //
    Real value;
    
    //  ten_day
    //      10-day moving average data.
    //
    TendMAType ten_day;

    //  fifty_day
    //      50-day moving average data.
    //
    FiftydMAType fifty_day;

    //  Default Constructor
    //
    inline StatisticalData() { }

    //  Construct by parts.
    //      ten     - ten day data.
    //      fifty   - fifty day data
    //
    inline StatisticalData(const Real&         value,
                           const TendMAType&   ten,
                           const FiftydMAType& fifty) : 
        value(value),
        ten_day(ten),
        fifty_day(fifty)
    { }

    //  Copy Constructor
    //      r - copied data.
    //
    inline StatisticalData(const StatisticalData& r) : 
        value(r.value),
        ten_day(r.ten_day),
        fifty_day(r.fifty_day)
    { }

    //  is_valid
    //      Return true always. Makes a set of data that
    //      matches the ticks in length and indexing.
    //      Used to make a Signal of statistical data.
    //
    static bool is_valid(const StatisticalData& sd) 
    {
        return (Real::is_valid(sd.value));
    }
};

//  Stream I/O operators.
//  operator<<
//      out - output text stream.
//      sd  - StatisticalData to stream.
//      returns - stream for continued use.
//
template<class Real>
ostream& operator<< (ostream& out, const StatisticalData<Real>& sd)
{
    if (constants::save_as_binary)
        out.write((char *)(&sd.value), sizeof(Real));
    else
        out << sd.value;

    out << sd.fifty_day
        << sd.ten_day;

    return out;
}

//  operator>>
//      in - input stream.
//      sd - StatisticalData to stream.
//      returns - stream for continued use.
//
template<class Real>
istream& operator>> (istream& in, StatisticalData<Real>& sd)
{
    if (constants::save_as_binary)
        in.read((char *)(&sd.value), sizeof(Real));
    else
        in >> sd.value;

    in >> sd.fifty_day
       >> sd.ten_day;

    return in;
}

typedef StatisticalData<FloatType>  FloatStatisticalData;
typedef StatisticalData<DoubleType> DoubleStatisticalData;

//  Keep a set of statistical data, one for each symbol.
//
typedef ExtendedContainer< FloatStatisticalData,
                           deque< FloatStatisticalData > >
                         FloatStatisticalDeque;
typedef ExtendedContainer< DoubleStatisticalData,
                           deque< DoubleStatisticalData > >
                         DoubleStatisticalDeque;


//  Date Indexed Float Statistical Data
typedef DateIndexedType<FloatStatisticalData>  IndexedFloatStatisticalData;
typedef DateIndexedType<DoubleStatisticalData> IndexedDoubleStatisticalData;

typedef Signal<FloatStatisticalData>  FloatStatisticalSignal;
typedef Signal<DoubleStatisticalData> DoubleStatisticalSignal;

typedef BufferedRecordReader< IndexedFloatStatisticalData > BufferedSDReader;


#endif // SOURCE_DATA_H

