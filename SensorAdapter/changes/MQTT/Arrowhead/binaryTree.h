
#ifndef BINARY_TREE_H
#define BINARY_TREE_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define max_size_of_id 100

typedef struct node{
    char providerID[max_size_of_id];
    struct node *left;
    struct node *right;
} node;

node *newNode(char *_providerID);

int cmpID(char *l, char *r);

void insertNode(node **root, node *child);

node *searchNode(node *root, char *value);

void printPreorder(node *root);

node *minValueNode(node *node);

node *deleteID(node *root, char *id);

#endif
