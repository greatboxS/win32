#ifndef __D_LIB_H__
#define __D_LIB_H__

extern __declspec(dllexport) int local_var;

class __declspec(dllexport)  ClassA
{
private:
    static ClassA *mInstance;
public:
    static ClassA *GetInstance();
    static void DestroyInstance();

    void Open();
    void Close();
};
#endif // __D_LIB_H__