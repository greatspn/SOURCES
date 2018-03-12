#!/bin/csh -f
@ retcode = 0

lpr -P$1 $2
if( $status != 0) @ retcode = $retcode + 1
/bin/rm $2
if( $status != 0) @ retcode = $retcode + 2

exit($retcode)
