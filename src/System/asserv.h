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
} Asserv;


ModuleType asservType;

#ifdef __cplusplus
}
#endif

#endif
