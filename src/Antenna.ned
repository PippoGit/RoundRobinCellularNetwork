package roundrobincellularnetwork;

simple Antenna
{
    parameters:
        	   	
    	//signals
        @signal[throughput](type=long);
    	@statistic[throughput](source="throughput"; record=mean, sum, vector);  
       
        @signal[numServedUser](type=long);
    	@statistic[numServedUser](source="numServedUser"; record=mean, sum, vector);       
       
       	@signal[numberRB](type=long);
    	@statistic[numberRB](source="numberRB"; record=mean, sum,vector);  

       	@signal[numberPkt](type=long);
    	@statistic[numberPkt](source="numberPkt"; record=mean, sum,vector);  
       
    gates:
 		// output out;   
 		input in[];
 		input pkt_in[];
 		
 		output out[];    
}
