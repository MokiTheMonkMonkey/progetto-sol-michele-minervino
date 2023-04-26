#include <bst.h>
#include "./../includes/bst.h"


void insTree(Mes * nodoIns,TreeNode * tree) {

    if(tree == NULL){

        COPYNODE(tree, nodoIns);
        return;

    }
    if(tree -> val < nodoIns -> val){

        insTree(nodoIns,tree -> right);

    }
    else{

        insTree(nodoIns,tree -> left);

    }


}

void freeTree(TreeNode * tree){

    if(!tree){

        return;

    }
    if(tree -> left){

        freeTree(tree -> left);

    }
    if(tree -> right){

        freeTree(tree -> right);

    }
    free(tree -> fileName);
    free(tree);

}

void printTree (TreeNode * tree){

    if(!tree){

        return;

    }
    printTree(tree -> left);
    fprintf(stderr,"%d %s",tree -> val,tree -> fileName);
    printTree(tree -> right);

}