#include "ToggleSwitch.h"
#include "sysInterface.h"

/**
 * \fn ErrorCode initToggleSwitch(Module *parent)
 * \brief Fonction permettant la cr�ation d'un module ToggleSwitch
 *
 * \param parent Module auquel on doit donner la fonctionnalit� ToggleSwitch, ne peut pas �tre NULL.
 * \return retourne NO_ERR si le module s'est bien sp�cialis� en ToggleSwitch, ERR_NOMEM sinon.
 */
ErrorCode initToggleSwitch(Module*);

/**
 * \fn ErrorCode configureToggleSwitch(Module *parent, void* args)
 * \brief Fonction permettant la configuration d'un module ToggleSwitch
 *
 * \param parent Module ToggleSwitch � configurer, ne peut pas �tre NULL.
 * \param args Argument de type OriginePort*.
 * \return NO_ERR si le module s'est bien configur�, un code d'erreur sinon.
 */
ErrorCode configureToggleSwitch(Module* parent, void* args);

/**
 * \fn ErrorCode updateToggleSwitch(Module *parent, OriginWord port)
 * \brief Fonction permettant la mise � jour d'un module ToggleSwitch
 *
 * \param parent ToggleSwitch � mettre � jour, ne peut pas �tre NULL.
 * \param port Num�ro du port par lequel la mise � jour doit se faire.
 * \return NO_ERR si le module s'est bien mis � jour, un code d'erreur sinon.
 */
ErrorCode updateToggleSwitch(Module*, OriginWord);

/**
 * \fn void resetToggleSwitch(Module *parent, OriginWord port)
 * \brief Fonction permettant le reset d'un module ToggleSwitch
 *
 * \param parent ToggleSwitch � remettre � 0, ne peut pas �tre NULL.
 * \param port Num�ro du port par lequel le reset doit se faire.
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
  // Si l'interrupteur est ferm�, on interrompt le flux.
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
    // On r�cup�re l'entr�e
    value = getInput(parent, 0);
  }
  
  // On met � jour la sortie ayant pour port <port>
  setOutput(parent, port, value);
  if(parent->isVerbose)
  {
    debug("st: %u\r\n", (uint8_t) (*(functionnality->state.value) & functionnality->state.mask));
  }
  return NO_ERR;
}