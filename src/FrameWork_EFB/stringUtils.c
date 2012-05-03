#include "stringUtils.h"

/* -----------------------------------------------------------------------------
 * byteToString
 * -----------------------------------------------------------------------------
 */
tEFBstring byteToString (byte b)
{
  uint8_t rest;
  int i = 0;
  uint8_t acc;
  acc = b;
  // La fonction retourne une variable statique, il faut donc utiliser le résultat avant de rappeler la fonction.
  static byte string[4];

  while (i < 3 && acc != 0)
  {
    rest = acc % 10;
    acc = (acc - rest )/ 10;
    string[i] = '0' + rest;
    i++;
  }

  /* On doit maintenant inverser le sens du string (les chiffres plus significatif au debut)
  Marche à tous les coup pour un octet (ecriture decimale sur 3 charactère au maximum
  si i=1, c'est à dire un chiffre: pas besoin de l'inverser, mais on met un 0 devant.
  si i=2, un nombre entre 10 et 99, on inverse string[i-1]=string[1] avec string[0]
  si i=3, un nombre entre 100 et 255, on inverse les chiffres aux extrèmités, string[i-1]=string[2] et string[0];*/

  if (i == 0)
  {
  	string[0] = '0';
    string[1] = '0';
    string[2] = '\0';
  }
  else if (i == 1)
  {
    string[1] = string[0];
    string[0] = '0';
    string[2] = '\0';
  }
  else
  {
    acc = string[i-1];
    string[i-1] = string[0];
    string[0] = acc;
    string[i] = '\0';
  }

  return string;
} /* byteToString */

/* -----------------------------------------------------------------------------
 * byteToHexaString
 * -----------------------------------------------------------------------------
 */
tEFBstring byteToHexaString (byte b)
{
  int8_t rest;
  int i = 0;
  uint8_t acc;
  acc = b;
  // La fonction retourne une variable statique, il faut donc utiliser le résultat avant de rappeler la fonction.
  static byte string[3];

  while (i < 2 && acc != 0)
  {
    rest = acc % 16;
    acc = (acc - rest )/ 16;
    if (rest < 10)
    {
      string[i] = '0' + rest;
    }
    else
    {
      // Exemple: si rest = 11, la lettre doit être B. Et on obtient grâce à ce calcul: 'A' + 11 -10 = 'A' + 1 = 'B'
      string[i] = 'A' + rest - 10;
    }
    i++;
  }

  // On inverse ensuite.
  if (i == 0)
  {
  	string[0] = '0';
    string[1] = '0';
    string[2] = '\0';
  }
  else if (i == 1)
  {
    string[1] = string[0];
    string[0] = '0';
    string[2] = '\0';
  }
  else // i = 2
  {
    acc = string[1];
    string[1] = string[0];
    string[0] = acc;
    string[2] = '\0';
  }

  return string;
} // byteToHexaString


/* -----------------------------------------------------------------------------
 * longToHexaString
 * -----------------------------------------------------------------------------
 */
tEFBstring longToHexaString (uint32_t L)
{
    tEFBstring pointeurByte;
    static unsigned char string[9];
    int i;
    *string = '\0';
    /* On cast le pointeur de long en un pointeur de char afin de lire le long comme quatre octet.
       On se place sur l'octet de poids fort et on décremente le pointeur. */
    pointeurByte = ((tEFBstring) &L) + 3;
    for (i = 0; i < 4; i++)
    {
      ustrcat (string, byteToHexaString (*pointeurByte));
      pointeurByte--;
    }
    return string;
} // longToHexaString


/* -----------------------------------------------------------------------------
 * ustrncat
 *
 * Si n = 0, on copie tous les char de source à la suite de dest.
 * -----------------------------------------------------------------------------
 */
tEFBerrCode ustrncat (tEFBstring dest, tEFBstring source, uint8_t n)
{
  unsigned int i = 0;
  unsigned int m = n;

  if (dest == NULL || source == NULL)
  {
    return EFBERR_BADARGS;
  }

  while (*dest != '\0')
  {
    dest++;
  }

  if (m == 0)
  {
    m = 0xffff;
  }

  while (*source != '\0' && i < m)
  {
    *dest = *source;
    dest++;
    source++;
    i++;
  }

  *dest = '\0';

  return EFB_OK;

} // ustrncat

/* -----------------------------------------------------------------------------
 * usprintf
 * -----------------------------------------------------------------------------
 */
tEFBerrCode usprintf (tEFBstring stringOut, char * formatEFBstring, ...)
{
  byte c;
  tEFBstring s;
  tEFBerrCode errCode;

  if (stringOut == NULL || formatEFBstring == NULL)
  {
    return EFBERR_BADARGS;
  }

  va_list ap;
  va_start (ap, formatEFBstring);
  *stringOut = '\0';

  while (*formatEFBstring != '\0')
  {
    if (*formatEFBstring == '%')
    {
      formatEFBstring++;
      switch (*formatEFBstring)
      {
        case 'u' :   // un byte en decimal
          c = (byte) va_arg (ap, int);
          errCode = ustrcat (stringOut, byteToString (c));
          if (errCode != EFB_OK)
          {
            va_end(ap);
            return errCode;
          }
          stringOut += 2; // On sait, qu'un byte en décimal s'ecrit sur au moins deux chiffres ( 00 01 02 03 ... 98 99 100 ... 254 255 ).
          break;

        case 'h' :   // un byte en hexa
          c = (byte) va_arg (ap, int);
          errCode = ustrcat (stringOut, byteToHexaString (c));
          if (errCode != EFB_OK)
          {
            va_end(ap);
            return errCode;
          }
          stringOut += 2; // On sait, qu'un byte en hexa s'ecrit toujours sur au moins deux chiffres.
          break;

        case 'c' :   // un byte (ascii)
          c = (byte) va_arg (ap, int);
          if (c != 0)
          {
            errCode = ustrncat (stringOut, &c, 1);
            if (errCode != EFB_OK)
            {
              va_end(ap);
              return errCode;
            }
            stringOut++; // on ajoute un caractère
          }
          break;

        case 's' :   // un string
          s = va_arg (ap, tEFBstring);
          if (s != NULL)
          {
            errCode = ustrcat (stringOut, s);
            if (errCode != EFB_OK)
            {
              va_end(ap);
              return errCode;
            }
            stringOut += strlen ( (char*) s);
          }
          break;

        case 'l' :   // un long en hexa
          errCode = ustrcat(stringOut, longToHexaString (va_arg(ap, uint32_t)));
          if (errCode != EFB_OK)
          {
            va_end(ap);
            return errCode;
          }
          stringOut += 8; // un long en hexa fait toujours 8 chiffres.
          break;

        case '%' :
          errCode = ustrcat(stringOut, (tEFBstring) "%");
          if (errCode != EFB_OK)
          {
            va_end(ap);
            return errCode;
          }
          stringOut++;
          break;

        default :
          va_end(ap);
          return EFBERR_USPRINTF_WRONG_FLAG;
      }
    }
    else
    {
      errCode = ustrncat(stringOut, (tEFBstring) formatEFBstring, 1);
      if (errCode != EFB_OK)
      {
        va_end(ap);
        return errCode;
      }
      stringOut++;
    }
    formatEFBstring++;
  }
  va_end(ap);
  return EFB_OK;
}

