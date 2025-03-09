#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "blockchain.h"
#include "avl.h"
#include <time.h>

#define CANDIDATES_FILE "candidates.txt"
#define MAX_CANDIDATES 8  // Adjust as needed

void initializeBlockchain(blockchain *bc) {
    bc->head = NULL;
    bc->tail = NULL;
    bc->merkle_count = 0;
    loadBlockchainFromFile(bc, "blockchain_data.bin"); 
    // Load from file if it exists
}

void castVote(char *voterID, char *candID, blockchain *bc) {
    printf("Casting vote for Voter ID: %s, Candidate ID: %s\n", voterID, candID);

    block *newBlock = (block *)malloc(sizeof(block));
    
    if (newBlock == NULL) {
        printf("Memory allocation failed\n");
        return;
    }

    newBlock->voterID = strdup(voterID);
    newBlock->candID = strdup(candID);
    newBlock->next = NULL;

    if (bc->head == NULL) {
        SHA256((unsigned char *)"", 0, newBlock->prevhash);
        bc->head = newBlock;
        bc->tail = newBlock;
    } else {
        unsigned char *prevBlockString = toString(bc->tail);
        SHA256(prevBlockString, strlen((char *)prevBlockString), newBlock->prevhash);
        free(prevBlockString);

        bc->tail->next = newBlock;
        bc->tail = newBlock;
    }

    addToMerkleTree(bc, newBlock->prevhash);

    // Update the saved Merkle root after casting a vote
    unsigned char *new_merkle_root = calculateMerkleRoot(bc);
    if (new_merkle_root) {
        memcpy(bc->merkle_root, new_merkle_root, SHA256_DIGEST_LENGTH);
        free(new_merkle_root);
    }
    printf("Vote casted successfully and Merkle root updated.\n");
     saveBlockchainToFile(bc, "blockchain_data.bin");
}

void addToMerkleTree(blockchain *bc, unsigned char *newHash) {
    if (bc->merkle_count >= MAX_MERKLE_TREE_SIZE) {
        printf("Merkle Tree is full, cannot add more blocks\n");
        return;
    }

    bc->merkle_tree[bc->merkle_count] = (unsigned char *)malloc(SHA256_DIGEST_LENGTH);
    memcpy(bc->merkle_tree[bc->merkle_count], newHash, SHA256_DIGEST_LENGTH);
    bc->merkle_count++;
}

unsigned char* calculateMerkleRoot(blockchain *bc) {
    int count = 0;
    block *current = bc->head;

    // Calculate the full hash of each block and store it in temp_tree
    unsigned char temp_tree[MAX_MERKLE_TREE_SIZE][SHA256_DIGEST_LENGTH];
    while (current != NULL) {
        unsigned char fullBlockHash[SHA256_DIGEST_LENGTH];
        unsigned char *blockString = toString(current);  // Generate full block string for hashing
        SHA256((unsigned char *)blockString, strlen((char *)blockString), fullBlockHash);
        free(blockString);

        memcpy(temp_tree[count], fullBlockHash, SHA256_DIGEST_LENGTH);
        count++;
        current = current->next;
    }

    // Build the Merkle root from the full block hashes
    while (count > 1) {
        int i = 0, j = 0;

        while (i < count) {
            unsigned char data[SHA256_DIGEST_LENGTH * 2];
            memcpy(data, temp_tree[i], SHA256_DIGEST_LENGTH);

            if (i + 1 < count) {
                memcpy(data + SHA256_DIGEST_LENGTH, temp_tree[i + 1], SHA256_DIGEST_LENGTH);
            } else {
                memcpy(data + SHA256_DIGEST_LENGTH, temp_tree[i], SHA256_DIGEST_LENGTH); // Duplicate last if odd number
            }

            // Hash the concatenated data to form the next level of the Merkle tree
            SHA256(data, SHA256_DIGEST_LENGTH * 2, temp_tree[j]);
            i += 2;
            j++;
        }
        count = j;
    }

    // Final root hash at the top of the tree
    unsigned char *root_hash = malloc(SHA256_DIGEST_LENGTH);
    if (root_hash == NULL) {
        printf("Memory allocation failed\n");
        return NULL;
    }
    memcpy(root_hash, temp_tree[0], SHA256_DIGEST_LENGTH);
    return root_hash;
}

int verifyChain(blockchain *bc) {
	int check = 1;
    printf("Starting blockchain verification...\n");

    if (bc->head == NULL) {
        printf("Blockchain is empty.\n");
        return -1;
    }

    block *curr = bc->head->next;
    block *prev = bc->head;
    int count = 1;

    while (curr) {
        printf("%d\t[%s]-[%s]\t", count++, curr->voterID, curr->candID);

        unsigned char calculatedHash[SHA256_DIGEST_LENGTH];
        unsigned char *prevBlockString = toString(prev);
        SHA256(prevBlockString, strlen((char *)prevBlockString), calculatedHash);
        free(prevBlockString);

        hashPrinter(calculatedHash, SHA256_DIGEST_LENGTH);
        printf(" - ");
        hashPrinter(curr->prevhash, SHA256_DIGEST_LENGTH);

        if (hashCompare(calculatedHash, curr->prevhash)) {
            printf(" Verified\n");
        } else {
            printf(" Alteration detected\n");
            check = 0;
        }

        prev = curr;
        curr = curr->next;
    }
    printf("Blockchain verification complete.\n");
    return check;
}

unsigned char *toString(block *b) {
    int voterID_len = strlen(b->voterID);
    int candID_len = strlen(b->candID);
    int len = voterID_len + candID_len + SHA256_DIGEST_LENGTH;

    unsigned char *str = (unsigned char *)malloc(len);
    if (!str) {
        printf("Memory allocation failed\n");
        return NULL;
    }

    strcpy((char *)str, b->voterID);
    strcat((char *)str, b->candID);
    memcpy(str + voterID_len + candID_len, b->prevhash, SHA256_DIGEST_LENGTH);

    return str;
}

void hashPrinter(unsigned char hash[], int length) {
    for (int i = 0; i < length; i++) {
        printf("%02x", hash[i]);
    }
    printf("\n");
}

int hashCompare(unsigned char *str1, unsigned char *str2) {
    return memcmp(str1, str2, SHA256_DIGEST_LENGTH) == 0;
}
void countVotes(blockchain *bc, Candidate *candidates, int numCandidates) {
    unsigned char *current_merkle_root = calculateMerkleRoot(bc);
    if (current_merkle_root == NULL) {
        printf("Error calculating Merkle root.\n");
        return;
    }

    // Check if the calculated Merkle root matches the stored Merkle root
    if (!hashCompare(current_merkle_root, bc->merkle_root)) {
        printf("Integrity disrupted; Merkle root does not match.\n");
        free(current_merkle_root);
        return;
    }

    printf("Integrity verified.\n");

    // Create an array to count votes for each candidate
    int candidate_votes[MAX_CANDIDATES] = {0};

    // Traverse the blockchain and count the votes for each candidate
    block *current = bc->head;
    while (current) {
        for (int i = 0; i < numCandidates; i++) {
            // Compare the candID from the blockchain with the candidate's id
            if (strcmp(current->candID, candidates[i].id) == 0) {
                candidate_votes[i]++;  // Increment vote count for the candidate
                break;
            }
        }
        current = current->next;
    }

    // Print the vote counts for each candidate
    printf("Vote counts per candidate:\n");
    for (int i = 0; i < numCandidates; i++) {
        printf("Candidate %d (%s): %d votes\n", i + 1, candidates[i].id, candidate_votes[i]);
    }

    free(current_merkle_root);
}



void printMerkleRoot(blockchain *bc) {
    printf("Current Merkle Root: ");
    hashPrinter(bc->merkle_root, SHA256_DIGEST_LENGTH);
}

// void manageCandidatesMenu() {
//     int choice;
//     do {
//         printf("\n=== Candidate Management ===\n");
//         printf("1. Add Candidate\n");
//         printf("2. Display Candidates\n");
//         printf("3. Clear All Candidates\n");
//         printf("0. Return to Main Menu\n");
//         printf("Choose an option: ");
//         scanf("%d", &choice);
//         getchar();

//         switch (choice) {
//             case 1:
//                 addCandidate();
//                 break;
//             case 2:
//                 displayCandidates();
//                 break;
//             case 3:
//                 clearAllCandidates();
//                 break;
//             case 0:
//                 printf("Returning to Main Menu.\n");
//                 break;
//             default:
//                 printf("Invalid choice! Please try again.\n");
//         }
//     } while (choice != 0);
// }

void destroyAndExit() {
    // Open the blockchain data file in write mode to clear its contents
    FILE *blockchainFile = fopen("blockchain_data.bin", "wb");
    if (blockchainFile == NULL) {
        perror("Error opening blockchain data file");
        return;
    }
    fclose(blockchainFile);  // Close the file to effectively empty it

    // Open the voter data file in write mode to clear its contents
    FILE *voterFile = fopen("voter_data.bin", "wb");
    if (voterFile == NULL) {
        perror("Error opening voter data file");
        return;
    }
    fclose(voterFile);  // Close the file to effectively empty it

    printf("Data destroyed. Both blockchain and voter data have been cleared.\n");
}

void generateUniqueID(char* idBuffer) {
    const char charset[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789"; // Alphanumeric characters
    int charsetSize = sizeof(charset) - 1;

    // Seed random number generator for randomness
    srand(time(NULL) ^ rand());

    // Generate a UUID-like string: format XXXXX-XXXXX
    for (int i = 0; i < 5; i++) {
        idBuffer[i] = charset[rand() % charsetSize];
    }
    idBuffer[5] = '-'; // Separator

    for (int i = 6; i < 11; i++) {
        idBuffer[i] = charset[rand() % charsetSize];
    }

    idBuffer[11] = '\0'; // Null-terminate the string
}

void addCandidate() {
    char name[100];
    char id[16];
    printf("Enter candidate name: ");
    fgets(name, sizeof(name), stdin);
    name[strcspn(name, "\n")] = '\0';
    generateUniqueID(id);
    FILE *file = fopen(CANDIDATES_FILE, "a");
    if (!file) {
        perror("Error opening candidates file");
        return;
    }

    
    char line[256];
   

    fprintf(file, "%s,%s\n", id, name);
    fclose(file);
    printf("Candidate '%s' added successfully with ID %s.\n", name, id);
}

// Function to display all candidates
void displayCandidates() {
    FILE *file = fopen(CANDIDATES_FILE, "r");
    if (!file) {
        perror("Error opening candidates file");
        return;
    }

    char line[256];
    printf("\n=== Candidates ===\n");
    while (fgets(line, sizeof(line), file)) {
        // Remove the newline character at the end of the line
        line[strcspn(line, "\n")] = '\0';

        // Use strtok to split the line based on comma
        char *id = strtok(line, ",");
        char *name = strtok(NULL, ",");  // Get the part after the comma (name)

        if (id && name) {
            // Print the candidate details
            printf("ID: %s, Name: %s\n", id, name);
        } else {
            // Handle case where the line format is incorrect
            printf("Error: Invalid candidate format in line: %s\n", line);
        }
    }

    fclose(file);
}




// Function to clear all candidates
void clearAllCandidates() {
    FILE *file = fopen(CANDIDATES_FILE, "w");
    if (!file) {
        perror("Error opening candidates file");
        return;
    }
    fclose(file);
    printf("All candidates cleared successfully.\n");
}

void loadCandidatesFromFile(Candidate *candidates, int *numCandidates) {
    FILE *file = fopen("candidates.txt", "r");
    if (file == NULL) {
        printf("Error: Could not open candidates.txt\n");
        return;
    }

    char line[256];
    *numCandidates = 0;  // Reset the candidate count

    // Read each line from the file
    while (fgets(line, sizeof(line), file) != NULL) {
        // Remove newline character from the line if present
        line[strcspn(line, "\n")] = '\0';

        // Use strtok to get the ID and Name (split by ',')
        char *id = strtok(line, ",");  // First token: Candidate ID
        char *name = strtok(NULL, ""); // Remaining part: Candidate Name

        // Ensure both ID and Name are available
        if (id && name) {
            // Store the candidate ID and Name in the array
            candidates[*numCandidates].id = strdup(id);  // Dynamically allocate memory for the ID
            strncpy(candidates[*numCandidates].name, name, sizeof(candidates[*numCandidates].name) - 1);
            candidates[*numCandidates].name[sizeof(candidates[*numCandidates].name) - 1] = '\0';  // Null-terminate the name
            (*numCandidates)++;

            if (*numCandidates >= MAX_CANDIDATES) {
                break;  // Stop reading if we have reached the maximum number of candidates
            }
        }
    }

    fclose(file);
    printf("Candidates loaded successfully. Total candidates: %d\n", *numCandidates);
}

int hasTimePassed(const char *filename) {
    FILE *file = fopen(filename, "r");
    if (file == NULL) {
        return 0;
    }

    long file_time;
    fscanf(file, "%ld", &file_time);
    fclose(file);

    time_t current_time = time(NULL);
    if (current_time >= file_time) {
        return 1;
    }
    return 0;
}

// Combined cleanup function
// void cleanupBlockchainAndTree(blockchain *bc, AVLTree *voterTree) {
//     if (bc != NULL) {
//         destroyBlockchain(bc);
//         printf("Blockchain resources cleaned up.\n");
//     }

//     if (voterTree != NULL) {
//         destroyAVLTree(voterTree);
//         printf("AVL tree resources cleaned up.\n");
//     }
// }

/*
void alterVote(blockchain *bc, char *voterID, char *newCandID) {
    if (bc->head == NULL) {
        printf("Blockchain is empty.\n");
        return;
    }

    block *current = bc->head;
    int found = 0;

    while (current != NULL) {
        if (strcmp(current->voterID, voterID) == 0) {
            printf("Original vote for Voter ID: %s was for Candidate ID: %s\n", voterID, current->candID);
            
            free(current->candID);
            current->candID = strdup(newCandID);
            printf("Vote altered successfully to Candidate ID: %s\n", newCandID);
              saveBlockchainToFile(bc, "blockchain_data.bin");

            found = 1;
            break;
        }
        current = current->next;
    }

    if (!found) {
        printf("Voter ID not found in the blockchain.\n");
    }
}
*/
