//
//  BitMessage.cpp
//

#include "BitMessage.h"
#include <json/json.h>
#include<boost/tokenizer.hpp>

#include <string>
#include <iostream>
#include <vector>
#include <utility>
#include <algorithm>
#include <functional>

#ifndef OT_USE_TR1
#include <chrono>
#include <thread>
#else
#include <boost/chrono.hpp>
#include <boost/thread.hpp>
#endif


namespace bmwrapper {
    
    
    BitMessage::BitMessage(std::string commstring) : NetworkModule(commstring, ModuleType::BITMESSAGE) {
        
        // Pass our config string to be parsed locally
        parseCommstring(commstring);
        
        // Start our XML-RPC interface
        m_xmllib = new XmlRPC(m_host, m_port, true, 10000);
        m_xmllib->setAuth(m_username, m_pass);
        
        // Runs to setup our counter
        checkAlive();
        
        // Not necessary because checkAlive will take care of this for us.
        // initializeUserData();
        
        // Thread Handler
        bm_queue = new BitMessageQueue();
        
        startQueue();   // Start Listener Thread
        
    }
    
    
    BitMessage::~BitMessage(){
        
        // Clean up Objects
        
        delete bm_queue;  // Queue will be stopped automatically upon deletion
        delete m_xmllib;
        
    }
    
    
    
    /*
     * Virtual Functions
     */
    
    
    bool BitMessage::accessible(){
        
        return m_serverAvailable;
        
    }
    
    
    bool BitMessage::pollStatus(){
        
        checkAlive();
        return accessible();
        
    }
    
    bool BitMessage::createAddress(std::string label){
        
        if(!accessible()){
            checkAlive();
            return false;
        }
        
        listAddresses();
        
        INSTANTIATE_MLOCK(m_localIdentitiesMutex);
        
        try{
            if(label == ""){
                std::cerr << "Will Not Create Address with Blank Label" << std::endl;
                mlock.unlock();
                return false;
            }
            
            
            for(unsigned int x = 0; x < m_localIdentities.size(); x++){
                if(m_localIdentities.at(x).getLabel().decoded() == label){
                    std::cerr << "Cannot Create Address: Label " << label << " already in Use" << std::endl;
                    mlock.unlock();
                    return false;
                }
                
            }
            
            OT_STD_FUNCTION(void()) firstCommand = OT_STD_BIND(&BitMessage::createRandomAddress, this, base64(label), false, 1, 1);
            bm_queue->addToQueue(firstCommand);
            
            checkLocalAddresses();
            
            mlock.unlock();
            return true;
        }
        catch(...){
            mlock.unlock();
            return false;
        }
    }
    
    
    bool BitMessage::createDeterministicAddress(std::string key, std::string label){
        
        if(!accessible()){
            checkAlive();
            return false;
        }
        
        listAddresses();
        
        INSTANTIATE_MLOCK(m_localIdentitiesMutex);
        
        try{
            
            if(label == ""){
                std::cerr << "Will Not Create Address with Blank Label" << std::endl;
                mlock.unlock();
                return false;
            }
            
            if(m_localIdentities.size() == 0){
                checkLocalAddresses();
                mlock.unlock();
                return false;
            }
            
            for(unsigned int x = 0; x < m_localIdentities.size(); x++){
                if(m_localIdentities.at(x).getLabel().decoded() == label){
                    std::cerr << "Cannot Create Address: Label " << label << " already in Use" << std::endl;
                    mlock.unlock();
                    return false;
                }
                
            }
            
            
            OT_STD_FUNCTION(void()) firstCommand = OT_STD_BIND(&BitMessage::createDeterministicAddresses, this, base64(key), 1, 0, 0, false, 1, 1);
            bm_queue->addToQueue(firstCommand);
            
            checkLocalAddresses();
            
            mlock.unlock();
            return true;
        }
        catch(...){
            mlock.unlock();
            return false;
        }
        
    }
    
    
    bool BitMessage::deleteLocalAddress(std::string address){
        
        if(!accessible()){
            checkAlive();
            return false;
        }
        
        try{
            
            OT_STD_FUNCTION(void()) firstCommand = OT_STD_BIND(&BitMessage::deleteAddress, this, address);
            bm_queue->addToQueue(firstCommand);
            
            checkLocalAddresses();
            return true;
            
        }
        catch(...){
            return false;
        }
    }
    
    
    bool BitMessage::addressAccessible(std::string address){
        
        if(!accessible()){
            checkAlive();
            return false;
        }
        
        INSTANTIATE_MLOCK(m_localIdentitiesMutex);
        
        for(unsigned int x = 0; x < m_localIdentities.size(); x++){
            if(m_localIdentities.at(x).getAddress() == address){
                mlock.unlock();
                return true;
            }
        }
        mlock.unlock();
        
        // If the address isn't acccessible, try and fetch the latest
        // Address book from the API server for another try later.
        // In most cases, you won't be checking for an address that you
        // Don't already know about from the addressbook.
        checkLocalAddresses();
        
        return false;
    }
    
    std::vector<std::pair<std::string, std::string> > BitMessage::getRemoteAddresses(){
        
        INSTANTIATE_MLOCK(m_localAddressBookMutex);
        
        std::vector<std::pair<std::string, std::string> > addresses;
        for(unsigned int x = 0; x < m_localAddressBook.size(); x++){
            std::pair<std::string, std::string> address(m_localAddressBook.at(x).getLabel().decoded(), m_localAddressBook.at(x).getAddress());
            addresses.push_back(address);
        }
        
        mlock.unlock();
        
        return addresses;
        
    }
    
    std::vector<std::pair<std::string, std::string> > BitMessage::getLocalAddresses(){
        
        INSTANTIATE_MLOCK(m_localIdentitiesMutex);
        
        std::vector<std::pair<std::string, std::string> > addresses;
        
        for(unsigned int x = 0; x < m_localIdentities.size(); x++){
            std::pair<std::string, std::string> address(m_localIdentities.at(x).getLabel().decoded(), m_localIdentities.at(x).getAddress());
            addresses.push_back(address);
        }
        
        mlock.unlock();
        
        return addresses;
    }
    
    
    bool BitMessage::checkLocalAddresses(){
        
        if(!accessible()){
            checkAlive();
            return false;
        }
        
        try{
            OT_STD_FUNCTION(void()) command = OT_STD_BIND(&BitMessage::listAddresses, this);
            bm_queue->addToQueue(command);
            return true;
        }
        catch(...){
            return false;
        }
    }
    
    
    bool BitMessage::checkRemoteAddresses(){
        
        if(!accessible()){
            checkAlive();
            return false;
        }
        
        try{
            OT_STD_FUNCTION(void()) command = OT_STD_BIND(&BitMessage::listAddressBookEntries, this);
            bm_queue->addToQueue(command);
            return true;
        }
        catch(...){
            return false;
        }
    }
    
    // Checks for new mail, returns true if there is new mail in the queue.
    bool BitMessage::checkMail(){
        
        if(!accessible()){
            checkAlive();
            return false;
        }
        
        try{
            OT_STD_FUNCTION(void()) getInboxMessages = OT_STD_BIND(&BitMessage::getAllInboxMessages, this);
            bm_queue->addToQueue(getInboxMessages);
            OT_STD_FUNCTION(void()) getSentMessages = OT_STD_BIND(&BitMessage::getAllSentMessages, this);
            bm_queue->addToQueue(getSentMessages);
            return true;
        }
        catch(...){
            return false;
            
        }
    }
    
    bool BitMessage::newMailExists(std::string address){
        
        if(!accessible()){
            checkAlive();
            return false;
        }
        
        if(m_localInbox.size() == 0){
            getAllInboxMessages(); // Blocking call, otherwise this may cause problems.
        }
        INSTANTIATE_MLOCK(m_localInboxMutex);
        
        if(address != ""){
            for(unsigned int x=0; x<m_localInbox.size(); x++){
                
                if(m_localInbox.at(x)->getTo() == address && m_localInbox.at(x)->getRead() == false){
                    mlock.unlock();
                    return true;
                }
            }
        }
        else{
            for(unsigned int x = 0; x < m_localInbox.size(); x++){
                if(m_localInbox.at(x)->getRead() == false){
                    mlock.unlock();
                    return true;
                }
            }
        }
        mlock.unlock();
        return false;
        
    }
    
    std::vector<_SharedPtr<NetworkMail> > BitMessage::getInbox(std::string address){
        
        if(m_localInbox.size() == 0){
            // Blocking call, otherwise this may cause problems.
            getAllInboxMessages();
        }
        
        INSTANTIATE_MLOCK(m_localInboxMutex);
        
        try{
            
            if(address != ""){
                std::vector<_SharedPtr<NetworkMail> > inboxForAddress;
                for(unsigned int x=0; x<m_localInbox.size(); x++){
                    if(m_localInbox.at(x)->getTo() == address)
                        inboxForAddress.push_back(m_localInbox.at(x));
                }
                mlock.unlock();
                return inboxForAddress;
            }
            else{
                mlock.unlock();
                return m_localInbox;
            }
        }
        catch(...){
            mlock.unlock();
            return std::vector<_SharedPtr<NetworkMail> >();
        }
        mlock.unlock();
        return std::vector<_SharedPtr<NetworkMail> >();
        
    }
    
    // Note that this is just a passthrough way of calling getInbox() to adhere to the interface.
    std::vector<_SharedPtr<NetworkMail> > BitMessage::getAllInboxes(){return getInbox("");}
    
    std::vector<_SharedPtr<NetworkMail> > BitMessage::getOutbox(std::string address){
        
        if(m_localOutbox.size() == 0){
            // Blocking call, otherwise this may cause problems.
            getAllSentMessages();
        }
        INSTANTIATE_MLOCK(m_localOutboxMutex);
        try{
            
            if(address != ""){
                std::vector<_SharedPtr<NetworkMail> > outboxForAddress;
                for(unsigned int x=0; x<m_localOutbox.size(); x++){
                    if(m_localOutbox.at(x)->getFrom() == address)
                        outboxForAddress.push_back(m_localOutbox.at(x));
                }
                mlock.unlock();
                return outboxForAddress;
            }
            else{
                mlock.unlock();
                return m_localOutbox;
            }
        }
        catch(...){
            mlock.unlock();
            return std::vector<_SharedPtr<NetworkMail> >();
        }
        mlock.unlock();
        return std::vector<_SharedPtr<NetworkMail> >();
        
    }
    
    std::vector<_SharedPtr<NetworkMail> > BitMessage::getAllOutboxes(){
        return getOutbox("");
    }
    
    std::vector<_SharedPtr<NetworkMail> > BitMessage::getUnreadMail(std::string address){
        
        std::vector<_SharedPtr<NetworkMail> > unreadMail;
        
        if(m_localInbox.size() == 0){
            // Blocking call, otherwise this may cause problems.
            getAllInboxMessages();
        }
        INSTANTIATE_MLOCK(m_localInboxMutex);
        try{
            
            if(address != ""){
                for(unsigned int x=0; x<m_localInbox.size(); x++){
                    if(m_localInbox.at(x)->getTo() == address && m_localInbox.at(x)->getRead() == false)
                        unreadMail.push_back(m_localInbox.at(x));
                }
                mlock.unlock();
                return unreadMail;
            }
            else{
                for(unsigned int x=0; x<m_localInbox.size(); x++){
                    if(m_localInbox.at(x)->getRead() == false)
                        unreadMail.push_back(m_localInbox.at(x));
                }
                mlock.unlock();
                return unreadMail;
            }
        }
        catch(...){
            mlock.unlock();
            return unreadMail;
        }
        mlock.unlock();
        return unreadMail;
    }
    
    // Note that this is just a passthrough way of calling getUnreadMail() to adhere to the interface.
    std::vector<_SharedPtr<NetworkMail> > BitMessage::getAllUnreadMail(){return getUnreadMail("");}
    
    bool BitMessage::deleteMessage(std::string messageID){
        
        if(!accessible()){
            checkAlive();
            return false;
        }
        
        if(m_localInbox.size() == 0){
            // Blocking call, otherwise this may cause problems.
            getAllInboxMessages();
        }
        INSTANTIATE_MLOCK(m_localInboxMutex);
        for(unsigned int x=0; x<m_localInbox.size(); x++){
            
            if(m_localInbox.at(x)->getMessageID() == messageID){
                m_localInbox.erase(m_localInbox.begin() + x);
            }
            try{
                OT_STD_FUNCTION(void()) command = OT_STD_BIND(&BitMessage::trashMessage, this, messageID);
                bm_queue->addToQueue(command);
                mlock.unlock();
                return true;
            }
            catch(...){
                mlock.unlock();
                return false;
            }
        }
        
        mlock.unlock();
        return false;
        
    }
    
    
    
    bool BitMessage::deleteOutMessage(std::string messageID){
        
        if(!accessible()){
            checkAlive();
            return false;
        }
        
        if(m_localOutbox.size() == 0){
            // Blocking call, otherwise this may cause problems.
            getAllSentMessages();
        }
        INSTANTIATE_MLOCK(m_localOutboxMutex);
        for(unsigned int x=0; x<m_localOutbox.size(); x++){
            
            if(m_localOutbox.at(x)->getMessageID() == messageID){
                m_localOutbox.erase(m_localOutbox.begin() + x);
            }
            try{
                OT_STD_FUNCTION(void()) command = OT_STD_BIND(&BitMessage::trashMessage, this, messageID);
                bm_queue->addToQueue(command);
                mlock.unlock();
                return true;
            }
            catch(...){
                mlock.unlock();
                return false;
            }
        }
        
        mlock.unlock();
        return false;
        
    }
    
    
    bool BitMessage::markRead(std::string messageID, bool read){
        
        if(!accessible()){
            checkAlive();
            return false;
        }
        
        if(m_localInbox.size() == 0){
            // Blocking call, otherwise this may cause problems.
            getAllInboxMessages();
        }
        
        INSTANTIATE_MLOCK(m_localInboxMutex);
        for(unsigned int x=0; x<m_localInbox.size(); x++){
            
            if(m_localInbox.at(x)->getMessageID() == messageID){
                m_localInbox.at(x)->setRead(read);
            }
            try{
                OT_STD_FUNCTION(void()) command = OT_STD_BIND(&BitMessage::getInboxMessageByID, this, messageID, read);
                bm_queue->addToQueue(command);
                mlock.unlock();
                return true;
            }
            catch(...){
                mlock.unlock();
                return false;
            }
        }
        mlock.unlock();
        return false;
    }
    
    
    bool BitMessage::sendMail(NetworkMail message){
        
        if(!accessible()){
            checkAlive();
            return false;
        }
        
        try{
            
            OT_STD_FUNCTION(void()) command = OT_STD_BIND(&BitMessage::sendMessage, this, message.getTo(), message.getFrom(), base64(message.getSubject()), base64(message.getMessage()), 2);
            bm_queue->addToQueue(command);
            return true;
        }
        catch(...){
            return false;
        }
        return false;
    }
    
    
    std::vector<std::pair<std::string,std::string> > BitMessage::getSubscriptions(){
        
        if(!accessible()){
            checkAlive();
            return std::vector<std::pair<std::string, std::string> >();
        }
        
        INSTANTIATE_MLOCK(m_localSubscriptionListMutex);
        
        if(m_localSubscriptionList.size() == 0){
            mlock.unlock();
            refreshSubscriptions();
            return std::vector<std::pair<std::string, std::string> >();
        }
        else{
            std::vector<std::pair<std::string, std::string> > subscriptionList;
            for(unsigned int x = 0; x < m_localSubscriptionList.size(); x++){
                std::pair<std::string, std::string> subscription(m_localSubscriptionList.at(x).getLabel().decoded(), m_localSubscriptionList.at(x).getAddress());
                subscriptionList.push_back(subscription);
            }
            mlock.unlock();
            return subscriptionList;
        }
        
        mlock.unlock();
        return std::vector<std::pair<std::string, std::string> >();
        
    }
    
    
    bool BitMessage::refreshSubscriptions(){
        
        if(!accessible()){
            checkAlive();
            return false;
        }
        
        try{
            OT_STD_FUNCTION(void()) command = OT_STD_BIND(&BitMessage::listSubscriptions, this);
            bm_queue->addToQueue(command);
            return true;
        }
        catch(...){
            return false;
        }
        return false;
    }
    
    
    
    /*
     * Message Queue Interaction
     */
    
    bool BitMessage::startQueue(){
        
        if(bm_queue != nullptr){
            // Will automatically check to determine if queue was already started to avoid issues.
            if(bm_queue->start())
                return true;
            else{
                std::cerr << "If you are seeing this, something went terribly wrong: bm_queue is a nullptr" << std::endl;
                return false;
            }
        }
        else
            return false;
    }
    
    bool BitMessage::stopQueue(){
        
        if(bm_queue != nullptr){
            // Will automatically check to determine if queue was already stopped.
            if(bm_queue->stop())
                return true;
            else
                return false;
        }
        else
            return false;
    }
    
    bool BitMessage::flushQueue(){
        try{
            bm_queue->clearQueue();
            return true;
        }
        catch(...){
            return false;
        }
    }
    
    int BitMessage::queueSize(){
        if(bm_queue != nullptr){
            return bm_queue->queueSize();
        }
        else{
            std::cerr << "Message Queue does not exist!" << std::endl;
            return 0;
        }
    }
    
    
    
    
    /*
     * Direct "Low-Level" API Functions
     */
    
    // Inbox Management
    
    
    void BitMessage::getAllInboxMessages(){
        
        Parameters params;
        std::vector<BitInboxMessage> inbox;
        
        XmlResponse result = m_xmllib->run("getAllInboxMessages", params);
        
        if(result.first == false){
            std::cerr << "Error: BitMessage getAllInboxMessages failed" << std::endl;
            setServerAlive(false);
            
        }
        else if(result.second.type() == xmlrpc_c::value::TYPE_STRING){
            std::size_t found;
            found=std::string(ValueString(result.second)).find("API Error");
            if(found!=std::string::npos){
                std::cerr << std::string(ValueString(result.second)) << std::endl;
            }
        }
        
        Json::Value root;
        Json::Reader reader;
        
        bool parsesuccess = reader.parse( ValueString(result.second), root );
        if ( !parsesuccess )
        {
            std::cerr  << "Failed to parse inbox" << std::endl; // << reader.getFormatedErrorMessages();
        }
        
        const Json::Value inboxMessages = root["inboxMessages"];
        for ( unsigned int index = 0; index < inboxMessages.size(); ++index ){
            
            // We need to sanitize our string, or else it will get cut off because of the newlines.
            std::string dirtyMessage = inboxMessages[index].get("message", "").asString();
            dirtyMessage.erase(std::remove(dirtyMessage.begin(), dirtyMessage.end(), '\n'), dirtyMessage.end());
            base64 cleanMessage(dirtyMessage, true);
            
            BitInboxMessage message(inboxMessages[index].get("msgid", "").asString(),
                                    inboxMessages[index].get("toAddress", "").asString(),
                                    inboxMessages[index].get("fromAddress", "").asString(),
                                    base64(inboxMessages[index].get("subject", "").asString(), true),
                                    cleanMessage, inboxMessages[index].get("encodingType", 0).asInt(),
                                    std::atoi(inboxMessages[index].get("receivedTime", 0).asString().c_str()),
                                    inboxMessages[index].get("read", false).asBool()
                                    );
            
            inbox.push_back(message);
            
        }
        
        // Lock so that we dont have a race condition.
        INSTANTIATE_MLOCK(m_localInboxMutex);
        
        // Populate our local inbox.
        m_localInbox.clear();
        m_localUnformattedInbox.clear();
        
        for(unsigned int x=0; x<inbox.size(); x++){
            
            m_localUnformattedInbox.push_back(inbox.at(x));
            
            _SharedPtr<NetworkMail> l_mail( new NetworkMail(inbox.at(x).getFromAddress(),
                                                            inbox.at(x).getToAddress(),
                                                            inbox.at(x).getSubject().decoded(),
                                                            inbox.at(x).getMessage().decoded(),
                                                            inbox.at(x).getRead(),
                                                            inbox.at(x).getMessageID(),
                                                            inbox.at(x).getReceivedTime())
                                           );
            
            m_localInbox.push_back(l_mail);
        }
        
        // New messages at the front
        std::reverse(m_localInbox.begin(), m_localInbox.end());
        
        // Release our lock so that others can access the inbox
        mlock.unlock();
        
    }
    
    
    void BitMessage::getInboxMessageByID(std::string msgID, bool setRead){
        
        Parameters params;
        
        params.push_back(ValueString(msgID));
        params.push_back(ValueBool(setRead));
        
        XmlResponse result = m_xmllib->run("getInboxMessageByID", params);
        
        if(result.first == false){
            std::cerr << "Error: BitMessage getInboxMessageByID failed" << std::endl;
            setServerAlive(false);
        }
        else if(result.second.type() == xmlrpc_c::value::TYPE_STRING){
            std::size_t found;
            found=std::string(ValueString(result.second)).find("API Error");
            if(found!=std::string::npos){
                std::cerr << std::string(ValueString(result.second)) << std::endl;
            }
        }
        
        
        Json::Value root;
        Json::Reader reader;
        
        bool parsesuccess = reader.parse( ValueString(result.second), root );
        if ( !parsesuccess )
        {
            std::cerr  << "Failed to parse inbox" << std::endl; //reader.getFormatedErrorMessages();
            BitInboxMessage message("", "", "", base64(""), base64(""), 0, 0, false);
        }
        
        const Json::Value inboxMessage = root["inboxMessage"];
        
        std::string dirtyMessage = inboxMessage[0u].get("message", "").asString();
        dirtyMessage.erase(std::remove(dirtyMessage.begin(), dirtyMessage.end(), '\n'), dirtyMessage.end());
        base64 cleanMessage(dirtyMessage, true);
        
        
        BitInboxMessage message(inboxMessage[0u].get("msgid", "").asString(),
                                inboxMessage[0u].get("toAddress", "").asString(),
                                inboxMessage[0u].get("fromAddress", "").asString(),
                                base64(inboxMessage[0u].get("subject", "").asString(), true),
                                cleanMessage, inboxMessage[0u].get("encodingType", 0).asInt(),
                                std::atoi(inboxMessage[0u].get("receivedTime", 0).asString().c_str()),
                                inboxMessage[0u].get("read", false).asBool()
                                );
        
    }
    
    
    void BitMessage::getAllSentMessages(){
        
        Parameters params;
        std::vector<BitSentMessage> outbox;
        
        XmlResponse result = m_xmllib->run("getAllSentMessages", params);
        
        if(result.first == false){
            std::cerr << "Error: BitMessage getAllSentMessages failed" << std::endl;
            setServerAlive(false);
        }
        else if(result.second.type() == xmlrpc_c::value::TYPE_STRING){
            std::size_t found;
            found=std::string(ValueString(result.second)).find("API Error");
            if(found!=std::string::npos){
                std::cerr << std::string(ValueString(result.second)) << std::endl;
            }
        }
        
        Json::Value root;
        Json::Reader reader;
        
        bool parsesuccess = reader.parse( ValueString(result.second), root );
        if ( !parsesuccess )
        {
            std::cerr  << "Failed to parse outbox" << std::endl; //reader.getFormatedErrorMessages();
        }
        
        const Json::Value sentMessages = root["sentMessages"];
        for ( unsigned int index = 0; index < sentMessages.size(); ++index ){
            
            // We need to sanitize our string, or else it will get cut off because of the newlines.
            std::string dirtyMessage = sentMessages[index].get("message", "").asString();
            dirtyMessage.erase(std::remove(dirtyMessage.begin(), dirtyMessage.end(), '\n'), dirtyMessage.end());
            base64 cleanMessage(dirtyMessage, true);
            
            BitSentMessage message(sentMessages[index].get("msgid", "").asString(),
                                   sentMessages[index].get("toAddress", "").asString(),
                                   sentMessages[index].get("fromAddress", "").asString(),
                                   base64(sentMessages[index].get("subject", "").asString(), true),
                                   cleanMessage,
                                   sentMessages[index].get("encodingType", 0).asInt(),
                                   sentMessages[index].get("lastActionTime", 0).asInt(),
                                   sentMessages[index].get("status", false).asString(),
                                   sentMessages[index].get("ackData", false).asString()
                                   );
            
            outbox.push_back(message);
        }
        
        // Lock so that we dont have a race condition.
        INSTANTIATE_MLOCK(m_localOutboxMutex);

        // Populate our local outbox.
        m_localOutbox.clear();
        m_localUnformattedOutbox.clear();
        for(unsigned int x=0; x<outbox.size(); x++){
            m_localUnformattedOutbox.push_back(outbox.at(x));
            _SharedPtr<NetworkMail> l_mail( new NetworkMail(outbox.at(x).getFromAddress(),
                                                            outbox.at(x).getToAddress(),
                                                            outbox.at(x).getSubject().decoded(),
                                                            outbox.at(x).getMessage().decoded(),
                                                            true,
                                                            outbox.at(x).getMessageID(),
                                                            0,
                                                            outbox.at(x).getLastActionTime())
                                           );
            
            m_localOutbox.push_back(l_mail);
        }
        
        // New messages at the front
        std::reverse(m_localOutbox.begin(), m_localOutbox.end());
        
        // Release our lock so that others can access the outbox
        mlock.unlock();
    }
    
    BitSentMessage BitMessage::getSentMessageByID(std::string msgID){
        
        Parameters params;
        
        params.push_back(ValueString(msgID));
        
        XmlResponse result = m_xmllib->run("getSentMessageByID", params);
        
        if(result.first == false){
            std::cerr << "Error: BitMessage getSentMessageByID failed" << std::endl;;
            setServerAlive(false);
            return BitSentMessage("", "", "", base64(""), base64(""), 0, 0, "", "");
        }
        else if(result.second.type() == xmlrpc_c::value::TYPE_STRING){
            std::size_t found;
            found=std::string(ValueString(result.second)).find("API Error");
            if(found!=std::string::npos){
                std::cerr << std::string(ValueString(result.second)) << std::endl;
                return BitSentMessage("", "", "", base64(""), base64(""), 0, 0, "", "");
            }
        }
        
        
        Json::Value root;
        Json::Reader reader;
        
        bool parsesuccess = reader.parse( ValueString(result.second), root );
        if ( !parsesuccess )
        {
            std::cerr  << "Failed to parse outbox" << std::endl; //reader.getFormatedErrorMessages();
            return BitSentMessage("", "", "", base64(""), base64(""), 0, 0, "", "");
        }
        
        const Json::Value sentMessage = root["sentMessage"];
        
        std::string dirtyMessage = sentMessage[0u].get("message", "").asString();
        dirtyMessage.erase(std::remove(dirtyMessage.begin(), dirtyMessage.end(), '\n'), dirtyMessage.end());
        base64 cleanMessage(dirtyMessage, true);
        
        
        BitSentMessage message(sentMessage[0u].get("msgid", "").asString(),
                               sentMessage[0u].get("toAddress", "").asString(),
                               sentMessage[0u].get("fromAddress", "").asString(),
                               base64(sentMessage[0u].get("subject", "").asString(), true),
                               cleanMessage,
                               sentMessage[0u].get("encodingType", 0).asInt(),
                               sentMessage[0u].get("lastActionTime", 0).asInt(),
                               sentMessage[0u].get("status", false).asString(),
                               sentMessage[0u].get("ackData", false).asString()
                               );
        
        return message;
        
    }
    
    
    BitSentMessage BitMessage::getSentMessageByAckData(std::string ackData){
        
        Parameters params;
        
        params.push_back(ValueString(ackData));
        
        XmlResponse result = m_xmllib->run("getSentMessageByAckData", params);
        
        if(result.first == false){
            std::cerr << "Error: BitMessage getSentMessageByAckData failed" << std::endl;
            setServerAlive(false);
            return BitSentMessage("", "", "", base64(""), base64(""), 0, 0, "", "");
        }
        else if(result.second.type() == xmlrpc_c::value::TYPE_STRING){
            std::size_t found;
            found=std::string(ValueString(result.second)).find("API Error");
            if(found!=std::string::npos){
                std::cerr << std::string(ValueString(result.second)) << std::endl;
                return BitSentMessage("", "", "", base64(""), base64(""), 0, 0, "", "");
            }
        }
        
        
        Json::Value root;
        Json::Reader reader;
        
        bool parsesuccess = reader.parse( ValueString(result.second), root );
        if ( !parsesuccess )
        {
            std::cerr  << "Failed to parse outbox" << std::endl; //reader.getFormatedErrorMessages();
            return BitSentMessage("", "", "", base64(""), base64(""), 0, 0, "", "");
        }
        
        const Json::Value sentMessage = root["sentMessage"];
        
        std::string dirtyMessage = sentMessage[0u].get("message", "").asString();
        dirtyMessage.erase(std::remove(dirtyMessage.begin(), dirtyMessage.end(), '\n'), dirtyMessage.end());
        base64 cleanMessage(dirtyMessage, true);
        
        
        BitSentMessage message(sentMessage[0u].get("msgid", "").asString(),
                               sentMessage[0u].get("toAddress", "").asString(),
                               sentMessage[0u].get("fromAddress", "").asString(),
                               base64(sentMessage[0u].get("subject", "").asString(), true),
                               cleanMessage,
                               sentMessage[0u].get("encodingType", 0).asInt(),
                               sentMessage[0u].get("lastActionTime", 0).asInt(),
                               sentMessage[0u].get("status", false).asString(),
                               sentMessage[0u].get("ackData", false).asString()
                               );
        
        return message;
        
    }
    
    
    std::vector<BitSentMessage> BitMessage::getSentMessagesBySender(std::string address){
        
        
        Parameters params;
        BitMessageOutbox outbox;
        
        params.push_back(ValueString(address));
        
        XmlResponse result = m_xmllib->run("getSentMessagesBySender", params);
        
        if(result.first == false){
            std::cerr << "Error: BitMessage getAllSentMessages failed" << std::endl;
            setServerAlive(false);
            return outbox;
        }
        else if(result.second.type() == xmlrpc_c::value::TYPE_STRING){
            std::size_t found;
            found=std::string(ValueString(result.second)).find("API Error");
            if(found!=std::string::npos){
                std::cerr << std::string(ValueString(result.second)) << std::endl;
                BitSentMessage message("", "", "", base64(""), base64(""), 0, 0, "", "");
                return outbox;
            }
        }
        
        Json::Value root;
        Json::Reader reader;
        
        const Json::Value sentMessages = root["sentMessages"];
        for ( unsigned int index = 0; index < sentMessages.size(); ++index ){  // Iterates over the sequence elements.
            
            // We need to sanitize our string, or else it will get cut off because of the newlines.
            std::string dirtyMessage = sentMessages[index].get("message", "").asString();
            dirtyMessage.erase(std::remove(dirtyMessage.begin(), dirtyMessage.end(), '\n'), dirtyMessage.end());
            base64 cleanMessage(dirtyMessage, true);
            
            BitSentMessage message(sentMessages[index].get("msgid", "").asString(),
                                   sentMessages[index].get("toAddress", "").asString(),
                                   sentMessages[index].get("fromAddress", "").asString(),
                                   base64(sentMessages[index].get("subject", "").asString(), true),
                                   cleanMessage,
                                   sentMessages[index].get("encodingType", 0).asInt(),
                                   std::atoi(sentMessages[index].get("lastActionTime", 0).asString().c_str()),
                                   sentMessages[index].get("status", false).asString(),
                                   sentMessages[index].get("ackData", false).asString()
                                   );
            
            outbox.push_back(message);
            
        }
        
        return outbox;
        
    }
    
    
    void BitMessage::trashMessage(std::string msgID){
        
        Parameters params;
        params.push_back(ValueString(msgID));
        
        XmlResponse result = m_xmllib->run("trashMessage", params);
        
        if(result.first == false){
            std::cerr << "Error: BitMessage trashMessage failed" << std::endl;
            setServerAlive(false);
        }
        else if(result.second.type() == xmlrpc_c::value::TYPE_STRING){
            std::size_t found;
            found=std::string(ValueString(result.second)).find("API Error");
            if(found!=std::string::npos){
                std::cerr << std::string(ValueString(result.second)) << std::endl;
            }
        }
        
    }
    
    
    bool BitMessage::trashSentMessageByAckData(std::string ackData){
        
        Parameters params;
        params.push_back(ValueString(ackData));
        
        XmlResponse result = m_xmllib->run("trashSentMessageByAckData", params);
        
        if(result.first == false){
            std::cerr << "Error: BitMessage trashSentMessageByAckData failed" << std::endl;
            setServerAlive(false);
            return false;
        }
        else if(result.second.type() == xmlrpc_c::value::TYPE_STRING){
            std::size_t found;
            found=std::string(ValueString(result.second)).find("API Error");
            if(found!=std::string::npos){
                std::cerr << std::string(ValueString(result.second)) << std::endl;
                return false;
            }
        }
        
        return true;
        
    }
    
    
    
    // Message Management
    
    void BitMessage::sendMessage(std::string fromAddress, std::string toAddress, base64 subject, base64 message, int encodingType){
        
        Parameters params;
        params.push_back(ValueString(fromAddress));
        params.push_back(ValueString(toAddress));
        params.push_back(ValueString(subject.encoded()));
        params.push_back(ValueString(message.encoded()));
        params.push_back(ValueInt(encodingType));
        
        
        XmlResponse result = m_xmllib->run("sendMessage", params);
        
        if(result.first == false){
            std::cerr << "Error: BitMessage sendMessage failed" << std::endl;
            setServerAlive(false);
        }
        else if(result.second.type() == xmlrpc_c::value::TYPE_STRING){
            std::size_t found;
            found=std::string(ValueString(result.second)).find("API Error");
            if(found!=std::string::npos){
                std::cerr << std::string(ValueString(result.second)) << std::endl;
            }
        }
    }
    
    
    std::string BitMessage::sendBroadcast(std::string fromAddress, base64 subject, base64 message, int encodingType){
        
        Parameters params;
        params.push_back(ValueString(fromAddress));
        params.push_back(ValueString(subject.encoded()));
        params.push_back(ValueString(message.encoded()));
        params.push_back(ValueInt(encodingType));
        
        
        XmlResponse result = m_xmllib->run("sendBroadcast", params);
        
        if(result.first == false){
            std::cerr << "Error: BitMessage sendBroadcast failed" << std::endl;
            setServerAlive(false);
            return "";
        }
        else if(result.second.type() == xmlrpc_c::value::TYPE_STRING){
            std::size_t found;
            found=std::string(ValueString(result.second)).find("API Error");
            if(found!=std::string::npos){
                std::cerr << std::string(ValueString(result.second)) << std::endl;
                return "";
            }
        }
        
        return std::string(ValueString(result.second));
        
    }
    
    
    
    // Subscription Management
    
    
    void BitMessage::listSubscriptions(){
        
        Parameters params;
        BitMessageSubscriptionList subscriptionList;
        
        XmlResponse result = m_xmllib->run("listSubscriptions", params);
        
        if(result.first == false){
            std::cerr << "Error: BitMessage listSubscriptions failed" << std::endl;
            setServerAlive(false);
        }
        else if(result.second.type() == xmlrpc_c::value::TYPE_STRING){
            std::size_t found;
            found=std::string(ValueString(result.second)).find("API Error");
            if(found!=std::string::npos){
                std::cerr << std::string(ValueString(result.second)) << std::endl;
            }
        }
        
        Json::Value root;
        Json::Reader reader;
        
        bool parsesuccess = reader.parse( ValueString(result.second), root );
        if ( !parsesuccess )
        {
            std::cerr  << "Failed to parse subscription list" << std::endl; //reader.getFormatedErrorMessages();
        }
        
        const Json::Value subscriptions = root["subscriptions"];
        for ( unsigned int index = 0; index < subscriptions.size(); ++index ){
            
            std::string dirtyLabel = subscriptions[index].get("label", "").asString();
            dirtyLabel.erase(std::remove(dirtyLabel.begin(), dirtyLabel.end(), '\n'), dirtyLabel.end());
            std::string cleanRipe(dirtyLabel);
            
            BitMessageSubscription subscription(subscriptions[index].get("address", "").asString(), subscriptions[index].get("enabled", "").asBool(), base64(cleanRipe, true));
            
            subscriptionList.push_back(subscription);
            
        }
        
        INSTANTIATE_MLOCK(m_localSubscriptionListMutex);
        m_localSubscriptionList = subscriptionList;
        mlock.unlock();
    }
    
    
    bool BitMessage::addSubscription(std::string address, base64 label){
        
        Parameters params;
        params.push_back(ValueString(address));
        params.push_back(ValueString(label.encoded()));
        
        
        XmlResponse result = m_xmllib->run("addSubscription", params);
        
        if(result.first == false){
            std::cerr << "Error: BitMessage createChan failed" << std::endl;
            setServerAlive(false);
            return false;
        }
        else if(result.second.type() == xmlrpc_c::value::TYPE_STRING){
            std::size_t found;
            found=std::string(ValueString(result.second)).find("API Error");
            if(found!=std::string::npos){
                std::cerr << std::string(ValueString(result.second)) << std::endl;
                return false;
            }
        }
        
        return true;
        
    }
    
    
    bool BitMessage::deleteSubscription(std::string address){
        
        Parameters params;
        params.push_back(ValueString(address));
        
        XmlResponse result = m_xmllib->run("deleteSubscription", params);
        
        if(result.first == false){
            std::cerr << "Error: BitMessage createChan failed" << std::endl;
            setServerAlive(false);
            return false;
        }
        else if(result.second.type() == xmlrpc_c::value::TYPE_STRING){
            std::size_t found;
            found=std::string(ValueString(result.second)).find("API Error");
            if(found!=std::string::npos){
                std::cerr << std::string(ValueString(result.second)) << std::endl;
                return false;
            }
        }
        
        return true;
        
    }
    
    
    
    // Channel Management
    
    
    std::string BitMessage::createChan(base64 password){
        
        Parameters params;
        params.push_back(ValueString(password.encoded()));
        
        XmlResponse result = m_xmllib->run("createChan", params);
        
        if(result.first == false){
            std::cerr << "Error: BitMessage createChan failed" << std::endl;
            setServerAlive(false);
            return "";
        }
        else if(result.second.type() == xmlrpc_c::value::TYPE_STRING){
            std::size_t found;
            found=std::string(ValueString(result.second)).find("API Error");
            if(found!=std::string::npos){
                std::cerr << std::string(ValueString(result.second)) << std::endl;
                return "";
            }
        }
        
        return std::string(ValueString(result.second));
        
    }
    
    
    bool BitMessage::joinChan(base64 password, std::string address){
        
        Parameters params;
        params.push_back(ValueString(password.encoded()));
        params.push_back(ValueString(address));
        
        XmlResponse result = m_xmllib->run("joinChan", params);
        
        if(result.first == false){
            std::cerr << "Error: BitMessage joinChan failed" << std::endl;
            setServerAlive(false);
            return false;
        }
        else if(result.second.type() == xmlrpc_c::value::TYPE_STRING){
            std::size_t found;
            found=std::string(ValueString(result.second)).find("API Error");
            if(found!=std::string::npos){
                std::cerr << std::string(ValueString(result.second)) << std::endl;
                return false;
            }
        }
        
        return true;
        
    }
    
    
    bool BitMessage::leaveChan(std::string address){
        
        Parameters params;
        params.push_back(ValueString(address));
        
        XmlResponse result = m_xmllib->run("leaveChan", params);
        
        if(result.first == false){
            std::cerr << "Error: BitMessage leaveChan failed" << std::endl;
            setServerAlive(false);
            return false;
        }
        else if(result.second.type() == xmlrpc_c::value::TYPE_STRING){
            std::size_t found;
            found=std::string(ValueString(result.second)).find("API Error");
            if(found!=std::string::npos){
                std::cerr << std::string(ValueString(result.second)) << std::endl;
                return false;
            }
        }
        
        return true;
    }
    
    
    
    
    // Address Management
    
    
    void BitMessage::listAddresses(){
        
        std::vector<xmlrpc_c::value> params;
        BitMessageIdentities responses;
        
        XmlResponse result = m_xmllib->run("listAddresses2", params);
        
        if(result.first == false){
            std::cerr << "Error: BitMessage listAddresses2 failed" << std::endl;
            setServerAlive(false);
        }
        else if(result.second.type() == xmlrpc_c::value::TYPE_STRING){
            std::size_t found;
            found=std::string(ValueString(result.second)).find("API Error");
            if(found!=std::string::npos){
                std::cerr << std::string(ValueString(result.second)) << std::endl;
            }
        }
        
        Json::Value root;
        Json::Reader reader;
        
        bool parsesuccess = reader.parse( ValueString(result.second), root );
        if ( !parsesuccess )
        {
            std::cerr << "Failed to parse configuration" << std::endl; //reader.getFormatedErrorMessages();
        }
        
        const Json::Value addresses = root["addresses"];
        for ( unsigned int index = 0; index < addresses.size(); ++index ){
            BitMessageIdentity entry(base64(addresses[index].get("label", "").asString(), true),
                                     addresses[index].get("address", "").asString(),
                                     addresses[index].get("stream", 0).asInt(),
                                     addresses[index].get("enabled", false).asBool(),
                                     addresses[index].get("chan", false).asBool()
                                     );
            
            responses.push_back(entry);
            
        }
        
        INSTANTIATE_MLOCK(m_localIdentitiesMutex);
        m_localIdentities = responses;
        mlock.unlock();
        
    }
    
    
    void BitMessage::createRandomAddress(base64 label, bool eighteenByteRipe, int totalDifficulty, int smallMessageDifficulty){
        
        Parameters params;
        params.push_back(ValueString(label.encoded()));
        params.push_back(ValueBool(eighteenByteRipe));
        params.push_back(ValueInt(totalDifficulty));
        params.push_back(ValueInt(smallMessageDifficulty));
        
        
        XmlResponse result = m_xmllib->run("createRandomAddress", params);
        
        if(result.first == false){
            std::cerr << "Error: BitMessage createRandomAddress failed" << std::endl;
            setServerAlive(false);
        }
        else if(result.second.type() == xmlrpc_c::value::TYPE_STRING){
            std::size_t found;
            found=std::string(ValueString(result.second)).find("API Error");
            if(found!=std::string::npos){
                std::cerr << std::string(ValueString(result.second)) << std::endl;
            }
        }
        
        INSTANTIATE_MLOCK(m_newestCreatedAddressMutex);
        newestCreatedAddress = std::string(ValueString(result.second));
        mlock.unlock();
        
    }
    
    
    void BitMessage::createDeterministicAddresses(base64 password, int numberOfAddresses, int addressVersionNumber, int streamNumber, bool eighteenByteRipe, int totalDifficulty, int smallMessageDifficulty){
        
        Parameters params;
        std::vector<BitMessageAddress> addressList;
        
        params.push_back(ValueString(password.encoded()));
        params.push_back(ValueInt(numberOfAddresses));
        params.push_back(ValueInt(addressVersionNumber));
        params.push_back(ValueInt(streamNumber));
        params.push_back(ValueBool(eighteenByteRipe));
        params.push_back(ValueInt(totalDifficulty));
        params.push_back(ValueInt(smallMessageDifficulty));
        
        
        XmlResponse result = m_xmllib->run("createDeterministicAddresses", params);
        
        if(result.first == false){
            std::cerr << "Error: BitMessage createDeterministicAddresses failed" << std::endl;
            setServerAlive(false);
        }
        else if(result.second.type() == xmlrpc_c::value::TYPE_STRING){
            std::size_t found;
            found=std::string(ValueString(result.second)).find("API Error");
            if(found!=std::string::npos){
                std::cerr << std::string(ValueString(result.second)) << std::endl;
            }
        }
        
        Json::Value root;
        Json::Reader reader;
        
        bool parsesuccess = reader.parse( ValueString(result.second), root );
        if ( !parsesuccess )
        {
            std::cerr  << "Failed to parse address list" << std::endl; //reader.getFormatedErrorMessages();
        }
        
        const Json::Value addresses = root["addresses"];
        for ( unsigned int index = 0; index < addresses.size(); ++index ){  // Iterates over the sequence elements.
            BitMessageAddress generatedAddress = addresses[index].asString();
            
            addressList.push_back(generatedAddress);
            
        }
        
        OT_STD_FUNCTION(void()) secondCommand = OT_STD_BIND(&BitMessage::listAddresses, this);
        bm_queue->addToQueue(secondCommand);
        
    }
    
    
    BitMessageAddress BitMessage::getDeterministicAddress(base64 password, int addressVersionNumber, int streamNumber){
        
        Parameters params;
        params.push_back(ValueString(password.encoded()));
        params.push_back(ValueInt(addressVersionNumber));
        params.push_back(ValueInt(streamNumber));
        
        
        XmlResponse result = m_xmllib->run("getDeterministicAddress", params);
        
        if(result.first == false){
            std::cerr << "Error: BitMessage getDeterministicAddress failed" << std::endl;
            setServerAlive(false);
            return "";
        }
        else if(result.second.type() == xmlrpc_c::value::TYPE_STRING){
            std::size_t found;
            found=std::string(ValueString(result.second)).find("API Error");
            if(found!=std::string::npos){
                std::cerr << std::string(ValueString(result.second)) << std::endl;
                return "";
            }
        }
        
        return std::string(ValueString(result.second));
        
    }
    
    
    void BitMessage::listAddressBookEntries(){
        
        Parameters params;
        
        BitMessageAddressBook addressBook;
        
        XmlResponse result = m_xmllib->run("listAddressBookEntries", params);
        
        if(result.first == false){
            std::cerr << "Error: BitMessage listAddressBookEntries failed" << std::endl;
            setServerAlive(false);
        }
        else if(result.second.type() == xmlrpc_c::value::TYPE_STRING){
            std::size_t found;
            found=std::string(ValueString(result.second)).find("API Error");
            if(found!=std::string::npos){
                std::cerr << std::string(ValueString(result.second)) << std::endl;
            }
        }
        
        Json::Value root;
        Json::Reader reader;
        
        bool parsesuccess = reader.parse( ValueString(result.second), root );
        if ( !parsesuccess )
        {
            std::cerr  << "Failed to parse address list" << std::endl; //reader.getFormatedErrorMessages();
        }
        
        const Json::Value addresses = root["addresses"];
        for ( unsigned int index = 0; index < addresses.size(); ++index ){
            
            std::string dirtyLabel = addresses[index].get("label", "").asString();
            dirtyLabel.erase(std::remove(dirtyLabel.begin(), dirtyLabel.end(), '\n'), dirtyLabel.end());
            std::string cleanRipe(dirtyLabel);
            
            BitMessageAddressBookEntry address(addresses[index].get("address", "").asString(), base64(cleanRipe, true));
            
            addressBook.push_back(address);
            
        }
        
        INSTANTIATE_MLOCK(m_localAddressBookMutex);
        m_localAddressBook = addressBook;
        mlock.unlock();
        
    }
    
    
    bool BitMessage::addAddressBookEntry(std::string address, base64 label){
        
        Parameters params;
        params.push_back(ValueString(address));
        params.push_back(ValueString(label.encoded()));
        
        XmlResponse result = m_xmllib->run("addAddressBookEntry", params);
        
        if(result.first == false){
            std::cerr << "Error: BitMessage addAddressBookEntry failed" << std::endl;
            setServerAlive(false);
            return false;
        }
        else if(result.second.type() == xmlrpc_c::value::TYPE_STRING){
            std::size_t found;
            found=std::string(ValueString(result.second)).find("API Error");
            if(found!=std::string::npos){
                std::cerr << std::string(ValueString(result.second)) << std::endl;
                return false;
            }
        }
        
        std::cerr << "BitMessage API Response: " << std::string(ValueString(result.second)) << std::endl;
        
        return true;
        
    }
    
    
    bool BitMessage::deleteAddressBookEntry(std::string address){
        
        Parameters params;
        params.push_back(ValueString(address));
        
        XmlResponse result = m_xmllib->run("deleteAddressBookEntry", params);
        
        if(result.first == false){
            std::cerr << "Error: BitMessage deleteAddressBookEntry failed" << std::endl;
            setServerAlive(false);
            return false;
        }
        else if(result.second.type() == xmlrpc_c::value::TYPE_STRING){
            std::size_t found;
            found=std::string(ValueString(result.second)).find("API Error");
            if(found!=std::string::npos){
                std::cerr << std::string(ValueString(result.second)) << std::endl;
                return false;
            }
        }
        
        std::cerr << "BitMessage API Response: " << std::string(ValueString(result.second)) << std::endl;
        
        return true;
        
    }
    
    
    void BitMessage::deleteAddress(std::string address){
        
        Parameters params;
        params.push_back(ValueString(address));
        
        XmlResponse result = m_xmllib->run("deleteAddress", params);
        
        if(result.first == false){
            std::cerr << "Error: BitMessage deleteAddress " << address << " failed" << std::endl;
            setServerAlive(false);
        }
        else if(result.second.type() == xmlrpc_c::value::TYPE_STRING){
            std::size_t found;
            found=std::string(ValueString(result.second)).find("API Error");
            if(found!=std::string::npos){
                std::cerr << std::string(ValueString(result.second)) << std::endl;
            }
        }
    }
    
    
    BitDecodedAddress BitMessage::decodeAddress(std::string address){
        
        Parameters params;
        
        params.push_back(ValueString(address));
        
        XmlResponse result = m_xmllib->run("decodeAddress", params);
        
        if(result.first == false){
            std::cerr << "Error Accessing BitMessage API" << std::endl;
            setServerAlive(false);
            return BitDecodedAddress("", 0, "", 0);
        }
        else if(result.second.type() == xmlrpc_c::value::TYPE_STRING){
            std::size_t found;
            found=std::string(ValueString(result.second)).find("API Error");
            if(found!=std::string::npos){
                std::cerr << std::string(ValueString(result.second)) << std::endl;
                return BitDecodedAddress("", 0, "", 0);
            }
        }
        
        
        Json::Value root;
        Json::Reader reader;
        
        bool parsesuccess = reader.parse( ValueString(result.second), root );
        if ( !parsesuccess )
        {
            std::cerr  << "Failed to parse configuration" << std::endl; //reader.getFormatedErrorMessages();
            return BitDecodedAddress("", 0, "", 0);
        }
        
        const Json::Value decodedAddress = root;
        
        std::string dirtyRipe = decodedAddress.get("ripe", "").asString();
        dirtyRipe.erase(std::remove(dirtyRipe.begin(), dirtyRipe.end(), '\n'), dirtyRipe.end());
        std::string cleanRipe(dirtyRipe);
        
        return BitDecodedAddress(decodedAddress.get("status", "").asString(), decodedAddress.get("addressVersion", "").asInt(), cleanRipe, decodedAddress.get("streamNumber", "").asInt());
        
    }
    
    
    
    
    
    // Other API Commands
    
    
    std::string BitMessage::helloWorld(std::string first, std::string second){
        
        Parameters params;
        params.push_back(ValueString(first));
        params.push_back(ValueString(second));
        
        XmlResponse result = m_xmllib->run("helloWorld", params);
        
        if(result.first == false){
            setServerAlive(false);
            return "";
        }
        else if(result.second.type() == xmlrpc_c::value::TYPE_STRING){
            std::size_t found;
            found=std::string(ValueString(result.second)).find("API Error");
            if(found!=std::string::npos){
                std::cerr << std::string(ValueString(result.second)) << std::endl;
                return "";
            }
        }
        
        return std::string(ValueString(result.second));
        
    }
    
    
    int BitMessage::add(int x, int y){
        
        Parameters params;
        params.push_back(ValueInt(x));
        params.push_back(ValueInt(y));
        
        XmlResponse result = m_xmllib->run("add", params);
        
        if(result.first == false){
            std::cerr << "Error: BitMessage add failed" << std::endl;
            setServerAlive(false);
            return -1;
        }
        else if(result.second.type() == xmlrpc_c::value::TYPE_STRING){
            std::size_t found;
            found=std::string(ValueString(result.second)).find("API Error");
            if(found!=std::string::npos){
                std::cerr << std::string(ValueString(result.second)) << std::endl;
                return -1;
            }
            std::cerr << "Unexpected Response to API Command: add" << std::endl;
            std::cerr << std::string(ValueString(result.second)) << std::endl;
            return -1;
        }
        else{
            return ValueInt(result.second);
        }
        
    }
    
    
    std::string BitMessage::getStatus(std::string ackData){
        
        Parameters params;
        
        params.push_back(ValueString(ackData));
        
        XmlResponse result = m_xmllib->run("getStatus", params);
        
        if(result.first == false){
            std::cerr << "Error Accessing BitMessage API" << std::endl;
            setServerAlive(false);
            return "";
        }
        else if(result.second.type() == xmlrpc_c::value::TYPE_STRING){
            std::size_t found;
            found=std::string(ValueString(result.second)).find("API Error");
            if(found!=std::string::npos){
                std::cerr << std::string(ValueString(result.second)) << std::endl;
                return "";
            }
        }
        
        return std::string(ValueString(result.second));
        
    }
    
    
    void BitMessage::setServerAlive(bool alive){
        
        if(alive){
            NetCounter::setAlive();
            m_serverAvailable = true;
        }
        else{
            NetCounter::dead();
            m_serverAvailable = false;
        }
        
    }
    
    void BitMessage::checkAlive(){
        
        if(helloWorld("Check","Alive") != "Check-Alive"){
            std::cerr << "Error: BitMessage API service is inaccessible" << std::endl;
            setServerAlive(false);
        }
        else{
            std::cerr << "BitMessage API Service is now accessible" << std::endl;
            setServerAlive(true);
            initializeUserData();
        }
        
    }
    
    void BitMessage::parseCommstring(std::string commstring){
        
        std::vector<std::string> parsedList;
        boost::tokenizer<boost::escaped_list_separator<char> > tokens(commstring);
        
        for(boost::tokenizer<boost::escaped_list_separator<char> >::iterator it=tokens.begin(); it!=tokens.end();++it){
            parsedList.push_back(*it);
        }
        
        if(parsedList.size() > 0)
            m_host = parsedList.at(0);
        else
            m_host = "localhost";
        
        if(parsedList.size() > 1)
            m_port = std::atoi(parsedList.at(1).c_str());
        else
            m_port = 8442;
        
        if(parsedList.size() > 2)
            m_username = parsedList.at(2);
        else
            m_username = "defaultuser";
        
        if(parsedList.size() > 3)
            m_pass = parsedList.at(3);
        else
            m_pass = "defaultpass";
        
    }
    
    void BitMessage::initializeUserData(){
        
        listAddresses(); // Populates Local Owned Addresses.
        listAddressBookEntries();  // Populates address book data.
        getAllInboxMessages(); // Populates local Inbox object.
        getAllSentMessages(); // Populates local Outbox (sent messages) object.
        listSubscriptions(); // Populates local subscriptions list.
        
    }
    
}
