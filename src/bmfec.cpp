//
// Created by Jonathan Rumion on 6/12/15.
//

#include "bmfec.h"
#include <fecpp.h>
#include<boost/tokenizer.hpp>
#include "decode.h"
#include "encode.h"
#include "base64.h"
#include "sha256.h"


namespace bmwrapper {


// FIXME
bool BmFEC::SendMail(NetworkMail message) {

    if(message.isFile()){

        // Resolve user paths ~
        FileSystemHandler fsHandler;

        std::string l_filename = fsHandler.expand_user(message.getMessage());

        if(fsHandler.FileExists(l_filename)){

            // Attempt to load file
            std::ifstream l_binaryFile(l_filename, std::ios::binary);
            if (l_binaryFile.is_open()) {

                // Get length of file and store it in a char buffer
                l_binaryFile.seekg(0, l_binaryFile.end);
                size_t l_fileLength = l_binaryFile.tellg();
                l_binaryFile.seekg(0, l_binaryFile.beg);

                const char * l_binaryFileBuffer = new char [l_fileLength];
                l_binaryFile.read ((char*)l_binaryFileBuffer, l_fileLength);


                // At this point our binary file has been loaded into l_binaryFileBuffer
                // We need to convert it to base64 and package it for transport now
                // We will use libb64 for encoding binaries
                libb64::encoder * l_encoder = new libb64::encoder;
                const char * l_base64encoderBuffer = new char [l_fileLength];
                l_encoder->encode((char*)l_binaryFileBuffer, l_fileLength, (char*)l_base64encoderBuffer);

                // Now we package our buffer into a string, specfying the length
                // Otherwise std::string will terminate at the first null character
                std::string l_base64StringBuffer((char *)l_base64encoderBuffer, l_fileLength);
                //std::string l_base64StringBuffer(std::to_string(sizeof(l_binaryFileBuffer)) + " " + std::to_string(sizeof(l_base64encoderBuffer)));

                // Now we are done with the filestream, so we can close it
                // And delete our buffers
                delete[] l_base64encoderBuffer;
                delete[] l_binaryFileBuffer;
                l_binaryFile.close();

                // Package a new message and send it back through this loop for FEC magic
                NetworkMail l_packagedMessage(message.getFrom(),
                                              message.getTo(),
                                              message.getSubject(),
                                              l_base64StringBuffer.data());

                return m_owner->sendMail(l_packagedMessage);

            }
            return false;
        }
        else{
            return false;
        }
    }
    else{
        // Else our message is ready to be FEC'd

        int l_maxSize = 255; // To account for room for our header
        int l_messageSize = message.getMessage().size();

        if(l_messageSize < l_maxSize){
            return m_owner->sendMail(message);
        }

        int fec_k = 0;
        int fec_n = 0;



        // Divide & Round Up
        //fec_n = l_messageSize / l_maxSize + (((l_messageSize < 0) ^ (l_maxSize > 0)) && (l_messageSize%l_maxSize));
        fec_n = 20;

        for(int x = fec_n-1; x >= 1; x--){

            if(l_messageSize % x == 0){
                fec_k = x;
                break;
            }

        }

        // We are defaulting to 1/3rd of the split pieces necessary for reconstruction
        //fec_k = fec_n / 3 + (((fec_n < 0) ^ (3 > 0)) && (fec_n%3));


        // Init an fec_code object to do the hard work for us
        fecpp::fec_code l_fecEngine(fec_k,fec_n);

        // Init class to store disassembled message contents into a Vector
        bmfec_message l_bmfecCollection(l_fecEngine.get_N());

        // Grab the sha256 sum of our message
        SHA256 l_sha256engine;
        std::string l_sha256sum = l_sha256engine(message.getMessage());


        // Disassemble our message and pass into Vector container class
        //
        // void encode(const byte input[], size_t size,
        // std::tr1::function<void (size_t, size_t, const byte[], size_t)> out) const
        //
        // It's important to note that message.getMessage() at this point should be a
        // base64 encoded string, it should not contain null data, but we encoded it
        // again anyways to account for plaintext messages that may come through here
        // being too large on their own.
        //
        l_fecEngine.encode(reinterpret_cast<const unsigned char*>(base64(message.getMessage()).encoded().c_str()),
                           message.getMessage().size(),
                           std::ref(l_bmfecCollection));
        // The std::ref/tr1::function above is the () operator function within our vector class (bmfec_message)
        // That indexes and stores the message information.


        // Once bmfec_message is assembled, we will parse through it to send each
        // message chunk piece by piece.
        //
        // In order for our message to be indexed properly, we need assemble a header for each piece in
        // bmfec_message's container vector.
        //
        for(unsigned int i = 0; i < l_bmfecCollection.getMessageCollection().size(); i++){

            //std::string l_part("Part: " + std::to_string(i) + "\n");
            //std::string l_of("Of: " + std::to_string(l_bmfecCollection.getMessageCollection().size()) + "\n");
            std::string l_sha("sha256: " + l_sha256sum + "\n");
            //std::string l_begin("--BEGIN DATA--\n");
            std::string l_content(l_bmfecCollection.getMessageCollection().at(i));
            //std::string l_end("\n--END DATA--\n");

            // Wrap up the Message
            //std::string l_messageContent(l_part + l_of + l_sha + l_begin + l_content + l_end);
            std::string l_messageContent(l_sha+l_content);

                // Take our final packed message and send it through our owner

            NetworkMail l_outboundMessage(message.getFrom(),
                                          message.getTo(),
                                          message.getSubject(),
                                          l_messageContent.data());

            m_owner->sendMail(l_outboundMessage);


        }







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