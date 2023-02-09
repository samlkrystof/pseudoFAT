//
// Created by Kryst on 08.02.2023.
//

#ifndef PSEUDOFAT_FILECLUSTER_H
#define PSEUDOFAT_FILECLUSTER_H

#include "sizes.h"

typedef struct {
    unsigned int nextCluster;
    char data[CLUSTER_SIZE - sizeof(unsigned int)];
} FileCluster;
#endif //PSEUDOFAT_FILECLUSTER_H
