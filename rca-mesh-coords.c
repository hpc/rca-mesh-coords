/**
 * Copyright (c) 2012-2013 Los Alamos National Security, LLC.
 *                         All rights reserved.
 *
 * This program was prepared by Los Alamos National Security, LLC at Los Alamos
 * National Laboratory (LANL) under contract No. DE-AC52-06NA25396 with the U.S.
 * Department of Energy (DOE). All rights in the program are reserved by the DOE
 * and Los Alamos National Security, LLC. Permission is granted to the public to
 * copy and use this software without charge, provided that this Notice and any
 * statement of authorship are reproduced on all copies. Neither the U.S.
 * Government nor LANS makes any warranty, express or implied, or assumes any
 * liability or responsibility for the use of this software.
 */

/* simple example application that uses rca to get mesh coordinates */

/* @author samuel k. gutierrez - samuel@lanl.gov */

/* to run:
 * aprun -n <X> rca-mesh-coords
 * aprun -n <X> -N <Y> rca-mesh-coords
 */

#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>

#include "rca_lib.h"
#include "pmi.h"

enum {
    SUCCESS = 0,
    FAILURE
};

static int pmi_rank = 0;
static int pmi_process_group_size = 0;
static int spawned = PMI_FALSE;
static bool delegate = false;
static PMI_BOOL pmi_initialized = PMI_FALSE;

static int
init_pmi(void)
{
    char *err = NULL;
    int rc = SUCCESS;

    /* sanity */
    if (PMI_SUCCESS != PMI_Initialized(&pmi_initialized) ||
        PMI_TRUE == pmi_initialized) {
        fprintf(stderr, "=== ERROR: PMI sanity failure\n");
        return FAILURE;
    }
    if (PMI_SUCCESS != PMI_Init(&spawned)) {
        err = "PMI_Init failure!";
        goto done;
    }
    if (PMI_SUCCESS != PMI_Get_size(&pmi_process_group_size)) {
        err = "PMI_Get_size failure!";
        goto done;
    }
    if (PMI_SUCCESS != PMI_Get_rank(&pmi_rank)) {
        err = "PMI_Get_rank failure!";
        goto done;
    }
    if (0 == pmi_rank) {
        delegate = true;
    }

done:
    if (NULL != err) {
        fprintf(stderr, "=== ERROR [rank:%d] %s\n", pmi_rank, err);
        rc = FAILURE;
    }

    return rc;
}

static int
fini_pmi(void)
{
    if (PMI_TRUE == pmi_initialized) {
        PMI_Finalize();
    }
    return SUCCESS;
}

static int
print_all_mesh_coords(void)
{
    int i, nid;
    rca_mesh_coord_t xyz;

    for (i = 0; i < pmi_process_group_size; ++i) {
        if (PMI_SUCCESS != PMI_Get_nid(i, &nid)) {
            fprintf(stderr, "PMI_Get_nid failure\n");
            return FAILURE;
        }
        rca_get_meshcoord(nid, &xyz);
        /*                       x,  y,  z */
        printf("nid %d coords - (%d, %d, %d)\n", nid,
               xyz.mesh_x, xyz.mesh_y, xyz.mesh_z);
    }

    return SUCCESS;
}

int
main(int argc, char **argv)
{
    int rc;

    if (SUCCESS != (rc = init_pmi())) {
        fprintf(stderr, "init_pmi failure -- cannot continue...\n");
        goto out;
    }
    if (delegate) {
        if (SUCCESS != (rc = print_all_mesh_coords())) {
            fprintf(stderr, "print_all_mesh_coords failure -- "
                    "cannot continue...\n");
            goto out;
        }
    }

out:
    (void)fini_pmi();
    return (rc == SUCCESS) ? EXIT_SUCCESS : EXIT_FAILURE;
}
