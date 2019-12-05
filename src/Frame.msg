cplusplus {{
#include "ResourceBlock.h"
typedef ResourceBlock* ResourceBlockPointer;
}}
class noncobject ResourceBlock;

message Frame {
    int dimensionFrame = 25;
    ResourceBlockPointer RBFrame[dimensionFrame];
    int last;
}
