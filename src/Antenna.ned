package roundrobincellularnetwork;

simple Antenna
{
    parameters:
        double timeslot @unit(s) = default(0.5s);
    	bool isBinomial=default(false); // binomial or uniform.
	    double lambda @unit(s)=default(0.5s);
		double successProbGroup1;
		double successProbGroup2;
		double successProbGroup3;
	   	
    	//signals

        @signal[throughput](type=long);
    	@statistic[throughput](source="throughput"; record=mean,vector);  
    	  
        @signal[responseTimeGlobal](type=simtime_t);
    	@statistic[responseTimeGlobal](source="responseTimeGlobal"; record=mean,vector);    
       
        @signal[NumServedUser](type=long);
    	@statistic[NumServedUser](source="NumServedUser"; record=mean,vector);       
       
       @signal[tptUser*](type=long);
       @statisticTemplate[tptUserTemplate](record=vector);  

       @signal[responseTime*](type=simtime_t);
       @statisticTemplate[responseTimeUserTemplate](record=mean,vector);         
       
    gates:
 		// output out;   
 		input in[];
 		output out[];    
}
