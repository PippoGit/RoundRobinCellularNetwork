package roundrobincellularnetwork;

simple Antenna
{
    parameters:
        	   	
    	//signals
        @signal[throughput](type=long);
    	@statistic[throughput](source="throughput"; record=stats, vector);  

        @signal[responseTime](type=simtime_t);
    	@statistic[responseTime](source="responseTime"; record=stats, vector);  
       
        @signal[numServedUser](type=long);
    	@statistic[numServedUser](source="numServedUser"; record=stats, vector);       
       
       	@signal[numberRB](type=long);
    	@statistic[numberRB](source="numberRB"; record=stats, vector);  

       	@signal[numberPkt](type=long);
    	@statistic[numberPkt](source="numberPkt"; record=stats, vector);  
		
		@signal[numberPktInService](type=long);
    	@statistic[numberPktInService](source="numberPktInService"; record=stats, vector);  
       
        @signal[numQ*](type=long);
        @statisticTemplate[numQTemplate](record=stats, vector);

    gates:
 		// output out;   
 		input in[];
 		input pkt_in[];
 		
 		output out[];    
}
