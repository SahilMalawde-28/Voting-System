#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "blockchain.h"
#include "avl.h"



//initalize function;
void initializeTree(AVLTree *tree) {
    tree->root = NULL;
        loadTreeFromBinaryFile(tree, "voter_data.bin");
}
/* 
Creates a new voter node and initializes it with the given voterID.
The voted status is set to 0 (not voted), and height is initialized to 1.
Returns a pointer to the newly created node.
*/
VoterNode *createVoterNode(char *voterID) {
    VoterNode *newNode = (VoterNode *)malloc(sizeof(VoterNode));
    strcpy(newNode->voterID, voterID);
    newNode->voted = 0;
    newNode->left = newNode->right = NULL;
    newNode->height = 1; //height is updated;
    return newNode;
}

int max(int a, int b) {
    return (a > b) ? a : b;
}
/*the below function calculates the height of tree
and return it.*/
int calculateNodeHeight(VoterNode *node) {
    if (node == NULL)
        return 0;
    return node->height;
}
// Rotations are used to restore balance in the AVL tree when it becomes unbalanced.
// These operations ensure that the tree remains balanced, keeping insertions, deletions,
// and searches efficient with a time complexity of O(log n).
/* 
Performs a right rotation on an unbalanced node to restore balance in the AVL tree.
It rotates the left subtree upwards, making it the new root. 
Returns the new root after rotation.
*/
VoterNode *performRightRotation(VoterNode *unbalancedNode) {
    VoterNode *newRoot = unbalancedNode->left;
    VoterNode *rightSubtree = newRoot->right;

    newRoot->right = unbalancedNode;
    unbalancedNode->left = rightSubtree;
// Update heights after rotation
    unbalancedNode->height = max(calculateNodeHeight(unbalancedNode->left), calculateNodeHeight(unbalancedNode->right)) + 1;
    newRoot->height = max(calculateNodeHeight(newRoot->left), calculateNodeHeight(newRoot->right)) + 1;

    return newRoot;
}
/* 
Performs a left rotation on an unbalanced node to balance the AVL tree.
Returns the new root after rotation.
*/
VoterNode *performLeftRotation(VoterNode *unbalancedNode) {
    VoterNode *newRoot = unbalancedNode->right;
    VoterNode *leftSubtree = newRoot->left;

    newRoot->left = unbalancedNode;
    unbalancedNode->right = leftSubtree;

    unbalancedNode->height = max(calculateNodeHeight(unbalancedNode->left), calculateNodeHeight(unbalancedNode->right)) + 1;
    newRoot->height = max(calculateNodeHeight(newRoot->left), calculateNodeHeight(newRoot->right)) + 1;

    return newRoot;
}
// The balance factor helps determine if a node is balanced.
// A balance factor between -1 and 1 means the node is balanced,
// while values outside this range trigger a rotation to rebalance the tree.
int getNodeBalance(VoterNode *node) {
    if (node == NULL)
        return 0;
    return calculateNodeHeight(node->left) - calculateNodeHeight(node->right);
}
/* 
Inserts a new voter node into the AVL tree, balancing it as necessary.
Returns the new root after insertion and balancing.
*/
VoterNode *insertVoterNode(VoterNode *node, char *voterID) {
    if (node == NULL)
        return createVoterNode(voterID);

    if (strcmp(voterID, node->voterID) < 0)
        node->left = insertVoterNode(node->left, voterID);
    else if (strcmp(voterID, node->voterID) > 0)
        node->right = insertVoterNode(node->right, voterID);
    else
        return node;

    node->height = 1 + max(calculateNodeHeight(node->left), calculateNodeHeight(node->right));

    int balance = getNodeBalance(node);
    // Perform rotations to balance the tree
    if (balance > 1 && strcmp(voterID, node->left->voterID) < 0)
        return performRightRotation(node);

    if (balance < -1 && strcmp(voterID, node->right->voterID) > 0)
        return performLeftRotation(node);

    if (balance > 1 && strcmp(voterID, node->left->voterID) > 0) {
        node->left = performLeftRotation(node->left);
        return performRightRotation(node);
    }

    if (balance < -1 && strcmp(voterID, node->right->voterID) < 0) {
        node->right = performRightRotation(node->right);
        return performLeftRotation(node);
    }

    return node;
}

void insertVoter(AVLTree *tree, char *voterID) {
    tree->root = insertVoterNode(tree->root, voterID);
    saveTreeToBinaryFile(tree, "voter_data.bin");
    return;
}
/* 
Updates the voting status of a voter.
If the voter has already voted, it returns 1. If not, it marks the voter as voted and returns 0.
*/
int updateVotingStatus(VoterNode *node, char *voterID) {
    if (node == NULL) {
        return -1;
    }

    if (strcmp(voterID, node->voterID) == 0) {
        if (node->voted == 1) {
            return 1;
        } else {
            node->voted = 1;
    
            return 0;
        }
    }

    if (strcmp(voterID, node->voterID) < 0)
        return updateVotingStatus(node->left, voterID);
    else
        return updateVotingStatus(node->right, voterID);
}
int updateVoting(AVLTree *tree, char *voterID){
return updateVotingStatus(tree->root, voterID);
}

// Function to search for a voter in the AVL tree by voterID
VoterNode *findVoter(VoterNode *node, char *voterID) {
    if (node == NULL) {
        return NULL;
    }

    int cmp = strcmp(voterID, node->voterID);
    if (cmp == 0) {
        return node;  // Voter found
    } else if (cmp < 0) {
        return findVoter(node->left, voterID);
    } else {
        return findVoter(node->right, voterID);
    }
}

/* 
Displays the voter ID and voting status (whether voted or not) for each voter in the AVL tree (in-order traversal).
*/
void displayVoterStatus(VoterNode *root) {
    if (root != NULL) {
        displayVoterStatus(root->left);
        printf("Voter ID: %s, Voted: %d\n", root->voterID, root->voted);
        displayVoterStatus(root->right);
    }
}

void displayTree(AVLTree *tree) {
    displayVoterStatus(tree->root);
}


// Function to write a single node to a binary file
void saveNodeToBinaryFile(FILE *file, VoterNode *node) {
    if (node == NULL) {
        return;
    }

    fwrite(node, sizeof(VoterNode), 1, file);

    saveNodeToBinaryFile(file, node->left);
    saveNodeToBinaryFile(file, node->right);
}

// Function to save the entire AVL tree to a binary file
// This function writes a single node (voterID, voted status) to a binary file.
// It will recursively save the entire AVL tree by traversing in-order (left subtree, root, right subtree)
// and writing each node's data to the file.
void saveTreeToBinaryFile(AVLTree *tree, const char *filename) {
    FILE *file = fopen(filename, "wb");  // Open file in write-binary mode
    if (file == NULL) {
        printf("Unable to open file %s for writing.\n", filename);
        return;
    }

    saveNodeToBinaryFile(file, tree->root);

    fclose(file);  // Close the file
    printf("Tree saved to %s successfully.\n", filename);
}
// Function to read a single node from a binary file
VoterNode *loadNodeFromBinaryFile(FILE *file) {
    VoterNode tempNode;
    if (fread(&tempNode, sizeof(VoterNode), 1, file) != 1) {
        return NULL;  // Return NULL if there are no more nodes
    }


    VoterNode *newNode = (VoterNode *)malloc(sizeof(VoterNode));
    *newNode = tempNode;
    newNode->left = loadNodeFromBinaryFile(file);  // Load left child
    newNode->right = loadNodeFromBinaryFile(file);  // Load right child
    return newNode;
}

// Function to load the entire AVL tree from a binary file
void loadTreeFromBinaryFile(AVLTree *tree, const char *filename) {
    FILE *file = fopen(filename, "rb");  // Open file in read-binary mode
    if (file == NULL) {
        printf("Unable to open file %s for reading.\n", filename);
        return;
    }

   
    tree->root = loadNodeFromBinaryFile(file);

    fclose(file);  // Close the file
    printf("Tree loaded from %s successfully.\n", filename);
}
// Function to save the blockchain to a binary file
void saveBlockchainToFile(blockchain *bc, const char *filename) {
    FILE *file = fopen(filename, "wb");
    if (!file) {
        perror("Failed to open file for saving blockchain");
        return;
    }

    // Iterate over each block in the blockchain
    block *current = bc->head;
    while (current != NULL) {
        // Write the length of voterID and candID strings
        size_t voterID_len = strlen(current->voterID) + 1;
        size_t candID_len = strlen(current->candID) + 1;
        fwrite(&voterID_len, sizeof(size_t), 1, file);
        fwrite(&candID_len, sizeof(size_t), 1, file);

        // Write the voterID and candID strings
        fwrite(current->voterID, sizeof(char), voterID_len, file);
        fwrite(current->candID, sizeof(char), candID_len, file);

        // Write the previous hash
        fwrite(current->prevhash, sizeof(unsigned char), SHA256_DIGEST_LENGTH, file);

        // Move to the next block
        current = current->next;
    }

    fclose(file);
    printf("Blockchain saved successfully to %s.\n", filename);
}
void loadBlockchainFromFile(blockchain *bc, const char *filename) {
    FILE *file = fopen(filename, "rb");
    if (!file) {
        perror("Failed to open file for loading blockchain");
        return;
    }

    // Initialize the blockchain as empty
    bc->head = bc->tail = NULL;

    while (1) {
        size_t voterID_len, candID_len;

        // Read the lengths of the voterID and candID strings
        if (fread(&voterID_len, sizeof(size_t), 1, file) != 1) break;
        if (fread(&candID_len, sizeof(size_t), 1, file) != 1) break;

        // Allocate memory for voterID and candID strings
        char *voterID = malloc(voterID_len);
        char *candID = malloc(candID_len);
        if (!voterID || !candID) {
            perror("Failed to allocate memory for voterID or candID");
            fclose(file);
            return;
        }

        // Read the voterID and candID strings
        if (fread(voterID, sizeof(char), voterID_len, file) != voterID_len) {
            free(voterID);
            free(candID);
            break;
        }
        if (fread(candID, sizeof(char), candID_len, file) != candID_len) {
            free(voterID);
            free(candID);
            break;
        }

        // Read the previous hash
        unsigned char prevhash[SHA256_DIGEST_LENGTH];
        if (fread(prevhash, sizeof(unsigned char), SHA256_DIGEST_LENGTH, file) != SHA256_DIGEST_LENGTH) {
            free(voterID);
            free(candID);
            break;
        }

        // Create a new block and add it to the blockchain
        block *newBlock = (block *)malloc(sizeof(block));
        if (!newBlock) {
            perror("Failed to allocate memory for newBlock");
            free(voterID);
            free(candID);
            fclose(file);
            return;
        }

        newBlock->voterID = voterID;
        newBlock->candID = candID;
        memcpy(newBlock->prevhash, prevhash, SHA256_DIGEST_LENGTH);
        newBlock->next = NULL;

        if (bc->head == NULL) {
            bc->head = bc->tail = newBlock;
        } else {
            bc->tail->next = newBlock;
            bc->tail = newBlock;
        }
    }

    fclose(file);
    printf("Blockchain loaded successfully from %s.\n", filename);
}

void displayVoterDataFromBinaryFile(const char *filename) {
    FILE *file = fopen(filename, "rb");
    if (!file) {
        perror("Error opening file");
        return;
    }

    VoterNode voter;
    printf("Voter Data:\n");
    while (fread(&voter, sizeof(VoterNode), 1, file) == 1) {
        printf("Voter ID: %s, Voted: %d\n", voter.voterID, voter.voted);
    }

    fclose(file);
}


