#include "system.h"

#include <unistd.h>

#include <cstddef>
#include <set>
#include <string>
#include <vector>

#include "linux_parser.h"
#include "process.h"
#include "processor.h"

using std::set;
using std::size_t;
using std::string;
using std::vector;

// TODO: Return the system's CPU
Processor& System::Cpu() { return cpu_; }

// TODO: Return a container composed of the system's processes
vector<Process>& System::Processes() {
  vector<int> pids = LinuxParser::Pids();
  processes_ = {};
  for (int& pid : pids) {
    Process process;
    int& p_pid = process.Pid();
    float& cpu_utilization = process.CpuUtilization();
    p_pid = pid;
    long elapsed_time = LinuxParser::UpTime() - LinuxParser::UpTime(pid);
    cpu_utilization = ((float)LinuxParser::ActiveJiffies(pid) / sysconf(_SC_CLK_TCK)) / elapsed_time;
    processes_.push_back(process);
  }
  std::sort(processes_.begin(), processes_.end(), compareProcesses);
  return processes_;
}
// TODO: Return the system's kernel identifier (string)
std::string System::Kernel() { return LinuxParser::Kernel(); }

// TODO: Return the system's memory utilization
float System::MemoryUtilization() { return LinuxParser::MemoryUtilization(); }

// TODO: Return the operating system name
std::string System::OperatingSystem() { return LinuxParser::OperatingSystem(); }

// TODO: Return the number of processes actively running on the system
int System::RunningProcesses() { return LinuxParser::RunningProcesses(); }

// TODO: Return the total number of processes on the system
int System::TotalProcesses() { return LinuxParser::TotalProcesses(); }

// TODO: Return the number of seconds since the system started running
long int System::UpTime() { return LinuxParser::UpTime(); }

bool System::compareProcesses(Process p, Process q) { return q < p; }
