#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <string>
#include <sstream>
#include "../include/constants.h"
#include "../include/tickers.h"
#include "../include/symbols.h"
#include "../include/parsers.h"
#include "../include/signals.h"
#include <boost/filesystem.hpp>


using namespace std;
using namespace boost::gregorian;

//  Backgrounder
//      Handle the background collection and application to the ticks.
//
class Backgrounder
{
public:
    static void update_bkg(DateIndex::IndexType i,
                           float                datum)
    {
        bkg.sample[i] += datum;
        count.sample[i] += 1;
    }

    //  record_backgrounds
    //      Compute the mean background relative change for each date in the
    //      interval and store the signals.
    //
    static void record_backgrounds()
    {
        // Compute mean background values.
        //
        for(int i = 0; i < DateIndex::interval(); i++)
            if (0 < count.sample[i]) 
                bkg.sample[i] /= count.sample[i];
        
        // Write to current directory.
        //
        bkg.save_to("background.dat");
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
    static FloatSignal count;
    static FloatSignal bkg;

};
FloatSignal Backgrounder::count;
FloatSignal Backgrounder::bkg;

// YahooCSVParser
//      Parse a Yahoo! CSV line and add it to a set.
//      Might throw a bad_lexical_cast or something...
//
class YahooCSVParser
{
public:
    //  operator()
    //      Parses a line of a Yahoo! CSV file.
    //      Also constructs a background of relative changes.
    //
    void operator()(const stringvector& elements, TickerSet& ts)
    {
        if( elements.size() < 7) return;
    
        static SafeLexicalCast<float, string> to_float(0.0);
        static SafeLexicalCast<long, string> to_long(-1);

        // 0 = Date      - parse into DateIndex
        // 1 = Open      - parse into float
        // 2 = High      - parse into float
        // 3 = Low       - parse into float
        // 4 = Close     - parse into float
        // 5 = Volume    - parse into long
        // 6 = Adj Close - parse into float

        DateIndex::IndexType i = DateIndex::from_string(elements[0]);
        FloatType fclose = to_float.cast(elements[6]) * 100.0;

        // construct a Ticker
        Ticker t(
            i,                                                  // date index.
            Tick(float_type_to_int_type<float, long>(fclose))); // adj close
                                                                // (pennies)

        // and push it onto the front of a list.
        ts.push_front(t);

        // If there's a previous element, diff it with this one.
        if (1 < ts.size())
        {
            Backgrounder::update_bkg(i,
                ts[0].value.inverse_delta_close(ts[1].value));
        }

    }
    
protected:
};

//  Snarf
//      Wrap up some calls to shell scripts that
//      in turn call snarf.
//      Implemented as a function object to optimize
//      construction of commandline:
// sh ../bin/snarf-YF-table.sh MSFT 08 22 2010 08 22 2011
//      using a couple of constants.
//
class Snarf
{
public:
    // Constructor
    //      Prepares the string construction...
    //
    inline Snarf() { }

    // Copy Constructor
    //      BOOST_FOREACH will make a copy of this class
    //      for each element of the Symbol set.
    //
    inline Snarf(const Snarf& sayft) {}

    // operator()
    //      Use one of these like a function in a BOOST_FOREACH loop.
    //      Will WGet a Yahoo! Finance Historical Data CSV file for
    //      a particluar Ticker Symbol, pre-process it into a DAT file
    //      extracting a background of mean relative changes, and
    //      delete the downloaded CSV file.
    //      sd - SymbolDescriptor to act on.
    //
    inline void operator()(const SymbolDescriptor& sd)
    {
        // buffer...
        static string command;
        command.clear();

        command += s_command_base;
        command += sd.Symbol;
        command += s_dates;
       
        // debugging...
        // ::puts(command.c_str());

        ::system(command.c_str());
        
        // Might consider spinning off a thread to do the preprocessing while
        // the next file is downloaded...
        // join previous thread.
        // adjust thread parameters
        // and spin it off while downloading the next one...
        
        // begin should be threaded stuff

        // if a file was downloaded...
        if(boost::filesystem::exists(sd.CSVFile))
        {
            if (boost::filesystem::is_empty(sd.CSVFile))
            {
                // delete it if empty
                boost::filesystem::remove(sd.CSVFile);
            }
            else
            {
                // preprocess it into a ticker file and save it.
                static TickerSet ticks;
                ticks.clear();

                FileParser(sd.CSVFile.c_str(),
                           ",").load_using(Snarf::yahoo_csv, ticks);
                boost::filesystem::create_directory(sd.Symbol);
                save_to(ticks, sd.DATFile.c_str());
                boost::filesystem::remove(sd.CSVFile);
                
                // Make a date index map to the data...
                static IntSignal datemap;
                BOOST_FOREACH(IntType& i, datemap.sample)
                {
                    i = IntType::invalid_value;
                }
                
                for(int j = 0; j < ticks.size(); j++)
                {
                    datemap.sample[ticks[j].index] = j;
                }
                
                static string datemapfilename;
                datemapfilename.clear();
                datemapfilename = sd.Symbol;
                datemapfilename += "/index";
                
                datemap.save_to(datemapfilename.c_str());
            }
        }
        
        // end should be threaded stuff.
    }
    
    //  set_start_and_end
    //      Construct a string representing the start and end dates
    //      Note that months are 0 indexed... this is 22 Sept. 2010 --> 22 Sept. 2011
    //      sh ../bin/snarf-YF-table.sh MSFT 08 22 2010 08 22 2011
    //      begin   - starting date (22 Sept. 2010)
    //      end     - ending date   (22 Sept. 2011)
    //
    static inline void set_start_and_end(const date& begin, const date& end)
    {
        stringstream sdates;
        sdates << ' ' << (begin.month() - 1)  
               << ' ' << begin.day() 
               << ' ' << begin.year()
               << ' ' << (end.month() - 1)  
               << ' ' << end.day() 
               << ' ' << end.year();
        s_dates = sdates.str();
    }
    
    //  up_some_lists
    //      Snarf up the lists from nasdaq and finviz.
    //
    static inline void up_some_lists()
    {
        ::system("sh ../bin/snarflists.sh");
    }
    
protected:
    static YahooCSVParser yahoo_csv;
    // Add boost::thread here...

    // constant parts of the commandline.
    static string s_dates;
    static const string s_command_base;
};
YahooCSVParser Snarf::yahoo_csv;
string Snarf::s_dates;
const string Snarf::s_command_base("sh ../bin/wget-YF-table.sh ");


const string& clean_up_symbol(const string& symbol)
{
    static string stemp;
    stemp.clear();
    
    stemp = boost::replace_first_copy(symbol, "$", "-P");
    
    static string stemptwo;
    stemptwo.clear();
    
    stemptwo = boost::replace_first_copy(stemp, ".", "-");
    
    return stemptwo;
}

// finviz
//      Parse a finviz file line.
//
inline void finviz(const stringvector& elements, SymbolDescriptorSet& sd)
{
    if( elements.size() < 6) return;

    // construct a SymbolDescriptor
    // and push it onto the back of a list.
    
    sd.push_back(SymbolDescriptor(
        clean_up_symbol(boost::erase_all_copy(elements[1], "\"")),   // Symbol
        boost::erase_all_copy(elements[2], "\""),   // Company
        boost::erase_all_copy(elements[3], "\""),   // Sector
        boost::erase_all_copy(elements[4], "\""),   // Industry
        boost::erase_all_copy(elements[5], "\""))); // Country
}

// nasdaqlisted
//      Parse a nasdaqlisted file.
//
inline void nasdaqlisted(const stringvector& elements, SymbolDescriptorSet& sd)
{
    if( elements.size() < 4) return;

    // 0 Symbol == Symbol
    // 1 Security Name == Company
    // 2 Market Category == n/a
    // 3 Test Issue == [YN] (skip if Y)
    // 4 Financial Status == [N <blah>]
    // 5 Round Lot Size == n/a
    
    // Kinda missing the sector and industry stuff... oh well.
    
    if( (6   >  elements[0].length()) &&  // last line is a date/time tag.
        ('N' == elements[3][0]      ) )   // not a test issue.
    {
        // construct a SymbolDescriptor
        // and push it onto the back of a list.
        sd.push_back(SymbolDescriptor(
            clean_up_symbol(elements[0]),   // Symbol
            elements[1],   // Company
            " ",           // Sector
            " ",           // Industry
            " "));         // Country
    }
}


// otherlisted
//      Parse an Other-Listed TXT file into a set of Symbol descriptors.
//
inline void otherlisted(const stringvector& elements, SymbolDescriptorSet& sd)
{
    if( elements.size() < 7) return;

    // 0 ACT Symbol == kinda like Symbol...
    // 1 Security Name == Company
    // 2 Exchange == n/a
    // 3 CQS Symbol == kinda like Symbol...
    // 4 ETF == n/a
    // 5 Round Lot Size == n/a
    // 6 Test Issue == [YN] (skip if Y)
    // 7 NASDAQ Symbol == kinda like Symbol...
    
    // Kinda missing the sector and industry stuff... oh well.
    
    if( (6   >  elements[0].length()) &&  // last line is a date/time tag.
        ('N' == elements[6][0]      ) )   // not a test issue.
    {
        // construct a SymbolDescriptor
        // and push it onto the back of a list.
        sd.push_back(SymbolDescriptor(
            clean_up_symbol(elements[0]),   // Symbol
            elements[1],   // Company
            " ",           // Sector
            " ",           // Industry
            " "));         // Country
    }
}


// MissingOrEmpty
//      Remove a Symbol from the list (and the file)
//      if no data was downloaded.
//
class MissingOrEmpty
{
public:
    // count
    //      number of tickers removed
    //
    static const int count() { return s_count; }
    
    // operator()
    //      defines the predicated. return true if element
    //      should be removed from the list.
    //      
    bool operator()(const SymbolDescriptor& sd)
    {
        if(!boost::filesystem::exists(sd.DATFile))
        {
            s_count++;
            return true;
        }
        else
            return false;
    }
    
private:
    static int s_count;
};
int MissingOrEmpty::s_count(0);


// main
//      Use this program to download a pile of historical
//      stock data.
//
int main(int argc, char * argv[])
{
    // Change directories into the lists directory.
    ::puts(constants::lists_path.base_path());
    WorkingDirectory current_dir(constants::lists_path.base_path());

    // Get and concatenate the lists.
    ::puts("Snarfing up lists....");
    Snarf::up_some_lists();
    SymbolDescriptorSet the_tickers;
    cout << "Parsing finviz.csv..." << endl;
    FileParser("finviz.csv",       ",").load_using(finviz,       the_tickers);
    cout << "Parsing nasdaqlisted.txt..." << endl;
    FileParser("nasdaqlisted.txt", "|").load_using(nasdaqlisted, the_tickers);
    cout << "Parsing otherlisted.txt..." << endl;
    FileParser("otherlisted.txt",  "|").load_using(otherlisted,  the_tickers);

    // Trim out all of the duplicates & sort by ticker symbol.
    the_tickers.sort();
    the_tickers.unique();
    
    // Change directories into the data directory.
    // makes call to wget-YF-table shell script a bunch simpler.
    ::puts(constants::data_path.base_path());
    current_dir.chdir(constants::data_path.base_path());

    // snarf the data building a background signal
    {
        // construct a string representing the start and end dates
        // Note that months are 0 indexed...
        // This is 22 Sept. 2010 --> 22 Sept. 2011:
        // sh ../bin/snarf-YF-table.sh MSFT 08 22 2010 08 22 2011
        Snarf::set_start_and_end(constants::start_date, constants::end_date);

        Snarf ocelot;
        // debugging - get data for the first Symbol (A)
        // ocelot(the_tickers.front());
        
        // normal - get data for all of the tickers.
        the_tickers.foreach(ocelot);
    }
    
    {
        // compute and write out the background signals.
        ::puts("Recording background signal...");
        Backgrounder::record_backgrounds();

        // apply the background signal.
        ::puts("Applying background signal...");
        Backgrounder apply_background;
        the_tickers.foreach(apply_background);
    }
    
    // remove all of the ticker symbols that didn't download from the list.
    {
        ::puts("Cleaning up lists.....");
        MissingOrEmpty m_or_e;
        the_tickers.remove_if(m_or_e);
        ::printf("Removed %i tickers from the list.\n", MissingOrEmpty::count());
    }
    
    // and write out to SymbolDescriptors.txt
    ::puts(constants::lists_path.base_path());
    current_dir.chdir(constants::lists_path.base_path());
    
    ::puts("Final list written to SymbolDescriptors.txt");
    save_to(the_tickers, constants::lists_path("SymbolDescriptors.txt"));
    
    return 0;
}




