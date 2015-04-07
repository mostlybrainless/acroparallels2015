//#define DEBUG

#include "archivator.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h> 


mesgStack *mesgStackInit(){
    LOGMESG(LOG_INFO, "mesgStackInit");
    mesgStack* stack = ( mesgStack*) malloc( sizeof(mesgStack));
    stack->n = 0;
    stack->first = NULL;
    return stack;
}

void mesgStackDestruct(mesgStack * stack){
    if ( stack != NULL){
        free(stack);
    }
}

int mesgStackAdd(mesgStack * stack, mesgStruct* mesg){
    LOGMESG(LOG_INFO, "mesgStackAdd");
    if (stack == NULL || mesg == NULL){
        return -1;
    }
    mesgStackElement* el = (mesgStackElement*) malloc( sizeof(mesgStackElement));
    el->mesg = mesg;
    el->next = NULL;
    stack->n++;
    addElement( &(stack->first),el);
    return 0;
}

void addElement( mesgStackElement** nextPP, mesgStackElement* el ){
    if (*nextPP != NULL) LOGMESG(LOG_INFO, "addElement, %lld",(long long) (*nextPP)->mesg->number );
    if ( *nextPP == NULL || el->mesg->number < (*nextPP)->mesg->number){
        mesgStackElement* tempeNextPP = *nextPP;
        *nextPP = el;
        el->next = tempeNextPP;
    }
    else{
        addElement( &((*nextPP)->next), el );
    }   
}

mesgStruct* mesgStackGetFirst(mesgStack * stack){
    LOGMESG(LOG_INFO, "mesgStackGetFirst");
    if ( stack == NULL){
        return NULL;
    }
    if ( stack->n == 0){
        LOGMESG(LOG_ERROR, "Trying to pop, when 0 elements in the stack");
        return NULL;
    }
    return stack->first->mesg;
}

void mesgStackDeleteFirst(mesgStack * stack){
    LOGMESG(LOG_INFO, "mesgStackDeleteFirst");
    if ( stack == NULL){
        return;
    }
    if ( stack->n == 0){
        return;
    }
    stack->n--;
    mesgStackElement* tempefirst = stack->first->next;
    free(stack->first);
    stack->first = tempefirst;
    return;
}

