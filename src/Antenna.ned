package roundrobincellularnetwork;

simple Antenna
{
    parameters:
        double timeslot @unit(s) = default(0.5s);
    	
    	
    	//signals
    	@signal[responseTime](type=double);
    	@statistic[responseTime](source="responseTime"; record=mean,vector);    
       
    gates:
 		// output out;   
 		input in[];
 		output out[];    
}
