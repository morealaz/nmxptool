#
#                     Configuration File for nmxptool
#
MyModuleId           MOD_NMXPTOOL
RingName             WAVE_RING             # Transport ring to write data to.

HeartBeatInterval    15                    # Heartbeat interval, in seconds.
LogFile              1                     # 1 -> Keep log, 0 -> no log file
                                           # 2 -> write to module log but not stderr/stdout
Verbosity            16                    # Set level of verbosity. Verbosity is a bitmap:
                                           # 1 Packet, 2 Channel, 4 Raw Stream,
                                           # 8 CRC32, 16 Connection flow,
                                           # 32 Packet Management, 64 Extra, 128 Date,
                                           # 256 Gap, 512 DOD, 1023 All messages.

NmxpHost             naqs1a.int.ingv.it    # Host address of the NaqsServer server
NmxpPortPDS          28000                 # Port number of the NaqsServer server (Default 28000)
NmxpPortDAP          28002                 # Port number of the DataServer server (Default 28002)

#ForceTraceBuf1      0                      # On systems that support TRACEBUF2
                                           # messages this flag will force the module
                                           # to create TRACEBUF messages instead.
                                           # Most people will never need this.

MaxTolerableLatency  120                   # Max tolerable latency for each channel. (Default 600 sec.)
                                           # In general, do not use with parameter TimeoutRecv.
MaxDataToRetrieve    60                    # Max amount of data of the past to retrieve
                                           # from the DataServer (default 60) [30..86400].

#TimeoutRecv          30                    # Time-out in seconds for flushing queued data for each channel.
                                           # Useful for Data On Demand (i.e. channel HL) (Default 0. No time-out)

DefaultNetworkCode   IV                    # Default network code where in Channel is not declared

                                           # Channel is like NET.STA.CHAN
                                           # NET  is optional and used only for output.
                                           # STA  can be '*', stands for all stations.
                                           # CHAN can contain '?', stands for any character.
                                           # Example:  *.HH?,N1.STA2.??Z,STA3.?H?

ChannelFile   /home/ew/naqs1a.list.txt    # State Channel file (It will not be updated.)
                                          # Allow data continuity between program restarts
                                          # and within available data buffered on NaqsServer.
                                          # Do not use with Channel parameter below.

# Channel              ES.BOB.HH?
# Channel              MN.TIR.HH?
# Channel              MDI.HH?
# Channel              DOI.HH?
# Channel              SALO.HH?
# Channel              MONC.HH?
# Channel              *.BHZ               # Channel selection

