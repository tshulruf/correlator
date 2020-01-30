#ifndef PARSERS_H
#define PARSERS_H

#include <string>
#include <vector>
#include <fstream>
#include <boost/algorithm/string.hpp>
#include <boost/foreach.hpp>
#include <boost/lexical_cast.hpp>


using namespace std;

// SafeLexicalCast
//      Wrap a try-catch block with a default return value
//      for Target boost:lexical_cast<Target, Source>(const Source&).
//
template<class Target, class Source>
class SafeLexicalCast
{
public:
    // Constructor
    //      t - default value to return on parsing error.
    //
    inline SafeLexicalCast(Target t) : _default(t) { }

    // cast
    //      Transform a string into a Target.
    //
    inline Target cast(const Source& s) throw ()
    {
        try { return boost::lexical_cast<Target>(s); }
        catch(...) { return _default; }
    }

protected:
    Target _default;

private:
    // No default constructor!
    inline SafeLexicalCast() { }
    
    // do not copy!
    inline SafeLexicalCast(const SafeLexicalCast& slc) { }
};


// handy type to simplify calls...

typedef vector<string> stringvector;


// sample_rule
//      Sample Rule Predicate for generating a vector
//      containing the strings in the file.
//
void sample_rule(const stringvector& elements, stringvector& file_contents)
{
    if(!elements.empty())
    {
        string buffer;
        BOOST_FOREACH(string element, elements)
        {
            buffer += element;
            buffer += ' ';
        }
        file_contents.push_back(buffer); 
    }
}


// sample call with cool unnamed temp:
//      if (!FileParser(constants::some_path("somefile"),
//                      ",|").load_using(sample_rule,
//                                       container)) return 0;


// LineParser
//      Contain the buffer for paring a line,
//      it's delimters, and a template function
//      to actually parse a line.
//      
class LineParser
{
public:
    // LineParser
    //      Constructor
    //      delims   - delimiters for each line (',' for CSV files, etc.)
    //
    inline LineParser(const char * delims) : _delims(delims) { }

    // parse_line
    //      Split a line and parse it given rules and any parameters.
    //      Might end up copying and pasting this one several times for
    //      extra parameters.
    //      r - predicate function for parsing a line.
    //      c - container for parsed data.
    //
    template<class Rules, class Container>
    inline void parse(const string& line, Rules& r, Container& c)
    {
        stringvector splitline;
        boost::split(splitline, line, boost::is_any_of(_delims.c_str()));

        // delimiters are escaped when in quotes.
        // put them back if they've been stripped.
        
        bool escaping = false;
        string       strtemp;
        stringvector elements;

        BOOST_FOREACH(string token, splitline)
        {
            strtemp += token;
            
            // start escaping if it starts with a quote.
            if(!token.empty() && ('"' == token[0]))
                escaping = true;

            // if escaping, check if done escaping.
            if( escaping                   &&
                !token.empty()                 &&
                ('"' == token[token.length() - 1]) )
                escaping = false;
            
            // if still escaping...
            if(escaping)
            {
                // put the delimiters back.
                strtemp += _delims;
            }
            else
            {
                // otherwise, done reassembling, 
                // add it to the return set.
                elements.push_back(strtemp);
                strtemp.clear();
            }
        }        

        // Apply the rules assembling c from the elements.
        r(elements, c);
    }

protected:
    // store away the delimiters for repeated use.
    string _delims;
    
private:
    // no default constructor!
    inline LineParser() {}

    // do not copy!
    inline LineParser(const LineParser& lb) {}
};


// FileParser
//      Load up a file name and pass it to a parser.
//      Separate the file opening and closing logic from the 
//      file parsing logic (above)
//
class FileParser
{
public:
    // FileParser
    //      Constructor
    //      filename - file to parse in the lists path.
    //      delims   - delimiters for each line (',' for CSV files, etc.)
    //
    inline FileParser(const char * filename, const char * delims) :
         _filename(filename),
         _delims(delims) {}
         
    // load_using
    //      Separate the file opening and closing logic from the 
    //      file parsing logic (above).
    //      Might end up copying and pasting this one several times for
    //      extra parameters.
    //      r - predicate function for parsing a line.
    //      c - container for parsed data.
    //
    template<class Rules, class Container>
    inline bool load_using(Rules& r, Container& c)
    {
        ifstream source(_filename.c_str());
        if(source.fail())
        {
            printf("Error failed to open file: %s\n", _filename.c_str());
            return false;
        }
        
        LineParser lp(_delims.c_str());
        string line;
        getline(source, line); // skip past the header.

        // might have to wrap this in a try/catch wrapper...        
        while(!source.eof())
        {
            getline(source, line);
            if (!line.empty()) lp.parse(line, r, c);
        }
        source.close();
        return true;
    }
    
protected:
    string _filename;
    string _delims;

private:
    // No default constructor!
    inline FileParser() {}
    
    // Do not copy!
    inline FileParser(const FileParser& fp) {}
};


#endif // PARSERS_H
