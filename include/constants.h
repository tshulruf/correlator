#ifndef CONSTANTS_H
#define CONSTANTS_H

#include "directories.h"
#include <boost/date_time/gregorian/gregorian.hpp>

namespace constants
{
    // Commonly used paths.
    PathMaker lists_path("/home/tom/Source/Correlator3/lists");
    PathMaker data_path("/home/tom/Source/Correlator3/data");
    PathMaker means_path("/home/tom/Source/Correlator3/means");
    PathMaker correlations_path("/home/tom/Source/Correlator3/correlations");
    
    // Start and end dates of data
    using namespace boost::gregorian;
    date start_date(from_simple_string("2011-03-16"));
    date end_date(from_simple_string("2012-03-16"));
    
    // Which set of data to correlate?
    const string deltaclose = "deltaclose";
    const string deltaadjclose = "deltaadjclose";
    const string deltaclosenobkg = "deltaclosenobkg";
    const string deltaadjclosenobkg = "deltaadjclosenobkg";
    const string& corellating = deltaadjclosenobkg;
    
    // Work with plain text or binary?
    bool save_as_binary = true;
}

#endif // CONSTANTS_H
