package roundrobincellularnetwork.simulations.AntennaTestSimulation;

import roundrobincellularnetwork.Antenna;


network AntennaTestSimulation
{
    submodules:
        antenna: Antenna {
            @display("p=169,91");
        }
}
