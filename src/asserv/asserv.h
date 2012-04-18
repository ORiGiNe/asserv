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
typedef enum inputAsserv InputAsserv;

enum inputAsserv
{
  AsservKp,
  AsservKi,
  AsservKd,
//  AsservAccuracy,
  AsservCommand,
  AsservDeriv,
  AsservMeasure
};
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
  ModuleValue integral; // Permet de connaitre l'int�grale de l'erreur
  OpFunc h; // fonctions remplissant le role des blocs H1, H2, et H3
};


void *initAsserv (Module *);
ErrorCode configureAsserv (Module *, void*);
ErrorCode updateAsserv(Module*, OriginWord);


#ifdef __cplusplus
}
#endif
#endif /* INC__IO */
