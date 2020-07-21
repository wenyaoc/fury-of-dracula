  
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

HistoryNode creatNode(PlaceId place, bool vampire, int trap, bool revealed){
    HistoryNode new = malloc(sizeof(* new));
    new->place = place;
    new->vampire = vampire;
    new->trapNumber = trap;
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
playerData addToHistory(playerData data, HistoryNode newNode) {
    if(data.first == NULL){
        data.first = newNode;
        data.turn++;
        return data;
    } 
    newNode->next = data.first;
    data.first = newNode;
    data.turn++;
    return data;
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

bool canGo(HistoryNode list, PlaceId place) {
    if(list == NULL) return true;
    HistoryNode curr = list;
    int counter = 0;
    while((curr != NULL) && counter < 5) {
        if(curr->place == place) {
            return false;
        } else if(curr->place >= DOUBLE_BACK_1 && curr->place <= DOUBLE_BACK_5) {
            HistoryNode node = findDBCity(curr);
            if(node != NULL) {
                if(place == node->place) 
                    return false;
            }
        }
        counter++;
        if(counter == 5 && curr->next != NULL) {   //if the 5th is HIDE
            if(curr->place == HIDE && curr->next->place == place) return false;
        }
        curr = curr->next;
    }
    return true;
}


int canDoubleBack(HistoryNode list, PlaceId place) {
    HistoryNode curr = list;
    int counter = 0;
    while((curr != NULL) && counter < 5) {
        if(curr->place >= DOUBLE_BACK_1 && curr->place <= DOUBLE_BACK_5) return 0;
        counter++;
        curr = curr->next;
    }
    curr = list;
    counter = 0;
    while((curr != NULL) && counter < 5) {
        if(curr->place == place) return counter + 1;
        counter++;
        curr = curr->next;
    }
    return 0;
}

bool canHide(HistoryNode list){
    HistoryNode curr = list;
    int counter = 0;
    while((curr != NULL) && counter < 5) {
        if(curr->place == HIDE) return false;
        counter++;
        curr = curr->next;
    }
    return true;
}



HistoryNode findDBCity(HistoryNode node) {
    int DBNumber = node->place - DOUBLE_BACK_1;
    int i = 0;
    HistoryNode curr = node;
    while (curr != NULL && i <= DBNumber) {
        i++;
        curr = curr->next;
    }
    if (curr == NULL)
        return NULL;
    if (curr->place == HIDE)
        return curr->next;
    return curr;
}