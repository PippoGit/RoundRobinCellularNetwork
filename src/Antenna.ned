package roundrobincellularnetwork;

simple Antenna
{
    parameters:
        	   	
    	//signals
        @signal[throughput](type=long);
    	@statistic[throughput](source="throughput"; record=sum, stats, vector);  

        @signal[responseTime](type=simtime_t);
    	@statistic[responseTime](source="responseTime"; record=stats, vector);  
       
        @signal[numServedUser](type=long);
    	@statistic[numServedUser](source="numServedUser"; record=sum, stats, vector);       
       
       	@signal[numberRB](type=long);
    	@statistic[numberRB](source="numberRB"; record=sum, stats, vector);  

       	@signal[numberPkt](type=long);
    	@statistic[numberPkt](source="numberPkt"; record=sum, stats, vector);  
       
        @signal[numQ*](type=long);
        @statisticTemplate[numQTemplate](record=sum, stats, vector);

    gates:
 		// output out;   
 		input in[];
 		input pkt_in[];
 		
 		output out[];    
}
