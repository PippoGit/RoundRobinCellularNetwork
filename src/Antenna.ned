package roundrobincellularnetwork;

simple Antenna
{
    parameters:
        double timeslot @unit(s) = default(0.5s);
    	
    	
    	//signals

        @signal[waitTime](type=double);
    	@statistic[waitTime](source="waitTime"; record=mean,vector);  
    	  
        @signal[responseTime](type=double);
    	@statistic[responseTime](source="responseTime"; record=mean,vector);    
       
       
    gates:
 		// output out;   
 		input in[];
 		output out[];    
}
