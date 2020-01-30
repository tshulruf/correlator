#include <stdio.h>
#include <stdarg.h>
#include <fstream>
#include "../include/tickers.h"
#include "../include/symbols.h"
#include "../include/constants.h"
#include "../include/signals.h"
#include "../include/accumulator.h"
#include "../include/progress_bar.h"
#include <boost/lexical_cast.hpp>
#include <boost/filesystem.hpp>
#include <boost/thread.hpp>

using namespace std;

//  AccumulationEngine
// For one day: update 4*_all_symbols.size() accumulators.
//               If there's valid data for that day and symbol,
//                  Add that symbol to a list.
//               If the list is greater than one in length,
//                  Write out list and current set of 
//                  Statistical data from each accumulator.
//
class AccumulationEngine
{
public:
    //  load_data
    //      Load up all of the support data for precomputing the
    //      statistical data for a cross-correlation.
    //      Loads the main list of symbol descriptors.
    //      Loads all of the ticker files into signals.
    //      Chews up about 100Mb of RAM.
    //
    static bool load_data()
    {
        WorkingDirectory current_dir(constants::lists_path.base_path());

        ::puts("Loading symbol descriptors...");
        load_from(_symbol, "SymbolDescriptors.txt");
        
        // Probably didn't run getdata first...
        //
        if (_symbol.size() == 0)
        {
            ::puts("Error loading symbol descriptors! Run getdata first.");
            return false;
        }
        
        current_dir.chdir(constants::data_path.base_path());
        
        _progress_bar.reset("Loading _all_ of the ticks...", _symbol.size());

        BOOST_FOREACH(SymbolDescriptor sd, _symbol)
        {
            TickerSignal temp;
            temp.load_from(sd.DATFile.c_str());
            _ticker.push_back(temp);

            _progress_bar.increment();
        }

        ::puts("\nLoading backgrounds.");
        _bdc.load_from("bkg_delta_close.dat");
        _bdac.load_from("bkg_delta_adjclose.dat");

        // Allocate room for the means and the moving averages.
        //
        _mdc.resize(_ticker.size());
        _mdac.resize(_ticker.size());
        _mdcb.resize(_ticker.size());
        _mdacb.resize(_ticker.size());

        _adc.resize(_ticker.size());
        _adac.resize(_ticker.size());
        _adcb.resize(_ticker.size());
        _adacb.resize(_ticker.size());

        // Every symbol should have a ticker.
        //
        return _ticker.size() == _symbol.size();
    }

    //  initialize_engine
    //      Reset the progress bar and use it as a counter.
    //
    inline static void initialize_engine() 
    {
        _progress_bar.reset("Pre-processing data...", DateIndex::last());
    }
    
    //  done
    //      Return true if the counter has reached the last date.
    //
    inline static bool done()
    {
        return (_progress_bar.count() >= DateIndex::last());
    }
    
    //  process_a_date
    //      Compute the statistical data for a day and store it.
    //
    static void process_a_date()
    {
        int date = _progress_bar.count();
        
        // make sure there's something to compute...
        int got_data_count = 0;
        for (int i = 0; i < _symbol.size(); ++i)
        {
            if (FloatType::is_valid(_ticker[i].sample[date].Close))
                ++got_data_count;

            if (1 < got_data_count) // need at least two to correlate.
                break;
        }

        if (1 < got_data_count)
        {
            _dates.push_back(date);
            
            //  Limit the queue size to 50 days.
            //
            if (50 < _dates.size())
            {
                _dates.pop_front();
            }

            reset_semaphore();

            // Construct some threads!
            boost::thread_group workers;

            for (int i = 0; i < boost::thread::hardware_concurrency(); i++)
                workers.create_thread(AccumulationCylinder(date));

            workers.join_all();

            write_out_data(date);
        }

        _progress_bar.increment();
    }
    
protected:
    //  reset_semaphore
    //      Set up the semaphore to loop through the symbols.
    //
    static void reset_semaphore()
    {
        static DoNothing slack; // only need one of these...
        EngineSemaphore::reset(slack);
        
        // start at -1, first thread incremenets... <hack />
        EngineSemaphore::decrement(slack);
    }

    //  write_out_data
    //      Write out the data. Builds five files, one record at a time.
    //
    static void write_out_data(int date)
    {
        // make sure there's something to write...
        {
            int got_data_count = 0;
            for (int i = 0; i < _symbol.size(); ++i)
            {
                if (FloatType::is_valid(_mdc[i].value)         &&
                    FloatType::is_valid(_mdc[i].fifty_day.mean))
                    ++got_data_count;

                if (1 < got_data_count) // need at least two to correlate.
                    break;
            }
            if (1 >= got_data_count) return; // nothing to do.
        }

        string sdate(boost::lexical_cast< string >(date));
        ofstream of_symbols(constants::lists_path(sdate.c_str()));

        string means_dir(constants::means_path(sdate.c_str()));
        boost::filesystem::create_directory(means_dir);
        means_dir += '/';

        ofstream of_mdc(
            string(means_dir + constants::deltaclose).c_str(),
            ios_base::binary);

        ofstream of_mdac(
            string(means_dir + constants::deltaadjclose).c_str(),
            ios_base::binary);

        ofstream of_mdcb(
            string(means_dir + constants::deltaclosenobkg).c_str(),
            ios_base::binary);

        ofstream of_mdacb(
            string(means_dir + constants::deltaadjclosenobkg).c_str(),
            ios_base::binary);

        for (int i = 0; i < _symbol.size(); ++i)
        {
            if (FloatType::is_valid(_mdc[i].value)         &&
                FloatType::is_valid(_mdc[i].fifty_day.mean))
            {
                of_symbols << _symbol[i].Symbol << endl;
                of_mdc     << _mdc[i];
                of_mdac    << _mdac[i];
                of_mdcb    << _mdcb[i];
                of_mdacb   << _mdacb[i];
            }
        }
        
        ofstream of_dates(string(means_dir + "dates").c_str());
        of_dates << "Directory represents data from "
                 << DateIndex::to_string(_dates.front())
                 << " to "
                 << DateIndex::to_string(_dates.back())
                 << endl;
    }

    //  _progress_bar
    //      Give the user a little feedback...
    //
    static ProgressBar           _progress_bar;

    //  _symbol
    //      The master list of symbol descriptors.
    //
    static SymbolDescriptorDeque _symbol;

    //  _ticker
    //      All of the historical data for the symbols, date indexed.
    //
    static TickerSignalDeque     _ticker;
    
    static list<int>             _dates;
    
    //  _m*
    //      Statistical data for each of the symbols.
    //
    static FloatStatisticalDeque _mdc;
    static FloatStatisticalDeque _mdac;
    static FloatStatisticalDeque _mdcb;
    static FloatStatisticalDeque _mdacb;
    
    //  _a*
    //      Moving averages for each of the symbols.
    //
    static FloatAccumulatorDeque _adc;
    static FloatAccumulatorDeque _adac;
    static FloatAccumulatorDeque _adcb;
    static FloatAccumulatorDeque _adacb;
    
    //  _b*
    //      Backgrounds.
    //
    static FloatSignal _bdc;
    static FloatSignal _bdac;

    //  EngineSemaphore
    //      Loop through all of the symbols using an index.
    //
    class AccumulationCylinder;

    typedef Semaphore< DoNothing,
                       AccumulationCylinder,
                       DoNothing > EngineSemaphore;
    friend class Semaphore< DoNothing,
                            AccumulationCylinder,
                            DoNothing >;

    //  AccumulationCylinder
    //      Process a chunk of data.
    //
    class AccumulationCylinder
    {
    public:
        //  Constructor
        //
        AccumulationCylinder(int date) : _isymbol(0), _idate(date) { }

        //  operator()()
        //      Thread Main. While there's something to update,
        //      Increment the count and update the one you got.
        //
        void operator()()
        {
            bool done = false;
            while(!done)
            {
                EngineSemaphore::increment(*this);

                if(AccumulationEngine::_symbol.size() > _isymbol)
                {
                    // Not all symbols trade every day, but only here
                    // if some symbols traded on this day.
if(FloatType::is_invalid(
    AccumulationEngine::_ticker[_isymbol].sample[_idate].Close))
{
    AccumulationEngine::_ticker[_isymbol].sample[_idate].DeltaClose = 0.0;
    AccumulationEngine::_ticker[_isymbol].sample[_idate].DeltaAdjClose = 0.0;
}

AccumulationEngine::_adc[_isymbol].update(
    AccumulationEngine::_ticker[_isymbol].sample[_idate].DeltaClose,
    AccumulationEngine::_mdc[_isymbol]);

AccumulationEngine::_adac[_isymbol].update(
    AccumulationEngine::_ticker[_isymbol].sample[_idate].DeltaAdjClose,
    AccumulationEngine::_mdac[_isymbol]);

AccumulationEngine::_adcb[_isymbol].update(
    AccumulationEngine::_ticker[_isymbol].sample[_idate].DeltaClose - 
        AccumulationEngine::_bdc.sample[_idate],
    AccumulationEngine::_mdcb[_isymbol]);

AccumulationEngine::_adacb[_isymbol].update(
    AccumulationEngine::_ticker[_isymbol].sample[_idate].DeltaAdjClose - 
        AccumulationEngine::_bdac.sample[_idate],
    AccumulationEngine::_mdacb[_isymbol]);
                }
                else
                    done = true;

                boost::this_thread::yield();
            }
        }

    protected:
        friend class Semaphore< DoNothing,
                                AccumulationCylinder,
                                DoNothing >;

        //  operator()(int)
        //      Pick an accumulator to update.
        //
        void operator()(int i)
        {
            _isymbol = i;
        }
        
        //  _isymbol
        //      Symbol index.
        //
        int _isymbol;
        
        //  _idate
        //      Current date index.
        //
        int _idate;
    };
    friend class AccumulationCylinder;
};

ProgressBar           AccumulationEngine::_progress_bar;
SymbolDescriptorDeque AccumulationEngine::_symbol;
TickerSignalDeque     AccumulationEngine::_ticker;
list<int>             AccumulationEngine::_dates;
FloatStatisticalDeque AccumulationEngine::_mdc;
FloatStatisticalDeque AccumulationEngine::_mdac;
FloatStatisticalDeque AccumulationEngine::_mdcb;
FloatStatisticalDeque AccumulationEngine::_mdacb;
FloatAccumulatorDeque AccumulationEngine::_adc;
FloatAccumulatorDeque AccumulationEngine::_adac;
FloatAccumulatorDeque AccumulationEngine::_adcb;
FloatAccumulatorDeque AccumulationEngine::_adacb;
FloatSignal           AccumulationEngine::_bdc;
FloatSignal           AccumulationEngine::_bdac;



// main
//      Use this program to pre-process a pile of historical
//      stock data.
//
int main(int argc, char * argv[])
{
    // Pre-compute a pile of statistics around the historical
    // stock data.
    
    // Load symbol descriptor set and all of the ticks.
    //
    if (!AccumulationEngine::load_data()) return 0;
    
    // For each day: update 4*_all_symbols.size() accumulators.
    //               If there's valid data for that day and symbol,
    //                  Add that symbol to a list.
    //               If the list is greater than one in length,
    //                  Write out list and current set of 
    //                  Statistical data from each accumulator.
    //
    AccumulationEngine::initialize_engine();
    while(!AccumulationEngine::done())
        AccumulationEngine::process_a_date();
    
    return 0;
}

