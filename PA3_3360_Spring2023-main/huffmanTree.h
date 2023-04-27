// MY CODE
#include <iostream>
#include <string>
#include <vector>
#include <queue>
#include <map>
#include <pthread.h>
using namespace std;

#ifndef HUFFMAN_TREE_HEADER_H
#define HUFFMAN_TREE_HEADER_H

struct TreeNode
{
    char symbol;
    int node_count;
    int frequency;
    TreeNode *left;
    TreeNode *right;
};

struct comparator
{
    bool operator()(TreeNode* left, TreeNode* right)
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
       // printTree(rootNode," ");
    }

    void printTree(TreeNode *root, string result) // prints the data of each tree nodes
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

     }
};

TreeNode* BinToSymbols(TreeNode* treeRoot, const string& binCode, size_t pos)
{
    if (pos == binCode.length())
    {
        if (treeRoot->left == nullptr && treeRoot->right == nullptr)
        {
            return treeRoot;
        }
    }
    if (binCode[pos] == '0')
        return BinToSymbols(treeRoot->left, binCode, pos + 1);
    else
        return BinToSymbols(treeRoot->right, binCode, pos + 1);
}

void splitString(string messageString, string delimiter, vector<string>& linesVector)
{
    size_t pos = 0;
    string letter;

    while ((pos = messageString.find(delimiter)) != string::npos)
    {
        letter = messageString.substr(0, pos);
        linesVector.push_back(letter);
        messageString.erase(0, pos + delimiter.length());
    }
    linesVector.push_back(messageString);
}

struct ThreadInfo //info for to create Nth threads for each lines
{
        TreeNode *treeRoot;
        HuffmanTree tree;
        string binCodeLine; // code from each line
        vector<int> indexes; // vector of indexes from each line
        char* originMessage;
        int* currentId;
        int id;
        pthread_mutex_t lockBinary;
        pthread_mutex_t lockDecompress;
        pthread_mutex_t lockPrint;
};

void *decompressionFunc(void *argument) //passing the information and the binary code for each threads (based on how many lines)
{
    ThreadInfo *arg = (ThreadInfo *)argument;
    string binCodeLine = arg->binCodeLine;
    vector<int> indexs = arg->indexes;
    int id = arg->id;
    int* currentId = arg->currentId;
    pthread_mutex_unlock(&(arg->lockBinary));


    TreeNode* decode = BinToSymbols(arg->treeRoot, binCodeLine, 0);
    char symbolFromBin = decode->symbol;

    while((*currentId) != id) continue;
    pthread_mutex_lock(&(arg->lockPrint));
    cout << "Symbol: " << decode->symbol << ", Frequency: " << decode->frequency << ", Code: " << binCodeLine << endl;
    pthread_mutex_unlock(&(arg->lockPrint));




    pthread_mutex_lock(&(arg->lockDecompress));
    (*(currentId))++;
    for (const auto& pos : indexs)
    {
        arg->originMessage[pos] = symbolFromBin;
    }
    pthread_mutex_unlock(&(arg->lockDecompress));
    return nullptr;
}
#endif
