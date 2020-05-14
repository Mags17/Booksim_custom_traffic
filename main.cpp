// $Id$

/*
 Copyright (c) 2007-2012, Trustees of The Leland Stanford Junior University
 All rights reserved.

 Redistribution and use in source and binary forms, with or without
 modification, are permitted provided that the following conditions are met:

 Redistributions of source code must retain the above copyright notice, this 
 list of conditions and the following disclaimer.
 Redistributions in binary form must reproduce the above copyright notice, this
 list of conditions and the following disclaimer in the documentation and/or
 other materials provided with the distribution.

 THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE 
 DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR
 ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
 ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

/*main.cpp
 *
 *The starting point of the network simulator
 *-Include all network header files
 *-initilize the network
 *-initialize the traffic manager and set it to run
 *
 *
 */
#include <sys/time.h>
#include <chrono>
#include <string>
#include <cstdlib>
#include <iostream>
#include <fstream>
#include <algorithm>





#include <sstream>
#include "src/booksim.hpp"
#include "src/routefunc.hpp"
#include "src/traffic.hpp"
#include "src/booksim_config.hpp"
#include "src/trafficmanager.hpp"
#include "src/random_utils.hpp"
#include "src/networks/network.hpp"
#include "src/injection.hpp"
#include "src/power/power_module.hpp"



///////////////////////////////////////////////////////////////////////////////
//Global declarations
//////////////////////

ifstream fDataset_in;
ofstream fDataset_out;
float inj_rate;

 /* the current traffic manager instance */
TrafficManager * trafficManager = NULL;


void ReadDatasetFile(BookSimConfig const & config, const int nRunNo, int numNodes)
{
   // int numNodes = 16;
    int nDatasetNo;
    //static ifstream fDataset_in;


        string mapping_file = config.GetStr("mapping_file");
        string str_DatasetIn = mapping_file;//dataset_file_1.txt";
        string line;
        string str_DatasetOut = str_DatasetIn;

        size_t len = str_DatasetIn.size();
        str_DatasetOut.resize(len-4);
        str_DatasetOut.append("_out.txt");

        mapping = new int[numNodes];

        fDataset_in.open(str_DatasetIn);
        fDataset_out.open(str_DatasetOut,std::fstream::app);

        if((fDataset_in.is_open()) && (fDataset_out.is_open()))
            cout << "Dataset file opened Succesfully" << endl;
        else
            cout << "Error in opening Dataset file " << endl;

       // getline(fDataset_in,line);
       // fDataset_out << line << endl;

        for(int temp = 1;temp < nRunNo; temp++)
        {
            getline(fDataset_in,line);
        }


    fDataset_in>>nDatasetNo;
    fDataset_out << nDatasetNo << "\t";

    cout << "Next mapping - " << nDatasetNo <<":\t";

    getline(fDataset_in,line);
    string line_temp = line;
    std::remove(line.begin(), line.end(), ',');
    std::remove(line.begin(), line.end(), '[');
    std::remove(line.begin(), line.end(), ']');

    stringstream line_in(line);

/*    for(int i=0;i<numNodes;i++)
    {
        fDataset_in>>mapping[i];
        cout<<mapping[i]<<" ";

        fDataset_out << " ";
        fDataset_out << mapping[i];
    }
 */
    for(int i=0;i<numNodes;i++)
    {
        line_in>>mapping[i];
        cout<<mapping[i]<<" ";

        fDataset_out << " ";
        fDataset_out << mapping[i];
    }

    //line_in >> inj_rate;
    //fDataset_out << line_temp;
    fDataset_out << " " << inj_rate << "\t";
      std::cout << endl << std::flush;

     fDataset_in.close();
     //fDataset_out.close(); // dont close it

}

int GetSimTime() {
  return trafficManager->getTime();
}

class Stats;
Stats * GetStats(const std::string & name) {
  Stats* test =  trafficManager->getStats(name);
  if(test == 0){
    cout<<"warning statistics "<<name<<" not found"<<endl;
  }
  return test;
}

/* printing activity factor*/
bool gPrintActivity;

int gK;//radix
int gN;//dimension
int gC;//concentration

int gNodes;

//generate nocviewer trace
bool gTrace;

ostream * gWatchOut;

#ifdef DEBUG_TEST
    ofstream debug_file;
#endif



/////////////////////////////////////////////////////////////////////////////

bool Simulate( BookSimConfig /*const*/ & config, int nCallNo )
{
  vector<Network *> net;
    bool result;
    int num_nodes;
  int subnets = config.GetInt("subnets");
  /*To include a new network, must register the network here
   *add an else if statement with the name of the network
   */
 // net.resize(subnets);
 // for (int i = 0; i < subnets; ++i) {
 //  ostringstream name;
//    name << "network_" << i;
//    net[i] = Network::New( config, name.str() );
//  }

  /*tcc and characterize are legacy
   *not sure how to use them 
   */

  //assert(trafficManager == NULL);
  //trafficManager = TrafficManager::New( config, net ) ;

  /*Start the simulation run
   */

//  num_nodes = net[0]->NumNodes();

  double total_time, g_total_time; /* Amount of time we've run */
  struct timeval start_time, end_time; /* Time before/after user code */
  struct timeval g_start_time, g_end_time; /* Time before/after user code */

  gettimeofday(&g_start_time, NULL);

  for(int nRunNo = 0; nRunNo < 1; nRunNo++)
  {
       net.resize(subnets);
      for (int i = 0; i < subnets; ++i) {
        ostringstream name;
        name << "network_" << i;
        net[i] = Network::New( config, name.str() );
      }

      //assert(trafficManager == NULL);
      //trafficManager = TrafficManager::New( config, net ) ;

      num_nodes = net[0]->NumNodes();

      ReadDatasetFile(config,nCallNo,num_nodes);
      config.Assign("injection_rate",inj_rate);

      trafficManager = TrafficManager::New( config, net ) ;

      total_time = 0.0;
      gettimeofday(&start_time, NULL);

      result = trafficManager->Run() ;

      if(!result)
          fDataset_out << "500.0" << endl;

      fDataset_out.close();

      gettimeofday(&end_time, NULL);
      total_time = ((double)(end_time.tv_sec) + (double)(end_time.tv_usec)/1000000.0)
                - ((double)(start_time.tv_sec) + (double)(start_time.tv_usec)/1000000.0);

      cout<<"Total run time "<<total_time<<endl;

      for (int i=0; i<subnets; ++i) {

        ///Power analysis
    //    if(config.GetInt("sim_power") > 0){
    //      Power_Module pnet(net[i], config);
    //      pnet.run();
    //    }

        delete net[i];
      }
        net.clear();
      delete trafficManager;
      trafficManager = NULL;
    }
  gettimeofday(&g_end_time, NULL);

  g_total_time = ((double)(g_end_time.tv_sec) + (double)(g_end_time.tv_usec)/1000000.0)
            - ((double)(g_start_time.tv_sec) + (double)(g_start_time.tv_usec)/1000000.0);

  cout<<"Grand total run time "<<g_total_time<<endl;


//  for (int i=0; i<subnets; ++i) {

//    ///Power analysis
////    if(config.GetInt("sim_power") > 0){
////      Power_Module pnet(net[i], config);
////      pnet.run();
////    }

//    delete net[i];
//  }

//  delete trafficManager;
//  trafficManager = NULL;

  return result;
}


int main( int argc, char **argv )
{

  BookSimConfig config;
  //argc = 2;
  //argv[1] = "examples/mesh88_lat";

  /*auto start = std::chrono::system_clock::now();
    std::time_t end_time = std::chrono::system_clock::to_time_t(start);
    cout << std::ctime(&end_time);*/
   //freopen("outputs/output_file", "w", stdout);
   //freopen("outputs/error_file", "w", stderr);

#ifdef DEBUG_TEST
    debug_file.open("outputs/debug_file.txt");
#endif

  if ( !ParseArgs( &config, argc, argv ) ) {
    cerr << "Usage: " << argv[0] << " configfile... [param=value...]" << endl;
    return 0;
 } 

  int nCallNo = atoi(argv[2]);
  inj_rate = atof(argv[3]);
  //cout << nRunNo << endl;
  /*initialize routing, traffic, injection functions
   */
  InitializeRoutingMap( config );

  gPrintActivity = (config.GetInt("print_activity") > 0);
  gTrace = (config.GetInt("viewer_trace") > 0);
  
  string watch_out_file = config.GetStr( "watch_out" );
  if(watch_out_file == "") {
    gWatchOut = NULL;
  } else if(watch_out_file == "-") {
    gWatchOut = &cout;
  } else {
    gWatchOut = new ofstream(watch_out_file.c_str());
  }
  

  /*configure and run the simulator
   */
  bool result = Simulate( config, nCallNo );

    #ifdef DEBUG_TEST
        debug_file.close();
    #endif

  return result ? -1 : 0;
}
