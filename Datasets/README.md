# Datasets

This directory contains the CSI measurement files collected using the ESP32 CSI Web Collecting tool. These datasets are intended for data analysis, data preprocessing, model training and evaluation for Wi-Fi sensing applications.

## Content

Two subdirectories can be found in this directory:

- 📁**Activity/**  
    Raw CSI measurements per activity and subject involved in CSI data collecting. The activities to which CSI for every subject were collected are:

  - **Fall**, abbreviated as *FA*.
  - **Get Up**, abbreviated as *GU*.
  - **Lie Down**, abbreviated as *LD*.
  - **Sit Down**, abbreviated as *SD*.
  - **Walk**, abbreviated as *WA*.
  
  The first column for each CSV file refers to a timestamp assigned by the microcontroller, followed by complex pairs, alternating between imaginary and real parts for each of the 64 subcarriers.  
  For more information related to how and where activities were performed please refer to the [paper](https://www.mdpi.com/1424-8220/25/19/6220).
- 📁**Breathing/**  
    Raw CSI measurements per breathing rate and subject. Data collected correspond to three different breathing rates: 10 Breaths per minute (BPM), 15 BPM, and 20 BPM, which files are structured in the corresponding subdirectories.  
    It is important to notice that breathing data have not been used nor tested previously. Thus, feel free to experiment with these for Wi-Fi sensing for breathing monitoring.

Numpy files *x_trn.npy* and *y_trn.npy* correspond to CSI amplitude of the 64 subcarriers calculated from Raw CSI files contained in the **Activity/** directory

## Usage
