#ifndef MESH_H
#define MESH_H

#include "../models/option.h"

// Mesh data structure to match Python return
struct MeshData {
    double* V;    // 2D value grid (flattened)  
    double* S;    // Space axis
    double* t;    // Time axis
    
    // Constructor
    MeshData(double* V_, double* S_, double* t_) 
        : V(V_), S(S_), t(t_) {}
    
    // Destructor to clean up memory
    ~MeshData() {
        delete[] V;
        delete[] S; 
        delete[] t;
    }
};

// Initialize mesh for PDE solving
MeshData initialize_mesh(
    const Option& option,
    double S_max,
    int N,
    int J
);

#endif // MESH_H 