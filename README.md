# simSCHD 
Evaluation of the Static Scheduling and Timing Budget
***

Abstract: ???

Keywords:
Multitasking, Multicore, Loosely Timed Model, Timing Budget, Static Scheduler, System Simulation, Timing Model, Hard Real Time, Model Based Development and Optimization, SystemC

## Overview
* A simulation platform for the evaluation of the scheduling and timing budget
* Simplifies the initial stages of the architecture development for Hard Real Time systems.
  * Allow for the simulation driven development and optimization of the architecture
* Flexible, highly abstract and independent of the implementation target (HW or SW)

* Input:
  * Top-level timing requirements
  * Outline of the processing flow
* Output:
  * Processing schedule and timing diagrams
  * List of the execution blocks and common resources as well as the requirements for both

* Benefits of using SystemC
  * Reduced update-simulation-analysis cycle which allows for simulation driven development and optimization of the architecture
  * High level of abstraction for the scheduler and execution core preferences to stay focused on the architectural tasks
  * Integration into the existing simulation frameworks and workflows
  * Open source

## Simulated System
### System Model
* Real-Time System is simulated as a number of concurrently running **Threads**
* Each **Thread** is a succession of the **Tasks** to execute and the **Events** to generate
* New **Thread** is initiated when the specific set of the **Events** has been generated 
  * Thread synchronization
* Each **Task** is a specification of
  * Execution time, 
  * Set of the **Execution Blocks** to be occupied for the duration of the execution time, and
  * Demand requested from the **Common Resources** by the **Execution Blocks** from the set.
* Competition for the **Execution Block** is resolved by the priorities which were specified in the **Thread** preferences

### Execution Blocks and Common Resources
* Each Common Resource simulates an instance which is shared by multiple Execution Blocks in the cooperative manner.
* The Common Resource is characterized by its capacity
* Examples
  * Shared memory interface with throughput limit. Capacity: _max throughput_.
  * Power source with the current limit. Capacity: _max current_.
  
### Block Diagram of the Simulation Platform
![alt text](https://github.com/timurkelin/simschd/blob/master/doc/block_diagram.PNG)

### Planner
* Executes multiple concurrent Threads
* Keeps the register of the Events generated by the running Threads
  * Events are used for the synchronization between the Threads 
* Assigns the available Execution Blocks to the Tasks which are called by the running Threads 
  * Resolves Thread priorities
* Transmits runtime configuration to the Execution Blocks and receives their state
  * Keeps the record of the available and busy Execution Blocks
  
### Execution Block
* Receives runtime configuration from the Planner
* Transmits requests with the demand values to the Common Resources
* Receives the information from the Common Resource about its total demand, combines the information from the multiple Common Resources and adjusts the execution time accordingly
* Transmits “available” status to the Planner when the execution time is expired

### Common Resource
* Receives requests with the demand values from the Execution Blocks
* Calculates total demand requested by all of the Execution Blocks which are using this Resource
* Broadcasts the value of the total demand to all of the Execution Blocks which are using this Resource

## Simulator Configuration and Preferences

## Examples

For more details please refer to [doc/simschd.pptx](https://github.com/timurkelin/simschd/tree/master/doc)

Source Directories:
```
doc            - documentation
mat            - matlab models  
examples       - application examples
schd_core      - Processing modules and core assembly
schd_common    - top-level functions  
schd_pref      - simulation and system preferences  
schd_dump      - data dump   
schd_report    - logging and reporting functions  
schd_systemc   - Files taken from SystemC sources  
schd_trace     - waveform trace  
schd_time      - simulation time and resolution  
tools          - supporting scripts  
makefile       - top-level makefile 
```
Prerequisites:   
   GCC      (4.8.5)  
   make     (3.82)  
   [SystemC  (2.3.3)](https://www.accellera.org/downloads/standards/systemc)  
   [Boost    (1.68.0)](https://www.boost.org/)  
   [matIO    (1.5.16)](https://sourceforge.net/projects/matio/)   
   [gtkwave  (3.3.95)](http://gtkwave.sourceforge.net/) or other VCD viewer   

Environment:
```
export  CC=$(command -v gcc)
export GCC=$(command -v gcc)
export CXX=$(command -v g++)
$BOOST_HOME    contains Boost   installation path
$MATIO_HOME    contains matIO   installation path
$SYSTEMC_HOME  contains SystemC installation path
```
Quick start:
```
Skim through the slides:
doc/simschd.pptx

Clean:
make clean

Build:
make all

Run:
./build/Release/out/simschd ./examples/test_short_gtkwave.json

Inspect the results:
In gtkwave File->Open New Window->trace.vcd 
Apply trace.trn translation file
```
## Timing diagrams for test_short
![alt text](https://github.com/timurkelin/simschd/blob/master/doc/test_short_waves.PNG)
