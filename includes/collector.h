#ifndef COLLECTOR_H
#define COLLECTOR_H

#include <util.h>
#include "./util.h"


typedef struct treeNode{

    long val;
    char * fileName;
    struct treeNode * left;
    struct treeNode * right;

}TreeNode;

extern TreeNode * tree;


void collectorExitFun();

int sock_connect();

void printTree (TreeNode * cTree);

void freeTree(TreeNode * cTree);

void insTree(Mes nodoIns,TreeNode ** cTree);

#endif