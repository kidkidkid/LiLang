lexical:
	g++ -std=c++11 ./test/lexical_test.cpp ./src/compiler/lexical.cpp -I./src/compiler -o lexical.out
	./lexical.out
	rm ./lexical.out