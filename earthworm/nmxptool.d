#
#                     Configuration File for nmxptool
#
MyModuleId           MOD_NMXPTOOL
RingName             WAVE_RING             # Transport ring to write data to.

HeartBeatInterval    15                    # Heartbeat interval, in seconds.
LogFile              1                     # 1 -> Keep log, 0 -> no log file
                                           # 2 -> write to module log but not stderr/stdout

#ForceTraceBuf1      0                     # On systems that support TRACEBUF2
                                           # messages this flag will force the module
                                           # to create TRACEBUF messages instead.
                                           # Most people will never need this.

Verbosity            16                    # Set level of verbosity. Verbosity is a bitmap:
                                           # 1 Packet, 2 Channel, 4 Raw Stream,
                                           # 8 CRC32, 16 Connection flow,
                                           # 32 Packet Management, 64 Extra, 128 Date,
                                           # 256 Gap, 512 DOD, 1023 All messages.
                                           # It is equivalent to the option -v.

NmxpHost             naqs1a.int.ingv.it    # Host address of NaqsServer/DataServer
                                           # It is equivalent to the option -H.

NmxpPortPDS          28000                 # Port number of NaqsServer (Default 28000)
                                           # It is equivalent to the option -P.

NmxpPortDAP          28002                 # Port number of DataServer(Default 28002)
                                           # It is equivalent to the option -D.

MaxTolerableLatency  120                   # Max tolerable latency for each channel. (Default 600 sec.)
                                           # In general, DO NOT use with parameter TimeoutRecv.
                                           # It is equivalent to the option -M.

#TimeoutRecv          30                   # Time-out in seconds for flushing queued data for each channel.
                                           # (Default 0. No time-out) [10..300].
                                           # Useful for retrieving Data On Demand with minimum delay.
                                           # It is equivalent to the option -T.

DefaultNetworkCode   IV                    # Default network code where in 'ChannelFile' or 'Channel' is not declared.
                                           # It is equivalent to the option -N.

                                           # nmxptool channel definition IS NOT equal to SCNL
                                           # It is NSC, that is NET.STA.CHAN
                                           # NET  is optional and used only for output.
                                           # STA  can be '*', stands for all stations.
                                           # CHAN can contain '?', stands for any character.
                                           # Example:  *.HH?,N1.STA2.??Z,STA3.?H?
                                           # Related to the parameters 'ChannelFile' and 'Channel'.

#MaxDataToRetrieve    180                  # Max amount of data of the past to retrieve from the
                                           # DataServer when program restarts (default 0) [0..86400].
                                           # 0 to disable connection to DataServer.
                                           # It is equivalent to the option -A. Related to 'ChannelFile'.
                                           # If 'MaxDataToRetrieve' is zero and 'ChannelFile' is used,
                                           # only data buffered by NaqsServer will be retrieved.

ChannelFile   /home/ew/naqs1a.list.txt    # List of channels. One for each line. (It will not be modified)
                                          # Load/Save time of last sample of each channel in a file
                                          # with the same name, same directory, appending suffix ".nmxpstate"
                                          # Allow data continuity between program restarts.
                                          # It is equivalent to the option -F. Related to 'MaxDataToRetrieve'.

    # DO NOT USE parameters 'Channel' and 'ChannelFile' together.
    # 'ChannelFile' is preferable. At restart you can retrieve data
    # of the past from DataServer. See 'MaxDataToRetrieve'.

# Example of nmxptool channel definition
# Channel              ES.BOB.HH?
# Channel              MN.TIR.HH?
# Channel              MDI.HH?
# Channel              DOI.HH?
# Channel              SALO.HH?
# Channel              MONC.HH?
# Channel              *.BHZ               # Channel selection

# Please, for other details about parameters, refer to the command line "nmxptool -h"


