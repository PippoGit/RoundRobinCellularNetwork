package roundrobincellularnetwork;

simple User
{
	parameters:
	    double lambda @unit(s)=default(0.5s);
	     
	    // Experimenting with signals
	   @signal[TESTtptUser*](type=long);
       @statisticTemplate[tptUserTemplate](record=mean, vector);  

       @signal[TESTrspTimeUser*](type=simtime_t);
       @statisticTemplate[responseTimeUserTemplate](record=mean,vector); 
       
       @signal[TESTCQIUser*](type=long);
       @statisticTemplate[CQIUserTemplate](record=mean,vector);   
       
       @signal[TESTNumRBUser*](type=long);
       @statisticTemplate[numberRBsUserTemplate](record=mean,vector);

       @signal[TESTservedUser*](type=long);
       @statisticTemplate[servedUserTemplate](record=mean,vector);  
       		
	gates:
	    input in;
	    output out;
}
