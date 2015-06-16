#pragma once
/*
 Created by Jonathan Rumion on 6/12/15.

 This is a set of classes and functions for working with 'zfec' split data.

 Whether for binary data (images, music, programs, etc), or just plain old string data that is too
 big for a single BitMessage message (256kb).



*/

#ifndef LIBBMWRAPPER_BMFEC_H
#define LIBBMWRAPPER_BMFEC_H

#include "base64.h"
#include "sha256.h"
#include "Network.h"
#include "Filesystem.h"

#include <fecpp.h>
#include <string>
#include <fstream>
#include <sstream>



namespace bmwrapper {

inline size_t log2_ceil(size_t n) {
    size_t i = 0;
    while (n) {
        n >>= 1;
        ++i;
    }
    return i;
}

template<typename T>
inline fecpp::byte get_byte(size_t byte_num, T input) {
    return (input >> ((sizeof(T) - 1 - (byte_num & (sizeof(T) - 1))) << 3));
}


struct ZfecChunk {

    std::string m_sha256sum;
    int m_K;
    int m_N;
    base64 m_data;

};

void write_zfec_header(std::ostream &output, size_t n, size_t k, size_t pad_bytes, size_t share_num);

class zfec_file_writer {

public:

    zfec_file_writer(const std::string &prefix, size_t n, size_t k, size_t pad_bytes);

    ~zfec_file_writer() {
        for (size_t i = 0; i != outputs.size(); ++i) {
            outputs[i]->close();
            delete outputs[i];
        }
    }

    void operator()(size_t block, size_t /*max_blocks*/,
                    const fecpp::byte buf[], size_t buflen) {
        outputs[block]->write((const char *) buf, buflen);
    }

private:
    // Have to use pointers instead of obj as copy constructor disabled
    std::vector<std::ofstream *> outputs;
};


class BmFEC {

public:

    BmFEC(NetworkModule *owner) : m_owner(owner){};

    bool SendMail(NetworkMail message);


private:

    NetworkModule *m_owner;

    void zfec_encode(size_t k, size_t n, const std::string &prefix, std::ifstream &in);

    FileSystemHandler fsHandler;

};

}


#endif //LIBBMWRAPPER_BMFEC_H
