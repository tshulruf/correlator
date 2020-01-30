#ifndef SYMBOLS_H
#define SYMBOLS_H

#include <string>
#include <stdio.h>
#include "extended_container.h"

using namespace std;

// SymbolDescriptor
//   Contain the details to describe a trading ticker symbol.
//   Used to represent a line in finviz.csv.
//
struct SymbolDescriptor
{
    // Container
    //      finviz | nasdaq*
    //
    string Symbol;     // 1|0,     MSFT
    string Company;    // 2|1,     Microsoft Corporation
    string Sector;     // 3,       Technology
    string Industry;   // 4,       Application Software
    string Country;    // 5,       USA
    
    // file names
    string CSVFile;     // MSFT.csv - Yahoo! Finance historical data file.
    string DATFile;     // MSFT.dat - processed historical data file.
    string LOGFile;     // MSFT.log - log file (used for debugging).
    
    // Default Constructor
    //
    inline SymbolDescriptor() {}
    
    // Construct by components (usually used.)
    //      Symbol      - the symbol.
    //      Company     - company name.
    //      Sector      - trading sector.
    //      Industry    - trading industry.
    //      Country     - country that symbol is traded
    //
    inline SymbolDescriptor(const string& Symbol,
                            const string& Company,
                            const string& Sector,
                            const string& Industry,
                            const string& Country) :
        Symbol(Symbol), Company(Company), Sector(Sector),
        Industry(Industry), Country(Country) 
    {
        CSVFile = Symbol + csv;
        DATFile = Symbol + dat;
        LOGFile = Symbol + log;
    }
        
    // Copy constructor.
    //
    inline SymbolDescriptor(const SymbolDescriptor& td) :
        Symbol(td.Symbol), Company(td.Company), Sector(td.Sector),
        Industry(td.Industry), Country(td.Country),
        CSVFile(td.CSVFile), DATFile(td.DATFile), LOGFile(td.LOGFile) {}
        
    // copy_details
    //      There are several preferred stocks (*-P*) that
    //      match up to other stocks.  Match up the details
    //      that aren't listed in the original files.
    //
    inline void copy_details(const SymbolDescriptor& from)
    {
        Sector = from.Sector;
        Industry = from.Industry;
        Country = from.Country;
    }

protected:

    // Helper strings.
    static const string csv;
    static const string dat;
    static const string log;
};

// instantiate helper static strings...
const string SymbolDescriptor::csv(".csv");
const string SymbolDescriptor::dat("/ticks");
const string SymbolDescriptor::log(".log");

// Operators
//
// operator==
//      a, b - SymbolDescriptors to compare.
//      returns - true if Symbols have the same symbol.
//
inline const bool operator==(const SymbolDescriptor& a, const SymbolDescriptor& b)
{
    return (a.Symbol == b.Symbol);
}

// operator<
//      a, b - SymbolDescriptors to compare.
//      returns - operator<(string, string) using Symbol symbols.
//
inline const bool operator<(const SymbolDescriptor& a, const SymbolDescriptor& b)
{
    return (a.Symbol < b.Symbol);
}

//  Stream I/O operators.
//  operator<<
//      out - output text stream.
//      td  - SymbolDescriptor to stream.
//      returns - stream for continued use.
//
ostream& operator<< (ostream& out, const SymbolDescriptor& td)
{
    out << td.Symbol << endl
        << td.Company << endl
        << td.Sector << endl
        << td.Industry << endl
        << td.Country << endl
        << td.CSVFile << endl
        << td.DATFile << endl
        << td.LOGFile << endl;

    return out;
}

//  readstring
//      Helper to extract a string out of a line of text.
//      in     - istream providing the line of text.
//      target - string out.
//
bool readstring(istream& in, string& target)
{
    stringbuf sb;
    in.get(sb);
    if(in.fail())
    {
        if(in.eof()) return false;
        else         in.clear(); // empty line.
    }
    else target = sb.str();
    in.get(); // \n
    return true;
}

//  operator>>
//      in - input stream.
//      td - SymbolDescriptor to stream.
//      returns - stream for continued use.
//
istream& operator>> (istream& in, SymbolDescriptor& td)
{
    readstring(in, td.Symbol);
    readstring(in, td.Company);
    readstring(in, td.Sector);
    readstring(in, td.Industry);
    readstring(in, td.Country);
    readstring(in, td.CSVFile);
    readstring(in, td.DATFile);
    readstring(in, td.LOGFile);
    
    return in;
}


// SymbolDescriptorSet
//   Contain a set of Symbol descriptors.
//
typedef ExtendedContainer<SymbolDescriptor, 
                          list< SymbolDescriptor > > SymbolDescriptorSet;
typedef ExtendedContainer<SymbolDescriptor, 
                          deque< SymbolDescriptor > > SymbolDescriptorDeque;


// For testing purposes, here's a SymbolDescriptorAction
// to print out all of the Symbol descriptors.
//
// print
//      Print a SymbolDescriptor to STDOUT.
//      td - SymbolDescriptor to print.
//
inline void print(SymbolDescriptor& td)
{
    printf("%s, %s, %s, %s, %s\n", 
        td.Symbol.c_str(),
        td.Company.c_str(),
        td.Sector.c_str(),
        td.Industry.c_str(),
        td.Country.c_str());
}


//  SymbolVector
//      A vector of ticker symbols. Contain the list of symbols
//      that have valid data on a particular day.
//
typedef ExtendedContainer< string, vector< string > > SymbolVector;

//  load_from
//      symbols  - SymbolVector to fill from a file. Failure to load clears symbols.
//      filename - File to load from.
//
void load_symbols_from(SymbolVector& symbols, const char * filename)
{
    if (!symbols.empty()) symbols.clear();
    
    ifstream iFile(filename);
    while (!iFile.eof())
    {
        string temp;
        if(readstring(iFile, temp))
            symbols.push_back(temp);
    }
}

//  save_to
//      Save the contents of a SymbolVector into a text file. One element per "line."
//      symbols  - container for the data.
//      filename - text file to save to.
//
void save_symbols_to(const SymbolVector& symbols, const char * filename)
{
    ofstream outfile(filename);
    BOOST_FOREACH(string symbol, symbols)
    {
        outfile << symbol << endl;
    }
}





#endif // SYMBOLS_H

