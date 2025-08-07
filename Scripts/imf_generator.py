"""
Script for extracting the IMF through EMD from CSI amplitude samples. 
It will create numpy files with the imfs in the imfs_per_subject folder.
imf_sample_builder.py script needs to be executed next for generating the
synthetic numpy files. 
"""

import sys
sys.path.append("../utils")
import os
from PyEMD import EEMD
#from PyEMD import EMD
import numpy as np
import matplotlib.pyplot as plt
import pandas as pd
import time
from CSI import CSI

CLASSES = ["WA"]
REMAINING_SUBJECTS = ["s17", "s19", "s21", "s22", "s23", "s24", "s2", "s3", "s4", "s5", "s6", "s7","s8","s9"]
IMFS_TO_EXTRACT = 7 #last IMFs shown is the residual
K_SUBCARRIERS = 10 #number of sensitive subcarriers to use 
TRUNCATE_SAMPLES = 850
SAVE_PATH = "./imfs_per_subject/"
USED_SUBCARRIERS = 52

activity_dataset = np.array([])

t = np.linspace(start = 0, stop = TRUNCATE_SAMPLES, num = TRUNCATE_SAMPLES)

for idx, activity in enumerate(CLASSES):
    activity_path = "../Datasets/RAW/" + activity +"/"
    subjects_list = os.listdir(activity_path)

    for subject in REMAINING_SUBJECTS:
        print("Changing to subject " + subject)
        file_list = os.listdir(activity_path + subject)
        file_idx = 0 
        for file in file_list: 
            print("opening " + activity_path + subject + "/" + file)
            csi_data = pd.read_csv(activity_path + subject + "/" + file, sep = ",", header = None).to_numpy()
            #Erase timestamp column
            csi_data = np.delete(csi_data, 0, 1)
            #Truncate to TRUNCATE_SAMPLES rows
            csi_data = csi_data[0:TRUNCATE_SAMPLES,:]
            csi_obj = CSI(csi = csi_data, fs = 50)
            csi_amps = csi_obj.get_amps()
            nulls = [0, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37] 
            #exclude null subcarriers from selection
            csi_amps = np.delete(csi_amps, nulls, 1)
            #keep only the 10 most sensitive
            #s, idx = csi_obj.variance_selection(csi_amps, K_SUBCARRIERS)
            eemd = EEMD(trials = 100) 
            eemd.noise_seed(5)
            file_observation = np.zeros(shape = (USED_SUBCARRIERS, IMFS_TO_EXTRACT + 1, TRUNCATE_SAMPLES), dtype = float)
            imfs_from_file = np.zeros(shape = (IMFS_TO_EXTRACT + 1, TRUNCATE_SAMPLES), dtype = float) #includes the residual
            for sc_index in range(USED_SUBCARRIERS):
                IMF = eemd.eemd(S = csi_amps[:,sc_index], T = t, max_imf= IMFS_TO_EXTRACT) 
                for n, imf in enumerate(IMF):
                    imfs_from_file[n,:] = np.array(imf)
                file_observation[sc_index, :, :] = imfs_from_file
                print(" = Completed for sc: " + str(sc_index) + "=")
            print("saving imfs from file " + file + "for subject " + subject)
            if not os.path.exists(SAVE_PATH + subject + "/" + activity):
                os.makedirs(SAVE_PATH + subject + "/" + activity)
            npy_save_path = SAVE_PATH + subject + "/" + activity + "/" + str(file_idx)
            file_idx += 1
            np.save(npy_save_path, file_observation)
            #test if saved
            test_saved = np.load(npy_save_path + ".npy")
            print("File " + npy_save_path + " saved with shape " + str(test_saved.shape) + 
                  "\n Max: " + str(np.max(test_saved)) + "; Min: " + str(np.min(test_saved)))
            