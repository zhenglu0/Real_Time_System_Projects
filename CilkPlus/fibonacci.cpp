#include <iostream>
#include <cilk/cilk.h>
#include <cilk/cilk_api.h>

using namespace std; 

int fib(int n)
{
    if (n < 2)
        return n;
    else
    {
        int x, y;

        x = cilk_spawn fib(n-1);
        y = cilk_spawn fib(n-2);

        cilk_sync;

        return (x+y);
    }
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
        fib (30);
    }
    __cilkrts_end_cilk();

    return 0;
}
