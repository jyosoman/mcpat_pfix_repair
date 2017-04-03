#ifndef _GLOB_BUFF_H
#define _GLOB_BUFF_H


#include "XML_Parse.h"
#include "logic.h"
#include "parameter.h"
#include "array.h"
#include "interconnect.h"
#include "basic_components.h"
#include "sharedcache.h"

class globalBuffer: public Component {
public:
    ParseXML *XML;
    InputParameter interface_ip;
    double clockRate, executionTime;
    double scktRatio, chip_PR_overhead, macro_PR_overhead;
    ArrayST * Buffer;
    globalBuffer(ParseXML *XML_interface, InputParameter* interface_ip_);
    void computeEnergy(bool is_tdp = true);
    void displayEnergy(uint32_t indent = 0, int plevel = 100, bool is_tdp = true);
    ~globalBuffer(){delete Buffer;Buffer=0;};
};
#endif