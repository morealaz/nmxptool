* template: #template#
plugin nmxptool_#pluginid# cmd="#pkgroot#/bin/nmxptool -H #srcaddr# -P #srcport# -S -1 -F #pkgroot#/config/nmxptool_channelfile_#srcaddr#.txt -v 16 -A 600 -k"
             timeout = 600
             start_retry = 60
             shutdown_wait = 10

