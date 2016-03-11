CC = clang++ -w

all: DiskMultiMap.cpp IntelWeb.cpp p4tester.cpp
	rm malicious.txt
	$(CC) DiskMultiMap.cpp IntelWeb.cpp p4tester.cpp -o p4tester
	$(CC) p4gen.cpp -o p4gen
	./p4gen sources.txt malicious.dat 10 3 malicious.txt
	./p4tester -b trial 100
	./p4tester -i trial malicious.txt
	./p4tester -s trial indicators.txt 12 stuffFound.txt
	##./p4tester -p trial omit.txt
	./p4tester -w stuffFound.txt graphtemplate.html result.html


