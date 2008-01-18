* template: #template#
plugin nmxptool_#plugind# cmd="#pkgroot#/bin/nmxptool -H #srcaddr# -P #srcport# -S -1 -F #pkgroot#/nmxp_channelfile_#srcaddr#.txt -A 180 -v 16 -k"
             timeout = 600
             start_retry = 60
             shutdown_wait = 10

