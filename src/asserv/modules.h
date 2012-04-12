#ifndef ...
#define ...

#ifdef __cplusplus
extern "C"
{
#endif


typedef struct module Module;
typedef enum moduleType ModuleType;


struct module
{
  ModuleValue *output;
  OriginWord nbOutputs;

  Module *inputModule;
  OriginWord nbInput;

  ModuleType type;
  void *functionnality;

};



#ifdef __cplusplus
}
#endif


#endif
