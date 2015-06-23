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
#include <vector>
#include <fstream>
#include <sstream>


namespace bmwrapper {


class bmfec_message {

public:

    bmfec_message(size_t n) : m_messageCollection(n) {}

    void operator()(size_t i, size_t, const unsigned char fec[], size_t fec_len)
    {
        m_messageCollection[i].append(reinterpret_cast<const char*>(fec), fec_len);
    }

    std::string getSha256(){return m_sha256sum;}
    std::vector<std::string> getMessageCollection(){return m_messageCollection;}

private:

    std::string m_sha256sum;
    std::vector<std::string> m_messageCollection;

};




class BmFEC {

public:

    BmFEC(NetworkModule *owner) : m_owner(owner){};

    bool SendMail(NetworkMail message);


private:

    NetworkModule *m_owner;

};

}




#endif //LIBBMWRAPPER_BMFEC_H
