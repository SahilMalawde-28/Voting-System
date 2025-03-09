#include "openssl/sha.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_MERKLE_TREE_SIZE 64
#define MAX_CANDIDATES 8

typedef struct block {
    char *voterID;
    char *candID;
    struct block *next;
    unsigned char prevhash[SHA256_DIGEST_LENGTH];
} block;

typedef struct blockchain {
    block *head;
    block *tail;
    unsigned char merkle_root[SHA256_DIGEST_LENGTH];
    unsigned char *merkle_tree[MAX_MERKLE_TREE_SIZE];
    int merkle_count;
} blockchain;

typedef struct {
    char * id;      // Candidate ID
    char name[50];  // Candidate Name
} Candidate;

void initializeBlockchain(blockchain *bc);
void castVote(char *voterID, char *candID, blockchain *bc);
int verifyChain(blockchain *bc);
unsigned char *toString(block *b);
void hashPrinter(unsigned char hash[], int length);
int hashCompare(unsigned char *str1, unsigned char *str2);
void countVotes(blockchain *bc, Candidate *candidates, int numCandidates);
void addToMerkleTree(blockchain *bc, unsigned char *newHash);
unsigned char* calculateMerkleRoot(blockchain *bc);
void displayCandidates();
void printMerkleRoot(blockchain *bc);
void addCandidate();
void displayCandidates();
void clearAllCandidates();
void loadCandidatesFromFile(Candidate *candidates, int *numCandidates);
int hasTimePassed(const char *filename);
void destroyAndExit();
// void manageCandidatesMenu() 
//void alterVote(blockchain *bc, char *voterID, char *newCandID);
