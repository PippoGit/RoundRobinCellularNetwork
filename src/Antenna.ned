package roundrobincellularnetwork;

simple Antenna
{
    parameters:
        	   	
    	//signals
        @signal[throughput](type=long);
    	@statistic[throughput](source="throughput"; record=mean,vector);  
       
        @signal[numServedUser](type=long);
    	@statistic[numServedUser](source="numServedUser"; record=mean,vector);       
       
       	@signal[numberRB](type=long);
    	@statistic[numberRB](source="numberRB"; record=mean,vector);  

       	@signal[numberPkt](type=long);
    	@statistic[numberPkt](source="numberPkt"; record=mean,vector);  
       
    gates:
 		// output out;   
 		input in[];
 		input pkt_in[];
 		
 		output out[];    
}
