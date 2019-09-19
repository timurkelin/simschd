# Evaluation of the Static Scheduler
SystemC Simulator of the Static Scheduler

Timing, timing hierarchy, time tick issues a list of tasks
Tasks, task dependencies, task durations, list of the executors for the task has the form: (resA | resB | ... ) & ( resX | resY ) & resZ
Executors. each executor can use a part of a common pool
common pools: if requested common resource exceeds the volume of the common pool then all executors slow down (DVFS, througput)

