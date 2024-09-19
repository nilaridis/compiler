.phony: build
build: compiler

compiler:
	cd src; bison -d sydc1.y
	# sydc1.tab.h
	# sydc1.tab.c

	cd src; flex lex.l
	# lex.yy.c

	cd src; gcc -o ../compiler sydc1.tab.c lex.yy.c symbol.c ast.c zyywrap.c mixal_generator.c

	# compiler.exe

	rm -f src/sydc1.tab.h
	rm -f src/sydc1.tab.c
	rm -f src/lex.yy.c


.phony: test
test: compiler
	./compiler < samples/inputfile5.txt 1.mixal -asm -overflow
	mixasm asm.mixal
.phony: clean
clean:
	rm -f compiler
	rm -f *.mixal
	rm -f source/sydc1.tab.h
	rm -f source/ysydc1tab.c
	rm -f source/lex.yy.c
	rm -f *.mix


