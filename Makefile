
TEST_MATRIX = 

.PHONY: clean cleanRes preprocess process buildHardware all cleanall

all: preprocess process buildHardware

cleanall: clean cleanRes

preprocess:
	(cd Scheduler/cppFiles; make buildPreprocessTests)
	(cd Scheduler/cppFiles; make preprocessTests)

process:
	(cd Scheduler/cppFiles; make all)
	(cd Scheduler/cppFiles; ./main $(TEST_MATRIX))

buildHardware:
	(cd ./Hardware; make all)


clean:
	(cd Scheduler/cppFiles; make clean)
	(cd Scheduler/cppFiles; make cleanPreprocess)
	(cd Hardware; make clean)

cleanRes:
	(cd Scheduler/cppFiles; make cleanRes)
	(cd Scheduler/test; rm -f *.csv)
	(rm -rf Scheduler/test/exported)
	(cd Hardware; make cleanll)






