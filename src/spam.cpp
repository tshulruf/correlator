#include "../include/accumulator.h"
#include "../include/date_index.h"
#include "../include/constants.h"
#include "../include/tickers.h"
#include "../include/symbols.h"
#include "../include/source_data.h"
#include "../include/numerictypes.h"
#include "../include/progress_bar.h"

#include <stdio.h>

using namespace std;

//  Backgrounder
//      Handle the background collection and application to the ticks.
//
class Backgrounder
{
public:
    static void load_bkg()
    {
        bkg.load_from("background.dat");
    }

    //  operator()
    //      Load up the ticks for a Symbol and apply the background to its close
    //      values. Used in a foreach loop.
    //      sd - Current symbol for processing.
    //
    inline void operator()(const SymbolDescriptor& sd)
    {
        static TickerSet ticks;
        ticks.clear();
        load_from(ticks, sd.DATFile.c_str());

        BOOST_FOREACH(Ticker& t, ticks)
        {
            t.value.apply_inv_delta(bkg.sample[t.index]);
        }

        save_to(ticks, sd.DATFile.c_str());
    }

protected:
    static FloatSignal bkg;

};
FloatSignal Backgrounder::bkg;


int main (int argc, char * argv[])
{
    // Change directories into the lists directory.
    ::puts(constants::lists_path.base_path());
    WorkingDirectory current_dir(constants::lists_path.base_path());

    // Load up the list of symbols.
    SymbolDescriptorSet the_tickers;
    load_from(the_tickers, "SymbolDescriptors.txt");

    // Change directories into the data directory.
    ::puts(constants::data_path.base_path());
    current_dir.chdir(constants::data_path.base_path());

    ::puts("Loading background...");
    Backgrounder::load_bkg();
    
    Backgrounder remove_background;
    the_tickers.foreach(remove_background);

    return 0;
}
