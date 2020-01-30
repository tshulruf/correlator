# Make file for various targets

all: getdata preprocess correlate mapnetworks

setup: directories scripts

directories:
	mkdir -p ../bin
	mkdir -p ../lists
	mkdir -p ../data
	mkdir -p ../means
	mkdir -p ../correlations

scripts: wget-YF-table.sh snarflists.sh
	cp wget-YF-table.sh ../bin/wget-YF-table.sh
	cp snarflists.sh ../bin/snarflists.sh

include_files = ../include/accumulator.h\
                ../include/constants.h\
                ../include/date_index.h\
                ../include/directories.h\
                ../include/extended_container.h\
                ../include/numerictypes.h\
                ../include/parsers.h\
                ../include/signals.h\
                ../include/source_data.h\
                ../include/symbols.h\
                ../include/tickers.h

linked_libraries = -lboost_date_time\
                   -lboost_filesystem\
                   -lboost_system\
                   -lboost_program_options\
                   -lboost_thread-mt

correlate: correlate.cpp $(include_files)
	g++ -O3 $(linked_libraries) correlate.cpp -o ../bin/correlate

getdata: scripts getdata.cpp $(include_files)
	g++ -O3 $(linked_libraries) getdata.cpp -o ../bin/getdata

mapnetworks: mapnetworks.cpp $(include_files)
	g++ -O3 $(linked_libraries) mapnetworks.cpp -o ../bin/mapnetworks

preprocess: preprocess.cpp $(include_files)
	g++ -O3 $(linked_libraries) preprocess.cpp -o ../bin/preprocess

text_export: text_export.cpp $(include_files)
	g++ -O3 $(linked_libraries) text_export.cpp -o ../bin/text_export

spam: spam.cpp $(include_files)
	g++ -g $(linked_libraries) spam.cpp -o ../bin/spam

editor_clean:
	rm -f *~
	rm -f ../include/*~

clean:
	rm -f ../bin/*
    
really_clean: clean editor_clean
	rm -f ../lists/*
	rm -rf ../data/*
	rm -rf ../means/*
	rm -f ../correlations/*

