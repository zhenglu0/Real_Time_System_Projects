#include <stdio.h>
#include <omp.h>

void doTaskA()
{
  printf("A\n");
}

void doTaskB()
{
  printf("B\n");
}

void doTaskG()
{
  printf("G\n");
}

void doTaskH()
{
  printf("H\n");
}

int main()

{
#pragma omp parallel sections

  {

#pragma omp section

    { 

      doTaskA();

      doTaskB();

    }

    // … other tasks here

#pragma omp section

    { 

      doTaskG();

      doTaskH();

    }

  }

  return 0;

}
