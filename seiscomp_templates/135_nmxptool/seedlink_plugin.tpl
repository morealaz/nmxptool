* template: #template#
plugin nmxptool_#statid# cmd="#pkgroot#/bin/nmxptool -H #srcaddr# -P #srcport# -S -1 -C #netid#.#statid#.HH? -k"
             timeout = 600
             start_retry = 60
             shutdown_wait = 10

