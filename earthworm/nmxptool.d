#
#                     Configuration File for nmxptool
#
MyModuleId         MOD_NMXPTOOL
RingName           WAVE_RING       # Transport ring to write data to.

HeartBeatInterval  30            # Heartbeat interval, in seconds.
LogFile            1             # 1 -> Keep log, 0 -> no log file
                                 # 2 -> write to module log but not stderr/stdout
Verbosity          1		 # Set level of verbosity.

nmxphost          naqs1a.int.ingv.it    # Host address of the NaqsServer server
nmxpport          28000                 # Port number of the NaqsServer server

#ForceTraceBuf1 0                # On systems that support TRACEBUF2
                                 # messages this flag will force the module
                                 # to create TRACEBUF messages instead.
                                 # Most people will never need this.

MaxTolerableLatency 120

DefaultNetworkCode	IV

Channel		ES.BOB.HH?
Channel		MN.TIR.HH?
Channel		MDI.HH?
Channel		DOI.HH?
Channel		SALO.HH?
Channel		MONC.HH?

