message Packet {
   //  int senderID;
    int receiverID;
    int serviceDemand;
    
    simtime_t startServiceTime;
    simtime_t startFrameTime;
    
    // Test
     simtime_t arrivalTime;     // inserted into the queue
     simtime_t servedTime;      // removed from the queue
     simtime_t frameTime;       // inserted into the frame    
}
