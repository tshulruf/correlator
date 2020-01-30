#ifndef CONTAINER_IO_H
#define CONTAINER_IO_H

#include <fstream>
#include <iostream>
#include <iterator>
#include "constants.h"

using namespace std;

//  Archiving
//      Use standard fstreams to save and load files from/to text.
//
//  load_from
//      Load from a text file, one element per "line."
//      c        - container for the data.
//      filename - text file to read in.
//
template<class STL_Container>
void load_from(STL_Container& c, const char * filename)
{
    if (!c.empty()) c.clear();
    
    ios_base::openmode iomode = ios_base::in;
    if(constants::save_as_binary) iomode |= ios_base::binary;
    
    ifstream iFile(filename, iomode);
    istream_iterator<typename STL_Container::value_type> itt(iFile);
    istream_iterator<typename STL_Container::value_type> eos;
    while(itt != eos)
    {
        c.push_back(*itt);
        itt++;
    }
}

//  save_to
//      Save the contents of a vector into a text file. One element per "line."
//      c        - container for the data.
//      filename - text file to save to.
//
template<class STL_Container>
void save_to(const STL_Container& c, const char * filename)
{
    ios_base::openmode iomode = ios_base::out;
    if(constants::save_as_binary) iomode |= ios_base::binary;

    ofstream outfile(filename, iomode);
    ostream_iterator<typename STL_Container::value_type> out_it(outfile);
    copy(c.begin(), c.end(), out_it);
}


//  BufferedRecordReader
//      Read a particular record out of a file that's a collection
//      of records.  Optimizes read time and memory and disk allocation.
//      Developed to work around ext4 minimum block size of 4048 bytes.
//      RecordType - Type of record that comprises file.
//                   RecordType must support copying.
//
template< class RecordType >
class BufferedRecordReader
{
public:
    //  Constructor
    //
    inline BufferedRecordReader() { }

    //  Construct and open a file.
    //      filename - file to attempt opening.
    //
    inline BufferedRecordReader(const char * filename) : _file(filename) { }

    // Status Functions

    //  is_open
    //      Return true if file is open.
    //
    inline bool is_open() { return _file.is_open(); }

    // maybe add a bunch more for debugging if neccessary...

    // Utility Functions
    
    //  close
    //      Close the file if it's open & invalidate the index.
    //
    inline void close()
    {
        if(_file.is_open()) _file.close();
        _lastread = IntType::invalid_value;
    }

    //  open
    //      Close any open file, reset the last index to invalid,
    //      and open up the new file.
    //      filename - file to attempt opening.
    //      return true if file is open.
    //
    bool open(const char * filename)
    {
        // close the last file and mark the buffer as unread.
        close();
        
        _file.open(filename);
        return _file.is_open();
    }

    //  read
    //      Read a record from the file at a particular index.
    //      index   - Index of the record in the file. (index >= 0)
    //      out     - Destination for the data.
    //                RecordType must support copying.
    //      returns true if data read into the destination object.
    //      returns false if file not open, istream not good,
    //                       index < 0, or read failed.
    //
    bool read(int index, RecordType& out)
    {
        if( (!_file.is_open()) ||
            (!_file.good())    ||
            (0 > index)        ) return false;
        
        // return last read data if asked for it again.
        if(index == _lastread)
        {
            out = _buffer;
            return true;
        }

        // Zero-based indexing
        _file.seekg(index * RecordType::Size());

        if(_file.good())
        {
            // Read new indexed data into the buffer.
            _file >> _buffer;

            if(_file.good())
            {
                // if no error, store index
                _lastread = index;
                out = _buffer;
                return true;
            }
            else return false;
        }
        else return false;
    }

protected:
    //  _buffer
    //  _lastread
    //      Don't read the same spot twice in a row...
    //
    RecordType  _buffer;
    IntType     _lastread;

    //  _file
    //      Input stream.
    //
    ifstream _file;
};


#endif // CONTAINER_IO_H
