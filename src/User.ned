package roundrobincellularnetwork;

simple User
{
	parameters:
	    double lambda @unit(s)=default(0.5s);
	    

 		
	gates:
	    input in;
	    output out;
}
