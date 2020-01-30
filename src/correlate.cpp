#include "../include/constants.h"
#include "../include/correlations.h"
#include "../include/progress_bar.h"
#include <boost/lexical_cast.hpp>
#include <boost/filesystem.hpp>


using namespace std;


//  CorrelationsVisitor
//      This represents a visitor on a particular element of
//      the massive cross-correlations matrix.
//      It will correlate a pair of statistical data elements.
//
class CorrelationsVisitor
{
protected:
    //  Statistical Data Storage
    //
    //  mean
    //      Statistical data for a particular day.
    //
    static FloatStatisticalDeque _mean;

public:
    //  load_statistical_data
    //      Load up a day's worth of statistical data.
    //      This will handle the directory traversal.
    //      date - day to look up in all of the symbol directories.
    //
    static bool load_statistical_data(const DateIndex::IndexType date)
    {
        cout << "Loading data for day " << date << "..." << endl;

        // Change directories into the lists directory.
        WorkingDirectory current_dir(constants::means_path.base_path());
        string filename = boost::lexical_cast<string>(date);
        filename += '/';
        filename += constants::corellating;
        
        if(boost::filesystem::exists(filename))
        {
            load_from(_mean, filename.c_str());
            return true;
        }
        else
            return false;
    }

    //  size
    //      Return the number of elements in the
    //      statistical data set for a particular day.
    //      Used to resize the cross correlations set.
    //
    static unsigned int size() { return _mean.size(); }

    //  Cross Correlation
    //
    //  correlator
    //      Memory and functionality used to correlate two elements.
    //
    FloatCorrelator correlator;
    
    //  operator()
    //      Cross correlate a pair of means. Called by ThreadMain.
    //      row, col - indexes into the means. Represent symbols.
    //      corrs    - output correlations.
    //
    void operator()(const RowColPair& rc,
                    FloatCrossCorrelation::CorrelationsRef corrs)
    {
        if((_mean.size() > rc.row) && (_mean.size() > rc.col))
            correlator.compute(corrs, _mean[rc.row], _mean[rc.col]);
    }
    
};
FloatStatisticalDeque CorrelationsVisitor::_mean;


//  CorrelationsThread
//      Contain the cross-correlations set and act as thread main.
//
class CorrelationsThread
{
protected:
    //  _correlation
    //      This is a day's slice.
    //
    static FloatCrossCorrelation _correlation;
    
    //  _progress_bar
    //      Give the user a little feedback...
    //
    static ProgressBar _progress_bar;

    //  _date
    //      Current date being corellated.
    //
    static DateIndex::IndexType _date;
    
public:
    //  set_size
    //      Set up the slice to hold a day's worth of
    //      cross-correlations.
    static bool initialize_day(const DateIndex::IndexType date)
    {
        _date = date;

        bool data_loaded = CorrelationsVisitor::load_statistical_data(date);

        if (data_loaded)
        {
            _correlation.size_for(CorrelationsVisitor::size());

            string banner = "Cross corellating day ";
            banner += boost::lexical_cast<string>(_date);
            banner += ". Might take a while...";

            _progress_bar.reset(banner.c_str(), _correlation.size());
        }
        else
        {
            cout << "Skipping day " << date << " - no data." << endl;
        }

        return data_loaded;
    }
    
    //  save_results
    //      Save the results of a cross-correlation to disk.
    //      date - specify the date index for an easy file name.
    //
    static void save_results()
    {
        // Change directories into the correlations directory.
        WorkingDirectory current_dir(constants::correlations_path.base_path());

        string sdate = boost::lexical_cast<string>(_date);

        cout << "\nSaving cross correlations to " 
             << constants::correlations_path.base_path() 
             << '/' << sdate << '.' << endl;
        _correlation.save_to(sdate.c_str());
    }
    
    //  opertator()
    //      This is thread main. Get an element to visit.
    //      Corellate the element (it's a pair of statistical
    //      data structures). If the element corellates over
    //      50 days, add it to the set of corellated elements.
    //      Increment the progress bar.
    //
    void operator()()
    {
        CorrelationsVisitor v; // is for Victory! Vandetta!
                               // And creepy snake aliens!

        FloatCrossCorrelation::Element visited;

        while(_correlation.get_next_element(visited))
        {
            _correlation.visit_element(visited, v);
            _progress_bar.increment();
            boost::this_thread::yield();
        }
    }
};
FloatCrossCorrelation CorrelationsThread::_correlation;
ProgressBar           CorrelationsThread::_progress_bar;
DateIndex::IndexType  CorrelationsThread::_date;


//  main
//      Main function that'll do a bunch of correlation.
//      argc - argument count (ignored)
//      argb - arguments      (ignored)
//
int main (int argc, char * argv[])
{
    for (DateIndex::IndexType idate = DateIndex::first();
         DateIndex::last() >= idate;
         ++idate)
    {
        if(CorrelationsThread::initialize_day(idate))
        {
            boost::thread_group workers;
            for (int i = 0; i < boost::thread::hardware_concurrency(); i++)
                workers.create_thread(CorrelationsThread());
            workers.join_all();

            CorrelationsThread::save_results();
        }
    }
    
    return 0;
}
