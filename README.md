# Evaluation of the Static Scheduler
SystemC Simulator of the Static Scheduler

Timing, timing hierarchy, time tick issues a named event ot a scheduler <br />
```
frames: 
[{ "name": <this_frame_string_name>, 
   "seq":  [{"delay":<time+unit>}, 
            {"event":<event_string_name>}, 
            {"frame":<another_frame_string_name>}, 
            .....
           ]
 },          
.....
]

generate:
[{ "name": <gen_string_name>, 
   "seq":  [{"delay":<time+unit>}, 
            {"event":<event_string_name>}, 
            {"frame":<frame_string_name>}, 
            .....
           ]
 },          
.....
]
```
Tasks, task dependencies, task durations, list of the executors for the task has the form: (resA | resB | ... ) & ( resX | resY ) & resZ <br />
Executors. each executor can use a part of a common pool <br />
common pools: if requested common resource exceeds the volume of the common pool then all executors slow down (DVFS, througput) <br />

