#include <iostream>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include "sundtekApi.hpp"

deviceManager* sundtekApi::findFmDevice() {
    deviceManager *dm=nullptr;
    for(auto iter=_devices.begin();iter!=_devices.end();++iter) {
        dm=(*iter);
        if (dm->IsFmDevice()) {
            break;
        }
    }
    return dm;
}

int sundtekApi::openFmDevice(deviceManager* dm) {
    if(dm->getFmHandle() > 0){
        return 0;
    }

    auto handle = net_open(dm->getFmNode(), O_RDWR);
    if(handle >= 0) {
        dm->setFmHandle(handle);
        return 0;
    }

    return handle;
}

sundtekApi::sundtekApi()
{
}

sundtekApi::~sundtekApi() {
    for(auto iter=_devices.begin();iter!=_devices.end();++iter) {
        auto dm = (*iter);
        delete dm;
    }
    _devices.clear();
}

int sundtekApi::searchDevices(){
    int i=0;
    int d=0;
    int fd;
    bool found;
    struct media_device_enum *device;
    deviceManager *dm=nullptr;

    fd = net_connect(0);
    if (fd<0) {
        std::cout << "connect failed" << std::endl;
        return fd;
    }

    while((device=net_device_enum(fd, &i, d))!=0) {
        do {
            std::cout << device->devicename << " ";
            
            found = false;
            for(auto iter=_devices.begin();iter!=_devices.end();++iter) {
                dm=(*iter);
                if (dm->getSerial() == device->serial) {
                    found=true;
                    break;
                }
            }

            if (found) {
                free(device);
                continue;
            }

            if ((device->capabilities & MEDIA_RADIO) ||
                    (device->capabilities & MEDIA_DAB)) {
                dm = new deviceManager();
                std::cout << "radio device found" << std::endl;
            } else {
                dm = nullptr;
                std::cout << "device found but no radio" << std::endl;
            }

            if (device->capabilities & MEDIA_RADIO) {
                dm->setFmNode(reinterpret_cast<char*>(device->radio_node));
                if (device->audio_playback_node[0]) {
                }
                if (device->audio_capture_node[0]) {
                }
                if (device->capabilities & MEDIA_RDS) {
                }
            }
            if (device->capabilities & MEDIA_DAB) {
                dm->setDabNode(reinterpret_cast<char*>(device->dab_node));
            }

            if (dm) {
                std::cout << "added new device " << device->devicename << std::endl;

                dm->setDeviceName(reinterpret_cast<char*>(device->devicename));
                dm->setSerial(device->serial);
                dm->setDeviceId(device->id);
                _devices.push_back(dm);
            }
            free(device);
        } while((device=net_device_enum(fd, &i, ++d))!=0);
        i++;
    };

    net_close(fd);
    
    return _devices.size();
}

int sundtekApi::searchFmStations(){
    const auto dm = findFmDevice();
    
    if(dm == nullptr){
        return -999;
    }

    auto result = openFmDevice(dm);
    if(result < 0) {
        std::cout << "open device returns error" << std::endl;
        return -998;
    }
    
    auto fd = dm->getFmHandle();
    int i;
    int e;
    int current_scan_index=-1;
    fm_scan_setup setup;
    fm_scan_parameters parameters;
    memset(&parameters, 0x0, sizeof(fm_scan_parameters));
    memset(&setup, 0x0, sizeof(fm_scan_setup));
    std::cout << "SCAN SETUP" << std::endl;
    net_ioctl(fd, FM_SCAN_SETUP, &setup);

    do {
        net_ioctl(fd, FM_SCAN_NEXT_FREQUENCY, &parameters);
        //printf("LOCK STAT: %x - %d - %d\n", parameters.status, parameters.VALID, parameters.READFREQ);
        switch(parameters.status) {
            case FM_SCAN_LOCKED:
            {
                std::cout << "FREQUENCY: " << parameters.READFREQ << " LOCKED" << std::endl;
                fm_rds_data data;
                memset(&data, 0, sizeof(fm_rds_data));
                net_ioctl(fd, FM_RDS_STATUS, &data);
                std::wcout << "Name" << data.PI <<  std::endl;
                /* if (console>=0) {
                        printf("FREQUENCY: %d ", parameters.READFREQ);
                        rv=write(console, "[LOCKED]\n", 9);
                } else {
                        fprintf(stdout, "%d [LOCKED]\n", parameters.READFREQ);
                }*/
                break;
            }
            case FM_SCAN_SEARCHING:
                usleep(10000);
                break;
            case FM_SCAN_COMPLETE:
            {
                std::cout <<"Scan completed" <<:: std::endl;
                break;
            }
        }
        /* if (console>=0 && running == 0)
                break;*/
    } while (parameters.status != FM_SCAN_COMPLETE);


    return 0;
}