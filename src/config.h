#pragma once

// Globals
extern bool verbose = false; 
extern long targetTemp = 35;
extern bool running = true;

// Pin definitions
const int tempPinAmbient = 14;
const int tempPinHeater = 15;
const int heaterOutput = 16;