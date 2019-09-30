# Evaluation of the Static Scheduler
SystemC Simulator of the Static Scheduler

Threads: triggering events (AND list of regex), generated events, tasks <br />
event "sim_start" to ignite the timing threads 
```
threads:
[{ name: <thread_name_string>
   start: [<event_name_regex>, <event_name_regex> ... ], 
   seq: [{"task":<task_name_string>}, OR
         {"event":<event_name_string>}, OR
         ....
        ]
 },
..... 
]
```
Tasks: task durations, list of the executors (AND list of regex) <br />
Executors: each executor can occupy a fraction of a common resource  <br />
Common resource: if requested common resource exceeds the volume of the common resource then all executors slow down (DVFS, throughput) <br />

Resolve recursive frames and events
