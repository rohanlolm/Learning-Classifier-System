::Make LCS Main 
::gcc -O1 LCS_main.c LCS_helperfs.c LCS_datafs.c LCS_geneticfs.c LCS_corefs.c -o LCS -Wall -Wextra -std=c99

::Make LCS Main without Genetics
gcc -O3 -flto LCS_main.c LCS_helperfs.c LCS_datafs.c LCS_corefs.c LCS_geneticfs.c -o LCS -Wall -Wextra -std=c99



::Make LCS tester 
::gcc -O1 LCS_tester.c LCS_helperfs.c LCS_datafs.c LCS_geneticfs.c LCS_corefs.c -o LCS_tester -Wall -std=c99

::gcc -g -ggdb -O0 LCS_main.c LCS_helperfs.c LCS_datafs.c LCS_geneticfs.c -o LCS -Wall -Wextra -std=c99


::GSL Compilation 
::gcc LCS_main.c LCS_helperfs.c LCS_datafs.c LCS_geneticfs.c -o LCS -Wall -Wextra -std=c99 -I"C:\Program Files (x86)\GnuWin32\include" -L"C:\Program Files (x86)\GnuWin32\lib" -lgslcblas -lgsl -lm

::training_data.txt 15 9468 testing_data.txt 2366 -l 1 -s 1 -k 0.7 -r -w 0.3 -g -e 150
