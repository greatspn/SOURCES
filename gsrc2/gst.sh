#!/bin/sh

#if ( ! $?GSPN2VERSION ) then
#	set path1 = $0;
#	set path2 = $path1:h
#	set path1 = $path2:h
#        setenv GSPN2VERSION $path1/install/version.csh
#	unset path1
#	unset path2
#endif
#if ( ! $?GSPN2BINS ) then
#        setenv GSPN2BINS `$GSPN2VERSION`
#endif
#if ( ! $?GSPN2HOME ) then
#        source ~/.GreatSPN2_$GSPN2BINS
#endif

dir=nets/
fil=$argv[1]
GST=$dir$fil.GST

${GREATSPN_SCRIPTDIR}/2bin/gre $dir$fil > /dev/null
/bin/cp $dir${fil}_cmark.c ${dir}GST_cmark.c
/bin/cp $dir${fil}_cres.c ${dir}GST_cres.c
/bin/cp $dir${fil}_cress.c ${dir}GST_cress.c
/bin/cp $dir${fil}_cpro.c ${dir}GST_cpro.c
cc -w -O -o $GST  ${dir}GST_cres.c ${dir}GST_cress.c ${dir}GST_cpro.c \
	${dir}GST_cmark.c $GSPNH2OME/2bin/gst.o ${GREATSPN_SCRIPTDIR}/2bin/compact.o
/bin/rm -f ${dir}GST_cres.c ${dir}GST_cress.c ${dir}GST_cpro.c \
	${dir}GST_cmark.c *.o ${dir}*.o
echo ""
echo "Start computing performance indices"
$GST $dir$fil > $dir$fil.sta
/bin/rm -f $GST
exit(0)
