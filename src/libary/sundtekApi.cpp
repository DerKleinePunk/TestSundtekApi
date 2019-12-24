#include <iostream>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <chrono>
#include "sundtekApi.hpp"

//https://stackoverflow.com/questions/24903904/how-to-set-console-cursor-position-for-stdout
//http://www.elektronik-labor.de/ElektorDSP/ElektroDSP7.html

uint8_t CharCodes[32] {
    0xE0, 0x85 , 0x82 , 0x8A , 0xE1 , 0x8D , 0xE2 , 0x95 , 0xE3 , 0x97 , 0x9C , 0x80 , 0x20 , 0x00 , 0xE9 , 0x20,
    0x83, 0x84 , 0x88 , 0x89 , 0x86 , 0x8B , 0x93 , 0x94 , 0x96 , 0x81 , 0x9B , 0x87 , 0x20 , 0x20 , 0x20 , 0x20
};

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

void sundtekApi::ReadRDSData(int handle) { 
    fm_rds_data data;
    uint8_t rdsdata[8];
    memset(&data, 0x00, sizeof(fm_rds_data));
    uint8_t print_program[9];
    memset(print_program, 0x00, 9);
    uint8_t program[9];
    memset(program, 0x00, 9);
    int x=0;
    uint8_t brkstat = 0;
    uint8_t radiotext[150];
    memset(radiotext, 0x00, 150);
    auto searchStart = std::chrono::system_clock::now();
    while(1) {
        net_ioctl(handle, FM_RDS_STATUS, &data);
        if (data.rdssync) {
            if (memcmp(rdsdata, data.data, 8)==0) {
                //Same Block read next
                continue;
            }
            memcpy(rdsdata, data.data, 8);

            if ((data.data[2]>>4) == 0 || (data.data[2]>>4) == 8) {
                //Figure 12 shows the format of type 0A groups and figure 13 the format of type 0B groups.
                //Program Name with Alternative frequency
                x = (data.data[3]&0x3)*2;
                program[x++] = GetAcsiiChar(data.data[6]);
                program[x++] = GetAcsiiChar(data.data[7]);
            }
            if ((data.data[2]>>4) == 2) {
                x = (data.data[3]&0x0f)*4;
                if ((data.data[3]&0x10) != brkstat) {
                    brkstat = data.data[3] & 0x10;
                    memset(radiotext, 0x00, 150);
                    //Trans restart
                }
                radiotext[x++]=(data.data[4]&0x7f);
                radiotext[x++]=(data.data[5]&0x7f);
                radiotext[x++]=(data.data[6]&0x7f);
                radiotext[x++]=(data.data[7]&0x7f);
            }
            usleep(1000);

            if (isprint(program[0]) && isprint(program[1]) && isprint(program[2]) && isprint(program[3])) {
                memcpy(print_program, program, 9);
                printf("%c", 0x1B);
                printf("[8;0H"); //Set Cursor to line 8
                fflush(stdout);
                std::cout << "Programm: " << print_program << std::endl;
                std::cout << "RDS Data found" << std::endl;
                break;
            }

            printf("%c", 0x1B);
            printf("[7;0H"); //Set Cursor to line 7

            printf("Radiotext: ");
            for (auto i=0;i<64;i++) {
                switch(radiotext[i]) {
                    case 0x00:
                        printf("%c", 0x20);
                        break;
                    default:
                        printf("%c", radiotext[i]);
                }
            }
            printf("\n");
            fflush(stdout);
        }
        auto now = std::chrono::system_clock::now();
        if(now > searchStart + std::chrono::seconds(2)) {
            printf("%c", 0x1B);
            printf("[7;0H"); //Set Cursor to line 7
            fflush(stdout);
            std::cout << "Radiotext: " << std::endl;
            std::cout << "Programm: " << std::endl;
            std::cout << "timeout waiting for RDS Data" << std::endl;
            break;
        }
    }
}

int8_t sundtekApi::GetAcsiiChar(int8_t zeichen) {
    if(zeichen < 0x7F) {
        return zeichen;
    }
    zeichen = zeichen - 0x7F;
    return CharCodes[zeichen];
}

sundtekApi::sundtekApi() {
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
        std::cout << "connect failed " << strerror(errno) << std::endl;
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
                    //Not Set On Stick Why
                }
                dm->setCanRDS();
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

/* @brief Search all FMS Stations and Safe FRQ
*/
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
        printf("%c", 0x1B);
        printf("[10;0H"); //Set Cursor to line 10
        switch(parameters.status) {
            case FM_SCAN_LOCKED: {
                std::cout << "FREQUENCY: " << parameters.READFREQ << " LOCKED" << std::endl;
                if(dm->getCanRDS()){
                    ReadRDSData(fd);
                }
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