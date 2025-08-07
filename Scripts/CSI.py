import numpy as np

class CSI:
    """
    CSI class
    
    Attributes:
        csi: (ndarray) CSI passed as a numpy array (samples x subcarriers)
        fs: (float) Sampling frequency used for capturing CSI
    """
    def __init__(self, csi, fs):
        self.csi = csi
        self.fs = fs 

    def __str__(self):
        return f"Shape of csi stored: {str(self.csi.shape)}, taken at {str(self.fs)} Hz"

    """
    Method for obtaining CSI amplitudes from defined CSI

    Returns: 
        ndarray: numpy array containing CSI amplitudes for each subcarrier across the number of samples
    """
    def get_amps(self):
        samples, cols = self.csi.shape
        scs = int(cols / 2)
        im = np.empty(shape = (samples, scs), dtype = float)
        re = np.empty(shape = (samples, scs), dtype = float)
        amps = np.empty(shape = (samples, scs), dtype = float)
        row_idx = 0
        re_idx = 0
        im_idx = 0 
        for csi_row in self.csi: 
            for comp_idx in range(cols):
                if comp_idx % 2 == 0: 
                    im[row_idx, im_idx] = csi_row[comp_idx]
                    im_idx += 1
                else:
                    re[row_idx,re_idx] = csi_row[comp_idx]
                    re_idx += 1
            amps[row_idx,:] = (np.sqrt(im[row_idx,:] ** 2 + re[row_idx,:] ** 2))
            row_idx += 1
            re_idx = 0
            im_idx = 0
        #replace nan with 0s before returning
        amps = np.nan_to_num(amps, nan = 0.0)
        return amps
    
    """ 
    Method for selecting the k most sensitive subcarriers based on variance selection method

    Args:
        data: (ndarray) numpy array containing CSI data to be selected
        k: (int) number of subcarriers to be selected

    Returns: 
        ndarray: (float) k most sensitive subcarriers
        ndarray: (int) indexes of the most sensitive subcarriers relative to data argument
    """
    def variance_selection(self, data, k):
        sensitive_idx = np.zeros(shape = (data.shape[0], 1), dtype = int)
        sensitive_sc = np.zeros(shape = (data.shape[0], k), dtype = float)

        data_normalized = self.normalize(data, 0,1)
       
        variance = np.var(data_normalized, axis = 0)
        sorted_idx = np.argsort(variance)[::-1]
        sensitive_idx = sorted_idx[:k]

        sensitive_sc = data[:, sensitive_idx]

        return sensitive_sc, sensitive_idx


    """
    Method for normalizing an array s in the range given by t_min and t_max

    Args:
        s: (ndarray) numpy array containing the data to be normalized
        t_min: (int) minimum desired value for s
        t_max: (int) maximum desired value for s

    Returns: 
        ndarray: normalized s in the interval [t_min, t_max]
    """
    def normalize(self, s, t_min, t_max):
        norm_arr = np.zeros(shape = s.shape)
        diff = t_max - t_min
        for sc in range (s.shape[1]):
            min_value = np.min(s[:, sc])
            max_value = np.max(s[:, sc])
            if min_value != max_value:
                norm_arr[:,sc] = diff * ((s[:,sc] - min_value)/(max_value - min_value)) + t_min
                norm_arr[:,sc] = np.nan_to_num(norm_arr[:,sc], nan=t_min)
        return norm_arr
