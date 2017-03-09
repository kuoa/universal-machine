# Universal Machine
The Universal Machine is a Virtual Machine presented at the Ninth annual ICFP Programming Contest. The VM is capable of running the UMIX operating system, which was written for the purpose of the competition. 


## Hierarchy
	- src -> contains vm.c which is the source file
	- test_files -> contains codex.umz and sandmark.umz that can be used as input files
	- Universal-Machine-Report.pdf -> the report file

## Run

	- make -> creates [vm] which takes the name of a file as the first argument
	- make sandmark -> compiles the vm and executes the sandmark.umz file
	- make codex -> compiles the vm and executes the codex.umz file, needs the decryption key: (\b.bb)(\v.vv)06FHPVboundvarHRAk
	- make clean -> removes the [vm] executable

