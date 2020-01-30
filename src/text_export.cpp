#include <stdio.h>
#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <boost/archive/xml_oarchive.hpp>
#include <boost/program_options.hpp>
#include <boost/filesystem.hpp>
#include "../include/tickers.h"
#include "../include/signals.h"
#include "../include/correlations.h"

namespace po = boost::program_options;
using namespace std;


// main
//      Use this program to export a DAT file to XML for inspection.
//
int main(int ac, char * av[])
{

    // Command line processing (stolen from tutorial)
    //
    vector<string> input_files;
    char kind = ' '; // for invalid.
    
    po::options_description desc("Allowed options");
    desc.add_options()
        ("help", "text_export - Re-Store a data file as a text file.")
        ("kind", po::value< char >(&kind), 
        "Kind of file to export:\n"
        "   b = background data\n"
        "   c = correlations\n"
        "   f = found correlations\n"
        "   m = date index map\n"
        "   p = preprocessed data\n" 
        "   t = ticks")
        ("input-files", po::value< vector<string> >(&input_files), 
          "Files to export. eg. text_export kind=t A.dat B.dat")
    ;
    
    po::positional_options_description p;
    p.add("input-files", -1);

    po::variables_map vm;
    po::store(po::command_line_parser(ac, av).
              options(desc).positional(p).run(), vm);
    po::notify(vm);

    if ((vm.count("help")))
    {
        cout << desc << endl;
        return 0;
    }
    if (0 == input_files.size())
    {
        cout << desc << endl;
        return 1;
    }

    BOOST_FOREACH(string filename, input_files)
    {
        if(boost::filesystem::exists(filename))
        {
            constants::save_as_binary = true;

            cout << "Extracting " << filename << " to " << filename << ".txt";
            
            try
            {
                string outfilename = filename;
                outfilename += ".txt";

                switch(kind)
                {
                case 'B':
                case 'b':           // background data
                    cout << " as background data..." << endl;
                    {
                        FloatSignal bkg;

                        constants::save_as_binary = true;
                        bkg.load_from(filename.c_str());

                        constants::save_as_binary = false;
                        bkg.save_to(outfilename.c_str());
                    }
                    break;

                case 'C':
                case 'c':           // set of correlations.
                    cout << " as a cross-correlations file... please wait... " << endl;
                    {
                        FloatCrossCorrelation fcc;
                        constants::save_as_binary = true;
                        fcc.load_from(filename.c_str());

                        constants::save_as_binary = false;
                        fcc.save_to(outfilename.c_str());
                    }
                    break;
                
                case 'F':
                case 'f':           // found correlations
                    cout << " as a found correlations file... please wait... " << endl;
                    {
                        RowColVector rcv;
                        constants::save_as_binary = true;
                        load_from(rcv, filename.c_str());

                        constants::save_as_binary = false;
                        save_to(rcv, outfilename.c_str());
                    }
                    break;
                
                case 'M':
                case 'm':           // date index map
                    cout << " as a date index map..." << endl;
                    {
                        IntSignal smap;

                        constants::save_as_binary = true;
                        smap.load_from(filename.c_str());

                        constants::save_as_binary = false;
                        smap.save_to(outfilename.c_str());
                    }
                    break;

                case 'P':
                case 'p':           // pre-processed ticks.
                    cout << " as a processed data file... " << endl;
                    {
                        FloatStatisticalDeque Delta;

                        constants::save_as_binary = true;
                        load_from(Delta, filename.c_str());

                        constants::save_as_binary = false;
                        save_to(Delta, outfilename.c_str());
                        
                    }
                    break;

                case 'T':
                case 't':           // ticks
                    cout << " as a set of ticks..." << endl;
                    {
                        TickerSet ticks;

                        constants::save_as_binary = true;
                        load_from(ticks, filename.c_str());

                        constants::save_as_binary = false;
                        save_to(ticks, outfilename.c_str());
                    }
                    break;
                    
                default:
                    cout << " as an invalid file type! (" << kind << ")" << endl
                         << desc << endl;
                }
            }
            catch(exception e)
            {
                cout << filename << " failed to load! Skipping..." << endl;
            }
        }
        else
        {
            cout << filename << " not found! Skipping." << endl;
        }
    }

    return 0;
}
