#pragma once
#include <inttypes.h>

typedef uint8_t(&serialArray)[100];

class deviceManager
{
private:
    uint8_t _serial[100];
    char* _radioNode[50];
    char* _dabNode[50];
    char* _deviceName[100];
    uint8_t _deviceId;
    int _fmHandle;
    bool _canRDS;
public:
    deviceManager();
    ~deviceManager();

    serialArray getSerial();
    void setFmNode(char* radioNode);
    void setDabNode(char* dabNode);
    void setDeviceId(uint8_t deviceId);
    void setSerial(serialArray value);
    void setDeviceName(char* name);
    void setCanRDS();
    bool IsFmDevice();
    int getFmHandle();
    void setFmHandle(int handle);
    char* getFmNode();
    bool getCanRDS();
};
