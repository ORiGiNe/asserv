#include "derivator.h"
#include "sysInterface.h"

/**
 * \fn ErrorCode initDerivator(Module *parent)
 * \brief Fonction permettant la création d'un module Derivator
 *
 * \param parent Module auquel on doit donner la fonctionnalité Derivator, ne peut pas être NULL.
 * \return retourne NO_ERR si le module s'est bien spécialisé en Derivator, ERR_NOMEM sinon.
 */
ErrorCode initDerivator(Module*);

/**
 * \fn ErrorCode configureDerivator(Module *parent, void* args)
 * \brief Fonction permettant la configuration d'un module Derivator
 *
 * \param parent Module Derivator à configurer, ne peut pas être NULL.
 * \param args Argument inutilisé.
 * \return NO_ERR si le module s'est bien configuré, un code d'erreur sinon.
 */
ErrorCode configureDerivator(Module*, void*);

/**
 * \fn ErrorCode updateDerivator(Module *parent, OriginWord port)
 * \brief Fonction permettant la mise à jour d'un module Derivator
 *
 * \param parent Derivator à mettre à jour, ne peut pas être NULL.
 * \param port Numéro du port par lequel la mise à jour doit se faire.
 * \return NO_ERR si le module s'est bien mis à jour, un code d'erreur sinon.
 */
ErrorCode updateDerivator(Module*, OriginWord);

/**
 * \fn void resetDerivator(Module *parent, OriginWord port)
 * \brief Fonction permettant le reset d'un module Derivator
 *
 * \param parent Derivator à remettre à 0, ne peut pas être NULL.
 * \param port Numéro du port par lequel le reset doit se faire.
 * \return void.
 */
void resetDerivator(Module* parent);


ModuleType derivatorType = {
  .init = initDerivator,
  .config = configureIdle,
  .update = updateDerivator,
  .reset = resetDerivator
};

ErrorCode initDerivator(Module *parent)
{
  Derivator *derivator = malloc (sizeof(Derivator));
  if (derivator == 0)
  {
    return ERR_NOMEM;
  }

  derivator->parent = parent;
  parent->fun = (void*)derivator;
  return NO_ERR;
}

ErrorCode updateDerivator(Module* parent, OriginWord port){
  ErrorCode error;
  ModuleValue value;
  ModuleValue deriv;

  // On met à jour l'entrée
  error = updateInput(parent, 0);
  if (error != NO_ERR)
  {
    return error;
  }
  // On récupère l'entrée
  value = getInput(parent, 0);
  // On calcule la derivée
  deriv = value - ((Derivator*)parent->fun)->oldValue;
  // On stock la valeur reçu pour le prochain calcul
  ((Derivator*)parent->fun)->oldValue = value;
  // On met à jour la sortie ayant pour port <port>
  setOutput(parent, port, deriv);

  return NO_ERR;
}

void resetDerivator(Module* parent)
{
  Derivator *derivator = (Derivator*)parent->fun;
  derivator->oldValue = 0;
}
