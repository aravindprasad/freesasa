/*
  Copyright Simon Mitternacht 2013-2015.

  This file is part of FreeSASA.

  FreeSASA is free software: you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.

  FreeSASA is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with FreeSASA.  If not, see <http://www.gnu.org/licenses/>.
*/
#include <stdlib.h>
#include <check.h>
int n_malloc_fails = 0;
int n_realloc_fails = 0;
int malloc_fail_freq = 1;
int realloc_fail_freq = 1;

// use these to test what happens when malloc and realloc fail,
// the global variables malloc_fail_freq and realloc_fail_freq
// can be set to let 
void*
broken_malloc(size_t s)
{
    ++n_malloc_fails;
    if (n_malloc_fails % malloc_fail_freq == 0) return NULL;
    return malloc(s);
}

void*
broken_realloc(void * ptr, size_t s)
{
    ++n_realloc_fails;
    if (n_realloc_fails % realloc_fail_freq == 0) return NULL;
    return realloc(ptr,s);
}

#define malloc(m) broken_malloc(m)
#define realloc(m,n) broken_realloc(m,n)

#define NB_CHUNK 1 // to force som reallocs to take place
#include "whole_lib_one_file.c"

int int_array[6] = {0,1,2,3,4,5};
char str_array[][2] = {"A","B","C","D","E","F"};
double v[18] = {0,0,0, 1,1,1, -1,1,-1, 2,0,-2, 2,2,0, -5,5,5};
double r[6]  = {4,2,2,2,2,2};
struct freesasa_coord coord = {.xyz = v, .n = 6, .is_const = 0};
struct atom a;
struct freesasa_structure structure = {
    .a = &a,
    .xyz = &coord,
    .number_atoms = 6,
    .number_residues = 1,
    .number_chains = 1, 
    .model = 1,
    .chains = "A",
    .res_first_atom = int_array,
    .res_desc = (char**)str_array
};
struct file_interval interval = {.begin = 0, .end = 1};
struct cell a_cell = {.nb = NULL, .atom = int_array, .n_nb=0, .n_atoms = 0};
struct cell_list a_cell_list = {.cell = &a_cell, .n = 1, .nx = 1, .ny =1, .nz = 1,
                                .d = 20, .x_min = 0, .x_max = 1, 
                                .y_min = 0, .y_max = 1,
                                .z_min = 0, .z_max = 1};

START_TEST (test_coord) 
{
    freesasa_set_verbosity(FREESASA_V_SILENT);
    ck_assert_ptr_eq(freesasa_coord_new(),NULL);
    ck_assert_ptr_eq(freesasa_coord_copy(&coord),NULL);
    ck_assert_ptr_eq(freesasa_coord_new_linked(v,1),NULL);
    ck_assert_ptr_eq(freesasa_coord_append(&coord,v,1),FREESASA_FAIL); 
    ck_assert_ptr_eq(freesasa_coord_append_xyz(&coord,v,v+1,v+2,1),FREESASA_FAIL); 
    freesasa_set_verbosity(FREESASA_V_NORMAL);
}
END_TEST

START_TEST (test_structure)
{
    FILE *file = fopen(DATADIR "1ubq.pdb","r");
    int n;
    freesasa_set_verbosity(FREESASA_V_SILENT);
    ck_assert_ptr_ne(file,NULL);
    ck_assert_ptr_eq(freesasa_structure_new(),NULL);
    ck_assert_ptr_eq(from_pdb_impl(file,interval,0),NULL);
    for (int i = 1; i < 50; ++i) {
        malloc_fail_freq = i; n_malloc_fails = 0;
        realloc_fail_freq = i; n_realloc_fails = 0;
        rewind(file);
        ck_assert_ptr_eq(freesasa_structure_from_pdb(file,0),NULL);
    }
    malloc_fail_freq = 1; n_malloc_fails = 0;
    realloc_fail_freq = 1; n_realloc_fails = 0;
    fclose(file);
    
    file = fopen(DATADIR "2jo4.pdb", "r");
    ck_assert_ptr_ne(file,NULL);
    for (int i = 1; i < 50; ++i) {
        malloc_fail_freq = i; n_malloc_fails = 0;
        realloc_fail_freq = i; n_realloc_fails = 0;
        rewind(file);
        ck_assert_ptr_eq(freesasa_structure_array(file,&n,FREESASA_SEPARATE_MODELS),NULL);
        ck_assert_ptr_eq(freesasa_structure_array(file,&n,FREESASA_SEPARATE_MODELS | FREESASA_SEPARATE_CHAINS),NULL);
    }
    malloc_fail_freq = 1; n_malloc_fails = 0;
    realloc_fail_freq = 1; n_realloc_fails = 0;
    fclose(file);
    freesasa_set_verbosity(FREESASA_V_NORMAL);
}
END_TEST

START_TEST (test_nb) 
{
    ck_assert_ptr_eq(cell_list_new(1,&coord),NULL);
    ck_assert_int_eq(fill_cells(&a_cell_list,&coord),FREESASA_FAIL);
    ck_assert_ptr_eq(freesasa_nb_alloc(10),NULL);
    for (int i = 1; i < 50; ++i) {
        malloc_fail_freq = i; n_malloc_fails = 0;
        realloc_fail_freq = i; n_realloc_fails = 0;
        ck_assert_ptr_eq(freesasa_nb_alloc(2*i),NULL);
        n_malloc_fails = 0; n_realloc_fails = 0;
        ck_assert_ptr_eq(freesasa_nb_new(&coord,r),NULL);
    }
    malloc_fail_freq = 1; n_malloc_fails = 0;
    realloc_fail_freq = 1; n_realloc_fails = 0;
}
END_TEST

int main(int argc, char **argv) {
    Suite *s = suite_create("Test that null-returning malloc breaks program gracefully.");

    TCase *tc = tcase_create("Basic");
    tcase_add_test(tc,test_coord);
    tcase_add_test(tc,test_structure);
    tcase_add_test(tc,test_nb);

    suite_add_tcase(s, tc);
    SRunner *sr = srunner_create(s);
    srunner_run_all(sr,CK_VERBOSE);

    return (srunner_ntests_failed(sr) == 0) ? EXIT_SUCCESS : EXIT_FAILURE;
}
