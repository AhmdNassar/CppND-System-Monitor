#include "linux_parser.h"

#include <dirent.h>
#include <unistd.h>

#include <iostream>
#include <string>
#include <vector>

using std::stof;
using std::string;
using std::to_string;
using std::vector;

// grab_components Function
vector<string> grab_components(const string& line) {
  std::istringstream iss(line);
  std::istream_iterator<string> beg(iss), end;
  vector<string> values(beg, end);
  return values;
}

// DONE: An example of how to read data from the filesystem
string LinuxParser::OperatingSystem() {
  string line;
  string key;
  string value;
  std::ifstream filestream(kOSPath);
  if (filestream.is_open()) {
    while (std::getline(filestream, line)) {
      std::replace(line.begin(), line.end(), ' ', '_');
      std::replace(line.begin(), line.end(), '=', ' ');
      std::replace(line.begin(), line.end(), '"', ' ');
      std::istringstream linestream(line);
      while (linestream >> key >> value) {
        if (key == "PRETTY_NAME") {
          std::replace(value.begin(), value.end(), '_', ' ');
          return value;
        }
      }
    }
  }
  return value;
}

// DONE: An example of how to read data from the filesystem
string LinuxParser::Kernel() {
  string os, kernel, version;
  string line;
  std::ifstream stream(kProcDirectory + kVersionFilename);
  if (stream.is_open()) {
    std::getline(stream, line);
    std::istringstream linestream(line);
    linestream >> os >> version >> kernel;
  }
  return kernel;
}

// BONUS: Update this to use std::filesystem
vector<int> LinuxParser::Pids() {
  vector<int> pids;
  DIR* directory = opendir(kProcDirectory.c_str());
  struct dirent* file;
  while ((file = readdir(directory)) != nullptr) {
    // Is this a directory?
    if (file->d_type == DT_DIR) {
      // Is every character of the name a digit?
      string filename(file->d_name);
      if (std::all_of(filename.begin(), filename.end(), isdigit)) {
        int pid = stoi(filename);
        pids.push_back(pid);
      }
    }
  }
  closedir(directory);
  return pids;
}

// TODO: Read and return the system memory utilization
float LinuxParser::MemoryUtilization() {
  std::ifstream stream(kProcDirectory + kMeminfoFilename);

  if (stream.is_open()) {
    string line, key;
    double memTotal, memFree, tempValue;
    std::map<string, double> memData;
    int cont = 0;

    while (getline(stream, line) && cont < 2) {
      cont++;
      std::istringstream lineStream(line);
      lineStream >> key >> tempValue;
      memData[key] = tempValue;
    }

    memTotal = memData["MemTotal:"];
    memFree = memData["MemFree:"];

    return ((memTotal - memFree) / memTotal);
  }
  return 0;
}

// TODO: Read and return the system uptime
long LinuxParser::UpTime() {
  std::ifstream filestream(kProcDirectory + kUptimeFilename);
  long upTime = 0;
  long idleTime;
  if (filestream.is_open()) {
    std::string line;
    std::getline(filestream, line);
    std::istringstream linestream(line);
    linestream >> upTime >> idleTime;
  }
  return upTime;
}

// TODO: Read and return the number of jiffies for the system
long LinuxParser::Jiffies() {
  string line;
  string key;
  long jiffies = 0;
  std::ifstream filestream(kProcDirectory + kStatFilename);
  if (filestream.is_open()) {
    std::getline(filestream, line);
    std::stringstream linestream(line);
    int value;
    // sum the values from the first line of the file
    // user + nice + system + idle + iowait + irq + softirq + steal
    for (int i = 0; i < 9; ++i) {
      if (i == 0) {
        linestream >> key;
      } else {
        linestream >> value;
        jiffies += value;
      }
    }
  }
  return jiffies;
}

// TODO: Read and return the number of active jiffies for a PID
// REMOVE: [[maybe_unused]] once you define the function
long LinuxParser::ActiveJiffies(int pid) {
  string line;
  string placeholder;
  long jiffies = 0;
  long process_jiffies = 0;
  string kPidDirectory = '/' + std::to_string(pid);
  std::ifstream filestream(kProcDirectory + kPidDirectory + kStatFilename);
  if (filestream.is_open()) {
    std::getline(filestream, line);
    std::istringstream linestream(line);
    for (int token_id = 1; token_id <= 17; ++token_id) {
      if (token_id == ProcessCpuStates::kCstime ||
          token_id == ProcessCpuStates::kCutime ||
          token_id == ProcessCpuStates::kStime ||
          token_id == ProcessCpuStates::kUtime) {
        linestream >> jiffies;
        process_jiffies += jiffies;
      } else {
        linestream >> placeholder;
      }
    }
  }
  return process_jiffies;
}

// TODO: Read and return the number of active jiffies for the system
long LinuxParser::ActiveJiffies() { return Jiffies() - IdleJiffies(); }

// TODO: Read and return the number of idle jiffies for the system
long LinuxParser::IdleJiffies() {
  string line;
  string key;
  long idleJiffies = 0;
  std::ifstream filestream(kProcDirectory + kStatFilename);
  if (filestream.is_open()) {
    std::getline(filestream, line);
    std::stringstream linestream(line);
    int value;
    // sum the iddle ticks
    // idle + iowait
    for (int i = 0; i < 6; ++i) {
      if (i == 0) {
        linestream >> key;
      } else if (i > 3) {
        linestream >> value;
        idleJiffies += value;
      } else {
        linestream >> value;
      }
    }
  }
  return idleJiffies;
}

void LinuxParser::CpuUtHelper(float& Idle, float& Total) {
  std::ifstream stream(kProcDirectory + kStatFilename);
  vector<string> keys{"user",   "nice", "system",  "idle",
                      "iowait", "irq",  "softirq", "steal"};

  std::map<string, float> values;
  string line, cpu;
  float value, NonIdle;

  std::getline(stream, line);
  std::istringstream linestream(line);

  linestream >> cpu;  // cpu word
  for (string key : keys) {
    linestream >> value;
    values[key] = value;
  }

  stream.close();

  Idle = values["idle"] + values["iowait"];
  NonIdle = values["user"] + values["nice"] + values["system"] + values["irq"] +
            values["softirq"] + values["steal"];
  Total = Idle + NonIdle;
}
// TODO: Read and return CPU utilization
vector<float> LinuxParser::CpuUtilization() {
  float totald, idled, PrevTotal, PrevIdle, Total, Idle;

  CpuUtHelper(PrevIdle, PrevTotal);

  sleep(1);

  CpuUtHelper(Idle, Total);

  // differentiate: actual value minus the previous one
  totald = Total - PrevTotal;
  idled = Idle - PrevIdle;
  float CPU_Percentage = (totald - idled) / totald;

  return {CPU_Percentage};
}

// TODO: Read and return the total number of processes
int LinuxParser::TotalProcesses() {
  std::ifstream stream(kProcDirectory + kStatFilename);
  string line, key;
  int value;

  while (getline(stream, line)) {
    std::istringstream linestream(line);
    linestream >> key;
    if (key == "processes") {
      linestream >> value;
      return value;
    }
  }

  return 0;
}

// TODO: Read and return the number of running processes
int LinuxParser::RunningProcesses() {
  std::ifstream stream(kProcDirectory + kStatFilename);
  string line, key;
  int value;

  while (getline(stream, line)) {
    std::istringstream linestream(line);
    linestream >> key;
    if (key == "procs_running") {
      linestream >> value;
      return value;
    }
  }

  return 0;
}

// TODO: Read and return the command associated with a process
// REMOVE: [[maybe_unused]] once you define the function
string LinuxParser::Command(int pid) {
  string cmd_line;
  string kPidDirectory = '/' + std::to_string(pid);
  std::ifstream filestream(kProcDirectory + kPidDirectory + kCmdlineFilename);
  if (filestream.is_open()) {
    std::getline(filestream, cmd_line);
  }
  return cmd_line;
}
// TODO: Read and return the memory used by a process
// REMOVE: [[maybe_unused]] once you define the function
string LinuxParser::Ram(int pid) {
  string line;
  string key;
  long ram;
  string kPidDirectory = '/' + std::to_string(pid);
  std::ifstream filestream(kProcDirectory + kPidDirectory + kStatusFilename);
  if (filestream.is_open()) {
    while (std::getline(filestream, line)) {
      std::istringstream linestream(line);
      linestream >> key;
      if (key == "VmSize:") {
        linestream >> ram;
        break;
      }
    }
  }
  return std::to_string(ram / 1000);
}
// TODO: Read and return the user ID associated with a process
// REMOVE: [[maybe_unused]] once you define the function
string LinuxParser::Uid(int pid) {
  string line;
  string key;
  string uid;
  string kPidDirectory = '/' + std::to_string(pid);
  std::ifstream filestream(kProcDirectory + kPidDirectory + kStatusFilename);
  if (filestream.is_open()) {
    while (std::getline(filestream, line)) {
      std::istringstream linestream(line);
      while (linestream >> key) {
        if (key == "Uid:") {
          linestream >> uid;
          break;
        }
      }
    }
  }
  return uid;
}
// TODO: Read and return the user associated with a process
// REMOVE: [[maybe_unused]] once you define the function
string LinuxParser::User(int pid) {
  string line;
  string user;
  string uid;
  string placeholder;
  std::ifstream filestream(kPasswordPath);
  if (filestream.is_open()) {
    while (std::getline(filestream, line)) {
      std::replace(line.begin(), line.end(), ':', ' ');
      std::istringstream linestream(line);
      linestream >> user >> placeholder >> uid;
      if (uid == Uid(pid)) {
        break;
      }
    }
  }
  return user;
}

// Read and return the uptime of a process
long LinuxParser::UpTime(int pid) {
  string line;
  string placeholder;
  long start_time = 0;
  string kPidDirectory = '/' + std::to_string(pid);
  std::ifstream filestream(kProcDirectory + kPidDirectory + kStatFilename);
  if (filestream.is_open()) {
    std::getline(filestream, line);
    std::istringstream linestream(line);
    for (int token_id = 1; token_id <= 22; ++token_id) {
      if (token_id == ProcessCpuStates::kStarttime) {
        linestream >> start_time;
      } else {
        linestream >> placeholder;
      }
    }
  }
  return start_time / sysconf(_SC_CLK_TCK);
}