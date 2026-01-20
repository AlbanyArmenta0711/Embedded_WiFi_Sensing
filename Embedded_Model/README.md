# Embedded_Model

This directory contains the models reported in the [paper](https://www.mdpi.com/1424-8220/25/19/6220).

## Usage

- Keras files can be executed through Python Environment using the dataset provided. However, its use can be extended for other CSI data while it has the same format.
- TFLite files contain the quantized models, and they require TensorFlow Lite for Espressif Chipsets to be implemented in an ESP32. Please refer to TensorFlow Lite documentation for converting this files into hex arrays to be added as header files in your own projects.
