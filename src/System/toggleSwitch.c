#include "ToggleSwitch.h"
#include "sysInterface.h"

/**
 * \fn ErrorCode initToggleSwitch(Module *parent)
 * \brief Fonction permettant la création d'un module ToggleSwitch
 *
 * \param parent Module auquel on doit donner la fonctionnalité ToggleSwitch, ne peut pas être NULL.
 * \return retourne NO_ERR si le module s'est bien spécialisé en ToggleSwitch, ERR_NOMEM sinon.
 */
ErrorCode initToggleSwitch(Module*);

/**
 * \fn ErrorCode configureToggleSwitch(Module *parent, void* args)
 * \brief Fonction permettant la configuration d'un module ToggleSwitch
 *
 * \param parent Module ToggleSwitch à configurer, ne peut pas être NULL.
 * \param args Argument de type OriginePort*.
 * \return NO_ERR si le module s'est bien configuré, un code d'erreur sinon.
 */
ErrorCode configureToggleSwitch(Module* parent, void* args);

/**
 * \fn ErrorCode updateToggleSwitch(Module *parent, OriginWord port)
 * \brief Fonction permettant la mise à jour d'un module ToggleSwitch
 *
 * \param parent ToggleSwitch à mettre à jour, ne peut pas être NULL.
 * \param port Numéro du port par lequel la mise à jour doit se faire.
 * \return NO_ERR si le module s'est bien mis à jour, un code d'erreur sinon.
 */
ErrorCode updateToggleSwitch(Module*, OriginWord);

/**
 * \fn void resetToggleSwitch(Module *parent, OriginWord port)
 * \brief Fonction permettant le reset d'un module ToggleSwitch
 *
 * \param parent ToggleSwitch à remettre à 0, ne peut pas être NULL.
 * \param port Numéro du port par lequel le reset doit se faire.
 * \return void.
 */
void resetToggleSwitch(Module* parent);


ModuleType toggleSwitchType = {
  .init = initToggleSwitch,
  .config = configureToggleSwitch,
  .update = updateToggleSwitch,
  .reset = resetIdle
};

ErrorCode initToggleSwitch(Module *parent)
{
  ToggleSwitch *toggleSwitch = malloc (sizeof(ToggleSwitch));
  if (toggleSwitch == 0)
  {
    return ERR_NOMEM;
  }

  toggleSwitch->parent = parent;
  parent->fun = (void*)toggleSwitch;
  return NO_ERR;
}

ErrorCode configureToggleSwitch(Module* parent, void* args)
{
  ToggleSwitch *toggleSwitch = (ToggleSwitch*)parent->fun;
  ToggleSwitchConfig* state = (ToggleSwitchConfig*)args;

  toggleSwitch->state = *state;
  return NO_ERR;
}

ErrorCode updateToggleSwitch(Module* parent, OriginWord port)
{
  ErrorCode error;
  ModuleValue value;
  ToggleSwitch *functionnality = (ToggleSwitch*)parent->fun;
  // Si l'interrupteur est fermé, on interrompt le flux.
  if ((*(functionnality->state.value) & functionnality->state.mask) == (functionnality->state.off & functionnality->state.mask))
  {
    value = 0;
  }
  else
  {
    error = updateInput(parent, 0);
    if (error != NO_ERR)
    {
      return error;
    }
    // On récupère l'entrée
    value = getInput(parent, 0);
  }
  
  // On met à jour la sortie ayant pour port <port>
  setOutput(parent, port, value);
  if(parent->isVerbose)
  {
    debug("st: %u\r\n", (uint8_t) (*(functionnality->state.value) & functionnality->state.mask));
  }
  return NO_ERR;
}