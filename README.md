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
Threads: triggering events (regex), generated events, tasks <br />
Tasks: task durations, list of the executors for the task in the form: (execA | execB | ... ) & ( execX | execY ) & execZ <br />
Executors: each executor can occupy a fraction of a common resource  <br />
Common resource: if requested common resource exceeds the volume of the common resource then all executors slow down (DVFS, throughput) <br />

Resolve recursive frames and events
