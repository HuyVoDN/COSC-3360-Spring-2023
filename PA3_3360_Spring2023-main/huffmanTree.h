#include <iostream>
#include <string>
#include <vector>
#include <queue>
#include <map>
#include <pthread.h>
using namespace std;

#ifndef HUFFMAN_TREE_HEADER_H
#define HUFFMAN_TREE_HEADER_H

struct TreeNode // items for the node of Huffman Tree
{
    char symbol;
    int node_count;
    int frequency;
    TreeNode *left;
    TreeNode *right;
};

struct comparator
{
    bool operator()(TreeNode* left, TreeNode* right) // took 10 years to figured this trick out since PA1
    {
        if (left->frequency != right->frequency)
        {
            return left->frequency > right->frequency;
        }
        if(left->symbol == right->symbol && left->symbol == '\0')
        {
            return left->node_count < right->node_count;
        }
        if(left->symbol == '\0')
            return false;
        if(right->symbol == '\0')
            return true;

        return left->symbol > right->symbol;
    }
};

class HuffmanTree
{
    private:
        TreeNode* rootNode;
    public:
    HuffmanTree()
    {
        rootNode = NULL;
    }
    TreeNode* getRoot()
    {
        return rootNode;
    }

    void buildHuffmanTree(priority_queue<TreeNode*, vector<TreeNode*>, comparator> TreePrioQueue) // tree builder
    {
        int step = 0;
        while (TreePrioQueue.size() > 1)
        {
            TreeNode *firstNode = TreePrioQueue.top();
            TreePrioQueue.pop();

            TreeNode *secondNode = TreePrioQueue.top();
            TreePrioQueue.pop();

            TreeNode *sumNode = new TreeNode;
            sumNode->symbol = '\0';
            sumNode->frequency = firstNode->frequency + secondNode->frequency;
            sumNode->left = firstNode;
            sumNode->right = secondNode;
            sumNode->node_count = step++;

            TreePrioQueue.push(sumNode);
        }
        rootNode = TreePrioQueue.top();
       // printTree(rootNode," "); // used to print the Tree in PA1
    }

   /* void printTree(TreeNode *root, string result) // tranverse and prints the data of each tree nodes
    {
         if (root == NULL)
         {
            return;
         }
        if (root->symbol != '\0')
        {
            cout << "Symbol: " << root->symbol << ", Frequency: " << root->frequency << ", Code:" << result << endl;
        }

        printTree(root->left, result + "0");
        printTree(root->right, result + "1");

     }*/// no longer needed but will keep here to error check, and in case, dumb errors could god damn happen.
}; 

TreeNode* BinToSymbols(TreeNode* treeRoot, const string& binCode, size_t pos) // tranverse the binary string then convert them to symbols
{
    if (pos == binCode.length())
    {
        if (treeRoot->left == nullptr && treeRoot->right == nullptr) // check for leaf node
        {
            return treeRoot;
        }
    }
    if (binCode[pos] == '0')
        return BinToSymbols(treeRoot->left, binCode, pos + 1); // if is 0 then assign that to the left leaf, since its the path
    else
        return BinToSymbols(treeRoot->right, binCode, pos + 1); // assign to right because it is pos of 1
}

void splitString(string messageString, string delimiter, vector<string>& linesVector) // used to split the string of binary codes 
{
    size_t pos = 0;
    string letter;

    while ((pos = messageString.find(delimiter)) != string::npos)// if delimiter string is found
    {
        letter = messageString.substr(0, pos); //extract beginning of the messagestring that got from the inputFile
        linesVector.push_back(letter); // add it to the vector
        messageString.erase(0, pos + delimiter.length()); //slowly remove the characters of the string so the index is correct
    }
    linesVector.push_back(messageString); // add the remainder to the end of the vector
}

struct ThreadInfo //info for to create Nth threads for each lines
{
        TreeNode *treeRoot;
        HuffmanTree tree;
        string binCodeLine; // code from each line
        vector<int> indexes; // vector of indexes from each line
        char* originMessage;
        int* currentId; // current Mutex Id
        int id;
        pthread_mutex_t lockBinary; //1st lock
        pthread_mutex_t lockPrint; //2nd lock
        pthread_mutex_t lockDecompress; //3rd lock
        pthread_cond_t waitTurn = PTHREAD_COND_INITIALIZER; // this is to block on a condition variable, stolen from PQ1 from Exam 2.
       
};

void *decompressionFunc(void *argument) //passing the information and the binary code for each threads (based on how many lines)
{
    ThreadInfo *arg = (ThreadInfo *)argument;
    string binCodeLine = arg->binCodeLine; // retrieve code pulled from thread 
    vector<int> indexs = arg->indexes; // retrieve positions pulled from thread
    int id = arg->id; // ids from thread
    int* currentId = arg->currentId; // check for current value of mutex
    pthread_mutex_unlock(&(arg->lockBinary)); // unlock here

    TreeNode* decode = BinToSymbols(arg->treeRoot, binCodeLine, 0); 
    char symbolFromBin = decode->symbol;

    pthread_mutex_lock(&(arg->lockPrint)); // lock
    while((*currentId) != id)
     {
         pthread_cond_wait(&(arg->waitTurn), &(arg->lockPrint)); //block off right here
     }
    // 2nd lock at output due to critical section of outputting since we're passing shits
    
    cout << "Symbol: " << decode->symbol << ", Frequency: " << decode->frequency << ", Code: " << binCodeLine << endl;
    pthread_cond_broadcast(&(arg->waitTurn));
    pthread_mutex_unlock(&(arg->lockPrint)); // unlock

// LOOOOOOOOOOOOOOOOOOOOCK IT UP /// 3rd lock at decompression due to critical section since we're basically CHANGING value
    pthread_mutex_lock(&(arg->lockDecompress)); // lock
    (*(currentId))++;
    for ( auto& pos : indexs) // for loop to iterates over the indexes vector and assigning
    {
        arg->originMessage[pos] = symbolFromBin;
    }
    pthread_mutex_unlock(&(arg->lockDecompress)); // unlock END FINISHED WITH CRITICAL SECTIONS
    return NULL;
}
#endif
