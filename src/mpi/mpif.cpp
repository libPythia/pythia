#include "common.hpp"

#define _GNU_SOURCE
#include <assert.h>
#include <dlfcn.h>
#include <mpi.h>
#include <stddef.h>

// Extra definitions
#define CONST const

extern "C" {

// Pointers on intercepted fonctions
void mpi_init_(int * e) {
    static void (*orig_mpi_init_)(int *) = 0;
    if (orig_mpi_init_ == 0) {
        orig_mpi_init_ = (void (*)(int *))dlsym(RTLD_NEXT, "mpi_init_");
    }

    orig_mpi_init_(e);

    pythia_init();
}
// void mpi_init_thread_(int * arg0, int * arg1, int * arg2) {
//
//
//     static void (*orig_mpi_init_thread_)(int *, int *, int *) = 0;
//     if (orig_mpi_init_thread_ == 0) {
//         orig_mpi_init_thread_ = (void (*)(int *, int *, int *))dlsym(RTLD_NEXT,
//     }
//
//     orig_mpi_init_thread_(arg0, arg1, arg2);
// }
void mpi_finalize_(int * arg0) {
    static void (*orig_mpi_finalize_)(int *) = 0;
    if (orig_mpi_finalize_ == 0) {
        orig_mpi_finalize_ = (void (*)(int *))dlsym(RTLD_NEXT, "mpi_finalize_");
    }

    pythia_deinit();

    orig_mpi_finalize_(arg0);
}
// void mpi_barrier_(MPI_Comm * arg0, int * arg1) {
//
//
//     static void (*orig_mpi_barrier_)(MPI_Comm *, int *) = 0;
//     if (orig_mpi_barrier_ == 0) {
//         orig_mpi_barrier_ = (void (*)(MPI_Comm *, int *))dlsym(RTLD_NEXT, "mpi_barrier_");
//     }
//
//     orig_mpi_barrier_(arg0, arg1);
// }
// void mpi_comm_size_(MPI_Comm * arg0, int * arg1, int * arg2) {
//
//
//     static void (*orig_mpi_comm_size_)(MPI_Comm *, int *, int *) = 0;
//     if (orig_mpi_comm_size_ == 0) {
//         orig_mpi_comm_size_ =
//                 (void (*)(MPI_Comm *, int *, int *))dlsym(RTLD_NEXT, "mpi_comm_size_");
//     }
//
//     orig_mpi_comm_size_(arg0, arg1, arg2);
// }
// void mpi_comm_rank_(MPI_Comm * arg0, int * arg1, int * arg2) {
//
//
//     static void (*orig_mpi_comm_rank_)(MPI_Comm *, int *, int *) = 0;
//     if (orig_mpi_comm_rank_ == 0) {
//         orig_mpi_comm_rank_ =
//                 (void (*)(MPI_Comm *, int *, int *))dlsym(RTLD_NEXT, "mpi_comm_rank_");
//     }
//
//     orig_mpi_comm_rank_(arg0, arg1, arg2);
// }
// void mpi_comm_get_parent_(MPI_Comm * arg0, int * arg1) {
//
//
//     static void (*orig_mpi_comm_get_parent_)(MPI_Comm *, int *) = 0;
//     if (orig_mpi_comm_get_parent_ == 0) {
//         orig_mpi_comm_get_parent_ =
//                 (void (*)(MPI_Comm *, int *))dlsym(RTLD_NEXT, "mpi_comm_get_parent_");
//     }
//
//     orig_mpi_comm_get_parent_(arg0, arg1);
// }
// void mpi_type_size_(MPI_Datatype * arg0, int * arg1, int * arg2) {
//
//
//     static void (*orig_mpi_type_size_)(MPI_Datatype *, int *, int *) = 0;
//     if (orig_mpi_type_size_ == 0) {
//         orig_mpi_type_size_ =
//                 (void (*)(MPI_Datatype *, int *, int *))dlsym(RTLD_NEXT, "mpi_type_size_");
//     }
//
//     orig_mpi_type_size_(arg0, arg1, arg2);
// }
// void mpi_cancel_(MPI_Request * arg0, int * arg1) {
//
//
//     static void (*orig_mpi_cancel_)(MPI_Request *, int *) = 0;
//     if (orig_mpi_cancel_ == 0) {
//         orig_mpi_cancel_ = (void (*)(MPI_Request *, int *))dlsym(RTLD_NEXT, "mpi_cancel_");
//     }
//
//     orig_mpi_cancel_(arg0, arg1);
// }
// int mpi_comm_create_(int * arg0, int * arg1, int * arg2, int * arg3) {
//
//
//     static int (*orig_mpi_comm_create_)(int *, int *, int *, int *) = 0;
//     if (orig_mpi_comm_create_ == 0) {
//         orig_mpi_comm_create_ =
//                 (int (*)(int *, int *, int *, int *))dlsym(RTLD_NEXT, "mpi_comm_create_");
//     }
//
//     int ret = orig_mpi_comm_create_(arg0, arg1, arg2, arg3);
//     return ret;
// }
// int mpi_comm_create_group_(int * arg0, int * arg1, int * arg2, int * arg3, int * arg4) {
//
//
//     static int (*orig_mpi_comm_create_group_)(int *, int *, int *, int *, int *) = 0;
//     if (orig_mpi_comm_create_group_ == 0) {
//         orig_mpi_comm_create_group_ =
//                 (int (*)(int *, int *, int *, int *, int *))dlsym(RTLD_NEXT,
//                                                                   "mpi_comm_create_group_");
//     }
//
//     int ret = orig_mpi_comm_create_group_(arg0, arg1, arg2, arg3, arg4);
//     return ret;
// }
// int mpi_comm_split_(int * arg0, int * arg1, int * arg2, int * arg3, int * arg4) {
//
//
//     static int (*orig_mpi_comm_split_)(int *, int *, int *, int *, int *) = 0;
//     if (orig_mpi_comm_split_ == 0) {
//         orig_mpi_comm_split_ =
//                 (int (*)(int *, int *, int *, int *, int *))dlsym(RTLD_NEXT, "mpi_comm_split_");
//     }
//
//     int ret = orig_mpi_comm_split_(arg0, arg1, arg2, arg3, arg4);
//     return ret;
// }
// int mpi_comm_dup_(int * arg0, int * arg1, int * arg2) {
//
//
//     static int (*orig_mpi_comm_dup_)(int *, int *, int *) = 0;
//     if (orig_mpi_comm_dup_ == 0) {
//         orig_mpi_comm_dup_ = (int (*)(int *, int *, int *))dlsym(RTLD_NEXT, "mpi_comm_dup_");
//     }
//
//     int ret = orig_mpi_comm_dup_(arg0, arg1, arg2);
//     return ret;
// }
// int mpi_comm_dup_with_info_(int * arg0, int * arg1, int * arg2, int * arg3) {
//
//
//     static int (*orig_mpi_comm_dup_with_info_)(int *, int *, int *, int *) = 0;
//     if (orig_mpi_comm_dup_with_info_ == 0) {
//         orig_mpi_comm_dup_with_info_ =
//                 (int (*)(int *, int *, int *, int *))dlsym(RTLD_NEXT, "mpi_comm_dup_with_info_");
//     }
//
//     int ret = orig_mpi_comm_dup_with_info_(arg0, arg1, arg2, arg3);
//     return ret;
// }
// int mpi_comm_split_type_(int * arg0, int * arg1, int * arg2, int * arg3, int * arg4) {
//
//
//     static int (*orig_mpi_comm_split_type_)(int *, int *, int *, int *, int *) = 0;
//     if (orig_mpi_comm_split_type_ == 0) {
//         orig_mpi_comm_split_type_ =
//                 (int (*)(int *, int *, int *, int *, int *))dlsym(RTLD_NEXT,
//                                                                   "mpi_comm_split_type_");
//     }
//
//     int ret = orig_mpi_comm_split_type_(arg0, arg1, arg2, arg3, arg4);
//     return ret;
// }
// int mpi_intercomm_create_(int * arg0,
//                           int * arg1,
//                           int * arg2,
//                           int * arg3,
//                           int * arg4,
//                           int * arg5,
//                           int * arg6) {
//
//
//     static int (*orig_mpi_intercomm_create_)(int *, int *, int *, int *, int *, int *, int *) =
//     0; if (orig_mpi_intercomm_create_ == 0) {
//         orig_mpi_intercomm_create_ = (int (*)(int *, int *, int *, int *, int *, int *, int *))
//                 dlsym(RTLD_NEXT, "mpi_intercomm_create_");
//     }
//
//     int ret = orig_mpi_intercomm_create_(arg0, arg1, arg2, arg3, arg4, arg5, arg6);
//     return ret;
// }
// int mpi_intercomm_merge_(int * arg0, int * arg1, int * arg2, int * arg3) {
//
//
//     static int (*orig_mpi_intercomm_merge_)(int *, int *, int *, int *) = 0;
//     if (orig_mpi_intercomm_merge_ == 0) {
//         orig_mpi_intercomm_merge_ =
//                 (int (*)(int *, int *, int *, int *))dlsym(RTLD_NEXT, "mpi_intercomm_merge_");
//     }
//
//     int ret = orig_mpi_intercomm_merge_(arg0, arg1, arg2, arg3);
//     return ret;
// }
// int mpi_cart_sub_(int * arg0, int * arg1, int * arg2, int * arg3) {
//
//
//     static int (*orig_mpi_cart_sub_)(int *, int *, int *, int *) = 0;
//     if (orig_mpi_cart_sub_ == 0) {
//         orig_mpi_cart_sub_ = (int (*)(int *, int *, int *, int *))dlsym(RTLD_NEXT,
//     }
//
//     int ret = orig_mpi_cart_sub_(arg0, arg1, arg2, arg3);
//     return ret;
// }
// int mpi_cart_create_(int * arg0,
//                      int * arg1,
//                      int * arg2,
//                      int * arg3,
//                      int * arg4,
//                      int * arg5,
//                      int * arg6) {
//
//
//     static int (*orig_mpi_cart_create_)(int *, int *, int *, int *, int *, int *, int *) = 0;
//     if (orig_mpi_cart_create_ == 0) {
//         orig_mpi_cart_create_ =
//                 (int (*)(int *, int *, int *, int *, int *, int *, int *))dlsym(RTLD_NEXT,
//                                                                                 "mpi_cart_create_");
//     }
//
//     int ret = orig_mpi_cart_create_(arg0, arg1, arg2, arg3, arg4, arg5, arg6);
//     return ret;
// }
// int mpi_graph_create_(int * arg0,
//                       int * arg1,
//                       int * arg2,
//                       int * arg3,
//                       int * arg4,
//                       int * arg5,
//                       int * arg6) {
//
//
//     static int (*orig_mpi_graph_create_)(int *, int *, int *, int *, int *, int *, int *) = 0;
//     if (orig_mpi_graph_create_ == 0) {
//         orig_mpi_graph_create_ = (int (*)(int *, int *, int *, int *, int *, int *, int *))dlsym(
//                 RTLD_NEXT,
//                 "mpi_graph_create_");
//     }
//
//     int ret = orig_mpi_graph_create_(arg0, arg1, arg2, arg3, arg4, arg5, arg6);
//     return ret;
// }
// int mpi_dist_graph_create_adjacent_(int * arg0,
//                                     int * arg1,
//                                     int * arg2,
//                                     int * arg3,
//                                     int * arg4,
//                                     int * arg5,
//                                     int * arg6,
//                                     int * arg7,
//                                     int * arg8,
//                                     int * arg9,
//                                     int * arg10) {
//
//
//     static int (*orig_mpi_dist_graph_create_adjacent_)(int *,
//                                                        int *,
//                                                        int *,
//                                                        int *,
//                                                        int *,
//                                                        int *,
//                                                        int *,
//                                                        int *,
//                                                        int *,
//                                                        int *,
//                                                        int *) = 0;
//     if (orig_mpi_dist_graph_create_adjacent_ == 0) {
//         orig_mpi_dist_graph_create_adjacent_ =
//                 (int (*)(int *,
//                          int *,
//                          int *,
//                          int *,
//                          int *,
//                          int *,
//                          int *,
//                          int *,
//                          int *,
//                          int *,
//                          int *))dlsym(RTLD_NEXT, "mpi_dist_graph_create_adjacent_");
//     }
//
//     int ret = orig_mpi_dist_graph_create_adjacent_(arg0,
//                                                    arg1,
//                                                    arg2,
//                                                    arg3,
//                                                    arg4,
//                                                    arg5,
//                                                    arg6,
//                                                    arg7,
//                                                    arg8,
//                                                    arg9,
//                                                    arg10);
//     return ret;
// }
// int mpi_dist_graph_create_(int * arg0,
//                            int * arg1,
//                            int * arg2,
//                            int * arg3,
//                            int * arg4,
//                            int * arg5,
//                            int * arg6,
//                            int * arg7,
//                            int * arg8,
//                            int * arg9) {
//
//
//     static int (*orig_mpi_dist_graph_create_)(int *,
//                                               int *,
//                                               int *,
//                                               int *,
//                                               int *,
//                                               int *,
//                                               int *,
//                                               int *,
//                                               int *,
//                                               int *) = 0;
//     if (orig_mpi_dist_graph_create_ == 0) {
//         orig_mpi_dist_graph_create_ =
//                 (int (*)(int *, int *, int *, int *, int *, int *, int *, int *, int *, int *))
//                         dlsym(RTLD_NEXT, "mpi_dist_graph_create_");
//     }
//
//     int ret =
//             orig_mpi_dist_graph_create_(arg0, arg1, arg2, arg3, arg4, arg5, arg6, arg7, arg8,
//             arg9);
//     return ret;
// }
// void mpi_send_(void * arg0, int * arg1, MPI_Datatype * arg2, int * arg3, int * arg4, int * arg5)
// {
//
//
//     static void (*orig_mpi_send_)(void *, int *, MPI_Datatype *, int *, int *, int *) = 0;
//     if (orig_mpi_send_ == 0) {
//         orig_mpi_send_ =
//                 (void (*)(void *, int *, MPI_Datatype *, int *, int *, int *))dlsym(RTLD_NEXT,
//                                                                                     "mpi_send_");
//     }
//
//     orig_mpi_send_(arg0, arg1, arg2, arg3, arg4, arg5);
// }
// void mpi_recv_(void * arg0,
//                int * arg1,
//                MPI_Datatype * arg2,
//                int * arg3,
//                int * arg4,
//                MPI_Status * arg5,
//                int * arg6) {
//
//
//     static void (
//             *orig_mpi_recv_)(void *, int *, MPI_Datatype *, int *, int *, MPI_Status *, int *) =
//             0;
//     if (orig_mpi_recv_ == 0) {
//         orig_mpi_recv_ =
//                 (void (*)(void *, int *, MPI_Datatype *, int *, int *, MPI_Status *, int
//                 *))dlsym(
//                         RTLD_NEXT,
//                         "mpi_recv_");
//     }
//
//     orig_mpi_recv_(arg0, arg1, arg2, arg3, arg4, arg5, arg6);
// }
// void mpi_sendrecv_(void * arg0,
//                    int arg1,
//                    MPI_Datatype arg2,
//                    int arg3,
//                    int arg4,
//                    void * arg5,
//                    int arg6,
//                    MPI_Datatype arg7,
//                    int arg8,
//                    int arg9,
//                    MPI_Comm arg10,
//                    MPI_Status * arg11,
//                    int * arg12) {
//
//
//     static void (*orig_mpi_sendrecv_)(void *,
//                                       int,
//                                       MPI_Datatype,
//                                       int,
//                                       int,
//                                       void *,
//                                       int,
//                                       MPI_Datatype,
//                                       int,
//                                       int,
//                                       MPI_Comm,
//                                       MPI_Status *,
//                                       int *) = 0;
//     if (orig_mpi_sendrecv_ == 0) {
//         orig_mpi_sendrecv_ = (void (*)(void *,
//                                        int,
//                                        MPI_Datatype,
//                                        int,
//                                        int,
//                                        void *,
//                                        int,
//                                        MPI_Datatype,
//                                        int,
//                                        int,
//                                        MPI_Comm,
//                                        MPI_Status *,
//                                        int *))dlsym(RTLD_NEXT, "mpi_sendrecv_");
//     }
//
//     orig_mpi_sendrecv_(arg0,
//                        arg1,
//                        arg2,
//                        arg3,
//                        arg4,
//                        arg5,
//                        arg6,
//                        arg7,
//                        arg8,
//                        arg9,
//                        arg10,
//                        arg11,
//                        arg12);
// }
// void mpi_sendrecv_replace_(void * arg0,
//                            int arg1,
//                            MPI_Datatype arg2,
//                            int arg3,
//                            int arg4,
//                            int arg5,
//                            int arg6,
//                            MPI_Comm arg7,
//                            MPI_Status * arg8,
//                            int * arg9) {
//
//
//     static void (*orig_mpi_sendrecv_replace_)(void *,
//                                               int,
//                                               MPI_Datatype,
//                                               int,
//                                               int,
//                                               int,
//                                               int,
//                                               MPI_Comm,
//                                               MPI_Status *,
//                                               int *) = 0;
//     if (orig_mpi_sendrecv_replace_ == 0) {
//         orig_mpi_sendrecv_replace_ = (void (*)(void *,
//                                                int,
//                                                MPI_Datatype,
//                                                int,
//                                                int,
//                                                int,
//                                                int,
//                                                MPI_Comm,
//                                                MPI_Status *,
//                                                int *))dlsym(RTLD_NEXT, "mpi_sendrecv_replace_");
//     }
//
//     orig_mpi_sendrecv_replace_(arg0, arg1, arg2, arg3, arg4, arg5, arg6, arg7, arg8, arg9);
// }
// void mpi_bsend_(void * arg0,
//                 int * arg1,
//                 MPI_Datatype * arg2,
//                 int * arg3,
//                 int * arg4,
//                 MPI_Comm * arg5,
//                 int * arg6) {
//
//
//     static void (*orig_mpi_bsend_)(void *, int *, MPI_Datatype *, int *, int *, MPI_Comm *, int
//     *) =
//             0;
//     if (orig_mpi_bsend_ == 0) {
//         orig_mpi_bsend_ = (void (*)(void *, int *, MPI_Datatype *, int *, int *, MPI_Comm *, int
//         *))
//                 dlsym(RTLD_NEXT, "mpi_bsend_");
//     }
//
//     orig_mpi_bsend_(arg0, arg1, arg2, arg3, arg4, arg5, arg6);
// }
// void mpi_ssend_(void * arg0,
//                 int * arg1,
//                 MPI_Datatype * arg2,
//                 int * arg3,
//                 int * arg4,
//                 MPI_Comm * arg5,
//                 int * arg6) {
//
//
//     static void (*orig_mpi_ssend_)(void *, int *, MPI_Datatype *, int *, int *, MPI_Comm *, int
//     *) =
//             0;
//     if (orig_mpi_ssend_ == 0) {
//         orig_mpi_ssend_ = (void (*)(void *, int *, MPI_Datatype *, int *, int *, MPI_Comm *, int
//         *))
//                 dlsym(RTLD_NEXT, "mpi_ssend_");
//     }
//
//     orig_mpi_ssend_(arg0, arg1, arg2, arg3, arg4, arg5, arg6);
// }
// void mpi_rsend_(void * arg0,
//                 int * arg1,
//                 MPI_Datatype * arg2,
//                 int * arg3,
//                 int * arg4,
//                 MPI_Comm * arg5,
//                 int * arg6) {
//
//
//     static void (*orig_mpi_rsend_)(void *, int *, MPI_Datatype *, int *, int *, MPI_Comm *, int
//     *) =
//             0;
//     if (orig_mpi_rsend_ == 0) {
//         orig_mpi_rsend_ = (void (*)(void *, int *, MPI_Datatype *, int *, int *, MPI_Comm *, int
//         *))
//                 dlsym(RTLD_NEXT, "mpi_rsend_");
//     }
//
//     orig_mpi_rsend_(arg0, arg1, arg2, arg3, arg4, arg5, arg6);
// }
// void mpi_isend_(void * arg0,
//                 int * arg1,
//                 MPI_Datatype * arg2,
//                 int * arg3,
//                 int * arg4,
//                 MPI_Comm * arg5,
//                 MPI_Request * arg6,
//                 int * arg7) {
//
//
//     static void (*orig_mpi_isend_)(void *,
//                                    int *,
//                                    MPI_Datatype *,
//                                    int *,
//                                    int *,
//                                    MPI_Comm *,
//                                    MPI_Request *,
//                                    int *) = 0;
//     if (orig_mpi_isend_ == 0) {
//         orig_mpi_isend_ = (void (*)(void *,
//                                     int *,
//                                     MPI_Datatype *,
//                                     int *,
//                                     int *,
//                                     MPI_Comm *,
//                                     MPI_Request *,
//                                     int *))dlsym(RTLD_NEXT, "mpi_isend_");
//     }
//
//     orig_mpi_isend_(arg0, arg1, arg2, arg3, arg4, arg5, arg6, arg7);
// }
// void mpi_ibsend_(void * arg0,
//                  int * arg1,
//                  MPI_Datatype * arg2,
//                  int * arg3,
//                  int * arg4,
//                  MPI_Comm * arg5,
//                  MPI_Request * arg6,
//                  int * arg7) {
//
//
//     static void (*orig_mpi_ibsend_)(void *,
//                                     int *,
//                                     MPI_Datatype *,
//                                     int *,
//                                     int *,
//                                     MPI_Comm *,
//                                     MPI_Request *,
//                                     int *) = 0;
//     if (orig_mpi_ibsend_ == 0) {
//         orig_mpi_ibsend_ = (void (*)(void *,
//                                      int *,
//                                      MPI_Datatype *,
//                                      int *,
//                                      int *,
//                                      MPI_Comm *,
//                                      MPI_Request *,
//                                      int *))dlsym(RTLD_NEXT, "mpi_ibsend_");
//     }
//
//     orig_mpi_ibsend_(arg0, arg1, arg2, arg3, arg4, arg5, arg6, arg7);
// }
// void mpi_issend_(void * arg0,
//                  int * arg1,
//                  MPI_Datatype * arg2,
//                  int * arg3,
//                  int * arg4,
//                  MPI_Comm * arg5,
//                  MPI_Request * arg6,
//                  int * arg7) {
//
//
//     static void (*orig_mpi_issend_)(void *,
//                                     int *,
//                                     MPI_Datatype *,
//                                     int *,
//                                     int *,
//                                     MPI_Comm *,
//                                     MPI_Request *,
//                                     int *) = 0;
//     if (orig_mpi_issend_ == 0) {
//         orig_mpi_issend_ = (void (*)(void *,
//                                      int *,
//                                      MPI_Datatype *,
//                                      int *,
//                                      int *,
//                                      MPI_Comm *,
//                                      MPI_Request *,
//                                      int *))dlsym(RTLD_NEXT, "mpi_issend_");
//     }
//
//     orig_mpi_issend_(arg0, arg1, arg2, arg3, arg4, arg5, arg6, arg7);
// }
// void mpi_irsend_(void * arg0,
//                  int * arg1,
//                  MPI_Datatype * arg2,
//                  int * arg3,
//                  int * arg4,
//                  MPI_Comm * arg5,
//                  MPI_Request * arg6,
//                  int * arg7) {
//
//
//     static void (*orig_mpi_irsend_)(void *,
//                                     int *,
//                                     MPI_Datatype *,
//                                     int *,
//                                     int *,
//                                     MPI_Comm *,
//                                     MPI_Request *,
//                                     int *) = 0;
//     if (orig_mpi_irsend_ == 0) {
//         orig_mpi_irsend_ = (void (*)(void *,
//                                      int *,
//                                      MPI_Datatype *,
//                                      int *,
//                                      int *,
//                                      MPI_Comm *,
//                                      MPI_Request *,
//                                      int *))dlsym(RTLD_NEXT, "mpi_irsend_");
//     }
//
//     orig_mpi_irsend_(arg0, arg1, arg2, arg3, arg4, arg5, arg6, arg7);
// }
// void mpi_irecv_(void * arg0,
//                 int * arg1,
//                 MPI_Datatype * arg2,
//                 int * arg3,
//                 int * arg4,
//                 MPI_Comm * arg5,
//                 MPI_Request * arg6,
//                 int * arg7) {
//
//
//     static void (*orig_mpi_irecv_)(void *,
//                                    int *,
//                                    MPI_Datatype *,
//                                    int *,
//                                    int *,
//                                    MPI_Comm *,
//                                    MPI_Request *,
//                                    int *) = 0;
//     if (orig_mpi_irecv_ == 0) {
//         orig_mpi_irecv_ = (void (*)(void *,
//                                     int *,
//                                     MPI_Datatype *,
//                                     int *,
//                                     int *,
//                                     MPI_Comm *,
//                                     MPI_Request *,
//                                     int *))dlsym(RTLD_NEXT, "mpi_irecv_");
//     }
//
//     orig_mpi_irecv_(arg0, arg1, arg2, arg3, arg4, arg5, arg6, arg7);
// }
// void mpi_wait_(MPI_Request * arg0, MPI_Status * arg1, int * arg2) {
//
//
//     static void (*orig_mpi_wait_)(MPI_Request *, MPI_Status *, int *) = 0;
//     if (orig_mpi_wait_ == 0) {
//         orig_mpi_wait_ =
//                 (void (*)(MPI_Request *, MPI_Status *, int *))dlsym(RTLD_NEXT, "mpi_wait_");
//     }
//
//     orig_mpi_wait_(arg0, arg1, arg2);
// }
// void mpi_test_(MPI_Request * arg0, int * arg1, MPI_Status * arg2, int * arg3) {
//
//
//     static void (*orig_mpi_test_)(MPI_Request *, int *, MPI_Status *, int *) = 0;
//     if (orig_mpi_test_ == 0) {
//         orig_mpi_test_ =
//                 (void (*)(MPI_Request *, int *, MPI_Status *, int *))dlsym(RTLD_NEXT,
//                 "mpi_test_");
//     }
//
//     orig_mpi_test_(arg0, arg1, arg2, arg3);
// }
// void mpi_waitany_(int * arg0, MPI_Request * arg1, int * arg2, MPI_Status * arg3, int * arg4) {
//
//
//     static void (*orig_mpi_waitany_)(int *, MPI_Request *, int *, MPI_Status *, int *) = 0;
//     if (orig_mpi_waitany_ == 0) {
//         orig_mpi_waitany_ =
//                 (void (*)(int *, MPI_Request *, int *, MPI_Status *, int *))dlsym(RTLD_NEXT,
//                                                                                   "mpi_waitany_");
//     }
//
//     orig_mpi_waitany_(arg0, arg1, arg2, arg3, arg4);
// }
// void mpi_testany_(int * arg0,
//                   MPI_Request * arg1,
//                   int * arg2,
//                   int * arg3,
//                   MPI_Status * arg4,
//                   int * arg5) {
//
//
//     static void (*orig_mpi_testany_)(int *, MPI_Request *, int *, int *, MPI_Status *, int *) =
//     0; if (orig_mpi_testany_ == 0) {
//         orig_mpi_testany_ = (void (*)(int *, MPI_Request *, int *, int *, MPI_Status *, int *))
//                 dlsym(RTLD_NEXT, "mpi_testany_");
//     }
//
//     orig_mpi_testany_(arg0, arg1, arg2, arg3, arg4, arg5);
// }
// void mpi_waitall_(int * arg0, MPI_Request * arg1, MPI_Status * arg2, int * arg3) {
//
//
//     static void (*orig_mpi_waitall_)(int *, MPI_Request *, MPI_Status *, int *) = 0;
//     if (orig_mpi_waitall_ == 0) {
//         orig_mpi_waitall_ =
//                 (void (*)(int *, MPI_Request *, MPI_Status *, int *))dlsym(RTLD_NEXT,
//                                                                            "mpi_waitall_");
//     }
//
//     orig_mpi_waitall_(arg0, arg1, arg2, arg3);
// }
// void mpi_testall_(int * arg0, MPI_Request * arg1, int * arg2, MPI_Status * arg3, int * arg4) {
//
//
//     static void (*orig_mpi_testall_)(int *, MPI_Request *, int *, MPI_Status *, int *) = 0;
//     if (orig_mpi_testall_ == 0) {
//         orig_mpi_testall_ =
//                 (void (*)(int *, MPI_Request *, int *, MPI_Status *, int *))dlsym(RTLD_NEXT,
//                                                                                   "mpi_testall_");
//     }
//
//     orig_mpi_testall_(arg0, arg1, arg2, arg3, arg4);
// }
// void mpi_waitsome_(int * arg0,
//                    MPI_Request * arg1,
//                    int * arg2,
//                    int * arg3,
//                    MPI_Status * arg4,
//                    int * arg5) {
//
//
//     static void (*orig_mpi_waitsome_)(int *, MPI_Request *, int *, int *, MPI_Status *, int *) =
//     0; if (orig_mpi_waitsome_ == 0) {
//         orig_mpi_waitsome_ = (void (*)(int *, MPI_Request *, int *, int *, MPI_Status *, int *))
//                 dlsym(RTLD_NEXT, "mpi_waitsome_");
//     }
//
//     orig_mpi_waitsome_(arg0, arg1, arg2, arg3, arg4, arg5);
// }
// void mpi_testsome_(int * arg0,
//                    MPI_Request * arg1,
//                    int * arg2,
//                    int * arg3,
//                    MPI_Status * arg4,
//                    int * arg5) {
//
//
//     static void (*orig_mpi_testsome_)(int *, MPI_Request *, int *, int *, MPI_Status *, int *) =
//     0; if (orig_mpi_testsome_ == 0) {
//         orig_mpi_testsome_ = (void (*)(int *, MPI_Request *, int *, int *, MPI_Status *, int *))
//                 dlsym(RTLD_NEXT, "mpi_testsome_");
//     }
//
//     orig_mpi_testsome_(arg0, arg1, arg2, arg3, arg4, arg5);
// }
// void mpi_probe_(int * source, int * tag, MPI_Comm * comm, MPI_Status * status, int * err) {
//
//
//     static void (*orig_mpi_probe_)(int *, int *, MPI_Comm *, MPI_Status *, int *) = 0;
//     if (orig_mpi_probe_ == 0) {
//         orig_mpi_probe_ =
//                 (void (*)(int *, int *, MPI_Comm *, MPI_Status *, int *))dlsym(RTLD_NEXT,
//                                                                                "mpi_probe_");
//     }
//
//     orig_mpi_probe_(source, tag, comm, status, err);
// }
// void mpi_iprobe_(int * source,
//                  int * tag,
//                  MPI_Comm * comm,
//                  int * flag,
//                  MPI_Status * status,
//                  int * err) {
//
//
//     static void (*orig_mpi_iprobe_)(int *, int *, MPI_Comm *, int *, MPI_Status *, int *) = 0;
//     if (orig_mpi_iprobe_ == 0) {
//         orig_mpi_iprobe_ = (void (*)(int *, int *, MPI_Comm *, int *, MPI_Status *, int *))dlsym(
//                 RTLD_NEXT,
//                 "mpi_iprobe_");
//     }
//
//     orig_mpi_iprobe_(source, tag, comm, flag, status, err);
// }
// void mpi_get_(void * arg0,
//               int * arg1,
//               MPI_Datatype * arg2,
//               int * arg3,
//               MPI_Aint * arg4,
//               int * arg5,
//               MPI_Datatype * arg6,
//               MPI_Win * arg7,
//               int * arg8) {
//
//
//     static void (*orig_mpi_get_)(void *,
//                                  int *,
//                                  MPI_Datatype *,
//                                  int *,
//                                  MPI_Aint *,
//                                  int *,
//                                  MPI_Datatype *,
//                                  MPI_Win *,
//                                  int *) = 0;
//     if (orig_mpi_get_ == 0) {
//         orig_mpi_get_ = (void (*)(void *,
//                                   int *,
//                                   MPI_Datatype *,
//                                   int *,
//                                   MPI_Aint *,
//                                   int *,
//                                   MPI_Datatype *,
//                                   MPI_Win *,
//                                   int *))dlsym(RTLD_NEXT, "mpi_get_");
//     }
//
//     orig_mpi_get_(arg0, arg1, arg2, arg3, arg4, arg5, arg6, arg7, arg8);
// }
// void mpi_put_(void * arg0,
//               int * arg1,
//               MPI_Datatype * arg2,
//               int * arg3,
//               MPI_Aint * arg4,
//               int * arg5,
//               MPI_Datatype * arg6,
//               MPI_Win * arg7,
//               int * arg8) {
//
//
//     static void (*orig_mpi_put_)(void *,
//                                  int *,
//                                  MPI_Datatype *,
//                                  int *,
//                                  MPI_Aint *,
//                                  int *,
//                                  MPI_Datatype *,
//                                  MPI_Win *,
//                                  int *) = 0;
//     if (orig_mpi_put_ == 0) {
//         orig_mpi_put_ = (void (*)(void *,
//                                   int *,
//                                   MPI_Datatype *,
//                                   int *,
//                                   MPI_Aint *,
//                                   int *,
//                                   MPI_Datatype *,
//                                   MPI_Win *,
//                                   int *))dlsym(RTLD_NEXT, "mpi_put_");
//     }
//
//     orig_mpi_put_(arg0, arg1, arg2, arg3, arg4, arg5, arg6, arg7, arg8);
// }
// void mpi_bcast_(void * arg0,
//                 int * arg1,
//                 MPI_Datatype * arg2,
//                 int * arg3,
//                 MPI_Comm * arg4,
//                 int * arg5) {
//
//
//     static void (*orig_mpi_bcast_)(void *, int *, MPI_Datatype *, int *, MPI_Comm *, int *) = 0;
//     if (orig_mpi_bcast_ == 0) {
//         orig_mpi_bcast_ = (void (*)(void *, int *, MPI_Datatype *, int *, MPI_Comm *, int
//         *))dlsym(
//                 RTLD_NEXT,
//                 "mpi_bcast_");
//     }
//
//     orig_mpi_bcast_(arg0, arg1, arg2, arg3, arg4, arg5);
// }
// void mpi_gather_(void * arg0,
//                  int * arg1,
//                  MPI_Datatype * arg2,
//                  void * arg3,
//                  int * arg4,
//                  MPI_Datatype * arg5,
//                  int * arg6,
//                  MPI_Comm * arg7,
//                  int * arg8) {
//
//
//     static void (*orig_mpi_gather_)(void *,
//                                     int *,
//                                     MPI_Datatype *,
//                                     void *,
//                                     int *,
//                                     MPI_Datatype *,
//                                     int *,
//                                     MPI_Comm *,
//                                     int *) = 0;
//     if (orig_mpi_gather_ == 0) {
//         orig_mpi_gather_ = (void (*)(void *,
//                                      int *,
//                                      MPI_Datatype *,
//                                      void *,
//                                      int *,
//                                      MPI_Datatype *,
//                                      int *,
//                                      MPI_Comm *,
//                                      int *))dlsym(RTLD_NEXT, "mpi_gather_");
//     }
//
//     orig_mpi_gather_(arg0, arg1, arg2, arg3, arg4, arg5, arg6, arg7, arg8);
// }
// void mpi_gatherv_(void * arg0,
//                   int * arg1,
//                   MPI_Datatype * arg2,
//                   void * arg3,
//                   int * arg4,
//                   int * arg5,
//                   MPI_Datatype * arg6,
//                   int * arg7,
//                   MPI_Comm * arg8) {
//
//
//     static void (*orig_mpi_gatherv_)(void *,
//                                      int *,
//                                      MPI_Datatype *,
//                                      void *,
//                                      int *,
//                                      int *,
//                                      MPI_Datatype *,
//                                      int *,
//                                      MPI_Comm *) = 0;
//     if (orig_mpi_gatherv_ == 0) {
//         orig_mpi_gatherv_ = (void (*)(void *,
//                                       int *,
//                                       MPI_Datatype *,
//                                       void *,
//                                       int *,
//                                       int *,
//                                       MPI_Datatype *,
//                                       int *,
//                                       MPI_Comm *))dlsym(RTLD_NEXT, "mpi_gatherv_");
//     }
//
//     orig_mpi_gatherv_(arg0, arg1, arg2, arg3, arg4, arg5, arg6, arg7, arg8);
// }
// void mpi_scatter_(void * arg0,
//                   int * arg1,
//                   MPI_Datatype * arg2,
//                   void * arg3,
//                   int * arg4,
//                   MPI_Datatype * arg5,
//                   int * arg6,
//                   MPI_Comm * arg7,
//                   int * arg8) {
//
//
//     static void (*orig_mpi_scatter_)(void *,
//                                      int *,
//                                      MPI_Datatype *,
//                                      void *,
//                                      int *,
//                                      MPI_Datatype *,
//                                      int *,
//                                      MPI_Comm *,
//                                      int *) = 0;
//     if (orig_mpi_scatter_ == 0) {
//         orig_mpi_scatter_ = (void (*)(void *,
//                                       int *,
//                                       MPI_Datatype *,
//                                       void *,
//                                       int *,
//                                       MPI_Datatype *,
//                                       int *,
//                                       MPI_Comm *,
//                                       int *))dlsym(RTLD_NEXT, "mpi_scatter_");
//     }
//
//     orig_mpi_scatter_(arg0, arg1, arg2, arg3, arg4, arg5, arg6, arg7, arg8);
// }
// void mpi_scatterv_(void * arg0,
//                    int * arg1,
//                    int * arg2,
//                    MPI_Datatype * arg3,
//                    void * arg4,
//                    int * arg5,
//                    MPI_Datatype * arg6,
//                    int * arg7,
//                    MPI_Comm * arg8,
//                    int * arg9) {
//
//
//     static void (*orig_mpi_scatterv_)(void *,
//                                       int *,
//                                       int *,
//                                       MPI_Datatype *,
//                                       void *,
//                                       int *,
//                                       MPI_Datatype *,
//                                       int *,
//                                       MPI_Comm *,
//                                       int *) = 0;
//     if (orig_mpi_scatterv_ == 0) {
//         orig_mpi_scatterv_ = (void (*)(void *,
//                                        int *,
//                                        int *,
//                                        MPI_Datatype *,
//                                        void *,
//                                        int *,
//                                        MPI_Datatype *,
//                                        int *,
//                                        MPI_Comm *,
//                                        int *))dlsym(RTLD_NEXT, "mpi_scatterv_");
//     }
//
//     orig_mpi_scatterv_(arg0, arg1, arg2, arg3, arg4, arg5, arg6, arg7, arg8, arg9);
// }
// void mpi_allgather_(void * arg0,
//                     int * arg1,
//                     MPI_Datatype * arg2,
//                     void * arg3,
//                     int * arg4,
//                     MPI_Datatype * arg5,
//                     MPI_Comm * arg6,
//                     int * arg7) {
//
//
//     static void (*orig_mpi_allgather_)(void *,
//                                        int *,
//                                        MPI_Datatype *,
//                                        void *,
//                                        int *,
//                                        MPI_Datatype *,
//                                        MPI_Comm *,
//                                        int *) = 0;
//     if (orig_mpi_allgather_ == 0) {
//         orig_mpi_allgather_ = (void (*)(void *,
//                                         int *,
//                                         MPI_Datatype *,
//                                         void *,
//                                         int *,
//                                         MPI_Datatype *,
//                                         MPI_Comm *,
//                                         int *))dlsym(RTLD_NEXT, "mpi_allgather_");
//     }
//
//     orig_mpi_allgather_(arg0, arg1, arg2, arg3, arg4, arg5, arg6, arg7);
// }
// void mpi_allgatherv_(void * arg0,
//                      int * arg1,
//                      MPI_Datatype * arg2,
//                      void * arg3,
//                      int * arg4,
//                      int * arg5,
//                      MPI_Datatype * arg6,
//                      MPI_Comm * arg7) {
//
//
//     static void (*orig_mpi_allgatherv_)(void *,
//                                         int *,
//                                         MPI_Datatype *,
//                                         void *,
//                                         int *,
//                                         int *,
//                                         MPI_Datatype *,
//                                         MPI_Comm *) = 0;
//     if (orig_mpi_allgatherv_ == 0) {
//         orig_mpi_allgatherv_ = (void (*)(void *,
//                                          int *,
//                                          MPI_Datatype *,
//                                          void *,
//                                          int *,
//                                          int *,
//                                          MPI_Datatype *,
//                                          MPI_Comm *))dlsym(RTLD_NEXT, "mpi_allgatherv_");
//     }
//
//     orig_mpi_allgatherv_(arg0, arg1, arg2, arg3, arg4, arg5, arg6, arg7);
// }
// void mpi_alltoall_(void * arg0,
//                    int * arg1,
//                    MPI_Datatype * arg2,
//                    void * arg3,
//                    int * arg4,
//                    MPI_Datatype * arg5,
//                    MPI_Comm * arg6,
//                    int * arg7) {
//
//
//     static void (*orig_mpi_alltoall_)(void *,
//                                       int *,
//                                       MPI_Datatype *,
//                                       void *,
//                                       int *,
//                                       MPI_Datatype *,
//                                       MPI_Comm *,
//                                       int *) = 0;
//     if (orig_mpi_alltoall_ == 0) {
//         orig_mpi_alltoall_ = (void (*)(void *,
//                                        int *,
//                                        MPI_Datatype *,
//                                        void *,
//                                        int *,
//                                        MPI_Datatype *,
//                                        MPI_Comm *,
//                                        int *))dlsym(RTLD_NEXT, "mpi_alltoall_");
//     }
//
//     orig_mpi_alltoall_(arg0, arg1, arg2, arg3, arg4, arg5, arg6, arg7);
// }
// void mpi_alltoallv_(void * arg0,
//                     int * arg1,
//                     int * arg2,
//                     MPI_Datatype * arg3,
//                     void * arg4,
//                     int * arg5,
//                     int * arg6,
//                     MPI_Datatype * arg7,
//                     MPI_Comm * arg8,
//                     int * arg9) {
//
//
//     static void (*orig_mpi_alltoallv_)(void *,
//                                        int *,
//                                        int *,
//                                        MPI_Datatype *,
//                                        void *,
//                                        int *,
//                                        int *,
//                                        MPI_Datatype *,
//                                        MPI_Comm *,
//                                        int *) = 0;
//     if (orig_mpi_alltoallv_ == 0) {
//         orig_mpi_alltoallv_ = (void (*)(void *,
//                                         int *,
//                                         int *,
//                                         MPI_Datatype *,
//                                         void *,
//                                         int *,
//                                         int *,
//                                         MPI_Datatype *,
//                                         MPI_Comm *,
//                                         int *))dlsym(RTLD_NEXT, "mpi_alltoallv_");
//     }
//
//     orig_mpi_alltoallv_(arg0, arg1, arg2, arg3, arg4, arg5, arg6, arg7, arg8, arg9);
// }
// void mpi_reduce_(void * arg0,
//                  void * arg1,
//                  int * arg2,
//                  MPI_Datatype * arg3,
//                  MPI_Op * arg4,
//                  int * arg5,
//                  MPI_Comm * arg6,
//                  int * arg7) {
//
//
//     static void (*orig_mpi_reduce_)(void *,
//                                     void *,
//                                     int *,
//                                     MPI_Datatype *,
//                                     MPI_Op *,
//                                     int *,
//                                     MPI_Comm *,
//                                     int *) = 0;
//     if (orig_mpi_reduce_ == 0) {
//         orig_mpi_reduce_ =
//                 (void (*)(void *, void *, int *, MPI_Datatype *, MPI_Op *, int *, MPI_Comm *, int
//                 *))
//                         dlsym(RTLD_NEXT, "mpi_reduce_");
//     }
//
//     orig_mpi_reduce_(arg0, arg1, arg2, arg3, arg4, arg5, arg6, arg7);
// }
// void mpi_allreduce_(void * arg0,
//                     void * arg1,
//                     int * arg2,
//                     MPI_Datatype * arg3,
//                     MPI_Op * arg4,
//                     MPI_Comm * arg5,
//                     int * arg6) {
//
//
//     static void (*orig_mpi_allreduce_)(void *,
//                                        void *,
//                                        int *,
//                                        MPI_Datatype *,
//                                        MPI_Op *,
//                                        MPI_Comm *,
//                                        int *) = 0;
//     if (orig_mpi_allreduce_ == 0) {
//         orig_mpi_allreduce_ =
//                 (void (*)(void *, void *, int *, MPI_Datatype *, MPI_Op *, MPI_Comm *, int
//                 *))dlsym(
//                         RTLD_NEXT,
//                         "mpi_allreduce_");
//     }
//
//     orig_mpi_allreduce_(arg0, arg1, arg2, arg3, arg4, arg5, arg6);
// }
// void mpi_reduce_scatter_(void * arg0,
//                          void * arg1,
//                          int * arg2,
//                          MPI_Datatype * arg3,
//                          MPI_Op * arg4,
//                          MPI_Comm * arg5,
//                          int * arg6) {
//
//
//     static void (*orig_mpi_reduce_scatter_)(void *,
//                                             void *,
//                                             int *,
//                                             MPI_Datatype *,
//                                             MPI_Op *,
//                                             MPI_Comm *,
//                                             int *) = 0;
//     if (orig_mpi_reduce_scatter_ == 0) {
//         orig_mpi_reduce_scatter_ =
//                 (void (*)(void *, void *, int *, MPI_Datatype *, MPI_Op *, MPI_Comm *, int
//                 *))dlsym(
//                         RTLD_NEXT,
//                         "mpi_reduce_scatter_");
//     }
//
//     orig_mpi_reduce_scatter_(arg0, arg1, arg2, arg3, arg4, arg5, arg6);
// }
// void mpi_scan_(void * arg0,
//                void * arg1,
//                int * arg2,
//                MPI_Datatype * arg3,
//                MPI_Op * arg4,
//                MPI_Comm * arg5,
//                int * arg6) {
//
//
//     static void (
//             *orig_mpi_scan_)(void *, void *, int *, MPI_Datatype *, MPI_Op *, MPI_Comm *, int *)
//             = 0;
//     if (orig_mpi_scan_ == 0) {
//         orig_mpi_scan_ =
//                 (void (*)(void *, void *, int *, MPI_Datatype *, MPI_Op *, MPI_Comm *, int
//                 *))dlsym(
//                         RTLD_NEXT,
//                         "mpi_scan_");
//     }
//
//     orig_mpi_scan_(arg0, arg1, arg2, arg3, arg4, arg5, arg6);
// }
// void mpi_ibarrier_(MPI_Fint * arg0, MPI_Fint * arg1, int * arg2) {
//
//
//     static void (*orig_mpi_ibarrier_)(MPI_Fint *, MPI_Fint *, int *) = 0;
//     if (orig_mpi_ibarrier_ == 0) {
//         orig_mpi_ibarrier_ =
//                 (void (*)(MPI_Fint *, MPI_Fint *, int *))dlsym(RTLD_NEXT, "mpi_ibarrier_");
//     }
//
//     orig_mpi_ibarrier_(arg0, arg1, arg2);
// }
// void mpi_ibcast_(void * arg0,
//                  int * arg1,
//                  MPI_Fint * arg2,
//                  int * arg3,
//                  MPI_Fint * arg4,
//                  MPI_Fint * arg5,
//                  int * arg6) {
//
//
//     static void (
//             *orig_mpi_ibcast_)(void *, int *, MPI_Fint *, int *, MPI_Fint *, MPI_Fint *, int *) =
//             0;
//     if (orig_mpi_ibcast_ == 0) {
//         orig_mpi_ibcast_ =
//                 (void (*)(void *, int *, MPI_Fint *, int *, MPI_Fint *, MPI_Fint *, int *))dlsym(
//                         RTLD_NEXT,
//                         "mpi_ibcast_");
//     }
//
//     orig_mpi_ibcast_(arg0, arg1, arg2, arg3, arg4, arg5, arg6);
// }
// void mpi_igather_(const void * arg0,
//                   int * arg1,
//                   MPI_Fint * arg2,
//                   void * arg3,
//                   int * arg4,
//                   MPI_Fint * arg5,
//                   int * arg6,
//                   MPI_Fint * arg7,
//                   MPI_Fint * arg8,
//                   int * arg9) {
//
//
//     static void (*orig_mpi_igather_)(const void *,
//                                      int *,
//                                      MPI_Fint *,
//                                      void *,
//                                      int *,
//                                      MPI_Fint *,
//                                      int *,
//                                      MPI_Fint *,
//                                      MPI_Fint *,
//                                      int *) = 0;
//     if (orig_mpi_igather_ == 0) {
//         orig_mpi_igather_ = (void (*)(const void *,
//                                       int *,
//                                       MPI_Fint *,
//                                       void *,
//                                       int *,
//                                       MPI_Fint *,
//                                       int *,
//                                       MPI_Fint *,
//                                       MPI_Fint *,
//                                       int *))dlsym(RTLD_NEXT, "mpi_igather_");
//     }
//
//     orig_mpi_igather_(arg0, arg1, arg2, arg3, arg4, arg5, arg6, arg7, arg8, arg9);
// }
// void mpi_igatherv_(const void * arg0,
//                    int * arg1,
//                    MPI_Fint * arg2,
//                    void * arg3,
//                    const int * arg4,
//                    const int * arg5,
//                    MPI_Fint * arg6,
//                    int * arg7,
//                    MPI_Fint * arg8,
//                    MPI_Fint * arg9,
//                    int * arg10) {
//
//
//     static void (*orig_mpi_igatherv_)(const void *,
//                                       int *,
//                                       MPI_Fint *,
//                                       void *,
//                                       const int *,
//                                       const int *,
//                                       MPI_Fint *,
//                                       int *,
//                                       MPI_Fint *,
//                                       MPI_Fint *,
//                                       int *) = 0;
//     if (orig_mpi_igatherv_ == 0) {
//         orig_mpi_igatherv_ = (void (*)(const void *,
//                                        int *,
//                                        MPI_Fint *,
//                                        void *,
//                                        const int *,
//                                        const int *,
//                                        MPI_Fint *,
//                                        int *,
//                                        MPI_Fint *,
//                                        MPI_Fint *,
//                                        int *))dlsym(RTLD_NEXT, "mpi_igatherv_");
//     }
//
//     orig_mpi_igatherv_(arg0, arg1, arg2, arg3, arg4, arg5, arg6, arg7, arg8, arg9, arg10);
// }
// void mpi_iscatter_(const void * arg0,
//                    int * arg1,
//                    MPI_Fint * arg2,
//                    void * arg3,
//                    int * arg4,
//                    MPI_Fint * arg5,
//                    int * arg6,
//                    MPI_Fint * arg7,
//                    MPI_Fint * arg8,
//                    int * arg9) {
//
//
//     static void (*orig_mpi_iscatter_)(const void *,
//                                       int *,
//                                       MPI_Fint *,
//                                       void *,
//                                       int *,
//                                       MPI_Fint *,
//                                       int *,
//                                       MPI_Fint *,
//                                       MPI_Fint *,
//                                       int *) = 0;
//     if (orig_mpi_iscatter_ == 0) {
//         orig_mpi_iscatter_ = (void (*)(const void *,
//                                        int *,
//                                        MPI_Fint *,
//                                        void *,
//                                        int *,
//                                        MPI_Fint *,
//                                        int *,
//                                        MPI_Fint *,
//                                        MPI_Fint *,
//                                        int *))dlsym(RTLD_NEXT, "mpi_iscatter_");
//     }
//
//     orig_mpi_iscatter_(arg0, arg1, arg2, arg3, arg4, arg5, arg6, arg7, arg8, arg9);
// }
// void mpi_iscatterv_(const void * arg0,
//                     const int * arg1,
//                     const int * arg2,
//                     MPI_Fint * arg3,
//                     void * arg4,
//                     int * arg5,
//                     MPI_Fint * arg6,
//                     int * arg7,
//                     MPI_Fint * arg8,
//                     MPI_Fint * arg9,
//                     int * arg10) {
//
//
//     static void (*orig_mpi_iscatterv_)(const void *,
//                                        const int *,
//                                        const int *,
//                                        MPI_Fint *,
//                                        void *,
//                                        int *,
//                                        MPI_Fint *,
//                                        int *,
//                                        MPI_Fint *,
//                                        MPI_Fint *,
//                                        int *) = 0;
//     if (orig_mpi_iscatterv_ == 0) {
//         orig_mpi_iscatterv_ = (void (*)(const void *,
//                                         const int *,
//                                         const int *,
//                                         MPI_Fint *,
//                                         void *,
//                                         int *,
//                                         MPI_Fint *,
//                                         int *,
//                                         MPI_Fint *,
//                                         MPI_Fint *,
//                                         int *))dlsym(RTLD_NEXT, "mpi_iscatterv_");
//     }
//
//     orig_mpi_iscatterv_(arg0, arg1, arg2, arg3, arg4, arg5, arg6, arg7, arg8, arg9, arg10);
// }
// void mpi_iallgather_(const void * arg0,
//                      int * arg1,
//                      MPI_Fint * arg2,
//                      void * arg3,
//                      int * arg4,
//                      MPI_Fint * arg5,
//                      MPI_Fint * arg6,
//                      MPI_Fint * arg7,
//                      int * arg8) {
//
//
//     static void (*orig_mpi_iallgather_)(const void *,
//                                         int *,
//                                         MPI_Fint *,
//                                         void *,
//                                         int *,
//                                         MPI_Fint *,
//                                         MPI_Fint *,
//                                         MPI_Fint *,
//                                         int *) = 0;
//     if (orig_mpi_iallgather_ == 0) {
//         orig_mpi_iallgather_ = (void (*)(const void *,
//                                          int *,
//                                          MPI_Fint *,
//                                          void *,
//                                          int *,
//                                          MPI_Fint *,
//                                          MPI_Fint *,
//                                          MPI_Fint *,
//                                          int *))dlsym(RTLD_NEXT, "mpi_iallgather_");
//     }
//
//     orig_mpi_iallgather_(arg0, arg1, arg2, arg3, arg4, arg5, arg6, arg7, arg8);
// }
// void mpi_iallgatherv_(const void * arg0,
//                       int * arg1,
//                       MPI_Fint * arg2,
//                       void * arg3,
//                       const int * arg4,
//                       const int * arg5,
//                       MPI_Fint * arg6,
//                       MPI_Fint * arg7,
//                       MPI_Fint * arg8,
//                       int * arg9) {
//
//
//     static void (*orig_mpi_iallgatherv_)(const void *,
//                                          int *,
//                                          MPI_Fint *,
//                                          void *,
//                                          const int *,
//                                          const int *,
//                                          MPI_Fint *,
//                                          MPI_Fint *,
//                                          MPI_Fint *,
//                                          int *) = 0;
//     if (orig_mpi_iallgatherv_ == 0) {
//         orig_mpi_iallgatherv_ = (void (*)(const void *,
//                                           int *,
//                                           MPI_Fint *,
//                                           void *,
//                                           const int *,
//                                           const int *,
//                                           MPI_Fint *,
//                                           MPI_Fint *,
//                                           MPI_Fint *,
//                                           int *))dlsym(RTLD_NEXT, "mpi_iallgatherv_");
//     }
//
//     orig_mpi_iallgatherv_(arg0, arg1, arg2, arg3, arg4, arg5, arg6, arg7, arg8, arg9);
// }
// void mpi_ialltoall_(const void * arg0,
//                     int * arg1,
//                     MPI_Fint * arg2,
//                     void * arg3,
//                     int * arg4,
//                     MPI_Fint * arg5,
//                     MPI_Fint * arg6,
//                     MPI_Fint * arg7,
//                     int * arg8) {
//
//
//     static void (*orig_mpi_ialltoall_)(const void *,
//                                        int *,
//                                        MPI_Fint *,
//                                        void *,
//                                        int *,
//                                        MPI_Fint *,
//                                        MPI_Fint *,
//                                        MPI_Fint *,
//                                        int *) = 0;
//     if (orig_mpi_ialltoall_ == 0) {
//         orig_mpi_ialltoall_ = (void (*)(const void *,
//                                         int *,
//                                         MPI_Fint *,
//                                         void *,
//                                         int *,
//                                         MPI_Fint *,
//                                         MPI_Fint *,
//                                         MPI_Fint *,
//                                         int *))dlsym(RTLD_NEXT, "mpi_ialltoall_");
//     }
//
//     orig_mpi_ialltoall_(arg0, arg1, arg2, arg3, arg4, arg5, arg6, arg7, arg8);
// }
// void mpi_ialltoallv_(const void * arg0,
//                      const int * arg1,
//                      const int * arg2,
//                      MPI_Fint * arg3,
//                      void * arg4,
//                      const int * arg5,
//                      const int * arg6,
//                      MPI_Fint * arg7,
//                      MPI_Fint * arg8,
//                      MPI_Fint * arg9,
//                      int * arg10) {
//
//
//     static void (*orig_mpi_ialltoallv_)(const void *,
//                                         const int *,
//                                         const int *,
//                                         MPI_Fint *,
//                                         void *,
//                                         const int *,
//                                         const int *,
//                                         MPI_Fint *,
//                                         MPI_Fint *,
//                                         MPI_Fint *,
//                                         int *) = 0;
//     if (orig_mpi_ialltoallv_ == 0) {
//         orig_mpi_ialltoallv_ = (void (*)(const void *,
//                                          const int *,
//                                          const int *,
//                                          MPI_Fint *,
//                                          void *,
//                                          const int *,
//                                          const int *,
//                                          MPI_Fint *,
//                                          MPI_Fint *,
//                                          MPI_Fint *,
//                                          int *))dlsym(RTLD_NEXT, "mpi_ialltoallv_");
//     }
//
//     orig_mpi_ialltoallv_(arg0, arg1, arg2, arg3, arg4, arg5, arg6, arg7, arg8, arg9, arg10);
// }
// void mpi_ireduce_(const void * arg0,
//                   void * arg1,
//                   int * arg2,
//                   MPI_Fint * arg3,
//                   MPI_Op arg4,
//                   int * arg5,
//                   MPI_Fint * arg6,
//                   MPI_Fint * arg7,
//                   int * arg8) {
//
//
//     static void (*orig_mpi_ireduce_)(const void *,
//                                      void *,
//                                      int *,
//                                      MPI_Fint *,
//                                      MPI_Op,
//                                      int *,
//                                      MPI_Fint *,
//                                      MPI_Fint *,
//                                      int *) = 0;
//     if (orig_mpi_ireduce_ == 0) {
//         orig_mpi_ireduce_ = (void (*)(const void *,
//                                       void *,
//                                       int *,
//                                       MPI_Fint *,
//                                       MPI_Op,
//                                       int *,
//                                       MPI_Fint *,
//                                       MPI_Fint *,
//                                       int *))dlsym(RTLD_NEXT, "mpi_ireduce_");
//     }
//
//     orig_mpi_ireduce_(arg0, arg1, arg2, arg3, arg4, arg5, arg6, arg7, arg8);
// }
// void mpi_iallreduce_(const void * arg0,
//                      void * arg1,
//                      int * arg2,
//                      MPI_Fint * arg3,
//                      MPI_Op arg4,
//                      MPI_Fint * arg5,
//                      MPI_Fint * arg6,
//                      int * arg7) {
//
//
//     static void (*orig_mpi_iallreduce_)(const void *,
//                                         void *,
//                                         int *,
//                                         MPI_Fint *,
//                                         MPI_Op,
//                                         MPI_Fint *,
//                                         MPI_Fint *,
//                                         int *) = 0;
//     if (orig_mpi_iallreduce_ == 0) {
//         orig_mpi_iallreduce_ = (void (*)(const void *,
//                                          void *,
//                                          int *,
//                                          MPI_Fint *,
//                                          MPI_Op,
//                                          MPI_Fint *,
//                                          MPI_Fint *,
//                                          int *))dlsym(RTLD_NEXT, "mpi_iallreduce_");
//     }
//
//     orig_mpi_iallreduce_(arg0, arg1, arg2, arg3, arg4, arg5, arg6, arg7);
// }
// void mpi_ireduce_scatter_(const void * arg0,
//                           void * arg1,
//                           const int * arg2,
//                           MPI_Fint * arg3,
//                           MPI_Op arg4,
//                           MPI_Fint * arg5,
//                           MPI_Fint * arg6,
//                           int * arg7) {
//
//
//     static void (*orig_mpi_ireduce_scatter_)(const void *,
//                                              void *,
//                                              const int *,
//                                              MPI_Fint *,
//                                              MPI_Op,
//                                              MPI_Fint *,
//                                              MPI_Fint *,
//                                              int *) = 0;
//     if (orig_mpi_ireduce_scatter_ == 0) {
//         orig_mpi_ireduce_scatter_ = (void (*)(const void *,
//                                               void *,
//                                               const int *,
//                                               MPI_Fint *,
//                                               MPI_Op,
//                                               MPI_Fint *,
//                                               MPI_Fint *,
//                                               int *))dlsym(RTLD_NEXT, "mpi_ireduce_scatter_");
//     }
//
//     orig_mpi_ireduce_scatter_(arg0, arg1, arg2, arg3, arg4, arg5, arg6, arg7);
// }
// void mpi_iscan_(const void * arg0,
//                 void * arg1,
//                 int * arg2,
//                 MPI_Fint * arg3,
//                 MPI_Op arg4,
//                 MPI_Fint * arg5,
//                 MPI_Fint * arg6,
//                 int * arg7) {
//
//
//     static void (*orig_mpi_iscan_)(const void *,
//                                    void *,
//                                    int *,
//                                    MPI_Fint *,
//                                    MPI_Op,
//                                    MPI_Fint *,
//                                    MPI_Fint *,
//                                    int *) = 0;
//     if (orig_mpi_iscan_ == 0) {
//         orig_mpi_iscan_ = (void (*)(const void *,
//                                     void *,
//                                     int *,
//                                     MPI_Fint *,
//                                     MPI_Op,
//                                     MPI_Fint *,
//                                     MPI_Fint *,
//                                     int *))dlsym(RTLD_NEXT, "mpi_iscan_");
//     }
//
//     orig_mpi_iscan_(arg0, arg1, arg2, arg3, arg4, arg5, arg6, arg7);
// }
// void mpi_comm_spawn_(char * command,
//                      char ** argv,
//                      int * maxprocs,
//                      MPI_Info * info,
//                      int * root,
//                      MPI_Comm * comm,
//                      MPI_Comm * intercomm,
//                      int * array_of_errcodes,
//                      int * error) {
//
//
//     static void (*orig_mpi_comm_spawn_)(char *,
//                                         char **,
//                                         int *,
//                                         MPI_Info *,
//                                         int *,
//                                         MPI_Comm *,
//                                         MPI_Comm *,
//                                         int *,
//                                         int *) = 0;
//     if (orig_mpi_comm_spawn_ == 0) {
//         orig_mpi_comm_spawn_ = (void (*)(char *,
//                                          char **,
//                                          int *,
//                                          MPI_Info *,
//                                          int *,
//                                          MPI_Comm *,
//                                          MPI_Comm *,
//                                          int *,
//                                          int *))dlsym(RTLD_NEXT, "mpi_comm_spawn_");
//     }
//
//     orig_mpi_comm_spawn_(command,
//                          argv,
//                          maxprocs,
//                          info,
//                          root,
//                          comm,
//                          intercomm,
//                          array_of_errcodes,
//                          error);
// }
// void mpi_send_init_(void * arg0,
//                     int * arg1,
//                     MPI_Datatype * arg2,
//                     int * arg3,
//                     int * arg4,
//                     MPI_Comm * arg5,
//                     MPI_Request * arg6,
//                     int * arg7) {
//
//
//     static void (*orig_mpi_send_init_)(void *,
//                                        int *,
//                                        MPI_Datatype *,
//                                        int *,
//                                        int *,
//                                        MPI_Comm *,
//                                        MPI_Request *,
//                                        int *) = 0;
//     if (orig_mpi_send_init_ == 0) {
//         orig_mpi_send_init_ = (void (*)(void *,
//                                         int *,
//                                         MPI_Datatype *,
//                                         int *,
//                                         int *,
//                                         MPI_Comm *,
//                                         MPI_Request *,
//                                         int *))dlsym(RTLD_NEXT, "mpi_send_init_");
//     }
//
//     orig_mpi_send_init_(arg0, arg1, arg2, arg3, arg4, arg5, arg6, arg7);
// }
// void mpi_bsend_init_(void * arg0,
//                      int * arg1,
//                      MPI_Datatype * arg2,
//                      int * arg3,
//                      int * arg4,
//                      MPI_Comm * arg5,
//                      MPI_Request * arg6,
//                      int * arg7) {
//
//
//     static void (*orig_mpi_bsend_init_)(void *,
//                                         int *,
//                                         MPI_Datatype *,
//                                         int *,
//                                         int *,
//                                         MPI_Comm *,
//                                         MPI_Request *,
//                                         int *) = 0;
//     if (orig_mpi_bsend_init_ == 0) {
//         orig_mpi_bsend_init_ = (void (*)(void *,
//                                          int *,
//                                          MPI_Datatype *,
//                                          int *,
//                                          int *,
//                                          MPI_Comm *,
//                                          MPI_Request *,
//                                          int *))dlsym(RTLD_NEXT, "mpi_bsend_init_");
//     }
//
//     orig_mpi_bsend_init_(arg0, arg1, arg2, arg3, arg4, arg5, arg6, arg7);
// }
// void mpi_ssend_init_(void * arg0,
//                      int * arg1,
//                      MPI_Datatype * arg2,
//                      int * arg3,
//                      int * arg4,
//                      MPI_Comm * arg5,
//                      MPI_Request * arg6,
//                      int * arg7) {
//
//
//     static void (*orig_mpi_ssend_init_)(void *,
//                                         int *,
//                                         MPI_Datatype *,
//                                         int *,
//                                         int *,
//                                         MPI_Comm *,
//                                         MPI_Request *,
//                                         int *) = 0;
//     if (orig_mpi_ssend_init_ == 0) {
//         orig_mpi_ssend_init_ = (void (*)(void *,
//                                          int *,
//                                          MPI_Datatype *,
//                                          int *,
//                                          int *,
//                                          MPI_Comm *,
//                                          MPI_Request *,
//                                          int *))dlsym(RTLD_NEXT, "mpi_ssend_init_");
//     }
//
//     orig_mpi_ssend_init_(arg0, arg1, arg2, arg3, arg4, arg5, arg6, arg7);
// }
// void mpi_rsend_init_(void * arg0,
//                      int * arg1,
//                      MPI_Datatype * arg2,
//                      int * arg3,
//                      int * arg4,
//                      MPI_Comm * arg5,
//                      MPI_Request * arg6,
//                      int * arg7) {
//
//
//     static void (*orig_mpi_rsend_init_)(void *,
//                                         int *,
//                                         MPI_Datatype *,
//                                         int *,
//                                         int *,
//                                         MPI_Comm *,
//                                         MPI_Request *,
//                                         int *) = 0;
//     if (orig_mpi_rsend_init_ == 0) {
//         orig_mpi_rsend_init_ = (void (*)(void *,
//                                          int *,
//                                          MPI_Datatype *,
//                                          int *,
//                                          int *,
//                                          MPI_Comm *,
//                                          MPI_Request *,
//                                          int *))dlsym(RTLD_NEXT, "mpi_rsend_init_");
//     }
//
//     orig_mpi_rsend_init_(arg0, arg1, arg2, arg3, arg4, arg5, arg6, arg7);
// }
// void mpi_recv_init_(void * arg0,
//                     int * arg1,
//                     MPI_Datatype * arg2,
//                     int * arg3,
//                     int * arg4,
//                     MPI_Comm * arg5,
//                     MPI_Request * arg6,
//                     int * arg7) {
//
//
//     static void (*orig_mpi_recv_init_)(void *,
//                                        int *,
//                                        MPI_Datatype *,
//                                        int *,
//                                        int *,
//                                        MPI_Comm *,
//                                        MPI_Request *,
//                                        int *) = 0;
//     if (orig_mpi_recv_init_ == 0) {
//         orig_mpi_recv_init_ = (void (*)(void *,
//                                         int *,
//                                         MPI_Datatype *,
//                                         int *,
//                                         int *,
//                                         MPI_Comm *,
//                                         MPI_Request *,
//                                         int *))dlsym(RTLD_NEXT, "mpi_recv_init_");
//     }
//
//     orig_mpi_recv_init_(arg0, arg1, arg2, arg3, arg4, arg5, arg6, arg7);
// }
// void mpi_start_(MPI_Request * arg0, int * arg1) {
//
//
//     static void (*orig_mpi_start_)(MPI_Request *, int *) = 0;
//     if (orig_mpi_start_ == 0) {
//         orig_mpi_start_ = (void (*)(MPI_Request *, int *))dlsym(RTLD_NEXT, "mpi_start_");
//     }
//
//     orig_mpi_start_(arg0, arg1);
// }
// void mpi_startall_(int * arg0, MPI_Request * arg1, int * arg2) {
//
//
//     static void (*orig_mpi_startall_)(int *, MPI_Request *, int *) = 0;
//     if (orig_mpi_startall_ == 0) {
//         orig_mpi_startall_ =
//                 (void (*)(int *, MPI_Request *, int *))dlsym(RTLD_NEXT, "mpi_startall_");
//     }
//
//     orig_mpi_startall_(arg0, arg1, arg2);
// }
}
