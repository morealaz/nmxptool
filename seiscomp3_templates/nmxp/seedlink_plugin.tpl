* template: $template
plugin $seedlink.source.id cmd="$seedlink.plugin_dir/nmxptool -H $sources.nmxp.address -P $sources.nmxp.port -N IV -F $pkgroot/nmxptool_channelfile_$sources.nmxp.address\.$sources.nmxp.port\.txt -v 16 --slink_network_id -k"
             timeout = 600
             start_retry = 30
             shutdown_wait = 15
