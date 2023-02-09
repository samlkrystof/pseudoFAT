//
// Created by Kryst on 08.02.2023.
//


#ifndef PSEUDOFAT_DIRCLUSTER_H
#define PSEUDOFAT_DIRCLUSTER_H

#include "sizes.h"

typedef struct {
    unsigned int nextCluster;
    unsigned int entries[(CLUSTER_SIZE - sizeof(unsigned int)) / sizeof(unsigned int)];
} DirCluster;


#endif //PSEUDOFAT_DIRCLUSTER_H
