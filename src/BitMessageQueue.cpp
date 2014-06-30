//
//  BitMessageQueue.cpp
//

#include "BitMessageQueue.h"

#include<boost/tokenizer.hpp>

namespace bmwrapper {
    
    bool BitMessageQueue::start() {
        
        if(m_stop){
            m_stop = false;
            m_thread = OT_THREAD(&BitMessageQueue::run, this);
            return true;
        }
        else{
            std::cerr << "BitMessageQueue is already running!" << std::endl;
            return false;
        }
    }
    
    
    bool BitMessageQueue::stop() {
        
        if(!m_stop){
            // Don't stop the thread in the middle of processing
            INSTANTIATE_MLOCK(m_processing);
            m_stop = true;
            m_thread.join();
            mlock.unlock();
            return true;
        }
        else{
            std::cerr << "BitMessageQueue is already stopped!" << std::endl;
            return false;
        }
    }
    
    
    bool BitMessageQueue::processing(){
        
        return OT_ATOMIC_ISTRUE(m_working);
        
    }
    
    
    void BitMessageQueue::addToQueue(OT_STD_FUNCTION(void()) command){
        
        MasterQueue.push(command);
        
    }
    
    
    
    
    int BitMessageQueue::queueSize(){
        
        return MasterQueue.size();
        
    }
    
    
    void BitMessageQueue::clearQueue(){
        
        MasterQueue.clear();
        
    }
    
    
    
    
    bool BitMessageQueue::parseNextMessage(){
        
        if(queueSize() == 0){
            return false;
        }
        
        // Don't let other functions interfere with our message parsing
        INSTANTIATE_MLOCK(m_processing);
        
        // Pull out our function to run
        OT_STD_FUNCTION(void()) message = MasterQueue.pop();
        
        message();
        
        mlock.unlock();
        
        // Let other functions know that we're done and they can continue.
        // This is primarily for when a request comes in to shut down the queue
        // While an action is in progress. This will notify our stop handler that it is safe
        // To shut down the thread.
        m_conditional.notify_one();
        
        return true;
    }
    
    
    BitMessageQueue::~BitMessageQueue(){
        
        try{
            stop();
        }
        
        catch(...){
            /* Placeholder */
        }
        
    }
    
}