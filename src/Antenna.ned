package roundrobincellularnetwork;

simple Antenna
{
    parameters:
        double timeslot @unit(s) = default(5s);
        
    gates:
 		// output out;   
 		input in[];
 		output out[];    
}
