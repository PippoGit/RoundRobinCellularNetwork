# RoundRobinCellularNetwork
Omnett++ Project to analyze performance of a simulated cellular network

The purpose of this study is to provide a performance analysis of around-robin serving
algorithm applied to a cellular-network system.  More specifically for this scenario
it wasconsidered a transmitting antenna (theBTS) servingnmobile users.  
The transmissionis time slotted in a sense that in a given interval of time (atimeslot) the mobile usersare served. 
For each timeslot, the antenna receives from all the users a CQI (ChannelQuality Indicator), a measure of the quality of the channel 
(expressed as the number ofbytes the user can safety send without issues, also known asResource BlockorRB),
thenthe BTS makes a frame of 25 RBs and it propagates it to all the users just before theend of the timeslot or as soon as the frame is full.

Giada Anastasi
Stefano Guazzelli
Filippo Scotto
