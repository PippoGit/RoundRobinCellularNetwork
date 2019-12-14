package roundrobincellularnetwork;

simple Antenna
{
    parameters:
        double timeslot @unit(s) = default(0.5s);
    	
    	
    	//signals
    	@signal[waitTime](type=double);
    	@statistic[waitTime](source="waitTime"; record=mean,vector);    
       
    gates:
 		// output out;   
 		input in[];
 		output out[];    
}
