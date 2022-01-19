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

using namespace std;
 
int main() {
    // keep track of execution time
    Stopwatch watch;    
 
    // ---------------- CONFIG STATE -------------------
    
    // create a network on GPU
    int randSeed = 42;
#ifdef __NO_CUDA__
    int numGPUs = 1;
    CARLsim sim("hello world", CPU_MODE, USER, numGPUs, randSeed);
#else
    //int numGPUs = 2;
    int numGPUs = 1;  // Patch Killian
    CARLsim sim("hello world", GPU_MODE, USER, numGPUs, randSeed);
#endif
 
    // configure the network
    // set up a COBA two-layer network with gaussian connectivity
    Grid3D gridIn(8,8,1); // pre is on a 13x9 grid
    Grid3D gridInh(8,8,1); // post is on a 3x3 grid
    Grid3D gridOut(8,8,1); // post is on a 3x3 grid
 
#ifdef __NO_CUDA__
    int gin=sim.createSpikeGeneratorGroup("input", gridIn, EXCITATORY_NEURON, 0, CPU_CORES);
    int ginh=sim.createGroup("inhib", gridInh, INHIBITORY_NEURON, 0, CPU_CORES);
    int gout=sim.createGroup("output", gridOut, EXCITATORY_NEURON, 0, CPU_CORES);
#else
    /*
    int gin = sim.createSpikeGeneratorGroup("input", gridIn, EXCITATORY_NEURON, 0, GPU_CORES);
    int ginh=sim.createGroup("inhib", gridInh, INHIBITORY_NEURON, 0, GPU_CORES);
    int gout = sim.createGroup("output", gridOut, EXCITATORY_NEURON, 0, GPU_CORES);  // Patch Killian
    */
    int gin = sim.createSpikeGeneratorGroup("input", gridIn, EXCITATORY_NEURON, 0, CPU_CORES);
    int ginh=sim.createGroup("inhib", gridInh, INHIBITORY_NEURON, 0, CPU_CORES);
    int gout = sim.createGroup("output", gridOut, EXCITATORY_NEURON, 0, CPU_CORES);  // Patch Killian
#endif
 
    double inhib_level = 3.02;//1;
    ofstream outputfile;
    outputfile.open ("voltages.csv");
    int runtime = 1000;
 
    sim.setNeuronParameters(gout, 0.02f, 0.2f, -65.0f, 8.0f); // RS
    sim.setNeuronParameters(ginh, 0.1f, 0.2f, -65.0f, 2.0f); // FS
    sim.connect(gin, gout, "full", RangeWeight(0.03), 1.0f);
    sim.connect(ginh, gout, "one-to-one", RangeWeight(1), 1.0f);
    sim.connect(gout, ginh, "one-to-one", RangeWeight(1), 1.0f);
 
    sim.setConductances(true);

    NeuronMonitor* nrn_mon = sim.setNeuronMonitor(gout,"DEFAULT");
    nrn_mon->setPersistentData(true);
 
    // ---------------- SETUP STATE -------------------
    // build the network
    watch.lap("setupNetwork");
    sim.setupNetwork();
 
    // set some monitors
    //sim.setSpikeMonitor(gin,"DEFAULT");
    SpikeMonitor* spk_mon =sim.setSpikeMonitor(gout,"DEFAULT");
    //sim.setConnectionMonitor(gin,gout,"DEFAULT");
 
    //setup some baseline input
    PoissonRate in(gridIn.N);
    in.setRates(1.5f);
    sim.setSpikeRate(gin,&in);
 
    // ---------------- RUN STATE -------------------
    watch.lap("runNetwork");
    spk_mon->startRecording();
    nrn_mon->startRecording();
 
    // run for a total of 10 seconds
    // at the end of each runNetwork call, SpikeMonitor stats will be printed
    for (int i=0; i<runtime; i++) {
        sim.runNetwork(0,1, false);
 
        for (int i = 0; i < (8*8); i++) {
            sim.setWeight(1,i,i,inhib_level,true);
            sim.setWeight(2,i,i,inhib_level,true);
        }

        /*
        if (i % 600 == 0) {
        	nrn_mon->print();
        	//printf("\n%f",nrn_mon->getVectorV()[0][0]);
        	std::vector< std::vector< float > > test = nrn_mon->getVectorV();
        	int arrSize = sizeof(test[0])/sizeof(test[0][0]);
        	cout << "\narray size: " << arrSize << " " << sizeof(test[0]) << " " << sizeof(test[0][0]);
        	//cout << test[0][0];
        }
        */
    }
 
    // print stopwatch summary
    watch.stop();

    spk_mon->stopRecording();
    nrn_mon->stopRecording();

    std::vector< std::vector< float > > v_vec = nrn_mon->getVectorV();
    for (int i=0; i<runtime; i++) {
        outputfile << v_vec[0][i];
        if (i != (runtime-1)) {outputfile << ",";}
    }
    //outputfile << "\n";

    printf("\n\n");
    spk_mon->print(false);

    outputfile.close();
    
    return 0;
}