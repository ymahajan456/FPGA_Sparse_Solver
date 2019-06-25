HARDWARE_PROJECT_NAME = project_LUDHardware
PYTHON3 = python3

.PHONY: all copyConfFile copyTestFiles genHardwareTcl genHdlFiles clean cleanall

all: genHardwareTcl

copyConfFile:
	mkdir -p ./confFiles
	cp ../Scheduler/outputFiles/hwConfig.json ./confFiles/

copyTestFiles:
	mkdir -p ./testFiles
	cp ../Scheduler/outputFiles/*.coe ./testFiles/

genHdlFiles: copyConfFile copyTestFiles
	mkdir -p ./autoHdlFiles/
	(cd ./generatorFiles; $(PYTHON3) genLUHDHardware.py)

genHardwareTcl: genHdlFiles
	mkdir -p ./vivadoProjects
	mkdir -p ./tclScripts/
	(cd ./generatorFiles; $(PYTHON3) genBDFiles.py)
	(cd ./generatorFiles; $(PYTHON3) genProjectTcl.py -n $(HARDWARE_PROJECT_NAME))
	
clean:
	rm -rf tclScripts
	rm -rf testFiles
	rm -rf autoHdlFiles
	rm -rf confFiles

cleanall: clean
	rm -rf vivadoProjects


