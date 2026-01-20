# Scripts

This directory contains the scripts for generating the datasets according to the methodology presented in the [paper](https://www.mdpi.com/1424-8220/25/19/6220). These scripts support data collection from the ESP32 CSI Web Collectin Tool, preprocessing, segmentation, and augmentation.

## Purpose

- Automate the process of dataset creation based on the collected data.
- Convert Raw CSI measurements into CSI amplitude for obtaining IMFs.
- IMF extraction.
- Prepare datasets for model training and testing.

## Usage

For working with the dataset provided:

1. Make sure that **CSI.py** file is in the same directory, as it contains basic functions for processing CSI.
2. Run **imf_generator.py**. It will obtain the intrinsic mode functions per subcarrier from each sample. These are saved into numpy arrays.
3. Once IMF numpy files have been generated, run **imf_sample_builder.py**. This will take the extracted mode functions and start combining them accordingly to generate synthetic samples. Numpy files generated have the shape (52, 8, 850), i.e., number of subcarriers, IMFs plus residue, and timesteps. The sum of all IMFs will constitute a complete synthetic sample. However, this is not done in this script as it can be desired to perform an analysis per IMF.
4. Run the **dataset_constructor.py** for generating training and test sets. These will be created for real and synthetic samples.
5. The files generated following this workflow are ready for model training. You can try different real/synthetic proportions for model training.

If you want to collect CSI, being the ESP32 connected to the computer and the tool configured to *Console* mode, set the COM port according to the Device Manager listing in the script XXXX and execute it.

## Notes

Check each script file individually for detailed usage instructions.
