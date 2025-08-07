"""
Script for generating the test set, training set, and synthetic training set
from numpy files
"""

import sys
sys.path.append("../utils")
import os
import numpy as np
import time
import matplotlib.pyplot as plt 

SYNTH_PATH = "./synthetic_samples/"
REAL_PATH = "./imfs_per_subject/"
DATASET_STORAGE_PATH = "./"
IMFS_EXTRACTED = 7 

#t = np.linspace(start = 0, stop = 850, num = 850)

#Get first the real samples 
dataset_FA = [] 
dataset_WA = [] 
dataset_SD = [] 
dataset_GU = []
dataset_LD = [] 

subject_file_list = os.listdir(REAL_PATH)
#70 (training) - 30 (testing)
#Remove last 6 subjects (30% of real samples)

"""
    Generate test set
"""
subjects_for_test = subject_file_list[len(subject_file_list) - 6:]
print(subjects_for_test)
for subject in subjects_for_test:
    subject_activity_list = os.listdir(REAL_PATH + subject + "/")
    for activity in subject_activity_list:
        files = os.listdir(REAL_PATH + subject + "/" + activity + "/")
        for file in files: 
            loaded = np.load(REAL_PATH + subject + "/" + activity + "/" + file)
            sample = np.zeros(shape = (52,850))
            for imf_idx in range(IMFS_EXTRACTED + 1):
                sample = sample + loaded[:, imf_idx, :]
            if activity == "WA":
                dataset_WA.append(sample.copy())
            elif activity == "FA":
                dataset_FA.append(sample.copy())
            elif activity == "SD":
                dataset_SD.append(sample.copy())
            elif activity == "LD":
                dataset_LD.append(sample.copy())
            elif activity == "GU":
                dataset_GU.append(sample.copy())

dataset_test_FA = np.array(dataset_FA)
dataset_test_GU = np.array(dataset_GU)
dataset_test_LD = np.array(dataset_LD)
dataset_test_SD = np.array(dataset_SD)
dataset_test_WA = np.array(dataset_WA)
dataset_test_len = len(dataset_FA) + len(dataset_GU) + len(dataset_LD) + len(dataset_SD) + len(dataset_WA)
y_tst = np.zeros(shape = (dataset_test_len,))
x_tst = np.zeros(shape = (dataset_test_len, 52, 850))


FA_end_idx = len(dataset_FA)
GU_end_idx = len(dataset_GU) + len(dataset_FA)
LD_end_idx = len(dataset_LD) + len(dataset_GU) + len(dataset_FA)
SD_end_idx = len(dataset_SD) + len(dataset_LD) + len(dataset_GU) + len(dataset_FA)
WA_end_idx = len(dataset_WA) + len(dataset_SD) + len(dataset_LD) + len(dataset_GU) + len(dataset_FA)
"""
    1 - FA
    2 - GU
    3 - LD
    4 - SD 
    5 - WA
"""
for idx in range(dataset_test_len):
    if idx < FA_end_idx:
        y_tst[idx] = 1
        x_tst[idx,:,:] = dataset_test_FA[idx, :, :]
    elif idx < GU_end_idx:
        y_tst[idx] = 2
        x_tst[idx,:,:] = dataset_test_GU[idx - FA_end_idx, :, :]
    elif idx < LD_end_idx:
        y_tst[idx] = 3
        x_tst[idx,:,:] = dataset_test_LD[idx - GU_end_idx, :, :]
    elif idx < SD_end_idx:
        y_tst[idx] = 4
        x_tst[idx,:,:] = dataset_test_SD[idx - LD_end_idx, :, :]
    elif idx < WA_end_idx:
        y_tst[idx] = 5
        x_tst[idx,:,:] = dataset_test_WA[idx - SD_end_idx, :, :]

print(y_tst)
print(x_tst)

np.save(DATASET_STORAGE_PATH + "x_tst", x_tst)
print("Test data saved as " + DATASET_STORAGE_PATH + "x_tst") 

np.save(DATASET_STORAGE_PATH + "y_tst", y_tst)
print("Test labels saved as " + DATASET_STORAGE_PATH + "y_tst") 



subject_file_list = subject_file_list[:len(subject_file_list) - 6]
print(subject_file_list)


for subject in subject_file_list: 
    subject_activity_list = os.listdir(REAL_PATH + subject + "/")
    for activity in subject_activity_list:
        files = os.listdir(REAL_PATH + subject + "/" + activity + "/")
        for file in files: 
            loaded = np.load(REAL_PATH + subject + "/" + activity + "/" + file)
            sample = np.zeros(shape = (52,850))
            for imf_idx in range(IMFS_EXTRACTED + 1):
                sample = sample + loaded[:, imf_idx, :]
            if activity == "WA":
                dataset_WA.append(sample.copy())
            elif activity == "FA":
                dataset_FA.append(sample.copy())
            elif activity == "SD":
                dataset_SD.append(sample.copy())
            elif activity == "LD":
                dataset_LD.append(sample.copy())
            elif activity == "GU":
                dataset_GU.append(sample.copy())


#    1 - FA
#    2 - GU
#    3 - LD
#    4 - SD 
#    5 - WA


dataset_FA = np.array(dataset_FA)
dataset_GU = np.array(dataset_GU)
dataset_LD = np.array(dataset_LD)
dataset_SD = np.array(dataset_SD)
dataset_WA = np.array(dataset_WA)
dataset_len = len(dataset_FA) + len(dataset_GU) + len(dataset_LD) + len(dataset_SD) + len(dataset_WA)
y_trn = np.zeros(shape = (dataset_len,))
x_trn = np.zeros(shape = (dataset_len, 52, 850))


FA_end_idx = len(dataset_FA)
GU_end_idx = len(dataset_GU) + len(dataset_FA)
LD_end_idx = len(dataset_LD) + len(dataset_GU) + len(dataset_FA)
SD_end_idx = len(dataset_SD) + len(dataset_LD) + len(dataset_GU) + len(dataset_FA)
WA_end_idx = len(dataset_WA) + len(dataset_SD) + len(dataset_LD) + len(dataset_GU) + len(dataset_FA)
for idx in range(dataset_len):
    if idx < FA_end_idx:
        y_trn[idx] = 1
        x_trn[idx,:,:] = dataset_FA[idx, :, :]
    elif idx < GU_end_idx:
        y_trn[idx] = 2
        x_trn[idx,:,:] = dataset_GU[idx - FA_end_idx, :, :]
    elif idx < LD_end_idx:
        y_trn[idx] = 3
        x_trn[idx,:,:] = dataset_LD[idx - GU_end_idx, :, :]
    elif idx < SD_end_idx:
        y_trn[idx] = 4
        x_trn[idx,:,:] = dataset_SD[idx - LD_end_idx, :, :]
    elif idx < WA_end_idx:
        y_trn[idx] = 5
        x_trn[idx,:,:] = dataset_WA[idx - SD_end_idx, :, :]

print(y_trn)
print(x_trn)

np.save(DATASET_STORAGE_PATH + "x_trn", x_trn)
print("Training data saved as " + DATASET_STORAGE_PATH + "x_trn") 

np.save(DATASET_STORAGE_PATH + "y_trn", y_trn)
print("Training labels saved as " + DATASET_STORAGE_PATH + "y_trn") 

#**********************************************************************
#    SYNTHETIC DATASET GENERATION
#**********************************************************************



synthetic_activity_list = os.listdir(SYNTH_PATH)
dataset_SYNTH_FA = []
dataset_SYNTH_WA = []
dataset_SYNTH_SD = []
dataset_SYNTH_LD = []
dataset_SYNTH_GU = []

for synth_activity in synthetic_activity_list: 
    activity_file_list = os.listdir(SYNTH_PATH + synth_activity + "/")
    for file in activity_file_list: 
        loaded = np.load(SYNTH_PATH + synth_activity + "/" + file)
        sample = np.zeros(shape = (52,850))
        for imf_idx in range(IMFS_EXTRACTED + 1):
            sample = sample + loaded[:, imf_idx, :]
        if synth_activity == "WA":
            dataset_SYNTH_WA.append(sample.copy())
        elif synth_activity == "FA":
            dataset_SYNTH_FA.append(sample.copy())
        elif synth_activity == "SD":
            dataset_SYNTH_SD.append(sample.copy())
        elif synth_activity == "LD":
            dataset_SYNTH_LD.append(sample.copy())
        elif synth_activity == "GU":
            dataset_SYNTH_GU.append(sample.copy())
            
dataset_FA = np.array(dataset_SYNTH_FA)
dataset_GU = np.array(dataset_SYNTH_GU)
dataset_LD = np.array(dataset_SYNTH_LD)
dataset_SD = np.array(dataset_SYNTH_SD)
dataset_WA = np.array(dataset_SYNTH_WA)
dataset_len = len(dataset_FA) + len(dataset_GU) + len(dataset_LD) + len(dataset_SD) + len(dataset_WA)
y_trn = np.zeros(shape = (dataset_len,))
x_trn = np.zeros(shape = (dataset_len, 52, 850))


FA_end_idx = len(dataset_FA)
GU_end_idx = len(dataset_GU) + len(dataset_FA)
LD_end_idx = len(dataset_LD) + len(dataset_GU) + len(dataset_FA)
SD_end_idx = len(dataset_SD) + len(dataset_LD) + len(dataset_GU) + len(dataset_FA)
WA_end_idx = len(dataset_WA) + len(dataset_SD) + len(dataset_LD) + len(dataset_GU) + len(dataset_FA)
for idx in range(dataset_len):
    if idx < FA_end_idx:
        y_trn[idx] = 1
        x_trn[idx,:,:] = dataset_FA[idx, :, :]
    elif idx < GU_end_idx:
        y_trn[idx] = 2
        x_trn[idx,:,:] = dataset_GU[idx - FA_end_idx, :, :]
    elif idx < LD_end_idx:
        y_trn[idx] = 3
        x_trn[idx,:,:] = dataset_LD[idx - GU_end_idx, :, :]
    elif idx < SD_end_idx:
        y_trn[idx] = 4
        x_trn[idx,:,:] = dataset_SD[idx - LD_end_idx, :, :]
    elif idx < WA_end_idx:
        y_trn[idx] = 5
        x_trn[idx,:,:] = dataset_WA[idx - SD_end_idx, :, :]

print(y_trn)
print(y_trn.shape)
print(x_trn)
print(x_trn.shape)

np.save(DATASET_STORAGE_PATH + "x_trn_synth", x_trn)
print("Training data saved as " + DATASET_STORAGE_PATH + "x_trn_synth") 

np.save(DATASET_STORAGE_PATH + "y_trn_synth", y_trn)
print("Training labels saved as " + DATASET_STORAGE_PATH + "y_trn_synth") 
        
