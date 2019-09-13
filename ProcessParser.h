#include <algorithm>
#include <iostream>
#include <math.h>
#include <thread>
#include <chrono>
#include <iterator>
#include <string>
#include <stdlib.h>
#include <stdio.h>
#include <vector>
#include <fstream>
#include <sstream>
#include <stdexcept>
#include <cerrno>
#include <cstring>
#include <dirent.h>
#include <time.h>
#include <unistd.h>
#include "constants.h"


using namespace std;

class ProcessParser{
private:
    static std::ifstream stream;
    public:
  	static string getCmd(string pid);
    static vector<string> getPidList();
    static std::string getVmSize(string pid);
    static std::string getCpuPercent(string pid);
    static long int getSysUpTime();
    static std::string getProcUpTime(string pid);
    static string getProcUser(string pid);
    static vector<string> getSysCpuPercent(string coreNumber = "");
    static float getSysRamPercent();
    static string getSysKernelVersion();
    static int getTotalThreads();
    static int getTotalNumberOfProcesses();
    static int getNumberOfRunningProcesses();
    static string getOSName();
    static std::string PrintCpuStats(std::vector<std::string> values1, std::vector<std::string>values2);
    static bool isPidExisting(string pid);
  	static int getNumberOfCores();
};

// TODO: Define all of the above functions below:

int ProcessParser::getNumberOfCores()
{
    
    string cord;
    string label = "cpu cores";
    stream = Util::getStream((Path::basePath() + "cpuinfo"));
    while (std::getline(stream, cord)) {
        if (cord.compare(0, label.size(),label) == 0) {
            istringstream buf(cord);
            istream_iterator<string> beg(buf), end;
            vector<string> vals(beg, end);
            return stoi(vals[3]);
        }
    }
    return 0;
}

//Get the ToTal number of Threads
int ProcessParser::getTotalThreads()
{
    string line;
    int sol = 0;
    string label = "Threads:";
    vector<string> _list = ProcessParser::getPidList();
    for (int i=0 ; i<_list.size();i++) {
      string pid = _list[i];
      //getting every process and reading their number of their threads
      stream = Util::getStream((Path::basePath() + pid + Path::statusPath()));
      while (std::getline(stream, line)) {
          if (line.compare(0, label.size(), label) == 0) {
              istringstream buf(line);
              istream_iterator<string> beg(buf), end;
              vector<string> vals(beg, end);
              sol += stoi(vals[1]);
              break;
          }
      }
    }
   return sol;
}

string ProcessParser::getProcUser(string pid)
{
    string cord;
    string label = "Uid:";
    string answer ="";
    stream = Util::getStream((Path::basePath() + pid + Path::statusPath()));
    //comment
    while (std::getline(stream, cord)) {
        if (cord.compare(0, label.size(),label) == 0) {
            istringstream buf(cord);
            istream_iterator<string> beg(buf), end;
            vector<string> vals(beg, end);
            answer =  vals[1];
            break;
        }
    }
    stream = Util::getStream("/etc/passwd");
    label =("x:" + answer);
    
    while (std::getline(stream, cord)) {
        if (cord.find(label) != std::string::npos) {
            answer = cord.substr(0, cord.find(":"));
            return answer;
        }
    }
    return "";
}

string ProcessParser::getCpuPercent(string pid)
{
    string cord;
    string val;
    float res;
    stream = Util::getStream((Path::basePath()+ pid + "/" + Path::statPath()));
    getline(stream, cord);
    string str = cord;
    istringstream buf(str);
    istream_iterator<string> beg(buf), end;
    vector<string> values(beg, end); // done!
    // collecting important times for processor stats analysis
    float utime = stof(ProcessParser::getProcUpTime(pid));
    float stime = stof(values[14]);
    float cutime = stof(values[15]);
    float cstime = stof(values[16]);
    float starttime = stof(values[21]);
    float uptime = ProcessParser::getSysUpTime();
    float freq = sysconf(_SC_CLK_TCK);
    float total_time = utime + stime + cutime + cstime;
    float seconds = uptime - (starttime/freq);
    res = 100.0*((total_time/freq)/seconds);
    return to_string(res);
}

//Get Virtual memory size of a process based on the pid
string ProcessParser::getVmSize(string pid)
{
      string cord;
    //Declare a search attribute for a file
      string name = "VmData";
      string value;
      float solution;
    
     stream = Util::getStream((Path::basePath() + pid + Path::statusPath()));
    while(std::getline(stream, cord)){
        // Searching cord by cord
        if (cord.compare(0, name.size(),name) == 0) {
            //using sstream to slice string line on ws for values
            istringstream buf(cord);
            istream_iterator<string> beg(buf), end;
            vector<string> values(beg, end);
            //Kilobytes to Megabytes conversion
            solution = (stof(values[1])/float(1024));
            break;
        }
    }
    return to_string(solution);
}



long int ProcessParser::getSysUpTime()
{
    string cord;
    stream = Util::getStream((Path::basePath() + Path::upTimePath()));
    getline(stream, cord);
    istringstream buf(cord);
    istream_iterator<string> beg(buf), end;
    vector<string> values(beg, end);
    return stoi(values[0]);
}



string ProcessParser::getProcUpTime(string pid)
{
    string cord;
    string val;
    float sol;
    stream = Util::getStream((Path::basePath() + pid + "/" +  Path::statPath()));
    getline(stream, cord);
    string str = cord;
    istringstream buf(str);
    istream_iterator<string> beg(buf), end;
    vector<string> values(beg, end); // completed!
    // Using sysconf to get clock ticks of the host machine
    return to_string(float(stof(values[13])/sysconf(_SC_CLK_TCK)));
}


vector<string> ProcessParser::getPidList()
{
    DIR* dir;
    // Basically, we are scanning /proc dir for all directories with numbers as their names
    // If we get valid check we store dir names in vector as list of machine pids
    vector<string> bucket;
    if(!(dir = opendir("/proc")))
        throw std::runtime_error(std::strerror(errno));

    while (dirent* dirp = readdir(dir)) {
        // this is a folder directory?
        if(dirp->d_type != DT_DIR)
            continue;
        // Is every character of the name a digit in the system?
        if (all_of(dirp->d_name, dirp->d_name + std::strlen(dirp->d_name), [](char c){ return std::isdigit(c); })) {
            bucket.push_back(dirp->d_name);
        }
    }
    //Validating process of directory closing
    if(closedir(dir))
        throw std::runtime_error(std::strerror(errno));
    return bucket;
}

string ProcessParser::getCmd(string pid)
{
    string cord = "";
    
    stream = Util::getStream((Path::basePath() + pid + Path::cmdPath()));
    std::getline(stream, cord);
    return cord;
}

vector<string> ProcessParser::getSysCpuPercent(string coreNumber)
{
    /* It is possible to use this method for selection of data for overall cpu or every core.
     when nothing is passed "cpu" line is read
     when, for example "0" is passed  -> "cpu0" -> data for first core is read */
    string cord;
    string label = "cpu" + coreNumber;
    string val;
    int result;
    stream = Util::getStream((Path::basePath() + Path::statPath()));
    while (std::getline(stream, cord)) {
        if (cord.compare(0, label.size(),label) == 0) {
            istringstream buf(cord);
            istream_iterator<string> beg(buf), end;
            vector<string> values(beg, end);
            // set of cpu data active and idle times;
            return values;
        }
    }
    return (vector<string>());
}

float ProcessParser::getSysRamPercent()
{
    string line;
    string label1 = "MemAvailable:";
    string label2 = "MemFree:";
    string label3 = "Buffers:";

    string val;
    int result;
    stream = Util::getStream((Path::basePath() + Path::memInfoPath()));
    float total_mem = 0;
    float free_mem = 0;
    float buffers = 0;
    while (std::getline(stream, line)) {
        if (total_mem != 0 && free_mem != 0)
            break;
        if (line.compare(0, label1.size(), label1) == 0) {
            istringstream buf(line);
            istream_iterator<string> beg(buf), end;
            vector<string> values(beg, end);
            total_mem = stof(values[1]);
        }
        if (line.compare(0, label2.size(), label2) == 0) {
            istringstream buf(line);
            istream_iterator<string> beg(buf), end;
            vector<string> values(beg, end);
            free_mem = stof(values[1]);
        }
        if (line.compare(0, label3.size(), label3) == 0) {
            istringstream buf(line);
            istream_iterator<string> beg(buf), end;
            vector<string> values(beg, end);
            buffers = stof(values[1]);
        }
    }
    //calculating usage:
    return float(100.0*(1-(free_mem/(total_mem-buffers))));
}


// getSysKernel will be declared below
string ProcessParser::getSysKernelVersion()
{
    string link;
    string name = "Linux version ";
    stream = Util::getStream((Path::basePath() + Path::versionPath()));
    while (std::getline(stream, link)) {
        if (link.compare(0, name.size(),name) == 0) {
            istringstream buf(link);
            istream_iterator<string> beg(buf), end;
            vector<string> vals(beg, end);
            return vals[2];
        }
    }
    return "";
}


int ProcessParser::getTotalNumberOfProcesses()
{
    string line;
    int result = 0;
    string name = "processes";
    stream = Util::getStream((Path::basePath() + Path::statPath()));
    while (std::getline(stream, line)) {
        if (line.compare(0, name.size(), name) == 0) {
            istringstream buf(line);
            istream_iterator<string> beg(buf), end;
            vector<string> values(beg, end);
            result += stoi(values[1]);
            break;
        }
    }
    return result;
}

//getTotalNumber of running processes

int ProcessParser::getNumberOfRunningProcesses()
{
    string line;
    int result = 0;
    string name = "procs_running";
    stream = Util::getStream((Path::basePath() + Path::statPath()));
    while (std::getline(stream, line)) {
        if (line.compare(0, name.size(), name) == 0) {
            istringstream buf(line);
            istream_iterator<string> beg(buf), end;
            vector<string> values(beg, end);
            result += stoi(values[1]);
            break;
        }
    }
    return result;
}
string ProcessParser::getOSName()
{
    string line;
    string name = "PRETTY_NAME=";

    stream = Util::getStream(("/etc/os-release"));

    while (std::getline(stream, line)) {
        if (line.compare(0, name.size(), name) == 0) {
              std::size_t found = line.find("=");
              found++;
              string result = line.substr(found);
              result.erase(std::remove(result.begin(), result.end(), '"'), result.end());
              return result;
        }
    }
    return "";
}

float get_sys_active_cpu_time(vector<string> values)
{
    return (stof(values[S_USER]) +
            stof(values[S_NICE]) +
            stof(values[S_SYSTEM]) +
            stof(values[S_IRQ]) +
            stof(values[S_SOFTIRQ]) +
            stof(values[S_STEAL]) +
            stof(values[S_GUEST]) +
            stof(values[S_GUEST_NICE]));
}

float get_sys_idle_cpu_time(vector<string>values)
{
    return (stof(values[S_IDLE]) + stof(values[S_IOWAIT]));
}

string ProcessParser::PrintCpuStats(vector<string> values1, vector<string> values2)
{
/*
Because CPU stats can be calculated only if you take measures in two different time,
this function has two paramaters: two vectors of relevant values.
We use a formula to calculate overall activity of processor.
*/
    float active_time = get_sys_active_cpu_time(values2)-get_sys_active_cpu_time(values1);
    float idle_time = get_sys_idle_cpu_time(values2) - get_sys_idle_cpu_time(values1);
    float total_time = active_time + idle_time;
    float result = 100.0*(active_time / total_time);
    return to_string(result);
}
//static bool isPidExisting
bool ProcessParser::isPidExisting(string pid){
    bool result = false;
    vector<string>_list = ProcessParser::getPidList();

    for (int i=0; i<_list.size(); i++){
        if(pid == _list[i]){
            result = true;
            break;
        }
    }

    return result;  
}


// Get the number of host cpu cores

  

  
  
  
  
 

