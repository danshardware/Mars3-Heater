#pragma once

// Globals
extern bool verbose = false; 
extern long targetTemp = 30;
extern bool running = true;

// Pin definitions
const int tempPinAmbient = 15;
const int tempPinHeater = 14;
const int heaterOutput = 16;