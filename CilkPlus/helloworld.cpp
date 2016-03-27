#include <iostream>
#include <cilk/cilk.h>
#include <cilk/cilk_api.h>

using namespace std; 

#define num_iteration 1000000

static void hello()
{
    cilk_for(int i = 0; i < num_iteration; i++) 
    {}    
    cout << "Hello " << endl;

    cilk_for(int i = 0; i < num_iteration; i++) 
    {}
    cout << "world! " << endl;
}

static void world()
{
    cilk_for(int i = 0; i < num_iteration; i++) 
    {}
    cout << "zheng! " << endl;
}

int main()
{
    __cilkrts_set_param("nworkers", "4");

    int nw = __cilkrts_get_nworkers();
    cout << "__cilkrts_get_nworkers() = " << nw << endl;

    int tw = __cilkrts_get_total_workers();
    cout << "___cilkrts_get_total_workers() = " << tw << endl;

    __cilkrts_init();

    for (int i = 0; i < 1000; ++i) {
        cilk_for(int i = 0; i < num_iteration; i++)
        { int tmp = i + 1;}
    }

    cout << "Done! "<< endl;
    __cilkrts_end_cilk();

    return 0;
}
