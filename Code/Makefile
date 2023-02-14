# 编译器设定和编译选项
CC = gcc
FLEX = flex
BISON = bison
GCFLAGS = -std=c99 -ggdb3 -fsanitize=address -I. -DNDEBUG
CFLAGS = -std=c99 -ggdb3 -Wall -Werror -fsanitize=address -I. -DNDEBUG

# 编译目标：src目录下的所有.c文件
CFILES = $(shell find ./ -name "*.c")
OBJS = $(CFILES:.c=.o)
LFILE = $(shell find ./ -name "*.l")
YFILE = $(shell find ./ -name "*.y")
LFC = $(shell find ./ -name "*.l" | sed s/[^/]*\\.l/lex.yy.c/)
YFC = $(shell find ./ -name "*.y" | sed s/[^/]*\\.y/syntax.tab.c/)
LFO = $(LFC:.c=.o)
YFO = $(YFC:.c=.o)

parser: syntax $(filter-out $(LFO),$(OBJS))
	$(CC) $(GCFLAGS) -o parser $(filter-out $(LFO),$(OBJS)) -lfl -ly

syntax: lexical syntax-c
	$(CC) $(GCFLAGS) -c $(YFC) -o $(YFO)

lexical: $(LFILE)
	$(FLEX) -o $(LFC) $(LFILE)

syntax-c: $(YFILE)
	$(BISON) -o $(YFC) -d -v $(YFILE)

-include $(patsubst %.o, %.d, $(OBJS))

test-symtab: symtab.c symtab.h ../Test/test-symtab.c
	$(CC) $(CFLAGS) symtab.c ../Test/test-symtab.c -O0 -o ../Test/test-symtab

test-visitor: symtab.c symtab.h ast.h ast.c eval.c ../Test/test-visitor.c
	$(CC) $(CFLAGS) ast.c print.c eval.c symtab.c ../Test/test-visitor.c -O0 -o ../Test/test-visitor

# 定义的一些伪目标
.PHONY: clean test
test:
	./test-symtab
	./test-visitor
	./parser ../Test/test1.cmm
clean:
	rm -f parser lex.yy.c syntax.tab.c syntax.tab.h syntax.output
	rm -f $(OBJS) $(OBJS:.o=.d)
	rm -f $(LFC) $(YFC) $(YFC:.c=.h)
	rm -f *.o
	rm -f test-symtab test-visitor
	rm -f *~