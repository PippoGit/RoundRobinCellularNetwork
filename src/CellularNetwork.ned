
package roundrobincellularnetwork;
network CellularNetwork
{
    parameters:
        int nUsers;
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
            user[i].inData_p <-- antenna.outData_p[i];
        }


}
