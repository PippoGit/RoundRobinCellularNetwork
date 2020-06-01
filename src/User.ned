package roundrobincellularnetwork;

simple User
{
    parameters:
        double lambda @unit(s) = default(0.5s);

        // Experimenting with signals
        @signal[tptUser*](type=double);
        @statisticTemplate[tptUserTemplate](record=mean,sum,vector);

        @signal[rspTimeUser*](type=simtime_t);
        @statisticTemplate[responseTimeUserTemplate](record=mean,vector);

        @signal[turnWaitingTimeUser*](type=simtime_t);
        @statisticTemplate[turnWaitingTimeUserTemplate](record=mean, vector);

        @signal[waitingTimeUser*](type=simtime_t);
        @statisticTemplate[waitingTimeUserTemplate](record=mean, vector);

        @signal[serviceTimeUser*](type=simtime_t);
        @statisticTemplate[serviceTimeUserTemplate](record=mean, vector);

        @signal[CQIUser*](type=long);
        @statisticTemplate[CQIUserTemplate](record=mean,sum);

        @signal[numRBsUser*](type=long);
        @statisticTemplate[numberRBsUserTemplate](record=mean,sum);

        @signal[numPktsUser*](type=long);
        @statisticTemplate[numberPktsUserTemplate](record=mean,sum);

        @signal[servedUser*](type=long);
        @statisticTemplate[servedUserTemplate](record=sum);

    gates:
        input in;
        output out;
}
