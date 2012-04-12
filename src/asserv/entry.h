#ifndef ASSERV_ENTRY_H
#define ASSERV_ENTRY_H

#define MAX_ENTRY_OUTPUT 42 /* FIXME */ // Le nombre d'entrées
//typedef uint16_t AsservValue; // Type des données utilisées dans la structure

typedef struct entry Entry;
struct entry
{
	AsservValue entries[MAX_ENTRY_OUTPUT];
	void (*updateEntry) (int, AsservValue);
};

Entry initEntry(void (*updateEntry) (int, AsservValue));

#endif