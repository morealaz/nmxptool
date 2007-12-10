#
#                     Configuration File for nmxptool
#
MyModuleId           MOD_NMXPTOOL
RingName             WAVE_RING             # Transport ring to write data to.

HeartBeatInterval    30                    # Heartbeat interval, in seconds.
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

#TimeoutRecv          30                    # Time-out in seconds for flushing queued data for each channel.
                                           # Useful for Data On Demand (i.e. channel HL) (Default 0. No time-out)

MaxTolerableLatency  120                   # Max tolerable latency for each channel. (Default 600 sec.)
                                           # In general, do not use with parameter TimeoutRecv.

DefaultNetworkCode   IV                    # Default network code where in Channel is not declared

                                           # Channel is like NET.STA.CHAN where NET. is optional
Channel              ES.BOB.HH?
Channel              MN.TIR.HH?
Channel              MDI.HH?
Channel              DOI.HH?
Channel              SALO.HH?
Channel              MONC.HH?

