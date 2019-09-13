#include <string>

using namespace std;
/*
Basic class for Process representation
It contains relevant attributes as shown below
*/
class Process {
private:
   
    string cpu;
    string mem;
    string upTime;
    string pid;
    string user;
    string cmd;

public:
    Process(string pid){
        this->pid = pid;
        this->user = ProcessParser::getProcUser(pid);
        //TODOs:
      this->upTime = ProcessParser::getProcUpTime(pid);  // This will get the time the processor was active for
      this->mem = ProcessParser::getVmSize(pid);  // Get the virtual memory size for the system
      this->cpu = ProcessParser::getCpuPercent(pid);// Get the percentage usage of the CPU at any time lamda
      this->cmd = ProcessParser::getCmd(pid); // Get the command for a particular process pid
      //complete for mem
      //complete for cmd
      //complete for cpu
      //complete for uptime
    }
    void setPid(int pid);
    string getPid()const;
    string getUser()const;
    string getCmd()const;
    int getCpu()const;
    int getMem()const;
    string getUpTime()const;
    string getProcess();
};
void Process::setPid(int pid){
    this->pid = pid;
}
string Process::getPid()const {
    return this->pid;
}
string Process::getProcess(){
    if(!ProcessParser::isPidExisting(this->pid))
        return "";
    this->mem = ProcessParser::getVmSize(this->pid);
    this->upTime = ProcessParser::getProcUpTime(this->pid);
    this->cpu = ProcessParser::getCpuPercent(this->pid); 

    return (this->pid + "   " //TODO: finish the string! this->user + "   "+ mem...cpu...upTime...;
                  + this->user + "   " 
                  + this->mem.substr(0, 5) +  "   " 
                  + this->cpu.substr(0, 5) + "    "
                  + this->upTime.substr(0, 5) + "   " 
                  + this-> cmd.substr(0, 30) + "..."); 
                 }
