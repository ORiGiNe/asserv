#include "ifaceme.h"
#include "sysInterface.h"

/**
 * \fn ErrorCode initIfaceME(Module *parent)
 * \brief Fonction permettant la création d'un module IfaceME
 *
 * \param parent Module auquel on doit donner la fonctionnalité IfaceME, ne peut pas être NULL.
 * \return retourne NO_ERR si le module s'est bien spécialisé en IfaceME, ERR_NOMEM sinon.
 */
ErrorCode initIfaceME(Module*);

/**
 * \fn ErrorCode configureIfaceME(Module *parent, void* args)
 * \brief Fonction permettant la configuration d'un module IfaceME
 *
 * \param parent Module IfaceME à configurer, ne peut pas être NULL.
 * \param args Argument de type IME.
 * \return NO_ERR si le module s'est bien configuré, un code d'erreur sinon.
 */
ErrorCode configureIfaceME(Module*, void*);

/**
 * \fn ErrorCode updateIfaceME(Module *parent, OriginWord port)
 * \brief Fonction permettant la mise à jour d'un module IfaceME
 *
 * \param parent IfaceME à mettre à jour, ne peut pas être NULL.
 * \param port Numéro du port par lequel la mise à jour doit se faire.
 * \return NO_ERR si le module s'est bien mis à jour, un code d'erreur sinon.
 */
ErrorCode updateIfaceME(Module*, OriginWord);

void resetIfaceME(Module* parent);


ModuleType ifaceMEType = {
  .init = initIfaceME,
  .config = configureIfaceME,
  .update = updateIfaceME,
  .reset = resetIfaceME
};

ErrorCode initIfaceME(Module *parent)
{
  IfaceME *ifaceme = malloc (sizeof(IfaceME));
  if (ifaceme == 0)
  {
    return ERR_NOMEM;
  }

  ifaceme->parent = parent;
  parent->fun = (void*)ifaceme;
  return NO_ERR;
}

ErrorCode configureIfaceME(Module *parent, void* args)
{
  IfaceME *ifaceme = (IfaceME*)parent->fun;
  IME* ime = (IME*)args;

  parent->ctl->coveredDistance = 0;

 // ime->resetEncoderValue(&ime->motor); TODO Pour test, à rajouter !
  ifaceme->ime = ime;
  ifaceme->measureUpToDate = 0;
  ifaceme->measure = 0;
  return NO_ERR;
}

ErrorCode updateIfaceME(Module* parent, OriginWord port){
  ModuleValue command;
  ErrorCode error;
  IME *ime = ((IfaceME*)parent->fun)->ime;

  // debug("I");
  // Faire la mesure ssi la mesure n'est plus valable
  if (((IfaceME*)parent->fun)->measureUpToDate == 0)
  {
     // debug("id: 0x%l\r\n", (uint32_t)ime->motor.id);
     
    // On effectue la mesure
    ((IfaceME*)parent->fun)->measure = ime->getEncoderValue(&ime->motor);
    ((IfaceME*)parent->fun)->measureUpToDate = 1;
    parent->ctl->coveredDistance = ((IfaceME*)parent->fun)->measure;
    
    // On met à jour la sortie
    setOutput(parent, port, ((IfaceME*)parent->fun)->measure);
    
    // On met à jour l'entrée
    error = updateInput(parent, 0);
    if (error != NO_ERR)
    {
      return error;
    }
    command = getInput(parent, 0);
    // On envoie la commande au système
    if(parent->ctl->stop == true)
    {
      return ERR_URGENT_STOP;
    }

    ((IfaceME*)parent->fun)->measureUpToDate = 0;
    ime->sendNewCommand(&ime->motor, command);
    
    if (parent->isVerbose)
    { 
      debug("w: 0x%l\r\n", (uint32_t)command);
    }
  }
  else
  {
    // On met à jour la sortie ayant pour port <port>
    setOutput(parent, port, ((IfaceME*)parent->fun)->measure);
  }
  return NO_ERR;
}

void resetIfaceME(Module* parent)
{
  IfaceME *ifaceME = (IfaceME*)parent->fun;
  ifaceME->measure = 0;
  ifaceME->measureUpToDate = false;
  ifaceME->ime->resetEncoderValue(&ifaceME->ime->motor);
}
