/**
 * \file operator.c
 * \brief Implémentation du module générique de calcul
 * \author Johwn
 * \date 14 avril 2012
 *
 * Permet de créer un module permettant d'effectuer des opérations sur un nombre
 * d'entrée et de sortie variable
 *
 */

#include "operator.h"
#include "sysInterface.h"

ErrorCode initOperator(Module*);
ErrorCode configureOperator(Module*, void*);
ErrorCode updateOperator(Module*, OriginWord);

ModuleType operatorType = {
  .init = initOperator,
  .config = configureOperator,
  .update = updateOperator,
  .reset = resetIdle
};


/**
 * \fn ErrorCode initOperator(Module *parent)
 * \brief Fonction permettant la création d'un module Operator
 *
 * \param parent Module auquel on doit donner la fonctionnalité Operator, ne peut pas être NULL.
 * \return NO_ERR si pas d'erreur, autre chose sinon.
 */
ErrorCode initOperator(Module *parent)
{
  Operator *operator = malloc (sizeof(Operator));
  if (operator == 0)
  {
    return ERR_NOMEM;
  }
  operator->parent = parent;
  parent->fun = (void*)operator;
  return NO_ERR;
}

/**
 * \fn ErrorCode configureOperator(Module *parent, void* args)
 * \brief Fonction permettant la configuration d'un module Operator
 *
 * \param parent Module contenant Operator à configurer, ne peut pas être NULL.
 * \return Module Operator configuré.
 */
ErrorCode configureOperator(Module* parent, void* args)
{
  ((Operator*)parent->fun)->func  = (OperatorFunction)args;
  return NO_ERR;
}

/*!
 * \fn ErrorCode updateOperator(Module *parent, void* args)
 * \brief Permet de mettre à jour Operator
 *
 * \param parent Module contenant Operator à mettre à jour, ne peut pas être NULL.
 * \return ALREADY_UPTODATE si déjà à jour, NO_ERR si pas à jour, autre erreur sinon.
 */
ErrorCode updateOperator(Module* parent, OriginWord port)
{
  (void) port;
  uint16_t i;
  ModuleValue result;
  ErrorCode error;
  
  // On met à jour les entrées
  for(i=0; i<parent->nbInputs; i++)
  {
    error = updateInput(parent, i); 
    if(error != NO_ERR)
    {
      return error;
    }
  }
   
  for(i=0; i<parent->nbOutputs; i++)
  {
    result = (((Operator*)parent->fun)->func)(parent, i);
    setOutput(parent, i, result);
  }
  if(parent->isVerbose)
  {
    debug("opr: %l\tp: %l\r\n", (uint32_t)result, (uint32_t)port);
  }
  return NO_ERR;
}


ModuleValue funCalcValueForMotor(Module* parent, OriginWord port)
{
  // debug("oii: %l %l\r\n", (uint32_t)getInput(parent, 0), (uint32_t)getInput(parent, 1));
  switch(port)
  {
    case 0:
      return getInput(parent, 0) + getInput(parent, 1);
    case 1:
      return getInput(parent, 0) - getInput(parent, 1);
    default:
      break;
  }
  return 0;
}

ModuleValue funCalcValueForAsserv(Module* parent, OriginWord port)
{
  switch(port)
  {
    case 0:
      return (getInput(parent, 0) + getInput(parent, 1)) / 2;
    case 1:
      return (getInput(parent, 0) - getInput(parent, 1)) / 2;
    default:
      break;
  }
  return 0;
}
