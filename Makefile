lexical:
	g++ -std=c++11 \
		./test/lexical_test.cpp ./src/compiler/lexical.cpp  \
		-I./src/compiler \
		-o lexical.out
	./lexical.out
	rm ./lexical.out

syntax:
	g++ -std=c++11 -O0\
		./test/syntax_test.cpp ./src/compiler/syntax.cpp ./src/compiler/lexical.cpp ./src/compiler/ast.cpp \
		-I./src/compiler \
		-o syntax.out
	./syntax.out
	rm ./syntax.out