#include "integrator.h"
#include "sysInterface.h"

/**
 * \fn ErrorCode initIntegrator(Module *parent)
 * \brief Fonction permettant la création d'un module Integrator
 *
 * \param parent Module auquel on doit donner la fonctionnalité Integrator, ne peut pas être NULL.
 * \return retourne NO_ERR si le module s'est bien spécialisé en Integrator, ERR_NOMEM sinon.
 */
ErrorCode initIntegrator(Module*);


/**
 * \fn ErrorCode updateIntegrator(Module *parent, OriginWord port)
 * \brief Fonction permettant la mise à jour d'un module Integrator
 *
 * \param parent Integrator à mettre à jour, ne peut pas être NULL.
 * \param port Numéro du port par lequel la mise à jour doit se faire.
 * \return NO_ERR si le module s'est bien mis à jour, un code d'erreur sinon.
 */
ErrorCode updateIntegrator(Module*, OriginWord);

/**
 * \fn void resetIntegrator(Module *parent, OriginWord port)
 * \brief Fonction permettant le reset d'un module Integrator
 *
 * \param parent Integrator à remettre à 0, ne peut pas être NULL.
 * \param port Numéro du port par lequel le reset doit se faire.
 * \return void.
 */
void resetIntegrator(Module* parent);


ModuleType integratorType = {
  .init = initIntegrator,
  .config = configureIdle,
  .update = updateIntegrator,
  .reset = resetIntegrator
};

ErrorCode initIntegrator(Module *parent)
{
  Integrator *integrator = malloc (sizeof(Integrator));
  if (integrator == 0)
  {
    return ERR_NOMEM;
  }

  integrator->parent = parent;
  parent->fun = (void*)integrator;
  return NO_ERR;
}

ErrorCode updateIntegrator(Module* parent, OriginWord port){
  ErrorCode error;
  ModuleValue value;
  
  // debug("F");
  
  // On met à jour l'entrée
  error = updateInput(parent, 0);
  if (error != NO_ERR)
  {
    return error;
  }
  
  // debug("F : 0x%l\r\n", (uint32_t)getInput(parent, 0));
  
  
  // On récupère l'entrée
  value = getInput(parent, 0);
  // On met à jour l'integrale
  ((Integrator*)parent->fun)->integrale += value;
  // On met à jour la sortie ayant pour port <port>
  setOutput(parent, port, ((Integrator*)parent->fun)->integrale);
  if(parent->isVerbose)
  {
    debug("\nint : 0x%l\r\n", (uint32_t)((Integrator*)parent->fun)->integrale);
  }
  return NO_ERR;
}

void resetIntegrator(Module* parent)
{
  Integrator *integrator = (Integrator*)parent->fun;
  integrator->integrale = 0;
}
