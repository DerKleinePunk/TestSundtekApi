#include "/opt/include/mediaclient.h"
#include "deviceManager.hpp"
#include <stdio.h>
#include <string.h>

deviceManager::deviceManager() {
    _radioNode[0] = 0;
    _dabNode[0] = 0;
    _fmHandle = -1;
}

deviceManager::~deviceManager() {
    if(_fmHandle > -1) {
        net_close(_fmHandle);
    }
}

serialArray deviceManager::getSerial(){
    return _serial;
}

void deviceManager::setFmNode(char* radioNode) {
    strcpy(reinterpret_cast<char*>(_radioNode), radioNode);
}

void deviceManager::setDabNode(char* dabNode){
    strcpy(reinterpret_cast<char*>(_dabNode), dabNode);
}

void deviceManager::setDeviceId(uint8_t deviceId){
    _deviceId = deviceId;
}

void deviceManager::setSerial(serialArray value){
    memcpy(_serial, value, 100);
}

void deviceManager::setDeviceName(char* name){
    strcpy(reinterpret_cast<char*>(_deviceName), name);
}

bool deviceManager::IsFmDevice() {
    if(strlen(reinterpret_cast<char*>(_radioNode)) > 0){
        return true;
    }
    return false;
}

int deviceManager::getFmHandle() {
    return _fmHandle;
}

void deviceManager::setFmHandle(int handle) {
    _fmHandle = handle;
}

char* deviceManager::getFmNode() {
    return reinterpret_cast<char*>(_radioNode);
}

