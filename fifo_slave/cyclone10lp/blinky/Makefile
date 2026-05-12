# Define your project settings
PROJECT = blinky
REVISION = blinky

QUARTUS_PATH=/home/hawk/altera_lite/25.1std/quartus/bin

# Primary targets
all: bitstream

# Execute the complete Intel Quartus design flow
bitstream: $(PROJECT).qpf $(PROJECT).qsf
	$(QUARTUS_PATH)/quartus_sh --flow compile $(PROJECT) -c $(REVISION)

# Remove temporary compilation artifacts and databases
clean:
	rm -rf db incremental_db output_files
	rm -f *.summary *.rpt *.smsg *.sof *.pof
