#pragma once
//
//  BitMessageQueue.h
//
#include <iostream>
#include "MsgQueue.h"

namespace bmwrapper {
    
    class BitMessage;
    
    class BitMessageQueue {
        
    public:
        
        BitMessageQueue() : m_stop(true), m_thread() { }
        ~BitMessageQueue();
        
        // Public Thread Managers
        bool start();
        bool stop();
        
        bool processing();
        // Queue Managers
        void addToQueue(OT_STD_FUNCTION(void()) command);
        
        int queueSize();
        void clearQueue();
        
    protected:
        
        OT_ATOMIC(m_stop);
        void run(){ while(!m_stop){parseNextMessage();} } // Obviously this will be our message parsing loop
        
    private:
        
        // Variables
        
        OT_THREAD m_thread;
        OT_MUTEX(m_processing);
        CONDITION_VARIABLE(m_conditional);
        OT_ATOMIC(m_working);
        
        MsgQueue<OT_STD_FUNCTION(void())> MasterQueue;
        
        // Functions
        
        bool parseNextMessage();
        
    };
    
}