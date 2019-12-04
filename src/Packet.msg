packet Packet {
    int senderID;
    int receiverID;
    simtime_t serviceDemand;  //dimension of packet, random (Filippo: perchè questo è simtime_t e quello sotto un double? non sono entrambi time di qualcosa?)
    double creationTime;
}
