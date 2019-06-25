# Execution Steps for Scheduler #

## Pre-process Matrices ##
* Test matrices are stored in the directory "_$(PROJECT_DIR)_/Scheduler/test" in the MATLAB compatible format with the extension ___".mat"___
* Test matrices are pre-processed using MATLAB file "_$(PROJECT_DIR)_/Scheduler/matlabFiles/processMparseMatrices.m"
* MATLAB generates a separate binary files for each matrix

1. make cleanall
2. make buildPreprocessTests
3. make preprocessTests
4. make
5. ./main $(MAT_NAME)
6. 