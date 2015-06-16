//
// Created by Jonathan Rumion on 6/15/15.
//

#include "Filesystem.h"
#include <iostream>
#include <sstream>
#include <cassert>
#include <fstream>

bool FileSystemHandler::CreateDirectory(std::string directory) {

    directory = expand_user(directory);

    if(FileExists(directory)){
        return false;
    }
    else{

        bool created = boost::filesystem::create_directory(directory);
        if(created) {
            return true;
        }
        else{
            return false;
        }
    }
}

bool FileSystemHandler::CreateFile(std::string file) {

    file = expand_user(file);

    if(FileExists(file)){
        return false;
    }
    else{

        std::ofstream output(file);
        return true;
    }
}

bool FileSystemHandler::FileExists(std::string file) {
    file = expand_user(file);
    return boost::filesystem::exists(file);
}


std::string FileSystemHandler::expand_user(std::string path) {
    if (! path.empty() && path[0] == '~') {
        assert(path.size() == 1 || path[1] == '/');  // or other error handling
        char const* home = getenv("HOME");
        if (home || ((home = getenv("USERPROFILE")))) {
            path.replace(0, 1, home);
        }
        else {
            char const *hdrive = getenv("HOMEDRIVE"),
                    *hpath = getenv("HOMEPATH");
            assert(hdrive);  // or other error handling
            assert(hpath);
            path.replace(0, 1, std::string(hdrive) + hpath);
        }
    }
    return path;
}