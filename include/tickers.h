#ifndef TICKERS_H
#define TICKERS_H

#include <string>
#include "extended_container.h"
#include "date_index.h"
#include "source_data.h"
#include "container_io.h"

using namespace std;

// Tick
//      Contain a set of values 
//
struct Tick
{
// From the Yahoo! Finance CSV file header
// Date,Open,High,Low,Close,Volume,Adj Close
// (Only interested in Adj Close.)
// (date is aggregated below...)
// Additional data
    // Closing values corrected for background trading energy.
    // CloseNoBkg(n) = round(Close(n) * DeltaBkg(n))
    //
    LongType CloseNoBkg;
    
    // Default Constructor (NaN's (and one -1) around).
    //
    inline Tick() { }

    // Construct from components
    //
    inline Tick(long c) : CloseNoBkg(c) { }
    
    // Copy Constructor
    //
    inline Tick(const Tick& t) : CloseNoBkg(t.CloseNoBkg) { }
    
    //  operator=
    //      Assignment operator.
    //
    Tick& operator=(const Tick& t)
    {
        if(this != &t) CloseNoBkg = t.CloseNoBkg;

        return *this;
    }
    
    // inverse_delta_close
    //      Compute the Inverse Delta Close relative to some other tick 
    //      (likely the previous one).
    //      Delta(n) = Value(n-1) / Value(n)
    //      Used to construct the background deltas.
    //      t - Previous ticker (n-1).
    //      Returns inverse delta close or invalid value.
    //
    inline FloatType inverse_delta_close(const Tick& t)
    {
        if(LongType::is_valid(t.CloseNoBkg) && 
          (LongType::is_valid(CloseNoBkg))  &&
          (0 != CloseNoBkg)                 )
        {
            // Note: Close is stored in hundreths. The 100's cancel.
            //
            return( float(t.CloseNoBkg)/ float(CloseNoBkg) );
        }
        return FloatType::invalid_value;
    }

    //  apply_delta
    //      Scale the close by the average change from the previous closes
    //      inv_delta - scaling factor = average (previous close / close)
    //
    inline void apply_inv_delta(float inv_delta)
    {
        if(FloatType::is_invalid(inv_delta))
            CloseNoBkg = LongType::invalid_value;

        if(LongType::is_valid(CloseNoBkg))
        {
            long corrected_close = 
                float_type_to_int_type<float, long>(
                    float(CloseNoBkg) * inv_delta);
            
            // Avoid overflow...
            if(LongType::is_valid(corrected_close))
                CloseNoBkg = corrected_close;
        }
    }

    //  is_valid
    //      Used to make a Signal of Ticks.
    //
    static bool is_valid(const Tick& t) 
    {
        return (LongType::is_valid(t.CloseNoBkg));
    }
};

//  Stream I/O operators.
//  operator<<
//      out - output text stream.
//      t   - Tick to stream.
//      returns - stream for continued use.
//
ostream& operator<< (ostream& out, const Tick& t)
{
    if (constants::save_as_binary)
        out.write((char *)(&t), sizeof(Tick));
    else
        out << t.CloseNoBkg;

    return out;
}

//  operator>>
//      in - input stream.
//      t  - Tick to stream.
//      returns - stream for continued use.
//
istream& operator>> (istream& in, Tick& t)
{
    if (constants::save_as_binary)
        in.read((char *)(&t), sizeof(Tick));
    else
        in >> t.CloseNoBkg;
       
    return in;
}


// Ticker
//   Each set of values is indexed by date. The point in time is days from a
//   start date.
//
typedef DateIndexedType<Tick> Ticker;


// TickerSet
//   Contain a set of tickers.
//
typedef ExtendedContainer< Ticker, deque< Ticker > > TickerSet;


//  BufferedTickerReader
//      Read a particular tick out of a file.
//
typedef BufferedRecordReader< Ticker > BufferedTickerReader;


//  TickerSignal
//      Contain a date-indexed set of ticker data.
//
typedef Signal< Tick > TickerSignal;

//  TickerSignalDeque
//      Contain _all_ of the tickers, date indexed.
//
typedef ExtendedContainer<TickerSignal, 
                          deque< TickerSignal > > TickerSignalDeque;

#endif // TICKERS_H
