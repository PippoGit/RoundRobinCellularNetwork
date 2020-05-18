package roundrobincellularnetwork;

simple Antenna
{
    parameters:
        	   	
    	//signals
        @signal[throughput](type=long);
    	@statistic[throughput](source="throughput"; record=mean,vector);  
       
        @signal[NumServedUser](type=long);
    	@statistic[NumServedUser](source="NumServedUser"; record=mean,vector);       
       
       	@signal[numberRB](type=long);
    	@statistic[numberRB](source="numberRB"; record=mean,vector);  

       
    gates:
 		// output out;   
 		input in[];
 		input pkt_in[];
 		
 		output out[];    
}
