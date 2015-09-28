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
#include <sys/stat.h>
#include <unistd.h>
#include <check.h>

extern Suite* pdb_suite();
extern Suite* classify_suite();
extern Suite* coord_suite();
extern Suite* structure_suite();
extern Suite* sasa_suite();
extern Suite* nb_suite();

int main(int argc, char **argv) {
    mkdir("./tmp/",S_IRWXU);

    // Suites added in order of complexity
    SRunner *sr = srunner_create(pdb_suite());
    srunner_add_suite(sr,classify_suite());
    srunner_add_suite(sr,coord_suite());
    srunner_add_suite(sr,structure_suite());
    srunner_add_suite(sr,sasa_suite());
    srunner_add_suite(sr,nb_suite());

    srunner_run_all(sr,CK_VERBOSE);

    return (srunner_ntests_failed(sr) == 0) ? EXIT_SUCCESS : EXIT_FAILURE;
}