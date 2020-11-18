def blur(I):
    idx = np.arange(1,I.shape[1]-1)
    I[:,idx][idx,:] = (5*I[:,idx][idx,:] + I[:,idx-1][idx,:] + I[:,idx+1][idx,:] + I[:,idx][idx-1,:] + I[:,idx][idx+1,:])
    return I