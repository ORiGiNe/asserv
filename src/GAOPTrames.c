#include "GAOPTrames.h"

GAOPtrame newGAOPTrame ()
{
  GAOPtrame t = pvPortMalloc (sizeof (struct GAOPtrame));
  return t;
}

void freeGAOPTrame  (GAOPtrame t)
{
  /* On utilise heap_2.c, on peut donc free.
  Les trames sont de taille constante on peut donc se permettre d'allouer et de free regulièrement.  */
  vPortFree (t);
}

byte computeGAOPChecksum (GAOPtrame t)
{
  byte csum;
  csum = GAOP_BEGIN;
  csum ^= t->seq;
  csum ^= t->size;
  csum ^= t->ODID;
  csum ^= t->command;
  for (int i = 0; i < t->size; i++)
  {
    csum ^= t->data[i];
  }

  return csum;
}

void toString(GAOPtrame t, char* string)
{
    string[0] = GAOP_BEGIN;
    string[1] = t->seq;
    string[2] = t->size;
    string[3] = t->ODID;
    for (int i = 0; i < t->size; i++)
    {
        string[4 + i] = data[i];
    }
    string[4 + t->size] = computeGAOPChecksum (t);
    string[4 + t->size + 1] = GAOP_END;
    string[4 + t->size + 2] = 0;
}

void GAOPnack (byte seq, GAOPtrame t)
{    
    t->seq = seq;
    t->size = 0;
    t->ODID = 0xFD;
}

void GAOPack (byte seq, GAOPtrame t)
{
    t->seq = seq;
    t->size = 0;
    t->ODID = 0xFE;
}
