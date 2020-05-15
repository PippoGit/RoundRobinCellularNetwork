package roundrobincellularnetwork;

simple User
{
	parameters:
	    double lambda @unit(s)=default(0.5s);
	    bool isBinomial=default(false); // binomial or uniform.
		double successProbGroup1;
		double successProbGroup2;
		double successProbGroup3;

 		
	gates:
	    input in;
	    output out;
}
