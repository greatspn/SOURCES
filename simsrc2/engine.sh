#
. great_package.sh

if ( ! -e $1.grg ) then
    if ( ! -e $1.ecs ) then
	if ( ! -e $1.pin ) then
		${GREATSPN_BINDIR}/pinvar $1 > /dev/null
	endif
	${GREATSPN_BINDIR}/struct $1 > /dev/null
    endif
    ${GREATSPN_BINDIR}/grg_prep $1 > /dev/null
endif
if ( ! -e $1.gmt ) then
    ${GREATSPN_BINDIR}/gmt_prep $1 > /dev/null
endif
if ( ! -e $1.gst ) then
    ${GREATSPN_BINDIR}/gst_prep $1 > /dev/null
endif
/bin/rm -f $1.sta $1.mpd
#
# use the following option not to trace the firing instances
${GREATSPN_BINDIR}/engine $* | ${GREATSPN_BINDIR}/measure $1 &
#
# use the following option to trace the firing instances
# ${GREATSPN_BINDIR}/engine $* -r | ${GREATSPN_BINDIR}/measure $1 &
exit(0)
