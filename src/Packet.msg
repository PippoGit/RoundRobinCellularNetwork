message Packet {
    int senderID;
    int receiverID;
    int serviceDemand;
    simtime_t arrivalTime;
    simtime_t startServiceTime;
    simtime_t startFrameTime;
}
