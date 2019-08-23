//Todo move findig to cmake
#pragma once
#include "/opt/include/mediaclient.h"
#include <vector>
#include "deviceManager.hpp"

class sundtekApi
{
private:
    std::vector<deviceManager*> _devices;
    deviceManager* findFmDevice();
    int openFmDevice(deviceManager* dm);
    void ReadRDSData(int handle);
    int8_t GetAcsiiChar(int8_t zeichen);
public:
    sundtekApi();
    ~sundtekApi();
    int searchDevices();
    int searchFmStations();
};


