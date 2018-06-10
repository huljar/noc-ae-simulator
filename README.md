# HaecComm

HaecComm is the simulator developed for the Diplom Thesis "Secure and Efficient Communication for Network-on-Chip under the Consideration of Multiple Paths". A small portion was already implemented before at the Chair of Privacy and Data Security of the TU Dresden Faculty of Computer Science. However, a large portion was refactored and the vast majority of the source code is written from scratch. The simulator is based on the framework OMNeT++ and written in C++.

## Installation

To run the application, a local installation of OMNeT++ is required. It can be downloaded from [their homepage](https://omnetpp.org). The simulator was developed and tested with version 5.2.1.

## Directory Structure

* `simulations`: contains the configuration files (*\*.ini*)
* `src`: contains the source files (C++ header and implementation files (*\*.h, \*.cc*) and NED files (*\*.ned*))
* `analysis`: contains Python scripts (*\*.py*) and Gnuplot scripts (*\*.gpi*) used to analyse the simulation results and produce graphical representations
* `doc`: reserved for future documentation files
* `tests`: contains test cases (unit and module tests)

## Running Simulations

To run a simulation, open the OMNeT++ IDE that comes with the framework and import the project by moving the directory that this README file resides in to the IDE's workspace. Afterwards, create a run configuration and select the desired configuration from the list of options ("Config name" dropdown menu). The source files will automatically be compiled before the simulation is executed.

## Obtaining The Results

In the current configuration, statistics are recorded when running simulations and saved to SQLite databases. They are stored in the `simulations/results` directory. The database schematics are documented in the OMNeT++ manual.

## Running The Tests

A number of test cases resides in the `tests/` directory. To run them, the simulator must be compiled as a shared library. This is achieved by editing the run configuration and selecting "opp_run" as the executable. When running this configuration now, a shared library version of the simulator will be compiled. Furthermore, it needs to be compiled in debug mode. Afterwards, the tests may be executed by running the `./runtest` script located in the `tests/` directory.
