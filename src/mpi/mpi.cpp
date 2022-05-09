#include "common.hpp"

#define _GNU_SOURCE
#include <assert.h>
#include <dlfcn.h>
#include <errno.h>
#include <mpi.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <sys/timeb.h>
#include <unistd.h>

// Extra definitions
#define CONST const

extern "C" {

// Pointers on intercepted fonctions
int MPI_Init(int * arg0, char *** arg1) {
    static int (*orig_MPI_Init)(int *, char ***) = 0;
    if (orig_MPI_Init == 0) {
        orig_MPI_Init = (int (*)(int *, char ***))dlsym(RTLD_NEXT, "MPI_Init");
    }

    int ret = orig_MPI_Init(arg0, arg1);

    pythia_init();

    return ret;
}

// int MPI_Init_thread(int * arg0, char *** arg1, int arg2, int * arg3) {
//
//
//     static int (*orig_MPI_Init_thread)(int *, char ***, int, int *) = 0;
//     if (orig_MPI_Init_thread == 0) {
//         orig_MPI_Init_thread =
//                 (int (*)(int *, char ***, int, int *))dlsym(RTLD_NEXT, "MPI_Init_thread");
//     }
//
//     int ret = orig_MPI_Init_thread(arg0, arg1, arg2, arg3);
//     return ret;
// }
// int MPI_Comm_size(MPI_Comm arg0, int * arg1) {
//
//
//     static int (*orig_MPI_Comm_size)(MPI_Comm, int *) = 0;
//     if (orig_MPI_Comm_size == 0) {
//         orig_MPI_Comm_size = (int (*)(MPI_Comm, int *))dlsym(RTLD_NEXT, "MPI_Comm_size");
//     }
//
//     int ret = orig_MPI_Comm_size(arg0, arg1);
//     return ret;
// }
// int MPI_Comm_rank(MPI_Comm arg0, int * arg1) {
//
//
//     static int (*orig_MPI_Comm_rank)(MPI_Comm, int *) = 0;
//     if (orig_MPI_Comm_rank == 0) {
//         orig_MPI_Comm_rank = (int (*)(MPI_Comm, int *))dlsym(RTLD_NEXT, "MPI_Comm_rank");
//     }
//
//     int ret = orig_MPI_Comm_rank(arg0, arg1);
//     return ret;
// }
// int MPI_Comm_get_parent(MPI_Comm * parent) {
//
//
//     static int (*orig_MPI_Comm_get_parent)(MPI_Comm *) = 0;
//     if (orig_MPI_Comm_get_parent == 0) {
//         orig_MPI_Comm_get_parent = (int (*)(MPI_Comm *))dlsym(RTLD_NEXT, "MPI_Comm_get_parent");
//     }
//
//     int ret = orig_MPI_Comm_get_parent(parent);
//     return ret;
// }
int MPI_Finalize() {
    static int (*orig_MPI_Finalize)(void) = 0;
    if (orig_MPI_Finalize == 0) {
        orig_MPI_Finalize = (int (*)(void))dlsym(RTLD_NEXT, "MPI_Finalize");
    }

    pythia_deinit();

    int ret = orig_MPI_Finalize();
    return ret;
}

// int MPI_Initialized(int * arg0) {
//
//
//     static int (*orig_MPI_Initialized)(int *) = 0;
//     if (orig_MPI_Initialized == 0) {
//         orig_MPI_Initialized = (int (*)(int *))dlsym(RTLD_NEXT, "MPI_Initialized");
//     }
//
//     int ret = orig_MPI_Initialized(arg0);
//     return ret;
// }
// int MPI_Abort(MPI_Comm arg0, int arg1) {
//
//
//     static int (*orig_MPI_Abort)(MPI_Comm, int) = 0;
//     if (orig_MPI_Abort == 0) {
//         orig_MPI_Abort = (int (*)(MPI_Comm, int))dlsym(RTLD_NEXT, "MPI_Abort");
//     }
//
//     int ret = orig_MPI_Abort(arg0, arg1);
//     return ret;
// }
// int MPI_Type_size(MPI_Datatype datatype, int * size) {
//
//
//     static int (*orig_MPI_Type_size)(MPI_Datatype, int *) = 0;
//     if (orig_MPI_Type_size == 0) {
//         orig_MPI_Type_size = (int (*)(MPI_Datatype, int *))dlsym(RTLD_NEXT, "MPI_Type_size");
//     }
//
//     int ret = orig_MPI_Type_size(datatype, size);
//     return ret;
// }
// int MPI_Cancel(MPI_Request * arg0) {
//
//
//     static int (*orig_MPI_Cancel)(MPI_Request *) = 0;
//     if (orig_MPI_Cancel == 0) {
//         orig_MPI_Cancel = (int (*)(MPI_Request *))dlsym(RTLD_NEXT, "MPI_Cancel");
//     }
//
//     int ret = orig_MPI_Cancel(arg0);
//     return ret;
// }
// int MPI_Comm_disconnect(MPI_Comm * comm) {
//
//
//     static int (*orig_MPI_Comm_disconnect)(MPI_Comm *) = 0;
//     if (orig_MPI_Comm_disconnect == 0) {
//         orig_MPI_Comm_disconnect = (int (*)(MPI_Comm *))dlsym(RTLD_NEXT, "MPI_Comm_disconnect");
//     }
//
//     int ret = orig_MPI_Comm_disconnect(comm);
//     return ret;
// }
// int MPI_Comm_free(MPI_Comm * comm) {
//
//
//     static int (*orig_MPI_Comm_free)(MPI_Comm *) = 0;
//     if (orig_MPI_Comm_free == 0) {
//         orig_MPI_Comm_free = (int (*)(MPI_Comm *))dlsym(RTLD_NEXT, "MPI_Comm_free");
//     }
//
//     int ret = orig_MPI_Comm_free(comm);
//     return ret;
// }
// int MPI_Comm_create(MPI_Comm comm, MPI_Group group, MPI_Comm * newcomm) {
//
//
//     static int (*orig_MPI_Comm_create)(MPI_Comm, MPI_Group, MPI_Comm *) = 0;
//     if (orig_MPI_Comm_create == 0) {
//         orig_MPI_Comm_create =
//                 (int (*)(MPI_Comm, MPI_Group, MPI_Comm *))dlsym(RTLD_NEXT, "MPI_Comm_create");
//     }
//
//     int ret = orig_MPI_Comm_create(comm, group, newcomm);
//     return ret;
// }
// int MPI_Comm_create_group(MPI_Comm comm, MPI_Group group, int tag, MPI_Comm * newcomm) {
//
//
//     static int (*orig_MPI_Comm_create_group)(MPI_Comm, MPI_Group, int, MPI_Comm *) = 0;
//     if (orig_MPI_Comm_create_group == 0) {
//         orig_MPI_Comm_create_group =
//                 (int (*)(MPI_Comm, MPI_Group, int, MPI_Comm *))dlsym(RTLD_NEXT,
//                                                                      "MPI_Comm_create_group");
//     }
//
//     int ret = orig_MPI_Comm_create_group(comm, group, tag, newcomm);
//     return ret;
// }
// int MPI_Comm_split(MPI_Comm comm, int color, int key, MPI_Comm * newcomm) {
//
//
//     static int (*orig_MPI_Comm_split)(MPI_Comm, int, int, MPI_Comm *) = 0;
//     if (orig_MPI_Comm_split == 0) {
//         orig_MPI_Comm_split =
//                 (int (*)(MPI_Comm, int, int, MPI_Comm *))dlsym(RTLD_NEXT, "MPI_Comm_split");
//     }
//
//     int ret = orig_MPI_Comm_split(comm, color, key, newcomm);
//     return ret;
// }
// int MPI_Comm_dup(MPI_Comm comm, MPI_Comm * newcomm) {
//
//
//     static int (*orig_MPI_Comm_dup)(MPI_Comm, MPI_Comm *) = 0;
//     if (orig_MPI_Comm_dup == 0) {
//         orig_MPI_Comm_dup = (int (*)(MPI_Comm, MPI_Comm *))dlsym(RTLD_NEXT, "MPI_Comm_dup");
//     }
//
//     int ret = orig_MPI_Comm_dup(comm, newcomm);
//     return ret;
// }
// int MPI_Comm_dup_with_info(MPI_Comm comm, MPI_Info info, MPI_Comm * newcomm) {
//
//
//     static int (*orig_MPI_Comm_dup_with_info)(MPI_Comm, MPI_Info, MPI_Comm *) = 0;
//     if (orig_MPI_Comm_dup_with_info == 0) {
//         orig_MPI_Comm_dup_with_info =
//                 (int (*)(MPI_Comm, MPI_Info, MPI_Comm *))dlsym(RTLD_NEXT,
//                 "MPI_Comm_dup_with_info");
//     }
//
//     int ret = orig_MPI_Comm_dup_with_info(comm, info, newcomm);
//     return ret;
// }
// int MPI_Comm_split_type(MPI_Comm comm, int split_type, int key, MPI_Info info, MPI_Comm *
// newcomm) {
//
//
//     static int (*orig_MPI_Comm_split_type)(MPI_Comm, int, int, MPI_Info, MPI_Comm *) = 0;
//     if (orig_MPI_Comm_split_type == 0) {
//         orig_MPI_Comm_split_type =
//                 (int (*)(MPI_Comm, int, int, MPI_Info, MPI_Comm *))dlsym(RTLD_NEXT,
//                                                                          "MPI_Comm_split_type");
//     }
//
//     int ret = orig_MPI_Comm_split_type(comm, split_type, key, info, newcomm);
//     return ret;
// }
// int MPI_Intercomm_create(MPI_Comm local_comm,
//                          int local_leader,
//                          MPI_Comm peer_comm,
//                          int remote_leader,
//                          int tag,
//                          MPI_Comm * newintercomm) {
//
//
//     static int (*orig_MPI_Intercomm_create)(MPI_Comm, int, MPI_Comm, int, int, MPI_Comm *) = 0;
//     if (orig_MPI_Intercomm_create == 0) {
//         orig_MPI_Intercomm_create = (int (*)(MPI_Comm, int, MPI_Comm, int, int, MPI_Comm
//         *))dlsym(
//                 RTLD_NEXT,
//                 "MPI_Intercomm_create");
//     }
//
//     int ret = orig_MPI_Intercomm_create(local_comm,
//                                         local_leader,
//                                         peer_comm,
//                                         remote_leader,
//                                         tag,
//                                         newintercomm);
//     return ret;
// }
// int MPI_Intercomm_merge(MPI_Comm intercomm, int high, MPI_Comm * newintracomm) {
//
//
//     static int (*orig_MPI_Intercomm_merge)(MPI_Comm, int, MPI_Comm *) = 0;
//     if (orig_MPI_Intercomm_merge == 0) {
//         orig_MPI_Intercomm_merge =
//                 (int (*)(MPI_Comm, int, MPI_Comm *))dlsym(RTLD_NEXT, "MPI_Intercomm_merge");
//     }
//
//     int ret = orig_MPI_Intercomm_merge(intercomm, high, newintracomm);
//     return ret;
// }
// int MPI_Cart_sub(MPI_Comm old_comm, CONST int * belongs, MPI_Comm * new_comm) {
//
//
//     static int (*orig_MPI_Cart_sub)(MPI_Comm, CONST int *, MPI_Comm *) = 0;
//     if (orig_MPI_Cart_sub == 0) {
//         orig_MPI_Cart_sub =
//                 (int (*)(MPI_Comm, CONST int *, MPI_Comm *))dlsym(RTLD_NEXT, "MPI_Cart_sub");
//     }
//
//     int ret = orig_MPI_Cart_sub(old_comm, belongs, new_comm);
//     return ret;
// }
// int MPI_Cart_create(MPI_Comm comm_old,
//                     int ndims,
//                     CONST int * dims,
//                     CONST int * periods,
//                     int reorder,
//                     MPI_Comm * comm_cart) {
//
//
//     static int (*orig_MPI_Cart_create)(MPI_Comm, int, CONST int *, CONST int *, int, MPI_Comm *)
//     =
//             0;
//     if (orig_MPI_Cart_create == 0) {
//         orig_MPI_Cart_create = (int (*)(MPI_Comm, int, CONST int *, CONST int *, int, MPI_Comm
//         *))
//                 dlsym(RTLD_NEXT, "MPI_Cart_create");
//     }
//
//     int ret = orig_MPI_Cart_create(comm_old, ndims, dims, periods, reorder, comm_cart);
//     return ret;
// }
// int MPI_Graph_create(MPI_Comm comm_old,
//                      int nnodes,
//                      CONST int * index,
//                      CONST int * edges,
//                      int reorder,
//                      MPI_Comm * comm_graph) {
//
//
//     static int (*orig_MPI_Graph_create)(MPI_Comm, int, CONST int *, CONST int *, int, MPI_Comm *)
//     =
//             0;
//     if (orig_MPI_Graph_create == 0) {
//         orig_MPI_Graph_create = (int (*)(MPI_Comm, int, CONST int *, CONST int *, int, MPI_Comm
//         *))
//                 dlsym(RTLD_NEXT, "MPI_Graph_create");
//     }
//
//     int ret = orig_MPI_Graph_create(comm_old, nnodes, index, edges, reorder, comm_graph);
//     return ret;
// }
// int MPI_Dist_graph_create_adjacent(MPI_Comm comm_old,
//                                    int indegree,
//                                    CONST int sources[],
//                                    CONST int sourceweights[],
//                                    int outdegree,
//                                    CONST int destinations[],
//                                    CONST int destweights[],
//                                    MPI_Info info,
//                                    int reorder,
//                                    MPI_Comm * comm_dist_graph) {
//
//
//     static int (*orig_MPI_Dist_graph_create_adjacent)(MPI_Comm,
//                                                       int,
//                                                       CONST int[],
//                                                       CONST int[],
//                                                       int,
//                                                       CONST int[],
//                                                       CONST int[],
//                                                       MPI_Info,
//                                                       int,
//                                                       MPI_Comm *) = 0;
//     if (orig_MPI_Dist_graph_create_adjacent == 0) {
//         orig_MPI_Dist_graph_create_adjacent =
//                 (int (*)(MPI_Comm,
//                          int,
//                          CONST int[],
//                          CONST int[],
//                          int,
//                          CONST int[],
//                          CONST int[],
//                          MPI_Info,
//                          int,
//                          MPI_Comm *))dlsym(RTLD_NEXT, "MPI_Dist_graph_create_adjacent");
//     }
//
//     int ret = orig_MPI_Dist_graph_create_adjacent(comm_old,
//                                                   indegree,
//                                                   sources,
//                                                   sourceweights,
//                                                   outdegree,
//                                                   destinations,
//                                                   destweights,
//                                                   info,
//                                                   reorder,
//                                                   comm_dist_graph);
//     return ret;
// }
// int MPI_Dist_graph_create(MPI_Comm comm_old,
//                           int n,
//                           CONST int sources[],
//                           CONST int degrees[],
//                           CONST int destinations[],
//                           CONST int weights[],
//                           MPI_Info info,
//                           int reorder,
//                           MPI_Comm * comm_dist_graph) {
//
//
//     static int (*orig_MPI_Dist_graph_create)(MPI_Comm,
//                                              int,
//                                              CONST int[],
//                                              CONST int[],
//                                              CONST int[],
//                                              CONST int[],
//                                              MPI_Info,
//                                              int,
//                                              MPI_Comm *) = 0;
//     if (orig_MPI_Dist_graph_create == 0) {
//         orig_MPI_Dist_graph_create = (int (*)(MPI_Comm,
//                                               int,
//                                               CONST int[],
//                                               CONST int[],
//                                               CONST int[],
//                                               CONST int[],
//                                               MPI_Info,
//                                               int,
//                                               MPI_Comm *))dlsym(RTLD_NEXT,
//                                               "MPI_Dist_graph_create");
//     }
//
//     int ret = orig_MPI_Dist_graph_create(comm_old,
//                                          n,
//                                          sources,
//                                          degrees,
//                                          destinations,
//                                          weights,
//                                          info,
//                                          reorder,
//                                          comm_dist_graph);
//     return ret;
// }
int MPI_Send(CONST void * buf, int count, MPI_Datatype datatype, int dest, int tag, MPI_Comm comm) {
    static int (*orig_MPI_Send)(CONST void *, int, MPI_Datatype, int, int, MPI_Comm) = 0;
    if (orig_MPI_Send == 0) {
        orig_MPI_Send =
                (int (*)(CONST void *, int, MPI_Datatype, int, int, MPI_Comm))dlsym(RTLD_NEXT,
                                                                                    "MPI_Send");
    }

    pythia_event(Pythia_MPI_fn::Send, dest, tag);
    int ret = orig_MPI_Send(buf, count, datatype, dest, tag, comm);
    return ret;
}

int MPI_Recv(void * buf,
             int count,
             MPI_Datatype datatype,
             int source,
             int tag,
             MPI_Comm comm,
             MPI_Status * status) {
    static int (*orig_MPI_Recv)(void *, int, MPI_Datatype, int, int, MPI_Comm, MPI_Status *) = 0;
    if (orig_MPI_Recv == 0)
        orig_MPI_Recv = (int (*)(void *, int, MPI_Datatype, int, int, MPI_Comm, MPI_Status *))dlsym(
                RTLD_NEXT,
                "MPI_Recv");

    pythia_event(Pythia_MPI_fn::Recv, source, tag);
    int ret = orig_MPI_Recv(buf, count, datatype, source, tag, comm, status);
    return ret;
}
// int MPI_Bsend(CONST void * arg0, int arg1, MPI_Datatype arg2, int arg3, int arg4, MPI_Comm arg5)
// {
//
//
//     static int (*orig_MPI_Bsend)(CONST void *, int, MPI_Datatype, int, int, MPI_Comm) = 0;
//     if (orig_MPI_Bsend == 0) {
//         orig_MPI_Bsend =
//                 (int (*)(CONST void *, int, MPI_Datatype, int, int, MPI_Comm))dlsym(RTLD_NEXT,
//                                                                                     "MPI_Bsend");
//     }
//
//     int ret = orig_MPI_Bsend(arg0, arg1, arg2, arg3, arg4, arg5);
//     return ret;
// }
// int MPI_Ssend(CONST void * arg0, int arg1, MPI_Datatype arg2, int arg3, int arg4, MPI_Comm arg5)
// {
//
//
//     static int (*orig_MPI_Ssend)(CONST void *, int, MPI_Datatype, int, int, MPI_Comm) = 0;
//     if (orig_MPI_Ssend == 0) {
//         orig_MPI_Ssend =
//                 (int (*)(CONST void *, int, MPI_Datatype, int, int, MPI_Comm))dlsym(RTLD_NEXT,
//                                                                                     "MPI_Ssend");
//     }
//
//     int ret = orig_MPI_Ssend(arg0, arg1, arg2, arg3, arg4, arg5);
//     return ret;
// }
// int MPI_Rsend(CONST void * arg0, int arg1, MPI_Datatype arg2, int arg3, int arg4, MPI_Comm arg5)
// {
//
//
//     static int (*orig_MPI_Rsend)(CONST void *, int, MPI_Datatype, int, int, MPI_Comm) = 0;
//     if (orig_MPI_Rsend == 0) {
//         orig_MPI_Rsend =
//                 (int (*)(CONST void *, int, MPI_Datatype, int, int, MPI_Comm))dlsym(RTLD_NEXT,
//                                                                                     "MPI_Rsend");
//     }
//
//     int ret = orig_MPI_Rsend(arg0, arg1, arg2, arg3, arg4, arg5);
//     return ret;
// }
// int MPI_Isend(CONST void * arg0,
//               int arg1,
//               MPI_Datatype arg2,
//               int arg3,
//               int arg4,
//               MPI_Comm arg5,
//               MPI_Request * arg6) {
//
//
//     static int (
//             *orig_MPI_Isend)(CONST void *, int, MPI_Datatype, int, int, MPI_Comm, MPI_Request *)
//             = 0;
//     if (orig_MPI_Isend == 0) {
//         orig_MPI_Isend =
//                 (int (*)(CONST void *, int, MPI_Datatype, int, int, MPI_Comm, MPI_Request
//                 *))dlsym(
//                         RTLD_NEXT,
//                         "MPI_Isend");
//     }
//
//     int ret = orig_MPI_Isend(arg0, arg1, arg2, arg3, arg4, arg5, arg6);
//     return ret;
// }
// int MPI_Ibsend(CONST void * arg0,
//                int arg1,
//                MPI_Datatype arg2,
//                int arg3,
//                int arg4,
//                MPI_Comm arg5,
//                MPI_Request * arg6) {
//
//
//     static int (
//             *orig_MPI_Ibsend)(CONST void *, int, MPI_Datatype, int, int, MPI_Comm, MPI_Request *)
//             = 0;
//     if (orig_MPI_Ibsend == 0) {
//         orig_MPI_Ibsend =
//                 (int (*)(CONST void *, int, MPI_Datatype, int, int, MPI_Comm, MPI_Request
//                 *))dlsym(
//                         RTLD_NEXT,
//                         "MPI_Ibsend");
//     }
//
//     int ret = orig_MPI_Ibsend(arg0, arg1, arg2, arg3, arg4, arg5, arg6);
//     return ret;
// }
// int MPI_Issend(CONST void * arg0,
//                int arg1,
//                MPI_Datatype arg2,
//                int arg3,
//                int arg4,
//                MPI_Comm arg5,
//                MPI_Request * arg6) {
//
//
//     static int (
//             *orig_MPI_Issend)(CONST void *, int, MPI_Datatype, int, int, MPI_Comm, MPI_Request *)
//             = 0;
//     if (orig_MPI_Issend == 0) {
//         orig_MPI_Issend =
//                 (int (*)(CONST void *, int, MPI_Datatype, int, int, MPI_Comm, MPI_Request
//                 *))dlsym(
//                         RTLD_NEXT,
//                         "MPI_Issend");
//     }
//
//     int ret = orig_MPI_Issend(arg0, arg1, arg2, arg3, arg4, arg5, arg6);
//     return ret;
// }
// int MPI_Irsend(CONST void * arg0,
//                int arg1,
//                MPI_Datatype arg2,
//                int arg3,
//                int arg4,
//                MPI_Comm arg5,
//                MPI_Request * arg6) {
//
//
//     static int (
//             *orig_MPI_Irsend)(CONST void *, int, MPI_Datatype, int, int, MPI_Comm, MPI_Request *)
//             = 0;
//     if (orig_MPI_Irsend == 0) {
//         orig_MPI_Irsend =
//                 (int (*)(CONST void *, int, MPI_Datatype, int, int, MPI_Comm, MPI_Request
//                 *))dlsym(
//                         RTLD_NEXT,
//                         "MPI_Irsend");
//     }
//
//     int ret = orig_MPI_Irsend(arg0, arg1, arg2, arg3, arg4, arg5, arg6);
//     return ret;
// }
// int MPI_Irecv(void * arg0,
//               int arg1,
//               MPI_Datatype arg2,
//               int arg3,
//               int arg4,
//               MPI_Comm arg5,
//               MPI_Request * arg6) {
//
//
//     static int (*orig_MPI_Irecv)(void *, int, MPI_Datatype, int, int, MPI_Comm, MPI_Request *) =
//     0; if (orig_MPI_Irecv == 0) {
//         orig_MPI_Irecv = (int (*)(void *, int, MPI_Datatype, int, int, MPI_Comm, MPI_Request *))
//                 dlsym(RTLD_NEXT, "MPI_Irecv");
//     }
//
//     int ret = orig_MPI_Irecv(arg0, arg1, arg2, arg3, arg4, arg5, arg6);
//     return ret;
// }
// int MPI_Sendrecv(CONST void * arg0,
//                  int arg1,
//                  MPI_Datatype arg2,
//                  int arg3,
//                  int arg4,
//                  void * arg5,
//                  int arg6,
//                  MPI_Datatype arg7,
//                  int arg8,
//                  int arg9,
//                  MPI_Comm arg10,
//                  MPI_Status * arg11) {
//
//
//     static int (*orig_MPI_Sendrecv)(CONST void *,
//                                     int,
//                                     MPI_Datatype,
//                                     int,
//                                     int,
//                                     void *,
//                                     int,
//                                     MPI_Datatype,
//                                     int,
//                                     int,
//                                     MPI_Comm,
//                                     MPI_Status *) = 0;
//     if (orig_MPI_Sendrecv == 0) {
//         orig_MPI_Sendrecv = (int (*)(CONST void *,
//                                      int,
//                                      MPI_Datatype,
//                                      int,
//                                      int,
//                                      void *,
//                                      int,
//                                      MPI_Datatype,
//                                      int,
//                                      int,
//                                      MPI_Comm,
//                                      MPI_Status *))dlsym(RTLD_NEXT, "MPI_Sendrecv");
//     }
//
//     int ret = orig_MPI_Sendrecv(arg0,
//                                 arg1,
//                                 arg2,
//                                 arg3,
//                                 arg4,
//                                 arg5,
//                                 arg6,
//                                 arg7,
//                                 arg8,
//                                 arg9,
//                                 arg10,
//                                 arg11);
//     return ret;
// }
// int MPI_Sendrecv_replace(void * arg0,
//                          int arg1,
//                          MPI_Datatype arg2,
//                          int arg3,
//                          int arg4,
//                          int arg5,
//                          int arg6,
//                          MPI_Comm arg7,
//                          MPI_Status * arg8) {
//
//
//     static int (*orig_MPI_Sendrecv_replace)(void *,
//                                             int,
//                                             MPI_Datatype,
//                                             int,
//                                             int,
//                                             int,
//                                             int,
//                                             MPI_Comm,
//                                             MPI_Status *) = 0;
//     if (orig_MPI_Sendrecv_replace == 0) {
//         orig_MPI_Sendrecv_replace =
//                 (int (*)(void *, int, MPI_Datatype, int, int, int, int, MPI_Comm, MPI_Status *))
//                         dlsym(RTLD_NEXT, "MPI_Sendrecv_replace");
//     }
//
//     int ret = orig_MPI_Sendrecv_replace(arg0, arg1, arg2, arg3, arg4, arg5, arg6, arg7, arg8);
//     return ret;
// }
// int MPI_Send_init(CONST void * arg0,
//                   int arg1,
//                   MPI_Datatype arg2,
//                   int arg3,
//                   int arg4,
//                   MPI_Comm arg5,
//                   MPI_Request * arg6) {
//
//
//     static int (*orig_MPI_Send_init)(CONST void *,
//                                      int,
//                                      MPI_Datatype,
//                                      int,
//                                      int,
//                                      MPI_Comm,
//                                      MPI_Request *) = 0;
//     if (orig_MPI_Send_init == 0) {
//         orig_MPI_Send_init =
//                 (int (*)(CONST void *, int, MPI_Datatype, int, int, MPI_Comm, MPI_Request
//                 *))dlsym(
//                         RTLD_NEXT,
//                         "MPI_Send_init");
//     }
//
//     int ret = orig_MPI_Send_init(arg0, arg1, arg2, arg3, arg4, arg5, arg6);
//     return ret;
// }
// int MPI_Bsend_init(CONST void * arg0,
//                    int arg1,
//                    MPI_Datatype arg2,
//                    int arg3,
//                    int arg4,
//                    MPI_Comm arg5,
//                    MPI_Request * arg6) {
//
//
//     static int (*orig_MPI_Bsend_init)(CONST void *,
//                                       int,
//                                       MPI_Datatype,
//                                       int,
//                                       int,
//                                       MPI_Comm,
//                                       MPI_Request *) = 0;
//     if (orig_MPI_Bsend_init == 0) {
//         orig_MPI_Bsend_init =
//                 (int (*)(CONST void *, int, MPI_Datatype, int, int, MPI_Comm, MPI_Request
//                 *))dlsym(
//                         RTLD_NEXT,
//                         "MPI_Bsend_init");
//     }
//
//     int ret = orig_MPI_Bsend_init(arg0, arg1, arg2, arg3, arg4, arg5, arg6);
//     return ret;
// }
// int MPI_Ssend_init(CONST void * arg0,
//                    int arg1,
//                    MPI_Datatype arg2,
//                    int arg3,
//                    int arg4,
//                    MPI_Comm arg5,
//                    MPI_Request * arg6) {
//
//
//     static int (*orig_MPI_Ssend_init)(CONST void *,
//                                       int,
//                                       MPI_Datatype,
//                                       int,
//                                       int,
//                                       MPI_Comm,
//                                       MPI_Request *) = 0;
//     if (orig_MPI_Ssend_init == 0) {
//         orig_MPI_Ssend_init =
//                 (int (*)(CONST void *, int, MPI_Datatype, int, int, MPI_Comm, MPI_Request
//                 *))dlsym(
//                         RTLD_NEXT,
//                         "MPI_Ssend_init");
//     }
//
//     int ret = orig_MPI_Ssend_init(arg0, arg1, arg2, arg3, arg4, arg5, arg6);
//     return ret;
// }
// int MPI_Rsend_init(CONST void * arg0,
//                    int arg1,
//                    MPI_Datatype arg2,
//                    int arg3,
//                    int arg4,
//                    MPI_Comm arg5,
//                    MPI_Request * arg6) {
//
//
//     static int (*orig_MPI_Rsend_init)(CONST void *,
//                                       int,
//                                       MPI_Datatype,
//                                       int,
//                                       int,
//                                       MPI_Comm,
//                                       MPI_Request *) = 0;
//     if (orig_MPI_Rsend_init == 0) {
//         orig_MPI_Rsend_init =
//                 (int (*)(CONST void *, int, MPI_Datatype, int, int, MPI_Comm, MPI_Request
//                 *))dlsym(
//                         RTLD_NEXT,
//                         "MPI_Rsend_init");
//     }
//
//     int ret = orig_MPI_Rsend_init(arg0, arg1, arg2, arg3, arg4, arg5, arg6);
//     return ret;
// }
// int MPI_Recv_init(void * arg0,
//                   int arg1,
//                   MPI_Datatype arg2,
//                   int arg3,
//                   int arg4,
//                   MPI_Comm arg5,
//                   MPI_Request * arg6) {
//
//
//     static int (*orig_MPI_Recv_init)(void *, int, MPI_Datatype, int, int, MPI_Comm, MPI_Request
//     *) =
//             0;
//     if (orig_MPI_Recv_init == 0) {
//         orig_MPI_Recv_init = (int (*)(void *, int, MPI_Datatype, int, int, MPI_Comm, MPI_Request
//         *))
//                 dlsym(RTLD_NEXT, "MPI_Recv_init");
//     }
//
//     int ret = orig_MPI_Recv_init(arg0, arg1, arg2, arg3, arg4, arg5, arg6);
//     return ret;
// }
// int MPI_Start(MPI_Request * arg0) {
//
//
//     static int (*orig_MPI_Start)(MPI_Request *) = 0;
//     if (orig_MPI_Start == 0) {
//         orig_MPI_Start = (int (*)(MPI_Request *))dlsym(RTLD_NEXT, "MPI_Start");
//     }
//
//     int ret = orig_MPI_Start(arg0);
//     return ret;
// }
// int MPI_Startall(int arg0, MPI_Request * arg1) {
//
//
//     static int (*orig_MPI_Startall)(int, MPI_Request *) = 0;
//     if (orig_MPI_Startall == 0) {
//         orig_MPI_Startall = (int (*)(int, MPI_Request *))dlsym(RTLD_NEXT, "MPI_Startall");
//     }
//
//     int ret = orig_MPI_Startall(arg0, arg1);
//     return ret;
// }
// int MPI_Wait(MPI_Request * arg0, MPI_Status * arg1) {
//
//
//     static int (*orig_MPI_Wait)(MPI_Request *, MPI_Status *) = 0;
//     if (orig_MPI_Wait == 0) {
//         orig_MPI_Wait = (int (*)(MPI_Request *, MPI_Status *))dlsym(RTLD_NEXT, "MPI_Wait");
//     }
//
//     int ret = orig_MPI_Wait(arg0, arg1);
//     return ret;
// }
// int MPI_Test(MPI_Request * arg0, int * arg1, MPI_Status * arg2) {
//
//
//     static int (*orig_MPI_Test)(MPI_Request *, int *, MPI_Status *) = 0;
//     if (orig_MPI_Test == 0) {
//         orig_MPI_Test = (int (*)(MPI_Request *, int *, MPI_Status *))dlsym(RTLD_NEXT,
//     }
//
//     int ret = orig_MPI_Test(arg0, arg1, arg2);
//     return ret;
// }
// int MPI_Waitany(int arg0, MPI_Request * arg1, int * arg2, MPI_Status * arg3) {
//
//
//     static int (*orig_MPI_Waitany)(int, MPI_Request *, int *, MPI_Status *) = 0;
//     if (orig_MPI_Waitany == 0) {
//         orig_MPI_Waitany =
//                 (int (*)(int, MPI_Request *, int *, MPI_Status *))dlsym(RTLD_NEXT,
//                 "MPI_Waitany");
//     }
//
//     int ret = orig_MPI_Waitany(arg0, arg1, arg2, arg3);
//     return ret;
// }
// int MPI_Testany(int arg0, MPI_Request * arg1, int * arg2, int * arg3, MPI_Status * arg4) {
//
//
//     static int (*orig_MPI_Testany)(int, MPI_Request *, int *, int *, MPI_Status *) = 0;
//     if (orig_MPI_Testany == 0) {
//         orig_MPI_Testany =
//                 (int (*)(int, MPI_Request *, int *, int *, MPI_Status *))dlsym(RTLD_NEXT,
//                                                                                "MPI_Testany");
//     }
//
//     int ret = orig_MPI_Testany(arg0, arg1, arg2, arg3, arg4);
//     return ret;
// }
// int MPI_Waitall(int arg0, MPI_Request * arg1, MPI_Status * arg2) {
//
//
//     static int (*orig_MPI_Waitall)(int, MPI_Request *, MPI_Status *) = 0;
//     if (orig_MPI_Waitall == 0) {
//         orig_MPI_Waitall =
//                 (int (*)(int, MPI_Request *, MPI_Status *))dlsym(RTLD_NEXT, "MPI_Waitall");
//     }
//
//     int ret = orig_MPI_Waitall(arg0, arg1, arg2);
//     return ret;
// }
// int MPI_Testall(int arg0, MPI_Request * arg1, int * arg2, MPI_Status * arg3) {
//
//
//     static int (*orig_MPI_Testall)(int, MPI_Request *, int *, MPI_Status *) = 0;
//     if (orig_MPI_Testall == 0) {
//         orig_MPI_Testall =
//                 (int (*)(int, MPI_Request *, int *, MPI_Status *))dlsym(RTLD_NEXT,
//                 "MPI_Testall");
//     }
//
//     int ret = orig_MPI_Testall(arg0, arg1, arg2, arg3);
//     return ret;
// }
// int MPI_Waitsome(int arg0, MPI_Request * arg1, int * arg2, int * arg3, MPI_Status * arg4) {
//
//
//     static int (*orig_MPI_Waitsome)(int, MPI_Request *, int *, int *, MPI_Status *) = 0;
//     if (orig_MPI_Waitsome == 0) {
//         orig_MPI_Waitsome =
//                 (int (*)(int, MPI_Request *, int *, int *, MPI_Status *))dlsym(RTLD_NEXT,
//                                                                                "MPI_Waitsome");
//     }
//
//     int ret = orig_MPI_Waitsome(arg0, arg1, arg2, arg3, arg4);
//     return ret;
// }
// int MPI_Testsome(int arg0, MPI_Request * arg1, int * arg2, int * arg3, MPI_Status * arg4) {
//
//
//     static int (*orig_MPI_Testsome)(int, MPI_Request *, int *, int *, MPI_Status *) = 0;
//     if (orig_MPI_Testsome == 0) {
//         orig_MPI_Testsome =
//                 (int (*)(int, MPI_Request *, int *, int *, MPI_Status *))dlsym(RTLD_NEXT,
//                                                                                "MPI_Testsome");
//     }
//
//     int ret = orig_MPI_Testsome(arg0, arg1, arg2, arg3, arg4);
//     return ret;
// }
// int MPI_Probe(int source, int tag, MPI_Comm comm, MPI_Status * status) {
//
//
//     static int (*orig_MPI_Probe)(int, int, MPI_Comm, MPI_Status *) = 0;
//     if (orig_MPI_Probe == 0) {
//         orig_MPI_Probe = (int (*)(int, int, MPI_Comm, MPI_Status *))dlsym(RTLD_NEXT,
//     }
//
//     int ret = orig_MPI_Probe(source, tag, comm, status);
//     return ret;
// }
// int MPI_Iprobe(int source, int tag, MPI_Comm comm, int * flag, MPI_Status * status) {
//
//
//     static int (*orig_MPI_Iprobe)(int, int, MPI_Comm, int *, MPI_Status *) = 0;
//     if (orig_MPI_Iprobe == 0) {
//         orig_MPI_Iprobe =
//                 (int (*)(int, int, MPI_Comm, int *, MPI_Status *))dlsym(RTLD_NEXT, "MPI_Iprobe");
//     }
//
//     int ret = orig_MPI_Iprobe(source, tag, comm, flag, status);
//     return ret;
// }
// int MPI_Barrier(MPI_Comm arg0) {
//
//
//     static int (*orig_MPI_Barrier)(MPI_Comm) = 0;
//     if (orig_MPI_Barrier == 0) {
//         orig_MPI_Barrier = (int (*)(MPI_Comm))dlsym(RTLD_NEXT, "MPI_Barrier");
//     }
//
//     int ret = orig_MPI_Barrier(arg0);
//     return ret;
// }
// int MPI_Bcast(void * arg0, int arg1, MPI_Datatype arg2, int arg3, MPI_Comm arg4) {
//
//
//     static int (*orig_MPI_Bcast)(void *, int, MPI_Datatype, int, MPI_Comm) = 0;
//     if (orig_MPI_Bcast == 0) {
//         orig_MPI_Bcast =
//                 (int (*)(void *, int, MPI_Datatype, int, MPI_Comm))dlsym(RTLD_NEXT, "MPI_Bcast");
//     }
//
//     int ret = orig_MPI_Bcast(arg0, arg1, arg2, arg3, arg4);
//     return ret;
// }
int MPI_Gather(CONST void * arg0,
               int arg1,
               MPI_Datatype arg2,
               void * arg3,
               int arg4,
               MPI_Datatype arg5,
               int root,
               MPI_Comm arg7) {
    static int (*orig_MPI_Gather)(CONST void *,
                                  int,
                                  MPI_Datatype,
                                  void *,
                                  int,
                                  MPI_Datatype,
                                  int,
                                  MPI_Comm) = 0;
    if (orig_MPI_Gather == 0) {
        orig_MPI_Gather =
                (int (*)(CONST void *, int, MPI_Datatype, void *, int, MPI_Datatype, int, MPI_Comm))
                        dlsym(RTLD_NEXT, "MPI_Gather");
    }

    int ret = orig_MPI_Gather(arg0, arg1, arg2, arg3, arg4, arg5, root, arg7);
    pythia_event(Pythia_MPI_fn::Gather, root);
    return ret;
}
// int MPI_Gatherv(CONST void * arg0,
//                 int arg1,
//                 MPI_Datatype arg2,
//                 void * arg3,
//                 CONST int * arg4,
//                 CONST int * arg5,
//                 MPI_Datatype arg6,
//                 int arg7,
//                 MPI_Comm arg8) {
//
//
//     static int (*orig_MPI_Gatherv)(CONST void *,
//                                    int,
//                                    MPI_Datatype,
//                                    void *,
//                                    CONST int *,
//                                    CONST int *,
//                                    MPI_Datatype,
//                                    int,
//                                    MPI_Comm) = 0;
//     if (orig_MPI_Gatherv == 0) {
//         orig_MPI_Gatherv = (int (*)(CONST void *,
//                                     int,
//                                     MPI_Datatype,
//                                     void *,
//                                     CONST int *,
//                                     CONST int *,
//                                     MPI_Datatype,
//                                     int,
//                                     MPI_Comm))dlsym(RTLD_NEXT, "MPI_Gatherv");
//     }
//
//     int ret = orig_MPI_Gatherv(arg0, arg1, arg2, arg3, arg4, arg5, arg6, arg7, arg8);
//     return ret;
// }
// int MPI_Scatter(CONST void * arg0,
//                 int arg1,
//                 MPI_Datatype arg2,
//                 void * arg3,
//                 int arg4,
//                 MPI_Datatype arg5,
//                 int arg6,
//                 MPI_Comm arg7) {
//
//
//     static int (*orig_MPI_Scatter)(CONST void *,
//                                    int,
//                                    MPI_Datatype,
//                                    void *,
//                                    int,
//                                    MPI_Datatype,
//                                    int,
//                                    MPI_Comm) = 0;
//     if (orig_MPI_Scatter == 0) {
//         orig_MPI_Scatter =
//                 (int (*)(CONST void *, int, MPI_Datatype, void *, int, MPI_Datatype, int,
//                 MPI_Comm))
//                         dlsym(RTLD_NEXT, "MPI_Scatter");
//     }
//
//     int ret = orig_MPI_Scatter(arg0, arg1, arg2, arg3, arg4, arg5, arg6, arg7);
//     return ret;
// }
// int MPI_Scatterv(CONST void * arg0,
//                  CONST int * arg1,
//                  CONST int * arg2,
//                  MPI_Datatype arg3,
//                  void * arg4,
//                  int arg5,
//                  MPI_Datatype arg6,
//                  int arg7,
//                  MPI_Comm arg8) {
//
//
//     static int (*orig_MPI_Scatterv)(CONST void *,
//                                     CONST int *,
//                                     CONST int *,
//                                     MPI_Datatype,
//                                     void *,
//                                     int,
//                                     MPI_Datatype,
//                                     int,
//                                     MPI_Comm) = 0;
//     if (orig_MPI_Scatterv == 0) {
//         orig_MPI_Scatterv = (int (*)(CONST void *,
//                                      CONST int *,
//                                      CONST int *,
//                                      MPI_Datatype,
//                                      void *,
//                                      int,
//                                      MPI_Datatype,
//                                      int,
//                                      MPI_Comm))dlsym(RTLD_NEXT, "MPI_Scatterv");
//     }
//
//     int ret = orig_MPI_Scatterv(arg0, arg1, arg2, arg3, arg4, arg5, arg6, arg7, arg8);
//     return ret;
// }
// int MPI_Allgather(CONST void * arg0,
//                   int arg1,
//                   MPI_Datatype arg2,
//                   void * arg3,
//                   int arg4,
//                   MPI_Datatype arg5,
//                   MPI_Comm arg6) {
//
//
//     static int (*orig_MPI_Allgather)(CONST void *,
//                                      int,
//                                      MPI_Datatype,
//                                      void *,
//                                      int,
//                                      MPI_Datatype,
//                                      MPI_Comm) = 0;
//     if (orig_MPI_Allgather == 0) {
//         orig_MPI_Allgather =
//                 (int (*)(CONST void *, int, MPI_Datatype, void *, int, MPI_Datatype, MPI_Comm))
//                         dlsym(RTLD_NEXT, "MPI_Allgather");
//     }
//
//     int ret = orig_MPI_Allgather(arg0, arg1, arg2, arg3, arg4, arg5, arg6);
//     return ret;
// }
// int MPI_Allgatherv(CONST void * arg0,
//                    int arg1,
//                    MPI_Datatype arg2,
//                    void * arg3,
//                    CONST int * arg4,
//                    CONST int * arg5,
//                    MPI_Datatype arg6,
//                    MPI_Comm arg7) {
//
//
//     static int (*orig_MPI_Allgatherv)(CONST void *,
//                                       int,
//                                       MPI_Datatype,
//                                       void *,
//                                       CONST int *,
//                                       CONST int *,
//                                       MPI_Datatype,
//                                       MPI_Comm) = 0;
//     if (orig_MPI_Allgatherv == 0) {
//         orig_MPI_Allgatherv = (int (*)(CONST void *,
//                                        int,
//                                        MPI_Datatype,
//                                        void *,
//                                        CONST int *,
//                                        CONST int *,
//                                        MPI_Datatype,
//                                        MPI_Comm))dlsym(RTLD_NEXT, "MPI_Allgatherv");
//     }
//
//     int ret = orig_MPI_Allgatherv(arg0, arg1, arg2, arg3, arg4, arg5, arg6, arg7);
//     return ret;
// }
int MPI_Alltoall(CONST void * arg0,
                 int arg1,
                 MPI_Datatype arg2,
                 void * arg3,
                 int arg4,
                 MPI_Datatype arg5,
                 MPI_Comm arg6) {
    static int (*orig_MPI_Alltoall)(CONST void *,
                                    int,
                                    MPI_Datatype,
                                    void *,
                                    int,
                                    MPI_Datatype,
                                    MPI_Comm) = 0;
    if (orig_MPI_Alltoall == 0) {
        orig_MPI_Alltoall =
                (int (*)(CONST void *, int, MPI_Datatype, void *, int, MPI_Datatype, MPI_Comm))
                        dlsym(RTLD_NEXT, "MPI_Alltoall");
    }

    int ret = orig_MPI_Alltoall(arg0, arg1, arg2, arg3, arg4, arg5, arg6);
    pythia_event(Pythia_MPI_fn::Alltoall);
    return ret;
}
// int MPI_Alltoallv(CONST void * arg0,
//                   CONST int * arg1,
//                   CONST int * arg2,
//                   MPI_Datatype arg3,
//                   void * arg4,
//                   CONST int * arg5,
//                   CONST int * arg6,
//                   MPI_Datatype arg7,
//                   MPI_Comm arg8) {
//
//
//     static int (*orig_MPI_Alltoallv)(CONST void *,
//                                      CONST int *,
//                                      CONST int *,
//                                      MPI_Datatype,
//                                      void *,
//                                      CONST int *,
//                                      CONST int *,
//                                      MPI_Datatype,
//                                      MPI_Comm) = 0;
//     if (orig_MPI_Alltoallv == 0) {
//         orig_MPI_Alltoallv = (int (*)(CONST void *,
//                                       CONST int *,
//                                       CONST int *,
//                                       MPI_Datatype,
//                                       void *,
//                                       CONST int *,
//                                       CONST int *,
//                                       MPI_Datatype,
//                                       MPI_Comm))dlsym(RTLD_NEXT, "MPI_Alltoallv");
//     }
//
//     int ret = orig_MPI_Alltoallv(arg0, arg1, arg2, arg3, arg4, arg5, arg6, arg7, arg8);
//     return ret;
// }
// int MPI_Reduce(CONST void * arg0,
//                void * arg1,
//                int arg2,
//                MPI_Datatype arg3,
//                MPI_Op arg4,
//                int arg5,
//                MPI_Comm arg6) {
//
//
//     static int (*orig_MPI_Reduce)(CONST void *, void *, int, MPI_Datatype, MPI_Op, int, MPI_Comm)
//     =
//             0;
//     if (orig_MPI_Reduce == 0) {
//         orig_MPI_Reduce = (int (*)(CONST void *, void *, int, MPI_Datatype, MPI_Op, int,
//         MPI_Comm))
//                 dlsym(RTLD_NEXT, "MPI_Reduce");
//     }
//
//     int ret = orig_MPI_Reduce(arg0, arg1, arg2, arg3, arg4, arg5, arg6);
//     return ret;
// }
// int MPI_Allreduce(CONST void * arg0,
//                   void * arg1,
//                   int arg2,
//                   MPI_Datatype arg3,
//                   MPI_Op arg4,
//                   MPI_Comm arg5) {
//
//
//     static int (*orig_MPI_Allreduce)(CONST void *, void *, int, MPI_Datatype, MPI_Op, MPI_Comm) =
//     0; if (orig_MPI_Allreduce == 0) {
//         orig_MPI_Allreduce = (int (*)(CONST void *, void *, int, MPI_Datatype, MPI_Op, MPI_Comm))
//                 dlsym(RTLD_NEXT, "MPI_Allreduce");
//     }
//
//     int ret = orig_MPI_Allreduce(arg0, arg1, arg2, arg3, arg4, arg5);
//     return ret;
// }
// int MPI_Reduce_scatter(CONST void * arg0,
//                        void * arg1,
//                        CONST int * arg2,
//                        MPI_Datatype arg3,
//                        MPI_Op arg4,
//                        MPI_Comm arg5) {
//
//
//     static int (*orig_MPI_Reduce_scatter)(CONST void *,
//                                           void *,
//                                           CONST int *,
//                                           MPI_Datatype,
//                                           MPI_Op,
//                                           MPI_Comm) = 0;
//     if (orig_MPI_Reduce_scatter == 0) {
//         orig_MPI_Reduce_scatter =
//                 (int (*)(CONST void *, void *, CONST int *, MPI_Datatype, MPI_Op,
//                 MPI_Comm))dlsym(
//                         RTLD_NEXT,
//                         "MPI_Reduce_scatter");
//     }
//
//     int ret = orig_MPI_Reduce_scatter(arg0, arg1, arg2, arg3, arg4, arg5);
//     return ret;
// }
// int MPI_Scan(CONST void * arg0,
//              void * arg1,
//              int arg2,
//              MPI_Datatype arg3,
//              MPI_Op arg4,
//              MPI_Comm arg5) {
//
//
//     static int (*orig_MPI_Scan)(CONST void *, void *, int, MPI_Datatype, MPI_Op, MPI_Comm) = 0;
//     if (orig_MPI_Scan == 0) {
//         orig_MPI_Scan = (int (*)(CONST void *, void *, int, MPI_Datatype, MPI_Op,
//         MPI_Comm))dlsym(
//                 RTLD_NEXT,
//                 "MPI_Scan");
//     }
//
//     int ret = orig_MPI_Scan(arg0, arg1, arg2, arg3, arg4, arg5);
//     return ret;
// }
// int MPI_Ibarrier(MPI_Comm arg0, MPI_Request * arg1) {
//
//
//     static int (*orig_MPI_Ibarrier)(MPI_Comm, MPI_Request *) = 0;
//     if (orig_MPI_Ibarrier == 0) {
//         orig_MPI_Ibarrier = (int (*)(MPI_Comm, MPI_Request *))dlsym(RTLD_NEXT, "MPI_Ibarrier");
//     }
//
//     int ret = orig_MPI_Ibarrier(arg0, arg1);
//     return ret;
// }
// int MPI_Ibcast(void * arg0,
//                int arg1,
//                MPI_Datatype arg2,
//                int arg3,
//                MPI_Comm arg4,
//                MPI_Request * arg5) {
//
//
//     static int (*orig_MPI_Ibcast)(void *, int, MPI_Datatype, int, MPI_Comm, MPI_Request *) = 0;
//     if (orig_MPI_Ibcast == 0) {
//         orig_MPI_Ibcast = (int (*)(void *, int, MPI_Datatype, int, MPI_Comm, MPI_Request
//         *))dlsym(
//                 RTLD_NEXT,
//                 "MPI_Ibcast");
//     }
//
//     int ret = orig_MPI_Ibcast(arg0, arg1, arg2, arg3, arg4, arg5);
//     return ret;
// }
// int MPI_Igather(const void * arg0,
//                 int arg1,
//                 MPI_Datatype arg2,
//                 void * arg3,
//                 int arg4,
//                 MPI_Datatype arg5,
//                 int arg6,
//                 MPI_Comm arg7,
//                 MPI_Request * arg8) {
//
//
//     static int (*orig_MPI_Igather)(const void *,
//                                    int,
//                                    MPI_Datatype,
//                                    void *,
//                                    int,
//                                    MPI_Datatype,
//                                    int,
//                                    MPI_Comm,
//                                    MPI_Request *) = 0;
//     if (orig_MPI_Igather == 0) {
//         orig_MPI_Igather = (int (*)(const void *,
//                                     int,
//                                     MPI_Datatype,
//                                     void *,
//                                     int,
//                                     MPI_Datatype,
//                                     int,
//                                     MPI_Comm,
//                                     MPI_Request *))dlsym(RTLD_NEXT, "MPI_Igather");
//     }
//
//     int ret = orig_MPI_Igather(arg0, arg1, arg2, arg3, arg4, arg5, arg6, arg7, arg8);
//     return ret;
// }
// int MPI_Igatherv(const void * arg0,
//                  int arg1,
//                  MPI_Datatype arg2,
//                  void * arg3,
//                  const int * arg4,
//                  const int * arg5,
//                  MPI_Datatype arg6,
//                  int arg7,
//                  MPI_Comm arg8,
//                  MPI_Request * arg9) {
//
//
//     static int (*orig_MPI_Igatherv)(const void *,
//                                     int,
//                                     MPI_Datatype,
//                                     void *,
//                                     const int *,
//                                     const int *,
//                                     MPI_Datatype,
//                                     int,
//                                     MPI_Comm,
//                                     MPI_Request *) = 0;
//     if (orig_MPI_Igatherv == 0) {
//         orig_MPI_Igatherv = (int (*)(const void *,
//                                      int,
//                                      MPI_Datatype,
//                                      void *,
//                                      const int *,
//                                      const int *,
//                                      MPI_Datatype,
//                                      int,
//                                      MPI_Comm,
//                                      MPI_Request *))dlsym(RTLD_NEXT, "MPI_Igatherv");
//     }
//
//     int ret = orig_MPI_Igatherv(arg0, arg1, arg2, arg3, arg4, arg5, arg6, arg7, arg8, arg9);
//     return ret;
// }
// int MPI_Iscatter(const void * arg0,
//                  int arg1,
//                  MPI_Datatype arg2,
//                  void * arg3,
//                  int arg4,
//                  MPI_Datatype arg5,
//                  int arg6,
//                  MPI_Comm arg7,
//                  MPI_Request * arg8) {
//
//
//     static int (*orig_MPI_Iscatter)(const void *,
//                                     int,
//                                     MPI_Datatype,
//                                     void *,
//                                     int,
//                                     MPI_Datatype,
//                                     int,
//                                     MPI_Comm,
//                                     MPI_Request *) = 0;
//     if (orig_MPI_Iscatter == 0) {
//         orig_MPI_Iscatter = (int (*)(const void *,
//                                      int,
//                                      MPI_Datatype,
//                                      void *,
//                                      int,
//                                      MPI_Datatype,
//                                      int,
//                                      MPI_Comm,
//                                      MPI_Request *))dlsym(RTLD_NEXT, "MPI_Iscatter");
//     }
//
//     int ret = orig_MPI_Iscatter(arg0, arg1, arg2, arg3, arg4, arg5, arg6, arg7, arg8);
//     return ret;
// }
// int MPI_Iscatterv(const void * arg0,
//                   const int * arg1,
//                   const int * arg2,
//                   MPI_Datatype arg3,
//                   void * arg4,
//                   int arg5,
//                   MPI_Datatype arg6,
//                   int arg7,
//                   MPI_Comm arg8,
//                   MPI_Request * arg9) {
//
//
//     static int (*orig_MPI_Iscatterv)(const void *,
//                                      const int *,
//                                      const int *,
//                                      MPI_Datatype,
//                                      void *,
//                                      int,
//                                      MPI_Datatype,
//                                      int,
//                                      MPI_Comm,
//                                      MPI_Request *) = 0;
//     if (orig_MPI_Iscatterv == 0) {
//         orig_MPI_Iscatterv = (int (*)(const void *,
//                                       const int *,
//                                       const int *,
//                                       MPI_Datatype,
//                                       void *,
//                                       int,
//                                       MPI_Datatype,
//                                       int,
//                                       MPI_Comm,
//                                       MPI_Request *))dlsym(RTLD_NEXT, "MPI_Iscatterv");
//     }
//
//     int ret = orig_MPI_Iscatterv(arg0, arg1, arg2, arg3, arg4, arg5, arg6, arg7, arg8, arg9);
//     return ret;
// }
// int MPI_Iallgather(const void * arg0,
//                    int arg1,
//                    MPI_Datatype arg2,
//                    void * arg3,
//                    int arg4,
//                    MPI_Datatype arg5,
//                    MPI_Comm arg6,
//                    MPI_Request * arg7) {
//
//
//     static int (*orig_MPI_Iallgather)(const void *,
//                                       int,
//                                       MPI_Datatype,
//                                       void *,
//                                       int,
//                                       MPI_Datatype,
//                                       MPI_Comm,
//                                       MPI_Request *) = 0;
//     if (orig_MPI_Iallgather == 0) {
//         orig_MPI_Iallgather = (int (*)(const void *,
//                                        int,
//                                        MPI_Datatype,
//                                        void *,
//                                        int,
//                                        MPI_Datatype,
//                                        MPI_Comm,
//                                        MPI_Request *))dlsym(RTLD_NEXT, "MPI_Iallgather");
//     }
//
//     int ret = orig_MPI_Iallgather(arg0, arg1, arg2, arg3, arg4, arg5, arg6, arg7);
//     return ret;
// }
// int MPI_Iallgatherv(const void * arg0,
//                     int arg1,
//                     MPI_Datatype arg2,
//                     void * arg3,
//                     const int * arg4,
//                     const int * arg5,
//                     MPI_Datatype arg6,
//                     MPI_Comm arg7,
//                     MPI_Request * arg8) {
//
//
//     static int (*orig_MPI_Iallgatherv)(const void *,
//                                        int,
//                                        MPI_Datatype,
//                                        void *,
//                                        const int *,
//                                        const int *,
//                                        MPI_Datatype,
//                                        MPI_Comm,
//                                        MPI_Request *) = 0;
//     if (orig_MPI_Iallgatherv == 0) {
//         orig_MPI_Iallgatherv = (int (*)(const void *,
//                                         int,
//                                         MPI_Datatype,
//                                         void *,
//                                         const int *,
//                                         const int *,
//                                         MPI_Datatype,
//                                         MPI_Comm,
//                                         MPI_Request *))dlsym(RTLD_NEXT, "MPI_Iallgatherv");
//     }
//
//     int ret = orig_MPI_Iallgatherv(arg0, arg1, arg2, arg3, arg4, arg5, arg6, arg7, arg8);
//     return ret;
// }
// int MPI_Ialltoall(const void * arg0,
//                   int arg1,
//                   MPI_Datatype arg2,
//                   void * arg3,
//                   int arg4,
//                   MPI_Datatype arg5,
//                   MPI_Comm arg6,
//                   MPI_Request * arg7) {
//
//
//     static int (*orig_MPI_Ialltoall)(const void *,
//                                      int,
//                                      MPI_Datatype,
//                                      void *,
//                                      int,
//                                      MPI_Datatype,
//                                      MPI_Comm,
//                                      MPI_Request *) = 0;
//     if (orig_MPI_Ialltoall == 0) {
//         orig_MPI_Ialltoall = (int (*)(const void *,
//                                       int,
//                                       MPI_Datatype,
//                                       void *,
//                                       int,
//                                       MPI_Datatype,
//                                       MPI_Comm,
//                                       MPI_Request *))dlsym(RTLD_NEXT, "MPI_Ialltoall");
//     }
//
//     int ret = orig_MPI_Ialltoall(arg0, arg1, arg2, arg3, arg4, arg5, arg6, arg7);
//     return ret;
// }
// int MPI_Ialltoallv(const void * arg0,
//                    const int * arg1,
//                    const int * arg2,
//                    MPI_Datatype arg3,
//                    void * arg4,
//                    const int * arg5,
//                    const int * arg6,
//                    MPI_Datatype arg7,
//                    MPI_Comm arg8,
//                    MPI_Request * arg9) {
//
//
//     static int (*orig_MPI_Ialltoallv)(const void *,
//                                       const int *,
//                                       const int *,
//                                       MPI_Datatype,
//                                       void *,
//                                       const int *,
//                                       const int *,
//                                       MPI_Datatype,
//                                       MPI_Comm,
//                                       MPI_Request *) = 0;
//     if (orig_MPI_Ialltoallv == 0) {
//         orig_MPI_Ialltoallv = (int (*)(const void *,
//                                        const int *,
//                                        const int *,
//                                        MPI_Datatype,
//                                        void *,
//                                        const int *,
//                                        const int *,
//                                        MPI_Datatype,
//                                        MPI_Comm,
//                                        MPI_Request *))dlsym(RTLD_NEXT, "MPI_Ialltoallv");
//     }
//
//     int ret = orig_MPI_Ialltoallv(arg0, arg1, arg2, arg3, arg4, arg5, arg6, arg7, arg8, arg9);
//     return ret;
// }
// int MPI_Ireduce(const void * arg0,
//                 void * arg1,
//                 int arg2,
//                 MPI_Datatype arg3,
//                 MPI_Op arg4,
//                 int arg5,
//                 MPI_Comm arg6,
//                 MPI_Request * arg7) {
//
//
//     static int (*orig_MPI_Ireduce)(const void *,
//                                    void *,
//                                    int,
//                                    MPI_Datatype,
//                                    MPI_Op,
//                                    int,
//                                    MPI_Comm,
//                                    MPI_Request *) = 0;
//     if (orig_MPI_Ireduce == 0) {
//         orig_MPI_Ireduce = (int (*)(const void *,
//                                     void *,
//                                     int,
//                                     MPI_Datatype,
//                                     MPI_Op,
//                                     int,
//                                     MPI_Comm,
//                                     MPI_Request *))dlsym(RTLD_NEXT, "MPI_Ireduce");
//     }
//
//     int ret = orig_MPI_Ireduce(arg0, arg1, arg2, arg3, arg4, arg5, arg6, arg7);
//     return ret;
// }
// int MPI_Iallreduce(const void * arg0,
//                    void * arg1,
//                    int arg2,
//                    MPI_Datatype arg3,
//                    MPI_Op arg4,
//                    MPI_Comm arg5,
//                    MPI_Request * arg6) {
//
//
//     static int (*orig_MPI_Iallreduce)(const void *,
//                                       void *,
//                                       int,
//                                       MPI_Datatype,
//                                       MPI_Op,
//                                       MPI_Comm,
//                                       MPI_Request *) = 0;
//     if (orig_MPI_Iallreduce == 0) {
//         orig_MPI_Iallreduce =
//                 (int (*)(const void *, void *, int, MPI_Datatype, MPI_Op, MPI_Comm, MPI_Request
//                 *))
//                         dlsym(RTLD_NEXT, "MPI_Iallreduce");
//     }
//
//     int ret = orig_MPI_Iallreduce(arg0, arg1, arg2, arg3, arg4, arg5, arg6);
//     return ret;
// }
// int MPI_Ireduce_scatter(const void * arg0,
//                         void * arg1,
//                         const int * arg2,
//                         MPI_Datatype arg3,
//                         MPI_Op arg4,
//                         MPI_Comm arg5,
//                         MPI_Request * arg6) {
//
//
//     static int (*orig_MPI_Ireduce_scatter)(const void *,
//                                            void *,
//                                            const int *,
//                                            MPI_Datatype,
//                                            MPI_Op,
//                                            MPI_Comm,
//                                            MPI_Request *) = 0;
//     if (orig_MPI_Ireduce_scatter == 0) {
//         orig_MPI_Ireduce_scatter = (int (*)(const void *,
//                                             void *,
//                                             const int *,
//                                             MPI_Datatype,
//                                             MPI_Op,
//                                             MPI_Comm,
//                                             MPI_Request *))dlsym(RTLD_NEXT,
//                                             "MPI_Ireduce_scatter");
//     }
//
//     int ret = orig_MPI_Ireduce_scatter(arg0, arg1, arg2, arg3, arg4, arg5, arg6);
//     return ret;
// }
// int MPI_Iscan(const void * arg0,
//               void * arg1,
//               int arg2,
//               MPI_Datatype arg3,
//               MPI_Op arg4,
//               MPI_Comm arg5,
//               MPI_Request * arg6) {
//
//
//     static int (*orig_MPI_Iscan)(const void *,
//                                  void *,
//                                  int,
//                                  MPI_Datatype,
//                                  MPI_Op,
//                                  MPI_Comm,
//                                  MPI_Request *) = 0;
//     if (orig_MPI_Iscan == 0) {
//         orig_MPI_Iscan =
//                 (int (*)(const void *, void *, int, MPI_Datatype, MPI_Op, MPI_Comm, MPI_Request
//                 *))
//                         dlsym(RTLD_NEXT, "MPI_Iscan");
//     }
//
//     int ret = orig_MPI_Iscan(arg0, arg1, arg2, arg3, arg4, arg5, arg6);
//     return ret;
// }
// int MPI_Get(void * arg0,
//             int arg1,
//             MPI_Datatype arg2,
//             int arg3,
//             MPI_Aint arg4,
//             int arg5,
//             MPI_Datatype arg6,
//             MPI_Win arg7) {
//
//
//     static int (
//             *orig_MPI_Get)(void *, int, MPI_Datatype, int, MPI_Aint, int, MPI_Datatype, MPI_Win)
//             = 0;
//     if (orig_MPI_Get == 0) {
//         orig_MPI_Get =
//                 (int (*)(void *, int, MPI_Datatype, int, MPI_Aint, int, MPI_Datatype, MPI_Win))
//                         dlsym(RTLD_NEXT, "MPI_Get");
//     }
//
//     int ret = orig_MPI_Get(arg0, arg1, arg2, arg3, arg4, arg5, arg6, arg7);
//     return ret;
// }
// int MPI_Put(CONST void * arg0,
//             int arg1,
//             MPI_Datatype arg2,
//             int arg3,
//             MPI_Aint arg4,
//             int arg5,
//             MPI_Datatype arg6,
//             MPI_Win arg7) {
//
//
//     static int (*orig_MPI_Put)(CONST void *,
//                                int,
//                                MPI_Datatype,
//                                int,
//                                MPI_Aint,
//                                int,
//                                MPI_Datatype,
//                                MPI_Win) = 0;
//     if (orig_MPI_Put == 0) {
//         orig_MPI_Put =
//                 (int (*)(CONST void *, int, MPI_Datatype, int, MPI_Aint, int, MPI_Datatype,
//                 MPI_Win))
//                         dlsym(RTLD_NEXT, "MPI_Put");
//     }
//
//     int ret = orig_MPI_Put(arg0, arg1, arg2, arg3, arg4, arg5, arg6, arg7);
//     return ret;
// }
// int MPI_Comm_spawn(CONST char * command,
//                    char * argv[],
//                    int maxprocs,
//                    MPI_Info info,
//                    int root,
//                    MPI_Comm comm,
//                    MPI_Comm * intercomm,
//                    int array_of_errcodes[]) {
//
//
//     static int (*orig_MPI_Comm_spawn)(CONST char *,
//                                       char *[],
//                                       int,
//                                       MPI_Info,
//                                       int,
//                                       MPI_Comm,
//                                       MPI_Comm *,
//                                       int[]) = 0;
//     if (orig_MPI_Comm_spawn == 0) {
//         orig_MPI_Comm_spawn =
//                 (int (*)(CONST char *, char *[], int, MPI_Info, int, MPI_Comm, MPI_Comm *,
//                 int[]))
//                         dlsym(RTLD_NEXT, "MPI_Comm_spawn");
//     }
//
//     int ret = orig_MPI_Comm_spawn(command,
//                                   argv,
//                                   maxprocs,
//                                   info,
//                                   root,
//                                   comm,
//                                   intercomm,
//                                   array_of_errcodes);
//     return ret;
// }
}
