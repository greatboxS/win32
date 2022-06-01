#include "d_lib.h"
#include <stdio.h>

ClassA *ClassA::mInstance = nullptr;

 int local_var = 0;

ClassA *ClassA::GetInstance()
{
    local_var++;
    if (!mInstance)
        mInstance = new ClassA;
    return mInstance;
}

void ClassA::DestroyInstance()
{
    local_var++;

    if(mInstance)
        delete mInstance;
}

void ClassA::Open()
{
    local_var++;

    printf("ClassA Open(%d)\n", local_var);
}

void ClassA::Close()
{
    local_var++;

    printf("ClassA Close(%d)\n", local_var);
}
