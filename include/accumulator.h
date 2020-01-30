#ifndef ACCUMULATOR_H
#define ACCUMULATOR_H

// #include "extended_list.h"
#include "numerictypes.h"
#include "source_data.h"
// #include <stdexcept>
#include <deque>
// #include <vector>
#include <math.h>

using namespace std;

//  MovingAverageN
//      Contain and efficiently compute an N-Element moving average.
//      The set of residuals and the squares of the residuals are
//      stored for easy computation of variance and correlation.
//
template<class Real, int N>
class MovingAverageN
{
public:
    //  NDay
    //      The residuals of the last N values of a set relative to it's mean.
    //
    typedef NDayType<Real, N>  NDay;
    
    //  RollingDataSet
    //      The last N values of a set.
    //
    typedef deque<Real> RollingDataSet;
    
    //  Constructor
    //
    inline MovingAverageN()
    {
        if(0 != N) _portion = Real(1) / Real(N);
    }
    
    //  initialized
    //      Return true if there are N samples in the rolling accumulator.
    //
    inline const bool& initialized() const { return (N == _running_value.size()); }

    //  update
    //      Add a new value into the rolling accumulator.
    //
    inline void update(const Real& new_value, NDay& _nday)
    {
        //  Push the new value to the end of the queue
        //  and update the mean.
        //  
        _mean += (new_value * _portion);
        _running_value.push_back(new_value);
        
        //  Limit the queue size to N elements.
        //
        if (N < _running_value.size())
        {
            //  Pull the oldest value's portion from the mean
            //  and remove it from the set.
            //
            _mean -= (_running_value.front() * _portion);
            _running_value.pop_front();
        }
        
        //  Compute the residuals and the root mean square of the residuals
        //  using the new mean and the current set.
        if (N == _running_value.size())
        {
            _nday.root_mean_square = Real::invalid_value;
            _nday.mean = _mean;

            for(int i = 0; i < N; i++)
            {
                //  Each residual is value - mean.
                //
                _nday.residual[i] = (_running_value[i] - _nday.mean);
                
                //  The root mean square of the residuals is the square root of
                //  the sum of the squares of the residuals.
                _nday.root_mean_square += (_nday.residual[i] * _nday.residual[i]);
            }

            _nday.root_mean_square = sqrt(_nday.root_mean_square);
        }
    }
    
    //  reset
    //      Resets to completely uninitialized.
    //
    inline void reset()
    {
        _running_value.clear();
        _mean = Real::invalid_value;
    }

protected:
    //  _running_values
    //      This is the set of N elements.
    //
    RollingDataSet _running_value;

    //  _portion
    //      1/N - used to minimize division.
    //
    Real _portion;

    //  _mean
    //      Mean of the N elements.
    //
    Real _mean;
};


//  MovingAverages
//      Calculate both 10- and 50-day moving averages for
//      a sequence of data.
//
template<class Real>
class MovingAverages
{
public:
    //  update
    //      Add a new value into the rolling accumulators.
    //
    inline void update(const Real& new_value, StatisticalData<Real>& sdata)
    {
        sdata.value = new_value;

        if(Real::is_valid(new_value))
        {
            _ten.update(new_value, sdata.ten_day);
            _fifty.update(new_value, sdata.fifty_day);
        }
    }

    //  reset
    //      Resets to completely uninitialized.
    //
    inline void reset()
    {
        _ten.reset();
        _fifty.reset();
    }

protected:
    //  ten_day_collector
    //      Tool to compute a 10-day moving average.
    //
    MovingAverageN<Real, 10> _ten;

    //  fifty_day_collector
    //      Tool to compute a 50-day moving average.
    //
    MovingAverageN<Real, 50> _fifty;
    
};

typedef MovingAverages<FloatType>  FloatMovingAverages;
typedef MovingAverages<DoubleType> DoubleMovingAverages;

typedef ExtendedContainer< FloatMovingAverages,
                           deque< FloatMovingAverages > > 
                         FloatAccumulatorDeque;
typedef ExtendedContainer< DoubleMovingAverages,
                           deque< DoubleMovingAverages > > 
                         DoubleAccumulatorDeque;


#endif // ACCUMULATOR_H

