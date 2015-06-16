/*
 * Created by Jonathan Rumion on 6/15/15.
 *
 * A utility class for working with the filesystem.
 *
*/

#ifndef LIBBMWRAPPER_FILESYSTEM_H
#define LIBBMWRAPPER_FILESYSTEM_H


/*
Filesystem.h - FileSystemHandler
 */

#include <boost/filesystem.hpp>
#include <string>

class FileSystemHandler {

public:

    bool CreateDirectory(std::string directory);
    bool FileExists(std::string file);
    bool CreateFile(std::string file);

    std::string expand_user(std::string path);



private:

};


#endif //LIBBMWRAPPER_FILESYSTEM_H
