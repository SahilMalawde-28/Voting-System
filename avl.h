
/* 
Structure to represent a voter in the AVL tree. 
It contains voterID, whether the voter has voted, 
left and right child pointers, and height of the node.
*/

typedef struct VoterNode {
    char voterID[8]; 
    int voted;
    struct VoterNode *left;
    struct VoterNode *right;
    int height;
} VoterNode;

typedef struct AVLTree {
    VoterNode *root;
} AVLTree;
void initializeTree(AVLTree *tree);
VoterNode *createVoterNode(char *voterID);
int max(int a, int b) ;
int calculateNodeHeight(VoterNode *node);
VoterNode *performRightRotation(VoterNode *unbalancedNode);
VoterNode *performLeftRotation(VoterNode *unbalancedNode) ;
int getNodeBalance(VoterNode *node) ;
VoterNode *insertVoterNode(VoterNode *node, char *voterID); 
void insertVoter(AVLTree *tree, char *voterID);
int updateVotingStatus(VoterNode *node, char *voterID);
int updateVoting(AVLTree *voterTree, char *voterID);
void displayVoterStatus(VoterNode *root);
void displayTree(AVLTree *tree);
void saveNodeToBinaryFile(FILE *file, VoterNode *node) ;
void saveTreeToBinaryFile(AVLTree *tree, const char *filename);
VoterNode *loadNodeFromBinaryFile(FILE *file);
void loadTreeFromBinaryFile(AVLTree *tree, const char *filename);
void saveBlockchainToFile(blockchain *bc, const char *filename);
void loadBlockchainFromFile(blockchain *bc, const char *filename);
VoterNode *findVoter(VoterNode *node, char *voterID);
void displayVoterDataFromBinaryFile(const char *filename);
