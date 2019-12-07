package roundrobincellularnetwork.simulations.AntennaTestSimulation;

import roundrobincellularnetwork.Antenna;
import roundrobincellularnetwork.User;


network AntennaTestSimulation
{
    submodules:
        antenna: Antenna {
            @display("p=169,91");
        }
        user2: User {
            @display("p=79,91");
        }
        user1: User {
            @display("p=169,28");
        }
    connections:
        user1.out --> antenna.in++;
        user2.out --> antenna.in++;
        antenna.out++ --> user1.in;
        antenna.out++ --> user2.in;
}
