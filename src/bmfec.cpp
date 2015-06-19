//
// Created by Jonathan Rumion on 6/12/15.
//

#include "bmfec.h"
#include <fecpp.h>
#include "decode.h"
#include "encode.h"
#include "base64.h"
#include "sha256.h"


namespace bmwrapper {

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


void BmFEC::zfec_encode(size_t k, size_t n, const std::string &prefix, std::ifstream &in) {

}

// FIXME
bool BmFEC::SendMail(NetworkMail message) {

    if(message.isFile()){

        // Resolve user paths ~
        std::string l_filename = fsHandler.expand_user(message.getMessage());

        if(fsHandler.FileExists(l_filename)){

            // Attempt to load file
            std::ifstream l_binaryFile(l_filename, std::ios::in|std::ios::binary);
            if (l_binaryFile.is_open()) {

                // Get length of file and store it in a char buffer
                l_binaryFile.seekg(0, l_binaryFile.end);
                unsigned long l_fileLength = (unsigned long)l_binaryFile.tellg(); // Will this limit our file sizes?
                l_binaryFile.seekg(0, l_binaryFile.beg);

                const char * l_binaryFileBuffer;
                l_binaryFileBuffer = new char [l_fileLength];
                l_binaryFile.read ((char*)l_binaryFileBuffer, l_fileLength);


                // At this point our binary file has been loaded into l_binaryFileBuffer
                // We need to convert it to base64 and package it for transport now
                // We will use libb64 for encoding binaries

                libb64::encoder l_encoder;
                const char * l_base64encoderBuffer = new char [l_fileLength];
                l_encoder.encode((char*)l_binaryFileBuffer, (int)l_fileLength, (char*)l_base64encoderBuffer);

                // Now we package our buffer into a string, specifying the length
                // Otherwise std::string will terminate at the first null character
                std::string l_base64StringBuffer((char *)l_base64encoderBuffer, l_fileLength);

                // Now we are done with the filestream, so we can close it
                // And delete our buffers
                delete l_base64encoderBuffer;
                delete l_binaryFileBuffer;
                l_binaryFile.close();


                // Package a new message and send it back through this loop for FEC magic
                NetworkMail l_packagedMessage(message.getFrom(),
                                              message.getTo(),
                                              message.getSubject(),
                                              l_base64StringBuffer);

                return SendMail(l_packagedMessage);

            }
            return false;
        }
        else{
            return false;
        }
    }
    else{
        // Else our message is ready to be FEC'd

        SHA256 l_sha256engine;
        std::string l_sha256sum = l_sha256engine(message.getMessage());

        // Create class to store disassembled message contents into a Vector
        // 
        // Disassemble our message and pass into Vector class
        //
        // void encode(const byte input[], size_t size,
        // std::tr1::function<void (size_t, size_t, const byte[], size_t)> out) const
        //
        // The tr1::function above will be a function within our vector class
        // That indexes and stores the relevant message information
        //
        // Once Vector is assembled, we will parse through it to send each
        // message chunk piece by piece.
        //


            // Package



    }

    return false;

}

}



/*
// FEC/Fecpp Passthrough Functions

bool BitMessage::setFecDefaultSize(int k, int n){

    // Must conform to 1 <= K <= N <= 256
    if( k <= 0 || n <= 0 || k > 256 || n > 256 || k > n ){
        return false;
    }
    else{
        m_fecK = k;
        m_fecN = n;
        return true;
    }

}
 */