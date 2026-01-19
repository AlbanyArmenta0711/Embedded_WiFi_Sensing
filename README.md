# The ESP32 CSI Web Collecting Tool with ESP32

This repository provides a complete ecosystem for Wi‑Fi Channel State Information (CSI) sensing using ESP32 devices. It includes:
- A **CSI collection tool** for ESP32 microcontrollers.
- A **human activity recognition (HAR)** and breathing‑monitoring dataset collected using the proposed tool.
- **Deep Learning models** trained on these datasets, optimized for deployment on ESP32 devices (external PSRAM is strongly recommended)

For questions or comments, feel free to contact us using the emails listed below, or open an Issue directly in this repository.

## What you Will Find
Each subdirectory contains its own detailed README.

### Repository Structure
- **Datasets/**
    Contains CSI measurement files captured with the ESP32 CSI Web Collecting Tool.
    These datasets support applications such as human activity recognition and breathing monitoring.
- **Embedded_Model/**
    Includes the models reported in the paper, provided in two formats:
  - **Keras** for Python-based experimentation.
  - **TensorFlow Lite** models for deployment using the [TensorFlow Lite Micro for Espressif Chipsets](https://github.com/espressif/esp-tflite-micro).
- **ESP32_CSI_Web_Collecting_Tool/** 
    Contains the full source code for the CSI collection tool, designed to be compiled and flashed using the [ESP-IDF SDK](https://www.espressif.com/en/products/sdks/esp-idf). The project was developed using the [ESP-IDF Visual Studio Code extension](https://github.com/espressif/vscode-esp-idf-extension/#quick-installation-guide).
- **Scripts/**
    Includes scripts used to preprocess raw CSI data and generate datasets as reported in the paper.

## Contact
**Jesus A. Armenta-Garcia**
Email: albany.armenta@uabc.edu.mx

**Felix F. Gonzalez-Navarro**
Email: fernando.gonzalez@uabc.edu.mx

## Citation
If you use this tool, dataset, or the models, please cite the following publication: 
Armenta-Garcia, J. A., Gonzalez-Navarro, F. F., Caro-Gutierrez, J., & Garcia-Reyes, C. I. (2025). Tools and Methods for Achieving Wi-Fi Sensing in Embedded Devices. Sensors, 25(19), 6220. https://doi.org/10.3390/s25196220

