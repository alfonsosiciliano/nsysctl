/*
 *Problem: with xo_set_flags(NULL, XOF_FLUSH) and libxo=text
 *         the first {:value} is not printed
 *
 * % cc -g problem-xo-flush.c -lxo -o problem-xo-flush
 *
 *  output xo_set_flags(NULL, XOF_FLUSH):
 *  % ./problem-xo-flush
 * TEST 1
 * Value1: <'1' should be here>
 * Value2:2 Str1:str1
 * Value3:3
 * TEST 2
 * Value4:4 Str2:str2
 * TEST 3
 * Str3:str3 Value5:5
 *
 */

#include <libxo/xo.h>
#include <stdio.h>

int main(int argc, char** argv)
{
    int value1 = 1, value2 = 2, value3 = 3;
    char *str1 = "str1";

    int value4 = 4;
    char *str2 = "str2";

    int value5 = 5;
    char *str3 = "str3";
    
    atexit(xo_finish_atexit);

    xo_set_flags(NULL, XOF_FLUSH);
    //xo_set_flags(NULL, 0);
    
    argc = xo_parse_args(argc, argv);
    if (argc < 0)
	exit(EXIT_FAILURE);

    xo_emit("{L:TEST 1\n}");
    /* This is the problem: value1 is not printd */
    xo_emit("{Lc:Value1}{:value1/%d}{P: }", value1);
    /* with or without '\n' */
    xo_emit("{L:\n}");
    xo_emit("{Lc:Value2}{:value2/%d}{P: }", value2);
    xo_emit("{Lc:Str1}{:str1/%s}", str1);
    xo_emit("{L:\n}");
    xo_emit("{Lc:Value3}{:value3/%d}", value3);
    xo_emit("{L:\n}");


    xo_emit("{L:TEST 2\n}");
    xo_emit("{Lc:Value4}{:value4/%d}{P: }{Lc:Str2}{:str2/%s}{L:\n}", value4, str2);

    
    xo_emit("{L:TEST 3\n}");
    xo_emit("{Lc:Str3}{:str3/%s}{P: }{Lc:Value5}{:value5/%d}{L:\n}", str3, value5);

    
    return 0;
}
