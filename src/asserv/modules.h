#ifndef ...
#define ...

#ifdef __cplusplus
extern "C"
{
#endif


typedef struct module Module;
typedef enum moduleType ModuleType;
typedef struct moduleInput ModuleInput;

struct moduleInput
{
  Module *module;
  OriginWord port;
};


struct module
{
  ModuleValue *outputs;
  OriginWord nbOutputs;

  ModuleInput *inputs;
  OriginWord nbInputs;

  ModuleType type;
  void *fun;

  ErrorCode (*update)(Module*);
};



#ifdef __cplusplus
}
#endif


#endif
