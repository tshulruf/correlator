#ifndef SIGNALS_H
#define SIGNALS_H

#include "extended_container.h"
#include "date_index.h"
#include "numerictypes.h"
#include "container_io.h"
#include <vector>

using namespace std;


//  Signals
//      A signal is a collection of samples.
//      T - Type of value. Must have a static class function is_valid(),
//          used to determine whether to store the data away.
//
template<class T>
struct Signal
{
    //  Records
    //      Records in this system are date indexed values, 
    //      either floats (closing or opening values) or longs (volume).
    //
    typedef DateIndexedType<T>  RecordType;

    //  Recording
    //      Store a set of records.
    //
    typedef vector<RecordType> Recording;

    //  SampleSet
    //      The container type.
    //
    typedef vector<T> SampleSet;

    // sample
    //      This is the container.
    //      Using a vector so call looks like
    //          Signal<float> background;
    //          background.sample[index] = blah;
    //
    SampleSet sample;
    
    // Construct with a set of invalid values.
    //      Typically NAN for float values and -1 for longs.
    //      Default value is used to skip the entry on save.
    //
    inline Signal() : sample(DateIndex::interval()) { }
    
    //  Serialization
    //
    //  save_to
    //      Save to a text file using STL fstreams.
    //      Skips invalid samples.
    //      filename - target file.
    //
    void save_to(const char * filename)
    {
        Recording record;
        
        for(int i = 0; i < DateIndex::interval(); i++)
        {
            // Only add valid samples to the recording.
            
            if (T::is_valid(sample[i]))
                record.push_back(RecordType(i, sample[i]));
        }
        
        ::save_to(record, filename);
    }

    //  load_from
    //      Load from a text file using STL fstreams.
    //      filename - source file.
    //
    void load_from(const char * filename)
    {
        Recording record;
        ::load_from(record, filename);
        
        BOOST_FOREACH(RecordType r, record)
        {
            sample[r.index] = r.value;
        }
    }
};

typedef Signal<IntType>     IntSignal;
typedef Signal<LongType>    LongSignal;
typedef Signal<FloatType>   FloatSignal;
typedef Signal<DoubleType>  DoubleSignal;

#endif // SIGNALS_H
