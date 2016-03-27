#SG: This makefile uses both the regular instalation of GCC and the "cilkplus" branch
CC = g++
CILKCC = /home/jing/cilkplus/cilkplus-install/bin/g++
FLAGS = -Wall -g
CILK = -ldl -fcilkplus -lcilkrts -lpthread
INC = -I/home/jing/cilkplus/cilkplus-install/include/cilk
LIBS = -L. -lm -lclustering -lrt
LINKER_OPTS = -Wl,-rpath=/home/jing/cilkplus/cilkplus-install/lib/
CLUSTERING_OBJECTS = single_use_barrier.o timespec_functions.o

all: clustering_distribution simple_task simple_task_cilk simple_task_utilization simple_task_utilization_cilk synthetic_task synthetic_task_cilk synthetic_task_utilization synthetic_task_sequential synthetic_task_utilization_cilk

#SG: The executable name and the code file name will be different for sequential or cilk version. This is for synthetic_task experiment only.

#synthetic_task
synthetic_task: synthetic_task.cpp
	$(CC) $(FLAGS) -fopenmp synthetic_task.cpp task_manager.o -o synthetic_task $(LIBS)

synthetic_task_cilk: synthetic_task_cilk.cpp
	$(CILKCC) $(FLAGS) $(CILK) synthetic_task_cilk.cpp task_manager_cilk.o -o synthetic_task_cilk $(INC) $(LIBS) $(LINKER_OPTS)

synthetic_task_sequential: synthetic_task.cpp
	$(CC) $(FLAGS) synthetic_task.cpp task_manager_sequential.o -o synthetic_task_sequential $(LIBS)

#synthetic_task_utilization
synthetic_task_utilization: synthetic_task.cpp
	$(CC) $(FLAGS) -fopenmp synthetic_task.cpp utilization_calculator.o -o synthetic_task_utilization $(LIBS)

synthetic_task_utilization_cilk: synthetic_task_cilk.cpp
	$(CILKCC) $(FLAGS) $(CILK) synthetic_task_cilk.cpp utilization_calculator_cilk.o -o synthetic_task_utilization_cilk $(INC) $(LIBS) $(LINKER_OPTS)

#simple_task and utilization
simple_task: simple_task.cpp
	$(CC) $(FLAGS) -fopenmp simple_task.cpp task_manager.o -o simple_task $(LIBS)

simple_task_cilk: simple_task_cilk.cpp
	$(CILKCC) $(FLAGS) $(CILK) simple_task_cilk.cpp task_manager_cilk.o -o simple_task_cilk $(INC) $(LIBS) $(LINKER_OPTS)

simple_task_utilization: simple_task.cpp
	$(CC) $(FLAGS) -fopenmp simple_task.cpp utilization_calculator.o -o simple_task_utilization $(LIBS)

simple_task_utilization_cilk: simple_task_cilk.cpp
	$(CILKCC) $(FLAGS) $(CILK) simple_task_cilk.cpp utilization_calculator_cilk.o -o simple_task_utilization_cilk $(INC) $(LIBS) $(LINKER_OPTS)

#clustering_distribution
clustering_distribution: libclustering.a utilization_calculator.o utilization_calculator_cilk.o task_manager.o task_manager_sequential.o task_manager_cilk.o clustering_launcher

#???_cilk.cpp
clustering_launcher: clustering_launcher.cpp
	$(CC) $(FLAGS) clustering_launcher.cpp -o clustering_launcher $(LIBS)

libclustering.a: $(CLUSTERING_OBJECTS)
	ar rcsf libclustering.a $(CLUSTERING_OBJECTS)

utilization_calculator.o: utilization_calculator.cpp
	$(CC) $(FLAGS) -c utilization_calculator.cpp

utilization_calculator_cilk.o: utilization_calculator_cilk.cpp
	$(CILKCC) $(FLAGS) $(CILK) -c utilization_calculator_cilk.cpp $(INC) $(LINKER_OPTS)

task_manager.o: task_manager.cpp
	$(CC) $(FLAGS) -c task_manager.cpp
	
task_manager_sequential.o: task_manager_sequential.cpp
	$(CC) $(FLAGS) -c task_manager_sequential.cpp

task_manager_cilk.o: task_manager_cilk.cpp
	$(CILKCC) $(FLAGS) $(CILK) -c task_manager_cilk.cpp $(INC) $(LINKER_OPTS)

single_use_barrier.o: single_use_barrier.cpp
	$(CC) $(FLAGS) -c single_use_barrier.cpp 

timespec_functions.o: timespec_functions.cpp
	$(CC) $(FLAGS) -c timespec_functions.cpp

clean:
	rm -f *.o *.rtps *.pyc libclustering.a clustering_launcher simple_task simple_task_utilization synthetic_task synthetic_task_utilization simple_task_cilk simple_task_utilization_cilk synthetic_task_cilk synthetic_task_utilization_cilk synthetic_task_sequential
