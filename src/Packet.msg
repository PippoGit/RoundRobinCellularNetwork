message Packet {
    int senderID;
    int receiverID;
    int serviceDemand;
    simtime_t arrivalTime;
    simtime_t startTime;
    simtime_t FrameTime;
    simtime_t EndService;
}
