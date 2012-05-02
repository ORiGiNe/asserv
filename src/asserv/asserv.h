/**
 * \file asserv.h
 * \brief Impl�mentation d'un module g�n�rique d'asservissement PID
 * \author Johwn
 * \date 14 avril 2012
 *
 * Permet de cr�er un module permettant d'asservir son entr�e en PID.
 */

#ifndef ASSERV_ASSERV_H
#define ASSERV_ASSERV_H

#include "types.h"
#include "defines.h"
#include "module.h"

#ifdef __cplusplus
extern "C"
{
#endif


/**
 * \enum InputAsserv
 * \brief D�finition des ports d'entr�es d'un asservissement
 *
 * InputAsserv est un ensemble de constantes pr�d�finies permettant
 * de sp�cifier le port d'entr�e de l'asservissement.
 */
typedef enum
{
  AsservKp, /*! Port contenant Kp */
  AsservKi, /*! Port contenant Ki */
  AsservKd, /*! Port contenant Kd */
  AsservCommand, /*! Port contenant la commande d'entr�e */
  AsservDeriv, /*! Port contenant la d�riv�e maximale de l'entr�e */
  AsservMeasure /*! Port contenant la mesure */
} InputAsserv;

/**
 * \struct OpFunc
 * \brief Structure permettant la configuration d'un asservissement via configureModule
 * 
 * OpFunc contient trois pointeurs de fonctions permettant de faire des conversions sur
 * les deux entr�es variables de l'asservissement et la sortie (commande : h1, measure : h2,
 * sortie : h3).
 */
typedef struct
{
  ModuleValue (*h1)(ModuleValue);
  ModuleValue (*h2)(ModuleValue);
  ModuleValue (*h3)(ModuleValue);
} OpFunc;

/**
 * \struct Asserv
 * \brief Structure contenant la fonctionnalit� Asserv
 *
 * Asserv contient le minimum des donn�es utiles � l'asservissement,
 * ainsi que le bloc OpFunc de l'asservissement
 */
typedef struct
{
  Module *parent;
  ModuleValue oldError; // Erreur 
  ModuleValue integral; // Permet de connaitre l'int�grale de l'erreur
  OpFunc h; // fonctions remplissant le role des blocs H1, H2, et H3
} Asserv;


/**
 * \fn ErrorCode initAsserv(Module *parent)
 * \brief Fonction d'initialisation d'une fonctionnalit� Asserv
 *
 * \param parent Module contenant la fonctionnalit� Asserv
 * \return Retourne NO_ERR si le module a �t� sp�cialis�, ERR_NOMEM si plus de m�moire
 */
ErrorCode initAsserv (Module *parent);

/**
 * \fn ErrorCode configureAsserv(Module *parent, void *args)
 * \brief Fonction de configuration d'une fonctionnalit� Asserv.
 *
 * \param parent Module contenant la fonctionnalit� Asserv � configurer.
 * \param args pointeur vers une structure de type OpFunc.
 * \return Retourne NO_ERR si le module s'est bien configur�, l'erreur correspondante sinon.
 */
ErrorCode configureAsserv (Module *parent, void *args);

/**
 * \fn ErrorCode updateAsserv(Module *parent, OriginWord port)
 * \brief Fonction d'update d'une fonctionnalit� Asserv
 *
 * \param parent Module contenant la fonctionnalit� Asserv � mettre � jour
 * \param port Port du module � configurer
 * \return NO_ERR si le module s'est bien mis � jour, l'erreur correspondante sinon.
 */
ErrorCode updateAsserv(Module *parent, OriginWord port);

ModuleType asservType;

#ifdef __cplusplus
}
#endif

#endif
