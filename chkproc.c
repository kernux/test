/*
  (C) Nelson Murilo - 2004/09/13
  Version 0.10
  C port from chkproc.pl code from Klaus Steding-Jessen <jessen@nic.br>
  and Cristine Hoepers <cristine@nic.br> +little output changes.

  2002/03/02 - Segmentation fault in ps for non ASCII user name, by RainbowHat

  2002/06/13 Updated by Kostya Kortchinsky <kostya.kortchinsky@renater.fr>
  - corrected the program return value ;
  - added a verbose mode displaying information about the hidden process.

  2002/08/08 - Value of MAX_PROCESSES was increased to 99999 (new versions
    of FreeBSD, HP-UX and others), reported by Morohoshi Akihiko, Paul
    and others.

  2002/09/03 - Eliminate (?) false-positives. Original idea from Aaron Sherman.

  2002/11/15 - Updated by Kostya Kortchinsky <kostya.kortchinsky@renater.fr>
  - ported to SunOS.

  2003/01/19 - Another Adore based lkm test. Original idea from Junichi Murakami

  2003/02/02 - More little fixes - Nelson Murilo

  2003/02/23 - Use of kill to eliminate false-positives abandonated, It is
  preferable false-positives that false-negatives. Uncomment kill() functions
  if you like  it.

  2003/06/07 - Fix for NPTL threading mechanisms - patch by Mike Griego

  2003/09/01 - Fix for ps mode detect, patch by Bill Dupree and others

  2004/04/03 - More fix for linux's threads - Nelson Murilo

  2004/09/13 - More and more fix for linux's threads - Nelson Murilo

  2005/02/23 - More and more and more fix for linux's threads - Nelson Murilo

  2005/10/28 - Bug fix for FreeBSD: chkproc was sending a SIGXFSZ (kill -25)
               to init, causing a reboot.  Patch by Nelson Murilo.
               Thanks to Luiz E. R. Cordeiro.

  2005/11/15 - Add check for Enye LKM - Nelson Murilo

  2005/11/25 - Fix for long lines in PS output - patch by Lantz Moore

  2006/01/05 - Add getpriority to identify LKMs, ideas from Yjesus(unhide) and
               Slider/Flimbo (skdet)

  2006/01/11 - Fix signal 25 on parisc linux and return of kill() -
               Thanks to Lantz Moore
  2014/11/19 - Fix threads in ps and Only in Linux, delete dir check.
*/

#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <sys/types.h>
#include <dirent.h>
#include <ctype.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <sys/stat.h>
#include <sys/resource.h>

#define ENYELKM "/proc/12345"

#define FIRST_PROCESS 1
#define MAX_PROCESSES 99999
#define MAX_BUF 1024


static char *pscmd = "ps -eL |sed  's/^[[:space:]]*//' |tr -s ' '|cut -d' ' -f2-";

int psproc[MAX_PROCESSES+1];

/*
 * read at most the first (size-1) chars into s and terminate with a '\0'.
 * stops reading after a newline or EOF.  if a newline is read, it will be
 * the last char in the string.  if no newline is found in the first
 * (size-1) chars, then keep reading and discarding chars until a newline
 * is found or EOF.
 */
char* readline(char *s, int size, FILE *stream) {
    char *rv = fgets(s, size, stream);

    if (strlen(s) == (size - 1) && s[size - 1] != '\n') {
        char buf[MAX_BUF];
        fgets(buf, MAX_BUF, stream);
        while (strlen(buf) == (MAX_BUF - 1) && buf[MAX_BUF - 1] != '\n') {
            fgets(buf, MAX_BUF, stream);
        }
    }

    return rv;
}

int main(int argc, char **argv) {
    char buf[MAX_BUF], *p, path[MAX_BUF];
    FILE *ps;
    DIR *proc = opendir("/proc");
    struct stat sb;
    int i, j, retps;
    long ret = 0L;

    if (!proc) {
        perror("proc");
        exit(1);
    }
    for (i = 1; i < argc; i++) {
        if (!memcmp(argv[i], "-h", 2)) {
            printf("%s is a tool to check hiden processes by brute force /proc and ps cmd.\n", argv[0]);
            printf("Usage: %s \n", argv[0]);
            return 0;
        } 
    }

    if (!(ps = popen(pscmd, "r"))) {
        perror("ps");
        exit(errno);
    }

    *buf = 0;
    readline(buf, MAX_BUF, ps); /* Skip header */

    for (i = FIRST_PROCESS; i <= MAX_PROCESSES; i++) { /* Init matrix */
        psproc[i] = 0;
    }
   // int c = 0;
    while (readline(buf, MAX_BUF, ps)) {
     //   c++;
        p = buf;
       // printf("buf is %s \n", buf);
        while (!isblank(*p) && !isdigit(*p)) /* Skip User */
            p++;
        while (isblank(*p)) /* Skip spaces */
            p++;
       // printf(">>%s<<\n", p);  /* -- DEBUG */
        ret = atol(p);
        if (ret < 0 || ret > MAX_PROCESSES) {
            fprintf(stderr, " OooPS, not expected %ld value\n", ret);
            exit(2);
        }
        //printf("%d\n",ret);
        psproc[ret] = 1;
    }
    pclose(ps);
    //printf("ps output is %d\n", c);

    /* Brute force */
    strcpy(buf, "/proc/");
    retps = 0;
    for (i = FIRST_PROCESS; i <= MAX_PROCESSES; i++) {
        snprintf(&buf[6], 6, "%d", i);
        if (!chdir(buf) || !kill(i, 0)) {
            if (!psproc[i]) {
                retps++;
                printf("PID %5d: not in ps output\n", i);
                if((j = readlink("./cwd", path, sizeof(path))) > 0) {
                    path[(j < sizeof(path)) ? j : sizeof(path) - 1] = 0;
                    printf("CWD %5d: %s\n", i, path);
                }else{
                    printf("read current work dir false!\n");
                }
                if((j = readlink("./exe", path, sizeof(path))) > 0) {
                    path[(j < sizeof(path)) ? j : sizeof(path) - 1] = 0;
                    printf("EXE %5d: %s\n", i, path);
                }else{
                    printf("read executable file path false!\n");
                } 
                
            }
        }    
    }
    if (retps) printf("You have % 5d process hidden for ps command\n", retps);

    kill(1, 100); /*  Check for SIGINVISIBLE Adore signal */
    if (kill(1, SIGXFSZ) < 0  && errno == 3) {
        printf("SIGINVISIBLE Adore found\n");
    }
    /* Check for Enye LKM */
    if (stat(ENYELKM, &sb) && kill(12345, 58) >= 0) {
        printf("Enye LKM found\n");
    }
    return (retps);
}

