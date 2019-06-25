#include "Hardware.h"

void exportHardwareConfig(struct HardwareConfig &hwConfig, const string configFileName)
{
    ofstream confFile(configFileName);
    confFile << "{ \"hwConfig\":" << endl << "    {";
    confFile << "\"mac_names\": [";
    
    int tmpInt;

    tmpInt = hwConfig.numMACUnits;
    for(char name = 'A'; name < 'A' + tmpInt - 1; name++)
    {
        confFile << "\"" << name << "\",";
    }
    confFile << "\"" << (char)('A' + tmpInt-1) << "\"]," << endl; 


    tmpInt = hwConfig.AUs.size() - hwConfig.numMACUnits;
    confFile << "\"div_names\": [";
    for(char name = 'A'; name < 'A' + tmpInt - 1; name++)
    {
        confFile << "\"" << name << "\",";
    }
    confFile << "\"" << (char)('A' + tmpInt-1) << "\"]," << endl; 

    confFile << "\"bram_names\": [";
    for(tmpInt = 0; tmpInt < hwConfig.bramNames.size()-1; tmpInt++)
    {
        confFile << "\"" << hwConfig.bramNames[tmpInt] << "\",";
    }
    confFile << "\"" << hwConfig.bramNames.back() << "\"]," << endl;

    confFile << "\"auMuxSelWidth\": " << hwConfig.AUMuxSelWidth << "," << endl;
    confFile << "\"bramMuxSelWidth\": " << hwConfig.BRAMMuxSelWidth << "," << endl;
    confFile << "\"bramAddrWidth\": " << hwConfig.BRAMAddressWidth << endl;
    confFile << "}" << endl;
    confFile << "}" << endl;


    confFile.close();
}