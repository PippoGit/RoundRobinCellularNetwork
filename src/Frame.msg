cplusplus {{
#include "ResourceBlock.h"
#include "constants.h"
}}

class noncobject ResourceBlock;

message Frame {
    ResourceBlock RBFrame[FRAME_SIZE];
    simtime_t sumWaitingTimes;
    simtime_t sumServiceTimes;
    int numPackets;
}
