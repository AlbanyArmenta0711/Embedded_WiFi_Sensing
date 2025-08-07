"""
Script that takes extracted IMFs from samples and builds synthetic samples
from the combination between them. 
"""

import sys
sys.path.append("../utils")
import os
#from PyEMD import EMD
import numpy as np
import matplotlib.pyplot as plt

LOAD_PATH = "./imfs_per_subject/"
IMFS_EXTRACTED = 7 #8, considering the residual 

subject_list = os.listdir(LOAD_PATH)
t = np.linspace(start = 0, stop = 850, num = 850)
for subject in subject_list:
    subject_activity_list = os.listdir(LOAD_PATH + subject + "/")
    for activity in subject_activity_list:
        files = os.listdir(LOAD_PATH + subject + "/" + activity + "/")
        for src_idx in range(len(files)):
            src_file_path = LOAD_PATH + subject + "/" + activity + "/" + files[src_idx]
            source_data = np.load(src_file_path)
            for dest_idx in range(src_idx + 1, len(files)):
                dest_file_path = LOAD_PATH + subject + "/" + activity + "/" + files[dest_idx]
                dest_data = np.load(LOAD_PATH + subject + "/" + activity + "/" + files[dest_idx])
                print("Generating artificial data from " + src_file_path + " and " + dest_file_path)
                artificial_sample = np.zeros(shape=(52,8,850))
                for imf_idx in range(IMFS_EXTRACTED + 1):
                    if imf_idx % 2 == 0: 
                        artificial_sample[:,imf_idx,:] = source_data[:, imf_idx, :]
                    else:
                        artificial_sample[:,imf_idx,:] = dest_data[:, imf_idx, :]

                save_path = "./thesis_environment/data_augmentation/synthetic_samples/" + activity + "/" + subject +"_" + str(src_idx) + "-" + str(dest_idx)
                np.save(save_path, artificial_sample)
                test_saved = np.load(save_path + ".npy")
                print("File " + save_path + " saved with shape " + str(test_saved.shape) + 
                    "\n Max: " + str(np.max(test_saved)) + "; Min: " + str(np.min(test_saved)))

                
