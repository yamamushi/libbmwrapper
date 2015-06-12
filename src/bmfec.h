//
// Created by Jonathan Rumion on 6/12/15.
//

#ifndef LIBBMWRAPPER_BMFEC_H
#define LIBBMWRAPPER_BMFEC_H

#include "base64.h"
#include <fecpp.h>
#include <string>
#include <fstream>
#include <sstream>

namespace bmfec {

size_t log2_ceil(size_t n) {
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

    BmFEC(int k, int n){};


private:

    void zfec_encode(size_t k, size_t n, const std::string &prefix, std::ifstream &in);

};

}


#endif //LIBBMWRAPPER_BMFEC_H
