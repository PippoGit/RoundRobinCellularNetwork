package roundrobincellularnetwork;

simple Antenna
{
    parameters:
        double timeslot @unit(s) = default(0.5s);
    	bool isBinomial=default(false); // binomial or uniform.
	    double lambda @unit(s)=default(0.5s);

	   	
    	//signals

        @signal[throughput](type=long);
    	@statistic[throughput](source="throughput"; record=mean,vector);  
    	  
        @signal[responseTime](type=double);
    	@statistic[responseTime](source="responseTime"; record=mean,vector);    
       
        @signal[NumServedUser](type=long);
    	@statistic[NumServedUser](source="NumServedUser"; record=mean,vector);       
       
       @signal[tptUser*](type=long);
       @statisticTemplate[tptUserTemplate](record=vector);  
       
       
    gates:
 		// output out;   
 		input in[];
 		output out[];    
}
