#ifndef ASSERV_ENTRY_H
#define ASSERV_ENTRY_H

#define MAX_ENTRY_OUTPUT 42 /* FIXME */ // Le nombre d'entr�es
//typedef uint16_t AsservValue; // Type des donn�es utilis�es dans la structure

typedef struct entry Entry;
struct entry
{
	AsservValue entries[MAX_ENTRY_OUTPUT];
	void (*updateEntry) (int, AsservValue);
};

Entry initEntry(void (*updateEntry) (int, AsservValue));

#endif