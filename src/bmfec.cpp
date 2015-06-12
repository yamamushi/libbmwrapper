//
// Created by Jonathan Rumion on 6/12/15.
//

#include "bmfec.h"

namespace bmfec {

void write_zfec_header(std::ostream &output,
                       size_t n, size_t k, size_t pad_bytes, size_t share_num) {
    // What a waste of effort to save, at best, 2 bytes. Blech.
    const size_t nbits = log2_ceil(n - 1);
    const size_t kbits = log2_ceil(k - 1);

    size_t out = (n - 1);
    out <<= nbits;
    out |= (k - 1);
    out <<= kbits;
    out |= pad_bytes;
    out <<= nbits;
    out |= share_num;

    size_t bitsused = 8 + kbits + nbits * 2;

    if (bitsused <= 16)
        out <<= (16 - bitsused);
    else if (bitsused <= 24)
        out <<= (24 - bitsused);
    else if (bitsused <= 32)
        out <<= (32 - bitsused);

    for (size_t i = 0; i != (bitsused + 7) / 8; ++i) {
        unsigned char b = get_byte(i + (sizeof(size_t) - (bitsused + 7) / 8), out);
        output.write((char *) &b, 1);
    }
}


// This will be replaced by a bitmessage transfer utility class

zfec_file_writer::zfec_file_writer(const std::string &prefix, size_t n, size_t k, size_t pad_bytes) {
    for (size_t i = 0; i != n; ++i) {
        std::ostringstream outname;
        outname << prefix << '.';

        if (n > 10 && i < 10)
            outname << '0';
        if (n > 100 && i < 100)
            outname << '0';

        outname << i << '_' << n << ".fec";

        std::ofstream *out = new std::ofstream(outname.str().c_str());

        if (!*out)
            throw std::runtime_error("Failed to write " + outname.str());

        write_zfec_header(*out, n, k, pad_bytes, i);
        outputs.push_back(out);
    }
}


void zfec_encode(size_t k, size_t n,
                 const std::string &prefix,
                 std::istream &in,
                 size_t in_len) {
    const size_t chunksize = 4096;

    fecpp::fec_code fec(k, n);

    std::vector<fecpp::byte> buf(chunksize * k);

    size_t pad_bytes = (in_len % k == 0) ? 0 : k - (in_len % k);

    zfec_file_writer file_writer(prefix, n, k, pad_bytes);

    while (in.good()) {
        in.read((char *) &buf[0], buf.size());
        size_t got = in.gcount();

        if (got == buf.size())
            fec.encode(&buf[0], buf.size(), std::ref(file_writer));
        else {
            // Handle final block by padding up to k bytes with 0s
            for (size_t i = 0; i != pad_bytes; ++i)
                buf[i + got] = 0;
            fec.encode(&buf[0], got + pad_bytes, std::ref(file_writer));
        }
    }
}

void zfec_encode(size_t k, size_t n,
                 const std::string &prefix,
                 std::ifstream &in) {
    in.seekg(0, std::ios::end);
    size_t in_length = in.tellg();
    in.seekg(0, std::ios::beg);

    zfec_encode(k, n, prefix, in, in_length);
}

}
