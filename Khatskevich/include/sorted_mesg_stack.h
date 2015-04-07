#ifndef _ARCHIVATOR_SORTED_MESG_STACK_
#define _ARCHIVATOR_SORTED_MESG_STACK_

#include "archivator.h"
#include "data_presentation.h"

typedef struct mesgStackElement{
    mesgStruct* mesg;
    struct mesgStackElement* next;
}mesgStackElement;

typedef struct{
    int n;
    mesgStackElement* first;
}mesgStack;



mesgStack *mesgStackInit();
void mesgStackDestruct(mesgStack * stack);
int mesgStackAdd(mesgStack * stack, mesgStruct* mesg);
void addElement( mesgStackElement** nextPP, mesgStackElement* el );
mesgStruct* mesgStackGetFirst(mesgStack * stack);
void mesgStackDeleteFirst(mesgStack * stack);

#endif // _ARCHIVATOR_SORTED_MESG_STACK_
