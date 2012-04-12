#ifndef ...
#define ...

#ifdef __cplusplus
extern "C"
{
#endif


typedef struct module Module;
typedef enum moduleType ModuleType;
typedef struct moduleInput ModuleInput;
typedef struct moduleOutput ModuleOutput;

struct moduleInput
{
  Module *module;
  OriginWord port;
};

struct moduleOutput
{
  ModuleValue value;
  OriginBool upToDate;
};

struct module
{
  ModuleOutput *outputs;
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
