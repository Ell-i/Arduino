/* Temporary implementation, under rewrite */

#include <stdio.h>
#include <stdarg.h>
#include <stm32f0xx.h>
#include <sys/types.h>
#include <sys/stat.h>

#if 1 
#include <Arduino.h>
#endif

extern void _exit( int status ) ;
extern void _kill( int pid, int sig ) ;
extern int _getpid ( void ) ;

extern int  _end ;

caddr_t 
_sbrk ( int incr ) {
    static unsigned char *heap = NULL ;
    unsigned char *prev_heap ;

    if ( heap == NULL )
    {
        heap = (unsigned char *)&_end ;
    }
    prev_heap = heap;

    heap += incr ;

    return (caddr_t) prev_heap ;
}

int 
link(char *oldname, char *linkname) {
    return -1;
}

int 
_close(int fd) {
    return -1;
}

int 
_fstat(int fd, struct stat *st) {
    st->st_mode = S_IFCHR;

    return 0;
}

int 
_isatty(int fd) {
    return 1;
}

int _lseek(int fd, int ptr, int dir) {
    return -1;
}

int _read(int fd, char *ptr, int len) {
    return -1;
}


int 
_write(int fd, char *ptr, int len) {
    register int cnt = len;

    digitalWrite(13, 1);
    digitalWrite(12, 0);

    while (cnt--) {
        USART2->TDR = *ptr++;

        while ((USART2->ISR & USART_ISR_TXE) == 0) {
            //yield();
        }
    }

    return len;
}

void _exit(int status) {
    printf( "Exiting with status %d.\n", status ) ;

    for (;;) 
        ;
}

void _kill(int pid, int sig) {
}

int _getpid (void) {
    return -1;
}
