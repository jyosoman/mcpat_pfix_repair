
#include"globalIBuffer.h"
globalBuffer::globalBuffer(ParseXML* XML_interface, InputParameter* interface_ip_): XML(XML_interface),
interface_ip(*interface_ip_){
        clockRate = XML->sys.gbuffer.clockRate;
    executionTime = XML->sys.total_cycles / clockRate;;
    int data;
    //**********************************IRF***************************************
    data = int(ceil(XML->sys.machine_bits / 32.0))*32;
    interface_ip.is_cache = false;
    interface_ip.pure_cam = false;
    interface_ip.pure_ram = true;
    interface_ip.line_sz = int(ceil(data / 32.0))*4;
    interface_ip.cache_sz = XML->sys.gbuffer.entryCount * interface_ip.line_sz;
    interface_ip.assoc = 1;
    interface_ip.nbanks = 1;
    interface_ip.out_w = interface_ip.line_sz * 8;
    interface_ip.access_mode = 1;
    interface_ip.throughput = 1.0 / clockRate;
    interface_ip.latency = 1.0 / clockRate;
    interface_ip.obj_func_dyn_energy = 0;
    interface_ip.obj_func_dyn_power = 0;
    interface_ip.obj_func_leak_power = 0;
    interface_ip.obj_func_cycle_t = 1;
    interface_ip.num_rw_ports = 0; //this is the transfer port for saving/restoring states when exceptions happen.
    interface_ip.num_rd_ports = 0 * XML->sys.number_of_cores;
    interface_ip.num_wr_ports = 2* XML->sys.number_of_cores;
    interface_ip.num_se_rd_ports = 0;
    Buffer = new ArrayST(&interface_ip, "Core Instruction Buffer", Core_device,true, OOO, true);
    Buffer->area.set_area(Buffer->area.get_area() + Buffer->local_result.area);
    area.set_area(area.get_area() + Buffer->local_result.area);
    
}

void globalBuffer::computeEnergy(bool is_tdp){
    if (is_tdp) {
        //init stats for Peak
        Buffer->stats_t.readAc.access = Buffer->l_ip.num_search_ports;
        Buffer->stats_t.readAc.miss = 0;
        Buffer->stats_t.readAc.hit = Buffer->stats_t.readAc.access - Buffer->stats_t.readAc.miss;
        Buffer->tdp_stats = Buffer->stats_t;
    } else {
        //init stats for Runtime Dynamic (RTP)
        Buffer->stats_t.readAc.access = XML->sys.gbuffer.numWrites;
        Buffer->stats_t.readAc.miss=XML->sys.gbuffer.numQueries-Buffer->stats_t.readAc.access;
        Buffer->stats_t.readAc.hit = Buffer->stats_t.readAc.access - Buffer->stats_t.readAc.miss;
        Buffer->rtp_stats = Buffer->stats_t;
    }

    Buffer->power_t.reset();
    Buffer->power_t.readOp.dynamic += Buffer->stats_t.readAc.access * Buffer->local_result.power.searchOp.dynamic//FA spent most power in tag, so use total access not hits
            + Buffer->stats_t.readAc.miss * Buffer->local_result.power.writeOp.dynamic;

    if (is_tdp) {
        Buffer->power = Buffer->power_t + Buffer->local_result.power *pppm_lkg;
        power = power + Buffer->power;
    } else {
        Buffer->rt_power = Buffer->power_t + Buffer->local_result.power *pppm_lkg;
        rt_power = rt_power + Buffer->rt_power;
    }
}

void globalBuffer::displayEnergy(uint32_t indent, int plevel, bool is_tdp){
    string indent_str(indent, ' ');
    string indent_str_next(indent + 2, ' ');
    bool long_channel = XML->sys.longer_channel_device;
    bool power_gating = XML->sys.power_gating;
    if (is_tdp) {
        cout << indent_str << "GBuffer:" << endl;
        cout << indent_str_next << "Area = " << Buffer->area.get_area()*1e-6 << " mm^2" << endl;
        cout << indent_str_next << "Peak Dynamic = " << Buffer->power.readOp.dynamic * clockRate << " W" << endl;
        cout << indent_str_next << "Subthreshold Leakage = "
                << (long_channel ? Buffer->power.readOp.longer_channel_leakage : Buffer->power.readOp.leakage) << " W" << endl;
        if (power_gating) cout << indent_str_next << "Subthreshold Leakage with power gating = "
                << (long_channel ? Buffer->power.readOp.power_gated_with_long_channel_leakage : Buffer->power.readOp.power_gated_leakage) << " W" << endl;
        cout << indent_str_next << "Gate Leakage = " << Buffer->power.readOp.gate_leakage << " W" << endl;
        cout << indent_str_next << "Runtime Dynamic = " << Buffer->rt_power.readOp.dynamic / executionTime << " W" << endl;
        cout << endl;
    } else {
        cout << indent_str_next << "GBuffer    Peak Dynamic = " << Buffer->rt_power.readOp.dynamic * clockRate << " W" << endl;
        cout << indent_str_next << "GBuffer    Subthreshold Leakage = " << Buffer->rt_power.readOp.leakage << " W" << endl;
        cout << indent_str_next << "GBuffer    Gate Leakage = " << Buffer->rt_power.readOp.gate_leakage << " W" << endl;
    }  
}