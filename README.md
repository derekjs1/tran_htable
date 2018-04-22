# tran_htable
When attempting to replicate the results that we achieved with the RSTM library a few modifications
to the cmake files are necessary in addition to those that were outlined in the project guidelines.
I have included the modified files with _c++11 appended to them that has the code necessary to make the
RSTM library build with c++11 compatibility. In addition to the c++11 files there are the header and main
that run the data structure. These files are wired up in such a way to be built with the rest of the bench marks
making use of their bench mark system, so when the benchmarks are built our data structure will produce an executable
that will be placed in the same location of the other bench mark executables. The change as specified in the project 
guidelines mentions placing "main" at line 22 we want you to place "main" at line 36. If it is placed 22 none of the bench marks will be built and the entire process will not work. I have also added a shell script that varies the number of threads by running the executable and passing in a thread parameter the benchmark system knows how to handle.

Modifying [rstm_path]/bench/bmharness.cpp:

This file will have to be modified to correctly use the Config in a way that our hash set expects. At line 26 there is Config::Config(). This needs to be modified as follows:

Config::Config() :
    bmname(""),
    duration(4),
    execute(500000),
    threads(1),
    nops_after_tx(0),
    elements(262144),
    lookpct(90),
    inspct(10),
    sets(1),
    ops(1),
    time(0),
    running(true),
    txcount(0)
{
}

After this has been modified appropriately this will produce the correct executable consistent with how we produced results for our report.

Place these files in: [rstm path]/bench/
  
1. t_htbl.hpp

2. main.cpp

Take the "_c++11" out of filename for the corresponding files.
These should go right in the root rsmt directory. (You are essentially overwriting what is already there so there will be a notification letting you know you're doing something bad.)

Place this file in : [rstm_path]/build/bench

1.  thread_loop.sh
