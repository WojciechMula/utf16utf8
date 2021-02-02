#pragma once

#define RUNINS( procedure) \
        {\
        event_collector collector;\
        event_aggregate all{};\
        for(size_t i = 0; i < repeat; i++) {\
          collector.start();\
          procedure();\
          event_count allocate_count = collector.end();\
          all << allocate_count;\
        }\
        double freq = (all.best.cycles() / all.best.elapsed_sec()) / 1000000000.0;\
        double insperunit = all.best.instructions() / double(volume);\
        double gbs = double(volume) / all.best.elapsed_ns();\
        if(collector.has_events()) {\
         printf("                               %8.3f ins/byte, %8.3f GHz, %8.3f GB/s \n", insperunit, freq, gbs);\
        } else {\
         printf("                               %8.3f GB/s \n", gbs);\
        }\
        }

#define RUN(name, procedure) \
    printf("%-30s\t: ", name); fflush(stdout);\
    RUNINS(procedure)
