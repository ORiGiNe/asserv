#ifndef ASSERV_ASSERV_H
#define ASSERV_ASSERV_H

#include "types.h"
#include "defines.h"

#include "module.h"

#ifdef __cplusplus
extern "C"
{
#endif



typedef struct opFunc	 OpFunc;
typedef struct asserv	 Asserv;


struct opFunc
{
  ModuleValue (*h1)(ModuleValue);
  ModuleValue (*h2)(ModuleValue);
  ModuleValue (*h3)(ModuleValue);
};

struct asserv
{
  Module *parent;
  ModuleValue oldError; // Erreur 
  ModuleValue integral; // Permet de connaitre l'intégrale de l'erreur
  OpFunc h; // fonctions remplissant le role des blocs H1, H2, et H3
};


void *initAsserv (Module *, void*);
ErrorCode updateAsserv(Module*, OriginWord);


#ifdef __cplusplus
}
#endif
#endif /* INC__IO */
