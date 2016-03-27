CC = g++
FLAGS = -Wall -g
LIBS = -L. -lclustering -lrt -lm
CLUSTERING_OBJECTS = single_use_barrier.o timespec_functions.o

all: clustering_distribution simple_task simple_task_utilization

simple_task: simple_task.cpp
	$(CC) $(FLAGS) -fopenmp simple_task.cpp task_manager.o -o simple_task $(LIBS)
	
simple_task_utilization: simple_task.cpp
	$(CC) $(FLAGS) -fopenmp simple_task.cpp utilization_calculator.o -o simple_task_utilization $(LIBS)
	
clustering_distribution: libclustering.a utilization_calculator.o task_manager.o clustering_launcher

clustering_launcher: clustering_launcher.cpp
	$(CC) $(FLAGS) clustering_launcher.cpp -o clustering_launcher $(LIBS)

libclustering.a: $(CLUSTERING_OBJECTS)
	ar rcsf libclustering.a $(CLUSTERING_OBJECTS)

utilization_calculator.o: utilization_calculator.cpp
	$(CC) $(FLAGS) -c utilization_calculator.cpp

task_manager.o: task_manager.cpp
	$(CC) $(FLAGS) -c task_manager.cpp
	
single_use_barrier.o: single_use_barrier.cpp
	$(CC) $(FLAGS) -c single_use_barrier.cpp
	
timespec_functions.o: timespec_functions.cpp
	$(CC) $(FLAGS) -c timespec_functions.cpp

clean:
	rm -f *.o *.rtps *.pyc libclustering.a clustering_launcher simple_task simple_task_utilization synthetic_task synthetic_task_utilization
