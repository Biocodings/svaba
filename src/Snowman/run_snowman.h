#ifndef SNOWMAN_RUN_H__
#define SNOWMAN_RUN_H__

#include <string>
#include <vector>
#include <unordered_map>
#include <pthread.h>
#include <ctime>
#include <ostream>

#include "SnowTools/GenomicRegionCollection.h"
#include "SnowTools/BWAWrapper.h"
#include "SnowTools/AlignedContig.h"
#include "SnowmanAssemblerEngine.h"
#include "SnowTools/DiscordantCluster.h"

#include "SnowmanBamWalker.h"
#include "workqueue.h"

class SnowTimer;

void makeVCFs();
int overlapSize(const BamRead& query, const BamReadVector& subject);
bool hasRepeat(const std::string& seq);
void parseRunOptions(int argc, char** argv);
void runSnowman(int argc, char** argv);
void learnParameters();
int countJobs(SnowTools::GRC &file_regions, SnowTools::GRC &run_regions);
void sendThreads(SnowTools::GRC& regions_torun);
bool runBigChunk(const SnowTools::GenomicRegion& region); 
SnowTools::GRC makeAssemblyRegions(const SnowTools::GenomicRegion& region);
void alignReadsToContigsOneAtATime(const ContigVector& contigs, std::vector<SnowTools::AlignedContig>& alc, BamReadVector& bav_this, SnowTimer& st,
				   SnowTools::DiscordantClusterMap& dmap, CigarMap& cigmap_n, CigarMap& cigmap_t);
void alignReadsToContigs(SnowTools::BWAWrapper& bw, const SnowTools::USeqVector& usv, BamReadVector& bav_this, std::vector<SnowTools::AlignedContig>& this_alc);

/** @brief p-thread work item that calls Snowman on a small region

    Detailed description follows here.
    @author X. XYZ, DESY
    @date March 2008
*/

class SnowmanWorkItem {

 private:
  SnowTools::GenomicRegion m_gr;
  int m_number;  

 public:
  SnowmanWorkItem(const SnowTools::GenomicRegion& gr, int number)  
    : m_gr(gr), m_number(number) {}
    ~SnowmanWorkItem() {}
    
    int getNumber() { return m_number; }
    
    bool run() { return runBigChunk(m_gr); }
    
};

// make a structure to store timing opt
struct SnowTimer {

  SnowTimer() {
    s = {"r", "m", "as", "bw", "cl", "wr", "sw"};
    for (auto& i : s)
      times[i] = 0;
    curr_clock = clock();
  }

  std::unordered_map<std::string, double> times;
  std::vector<std::string> s;

  clock_t curr_clock;

  void stop(std::string part) { 
    times[part] += (clock() - curr_clock); 
    curr_clock = clock();
  }
  void start() { curr_clock = clock(); }

  // print it
  friend std::ostream& operator<<(std::ostream &out, const SnowTimer st) {

    double total_time = 0;
    for (auto& i : st.times)
      total_time += i.second;
    if (total_time == 0)
      return out;

    char buffer[140];
    
    auto itr = st.times.find("r");
    auto itm = st.times.find("m");
    //auto itc = st.times.find("cl");
    auto ita = st.times.find("as");
    //auto itb = st.times.find("bw");
    auto its = st.times.find("sw");

    sprintf (buffer, "R: %2d%% M: %2d%% A: %2d%% P: %2d%%", 
	     SnowTools::percentCalc<double>(itr->second, total_time),
	     SnowTools::percentCalc<double>(itm->second, total_time),
	     //SnowTools::percentCalc<double>(itc->second, total_time),
	     SnowTools::percentCalc<double>(ita->second, total_time),
	     //SnowTools::percentCalc<double>(itb->second, total_time),
	     SnowTools::percentCalc<double>(its->second, total_time));
    out << std::string(buffer);
    return out;
  }

};


#endif
