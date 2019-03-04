/*
 *Problem: with xo_set_flags(NULL, XOF_FLUSH) and libxo=text
 *         the first value after a label
 *         xo_emit("{L:Label}{:value}");
 *         is not printed.
 *
 * % cc -g problem-xof-flush.c -lxo -o problem-xof-flush
 *./problem-xof-flush
 * TEST 1
 * Value1:
 * TEST 2
 * Value2:2
 * TEST 3
 * 3Value3:
 */

#include <libxo/xo.h>
//#include <stdio.h>

int main(int argc, char** argv)
{
    int value1 = 1, value2 = 2, value3 = 3;
    
    atexit(xo_finish_atexit);
    xo_set_flags(NULL, XOF_FLUSH);
    //xo_set_flags(NULL, 0); // No problem
    argc = xo_parse_args(argc, argv);
    if (argc < 0)
	exit(EXIT_FAILURE);


    xo_emit("{L:TEST 1\n}");
    /* Problem: value1 is not printed */
    xo_emit("{Lc:Value1}{:value1/%d}", value1);

    /* No problem (first the value)*/
    /*
     * xo_emit("{:value1/%d}{Lc:Value1}", value1);
     */

    /* No problem (split) */
    /*
     * xo_emit("{Lc:Value1}");
     * xo_emit("{:value1/%d}", value1);
    */
    
    xo_emit("{L:\n}");

    
    /* 
     * after the first 'xo_emit "{Lc:Value1}{:value1/%d}" ' is all right
     */
    xo_emit("{L:TEST 2\n}");
    xo_emit("{Lc:Value2}{:value2/%d}", value2);
    xo_emit("{L:\n}");

    xo_emit("{L:TEST 3\n}");
    xo_emit("{:value3/%d}{Lc:Value3}", value3);
    xo_emit("{L:\n}");

    return 0;
}
