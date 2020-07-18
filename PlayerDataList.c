#include <assert.h>
#include <stdio.h>

#include "Game.h"
#include "PlayerDataList.h"
#include "Places.h"
#include "Map.h"

void ListFree(HistoryNode node) {
    HistoryNode curr = node;
    HistoryNode next ;
    while (curr != NULL) {
        next = curr->next;
        free(curr);
        curr = next;
    }
}

HistoryNode creatNode(PlaceId place, bool vampire, bool trap, bool revealed){
    HistoryNode new = malloc(sizeof(* new));
    new->place = place;
    new->vampire = vampire;
    if (trap == true)
        new->trapNumber = 1;
    else
        new->trapNumber = 0;
    new->revealed = revealed; 
    new->next = NULL;
    return new;
}

HistoryNode copyNode(HistoryNode prevNode) { 
    HistoryNode new = malloc(sizeof(* new));
    new->place = prevNode->place;
    new->vampire = prevNode->vampire;
    new->trapNumber = prevNode->trapNumber;
    new->revealed = prevNode->revealed;
    new->next = NULL;
    return new;
}


// add the node to the head of the list  
void addToHead(playerData data, HistoryNode newNode) {
    if(data.first == NULL){
        data.first = newNode;
        data.totalNumber++;
        return;
    } 
    newNode->next = data.first;
    data.first = newNode;
    data.totalNumber++;
}

// add the node to the end of the list
HistoryNode addToTail(HistoryNode list, HistoryNode newNode) {
    if (list == NULL) return newNode;

    HistoryNode curr = list;
    while(curr->next != NULL) {
        curr = curr->next;
    }
    curr->next = newNode;
    return list;
}

// get a copy of the last N Node in the history list
// returns: the head of the new list
// !!!!!! make sure you free it after you use it !!!!!!
HistoryNode getLastN(playerData data, int n){
    HistoryNode curr = data.first;
    if(curr == NULL) return NULL;
    HistoryNode newHead = copyNode(curr);
    HistoryNode newCurr = newHead;
    curr = curr->next;
    int i = 1;
    while((curr != NULL) && i < n){
        newCurr->next = copyNode(curr);
        newCurr = newCurr->next;
        curr = curr->next;
        i++;
    }
    return newHead;
}


