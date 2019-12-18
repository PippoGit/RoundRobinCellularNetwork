package roundrobincellularnetwork;

simple Antenna
{
    parameters:
        double timeslot @unit(s) = default(0.5s);
    	bool isBinomial=default(false); // binomial or uniform.
    	
    	//signals

        @signal[throughput](type=double);
    	@statistic[throughput](source="throughput"; record=mean,vector);  
    	  
        @signal[responseTime](type=double);
    	@statistic[responseTime](source="responseTime"; record=mean,vector);    
       
        @signal[NumServedUser](type=double);
    	@statistic[NumServedUser](source="NumServedUser"; record=mean,vector);       
       
       @signal[tptUser*](type=double);
       @statisticTemplate[tptUserTemplate](record=vector);  
       
       
    gates:
 		// output out;   
 		input in[];
 		output out[];    
}
