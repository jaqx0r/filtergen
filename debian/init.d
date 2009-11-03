#! /bin/sh
#
# initscript for filtergen to create/destroy firewall rules

PATH=/sbin:/bin:/usr/sbin:/usr/bin
FILTERGEN=/usr/sbin/filtergen
CONF=/etc/filtergen/filtergen.conf
RULES=/etc/filtergen/rules.filter
LOG=/var/log/filtergen/rules.log

test -x $FILTERGEN || exit 0

# die if there's a problem
set -e

# load config options
if [ -f $CONF ]; then
    . /etc/filtergen/filtergen.conf
fi
# sanity check, defaults
case "$GENERATE" in
    true|false) ;;
    *) GENERATE="false" ;;
esac
case "$BACKEND" in
    iptables|ipchains|ipfilter|cisco) ;;
    *) BACKEND="iptables" ;;
esac

case "$1" in
    start)
	if [ "$GENERATE" = "true" ]; then
	    echo -n "Generating $BACKEND packet filter"
	    case "$BACKEND" in
		iptables|ipchains)
		    # save the generated rules to the log file for perusal
		    $FILTERGEN $FGOPTS -t $BACKEND $RULES | tee $LOG | /bin/sh
		    ;;
		*)
		    echo -n ": Operation not permitted with $BACKEND backend"
		    ;;
	    esac	
	    echo "."
	fi
	;;
    stop)
	echo -n "Flushing $BACKEND packet filter"
	case "$BACKEND" in
	    iptables|ipchains)
		$FILTERGEN -t $BACKEND -F ACCEPT | /bin/sh
		;;
	    *)
		echo -n ": Operation not supported with $BACKEND backend"
		;;
	esac
	echo "."
	;;
    restart|reload|force-reload)
	$0 stop
	$0 start
	;;
    check)
	# echo the output of the current rules to stdout for checking,
	# along with any error message
	$FILTERGEN $FGOPTS -t $BACKEND $RULES
	;;
    *)
	N=/etc/init.d/$NAME
	echo "Usage: $N {start|stop|restart|reload|force-reload|check}" >&2
	exit 1
	;;
esac

exit 0
