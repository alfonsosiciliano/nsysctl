/*
 * % cc -g problem-xo-flush.c -lxo -o problem-xo-flush
 *
 *  output xo_set_flags(NULL, XOF_FLUSH):
 *  % ./problem-xo-flush
 *     Value1: Value2:2 Format:fmtstr
 *     Value3:3
 * -"1" of :value1 isn't printed-
 *
 *  output xo_set_flags(NULL, 0):
 *  % ./problem-xo-flush
 *     Value1:1 Value2:2 Format:fmtstr
 *     Value3:3
 * -OK-
 *
 * output xo_set_flags(NULL, XOF_FLUSH):
 * % ./problem-xo-flush --libxo=xml,pretty
 *   <value1>1</value1>
 *   <value2>2</value2>
 *   <format>fmtstr</format>
 *   <value3>3</value3>
 * -OK- the problem is with libxo=text
 */

#include <libxo/xo.h>
#include <stdio.h>

int main(int argc, char** argv)
{
    int value1 = 1, value2 = 2, value3 = 3;
    char *fmt = "fmtstr";
    
    atexit(xo_finish_atexit);

    xo_set_flags(NULL, XOF_FLUSH);
    //xo_set_flags(NULL, 0);
    
    argc = xo_parse_args(argc, argv);
    if (argc < 0)
	exit(EXIT_FAILURE);

    xo_emit("{Lc:Value1}{:value1/%d}{P: }", value1);
    xo_emit("{Lc:Value2}{:value2/%d}{P: }", value2);
    xo_emit("{Lc:Format}{:format/%s}", fmt);
    xo_emit("{L:\n}");
    xo_emit("{Lc:Value3}{:value3/%d}", value3);
    xo_emit("{L:\n}");
    
    return 0;
}
