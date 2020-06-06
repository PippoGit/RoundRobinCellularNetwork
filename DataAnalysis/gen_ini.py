NUM_RNGS = 4

def gen_rng_association(n):
    
    txt = "num-rngs = " + str(n*NUM_RNGS) + "\n"
    txt += "**.nUsers    = " + str(n) + "\n"															

    for user in range(0, n):
        txt += "**.generatore[" + str(user) + "].rng-0  = " + str(user*NUM_RNGS + 0) + "\n"
        txt += "**.generatore[" + str(user) + "].rng-1  = " + str(user*NUM_RNGS + 1) + "\n"
        txt += "**.user[" + str(user) + "].rng-0        = " + str(user*NUM_RNGS + 2) + "\n"
        txt += "**.user[" + str(user) + "].rng-1        = " + str(user*NUM_RNGS + 3) + "\n"

    return txt
    
