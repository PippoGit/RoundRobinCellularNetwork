package roundrobincellularnetwork;

simple User
{
	parameters:
    double lambda @unit(s)=default(0.5s);
      
    // Experimenting with signals
    @signal[TESTtptUser*](type=long);
    @statisticTemplate[tptUserTemplate](record=mean, sum);  

    @signal[TESTrspTimeUser*](type=simtime_t);
    @statisticTemplate[responseTimeUserTemplate](record=mean, vector); 
    
    @signal[TESTCQIUser*](type=long);
    @statisticTemplate[CQIUserTemplate](record=mean, sum);   
    
    @signal[TESTNumRBUser*](type=long);
    @statisticTemplate[numberRBsUserTemplate](record=mean, sum);

    @signal[TESTservedUser*](type=long);
    @statisticTemplate[servedUserTemplate](record=count);  
       		
	gates:
    input in;
    output out;
}
