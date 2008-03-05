* template: #template#
plugin #pluginid# cmd="#pkgroot#/bin/nmxptool -H #srcaddr# -P #srcport# -N IV -F #pkgroot#/nmxptool_channelfile_#srcaddr#.txt -v 16 -k"
             timeout = 600
             start_retry = 60
             shutdown_wait = 10

