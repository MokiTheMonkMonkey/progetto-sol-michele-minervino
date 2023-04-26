#ifndef BST_H
#define BST_H

#include "./util.h"

#define COPYNODE( loc , ins ){          \
                                       \
    loc = _malloc(sizeof(TreeNode));    \
    int nameSize = (int)strnlen(ins -> nome,MAXNAME) + 1;       \
    loc -> fileName = _malloc(nameSize);        \
    loc -> val = ins -> val;        \
    strncpy(loc -> fileName,ins -> nome,nameSize);      \
    loc -> right = NULL;        \
    loc -> left = NULL;         \
        \
}

typedef struct treeNode{

    long val;
    char * fileName;
    struct treeNode * left;
    struct treeNode * right;

}TreeNode;


void printTree (TreeNode * tree);

void freeTree(TreeNode * tree);

void insTree(Mes * nodoIns,TreeNode * tree);

#endif