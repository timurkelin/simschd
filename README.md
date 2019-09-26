# Evaluation of the Static Scheduler
SystemC Simulator of the Static Scheduler

Timing, timing hierarchy, time tick issues a named event to the scheduler <br />
```
frames: 
[{ "name": <this_frame_string_name>, 
   "seq":  [{"delay":<time+unit>}, OR
            {"event":<event_string_name>}, OR
            {"frame":<another_frame_string_name>}, 
            .....
           ]
 },          
.....
]

generate:
[{ "name": <gen_string_name>, 
   "seq":  [{"delay":<time+unit>}, OR
            {"event":<event_string_name>}, OR
            {"frame":<frame_string_name>}, OR
            .....
           ]
 },          
.....
]
```
Threads: triggering events (regex), generated events, tasks
Tasks: task durations, list of the executors for the task in the form: (resA | resB | ... ) & ( resX | resY ) & resZ <br />
Executors. each executor can occupy a fraction of a common pool <br />
common pools: if requested common resource exceeds the volume of the common pool then all executors slow down (DVFS, throughput) <br />

Resolve recursive frames and events
