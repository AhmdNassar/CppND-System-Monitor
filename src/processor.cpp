#include "processor.h"

#include <iostream>

#include "linux_parser.h"
using std::cout;

// TODO: Return the aggregate CPU utilization
float Processor::Utilization() { return LinuxParser::CpuUtilization()[0]; }