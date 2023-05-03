#ifndef BST_H
#define BST_H

#include "./util.h"


typedef struct treeNode{

    long val;
    char * fileName;
    struct treeNode * left;
    struct treeNode * right;

}TreeNode;


void printTree (TreeNode * tree);

void freeTree(TreeNode * tree);

void insTree(Mes nodoIns,TreeNode ** tree);

#endif