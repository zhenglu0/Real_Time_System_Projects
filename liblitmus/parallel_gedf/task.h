#ifndef RT_GOMP_TASK_H
#define RT_GOMP_TASK_H

// Task struct type used by task_manager.cpp to control a task.
typedef struct
{
	int (*init)(int argc, char *argv[]);
	int (*run)(int argc, char *argv[]);
	int (*finalize)(int argc, char *argv[]);
}
task_t;

// Task struct that should be defined by the real time task.
extern task_t task;

#endif /* RT_GOMP_TASK_H */
