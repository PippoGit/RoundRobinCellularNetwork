package roundrobincellularnetwork;

simple User
{
    parameters:
        double lambda @unit(s) = default(0.5s);

        // Experimenting with signals
        @signal[tptUser*](type=double);
        @statisticTemplate[tptUserTemplate](record=stats,histogram, vector);

        @signal[rspTimeUser*](type=simtime_t);
        @statisticTemplate[responseTimeUserTemplate](record=stats,histogram,vector);

        @signal[turnWaitingTimeUser*](type=simtime_t);
        @statisticTemplate[turnWaitingTimeUserTemplate](record=stats,histogram, vector);

        @signal[waitingTimeUser*](type=simtime_t);
        @statisticTemplate[waitingTimeUserTemplate](record=stats,histogram, vector);

        @signal[serviceTimeUser*](type=simtime_t);
        @statisticTemplate[serviceTimeUserTemplate](record=stats,histogram, vector);

        @signal[CQIUser*](type=long);
        @statisticTemplate[CQIUserTemplate](record=stats);

        @signal[numRBsUser*](type=long);
        @statisticTemplate[numberRBsUserTemplate](record=stats);

        @signal[numPktsUser*](type=long);
        @statisticTemplate[numberPktsUserTemplate](record=stats);

        @signal[servedUser*](type=long);
        @statisticTemplate[servedUserTemplate](record=sum);

    gates:
        input in;
        output out;
}
