#ifndef DIRECTORIES_H
#define DIRECTORIES_H

#include <string>
#include <filesystem>
#include <unistd.h>

using namespace std;

// WorkingDirectory
//      Maintain previous working directory while
//      changing into other working directories.
//      Returns user to original directory when done.
//      Should be used like a singleton...
//
class WorkingDirectory
{
public:
    // Constructor
    //      Default constructor stays in original working directory.
    //
    inline WorkingDirectory() {}
    
    // Constructor
    //      Changes to a new working directory.
    //      Fails silently if ::chdir fails.
    //      dir - directory to change into.
    //
    inline WorkingDirectory(const char * dir)
    {
        ::chdir(dir);
    }
    
    // Destructor
    //      Return to original working directory.
    //
    inline ~WorkingDirectory()
    {
        ::chdir(s_original_dir.c_str());
    }
    
    // chdir
    //      Change to a new working directory.
    //      Fails silently if ::chdir fails.
    //      dir - directory to change into.
    //
    inline void chdir(const char * dir) { ::chdir(dir); }
    
protected:
    static string s_original_dir;
    
    // probably ought to enforce the singleton..
};

// s_original_dir
//      starting working directory from os.
//
string WorkingDirectory::s_original_dir(std::filesystem::current_path());


// PathMaker
//      Construct a path relative to some base.
//      Note that these are only string operations.
//      if paths are invalid, your code will break.
//
class PathMaker
{
public:
    // Constructor
    //      Store the base path.
    //
    inline PathMaker(const char * base_path)
        : _base_path(base_path) {  }

    // operator()
    //      Make a path relative to a base path.
    //      filename - file name appended to base path (base_path/filename)
    //
    inline const char * operator()(const char * filename) const
    {
        static string datapath(_base_path);
        datapath = _base_path;
        datapath += '/';
        datapath += filename;
        return datapath.c_str();
    }
    
    // base_path
    //      accessor: return the base path to use this as a directory name.
    //
    inline const char * base_path() const { return _base_path.c_str(); }

protected:

    // Container
    //
    const string _base_path;
    
private:
    // no default constructor!
    inline PathMaker() {}
    
    // do not copy!
    inline PathMaker(const PathMaker& pm) {}
};


#endif // DIRECTORIES_H

