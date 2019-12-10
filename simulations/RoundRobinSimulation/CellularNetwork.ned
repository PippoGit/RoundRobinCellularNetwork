package roundrobincellularnetwork.simulations.RoundRobinSimulation;

import roundrobincellularnetwork.Antenna;
import roundrobincellularnetwork.User;

network CellularNetwork
{
    parameters:
        int nUsers = default(10);
    submodules:
        antenna: Antenna {
            // 
            @display("p=113,223");
        }
        user[nUsers]: User {
            @display("p=279,145");
        }
    connections:
        for i=0..nUsers-1 {
            user[i].in <-- antenna.out++;
            user[i].out --> antenna.in++;
        }


}
