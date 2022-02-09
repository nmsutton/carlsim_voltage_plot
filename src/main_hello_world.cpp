/* * Copyright (c) 2016 Regents of the University of California. All rights reserved.
*
* Redistribution and use in source and binary forms, with or without
* modification, are permitted provided that the following conditions
* are met:
*
* 1. Redistributions of source code must retain the above copyright
*    notice, this list of conditions and the following disclaimer.
*
* 2. Redistributions in binary form must reproduce the above copyright
*    notice, this list of conditions and the following disclaimer in the
*    documentation and/or other materials provided with the distribution.
*
* 3. The names of its contributors may not be used to endorse or promote
*    products derived from this software without specific prior written
*    permission.
*
* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
* "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
* LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
* A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR
* CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
* EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
* PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
* PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
* LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
* NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
* SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*
* *********************************************************************************************** *
* CARLsim
* created by: (MDR) Micah Richert, (JN) Jayram M. Nageswaran
* maintained by:
* (MA) Mike Avery <averym@uci.edu>
* (MB) Michael Beyeler <mbeyeler@uci.edu>,
* (KDC) Kristofor Carlson <kdcarlso@uci.edu>
* (TSC) Ting-Shuo Chou <tingshuc@uci.edu>
* (HK) Hirak J Kashyap <kashyaph@uci.edu>
*
* CARLsim v1.0: JM, MDR
* CARLsim v2.0/v2.1/v2.2: JM, MDR, MA, MB, KDC
* CARLsim3: MB, KDC, TSC
* CARLsim4: TSC, HK
* CARLsim5: HK, JX, KC
* CARLsim6: LN, JX, KC, KW
*
* CARLsim available from http://socsci.uci.edu/~jkrichma/CARLsim/
* Ver 12/31/2016
*/

// include CARLsim user interface
#include <carlsim.h>
// include stopwatch for timing
#include <stopwatch.h>
#include <vector>
#include <iostream>
// write to file
#include <fstream>
#include "periodic_spikegen_custom.cpp"

using namespace std;
 
int main() {
    // keep track of execution time
    Stopwatch watch;    
 
    // ---------------- CONFIG STATE -------------------    
    // create a network on GPU
    int randSeed = 42; int numGPUs = 1;
    CARLsim sim("hello world", GPU_MODE, USER, numGPUs, randSeed);
 
    // configure the network
    // set up a COBA two-layer network with gaussian connectivity
    int x_size = 1;
    int y_size = 1;
    Grid3D gridIn(x_size,y_size,1);
    Grid3D gridInh(x_size,y_size,1);
    Grid3D gridOut(x_size,y_size,1);
 
#ifdef __NO_CUDA__
    int gin=sim.createSpikeGeneratorGroup("input", gridIn, EXCITATORY_NEURON, 0, CPU_CORES);
    int ginh=sim.createGroup("inhib", gridInh, INHIBITORY_NEURON, 0, CPU_CORES);
    int gout=sim.createGroup("output", gridOut, EXCITATORY_NEURON, 0, CPU_CORES);
#else
    int gin = sim.createSpikeGeneratorGroup("input", gridIn, EXCITATORY_NEURON, 0, GPU_CORES);
    int ginh=sim.createGroup("inhib", gridInh, INHIBITORY_NEURON, 0, GPU_CORES);
    int gout = sim.createGroup("output", gridOut, EXCITATORY_NEURON, 0, GPU_CORES);  // Patch Killian
#endif
 
    vector<vector<int>> nrn_spk; // for total firing recording
    int s_num, spk_time;
    int nrn_size;
    double inhib_level = 0.5;//1;
    ofstream outputfile;
    ofstream outputfile2;
    outputfile.open ("voltages.csv");
    outputfile2.open ("spike_times.csv");
    int runtime = 1000;
 
    //sim.setNeuronParameters(gout, 0.02f, 0.2f, -65.0f, 8.0f); // RS
    sim.setNeuronParameters(gout, 0.1f, 0.2f, -65.0f, 2.0f); // FS
    sim.setNeuronParameters(ginh, 0.1f, 0.2f, -65.0f, 2.0f); // FS
    sim.connect(gin, gout, "one-to-one", 0.094f, 1.0f);
    sim.connect(ginh, gout, "one-to-one", 0.0f, 1.0f);
    sim.connect(gout, ginh, "one-to-one", 0.0f, 1.0f);    
 
    sim.setConductances(true);

    // baseline input        
    
    PeriodicSpikeGenerator in(100.0f, true);
    sim.setSpikeGenerator(gin, &in);    
    

    NeuronMonitor* nrn_mon = sim.setNeuronMonitor(gout,"DEFAULT");
    NeuronMonitor* nrn_mon2 = sim.setNeuronMonitor(gin,"DEFAULT");
    nrn_mon->setPersistentData(true);
    nrn_mon2->setPersistentData(true);
 
    // ---------------- SETUP STATE -------------------
    // build the network
    watch.lap("setupNetwork");
    sim.setupNetwork();
 
    // set some monitors
    SpikeMonitor* spk_mon =sim.setSpikeMonitor(gout,"DEFAULT");
    SpikeMonitor* spk_mon2 =sim.setSpikeMonitor(gin,"DEFAULT");
    SpikeMonitor* rep_spk_mon = spk_mon;
    NeuronMonitor* rep_nrn_mon = nrn_mon;
 
    //setup some baseline input    
    /*
    PoissonRate in(gridIn.N);
    in.setRates(100.0f);
    sim.setSpikeRate(gin,&in);
    */
    //sim.setExternalCurrent(gout, 3.9f);

    // ---------------- RUN STATE -------------------
    watch.lap("runNetwork");
    spk_mon->startRecording();
    spk_mon2->startRecording();
    nrn_mon->startRecording();
    nrn_mon2->startRecording();

    /*
    for (int i = 0; i < (x_size*y_size); i++) {
        sim.setWeight(1,i,i,inhib_level,true);
        sim.setWeight(2,i,i,inhib_level,true);
    }
    */    
 
    // run simulation
    // at the end of each runNetwork call, SpikeMonitor stats will be printed
    for (int i=0; i<runtime; i++) {
        sim.runNetwork(0,1, false);
    }

    // print stopwatch summary
    watch.stop();

    spk_mon->stopRecording();
    spk_mon2->stopRecording();
    nrn_mon->stopRecording();
    nrn_mon2->stopRecording();

    // save voltage data
    vector<vector<float>> v_vec = rep_nrn_mon->getVectorV(); // select spk mon
    for (int i=0; i<runtime; i++) {
        outputfile << v_vec[0][i];
        if (i != (runtime-1)) {outputfile << ",";}
    }
    // save spike times
    printf("spike times:\n");
    nrn_spk = rep_spk_mon->getSpikeVector2D(); // select nrn mon
    nrn_size = nrn_spk.size();
    for (int i = 0; i < nrn_size; i++) {
        s_num = nrn_spk[i].size();
        for (int j = 0; j < s_num; j++) {
            spk_time = nrn_spk[i][j];
            printf("%d",spk_time);
            outputfile2 << spk_time;
            if (j != (s_num-1)) {
                printf(",");
                outputfile2 << ",";
            }
        }
    }
    printf("\n");

    printf("\n\n");
    spk_mon->print(false);
    spk_mon2->print(false);

    outputfile.close();
    outputfile2.close();
    
    return 0;
}