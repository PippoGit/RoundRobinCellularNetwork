package roundrobincellularnetwork.simulations.RoundRobinSimulation;

import roundrobincellularnetwork.Antenna;
import roundrobincellularnetwork.Generatore;
import roundrobincellularnetwork.User;

network CellularNetwork
{
    parameters:
        int nUsers = default(10);
        double timeslot @unit(s) = default(0.5s);

        submodules:
        antenna: Antenna {
            // 
            @display("p=113,223");
        }
        user[nUsers]: User {
            @display("p=279,145");
        }
        generatore[nUsers]: Generatore {
            @display("p=55,119");
        }
    connections:
        for i=0..nUsers-1 {
            generatore[i].out-->antenna.pkt_in++;
            user[i].in <-- antenna.out++;
            user[i].out --> antenna.in++;
        }


}
