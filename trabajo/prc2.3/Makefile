all: minishell 

minishell: minishell.o orden.o
	cc -o minishell minishell.o orden.o

original: minishell.o orden_original.o
	cc -o original minishell.o orden_original.o

minishell.o: minishell.c orden.h
	cc -c minishell.c

orden.o: orden.c orden.h
	cc -c orden.c

orden_original.o: orden_original.c orden.h
	cc -c orden_original.c


clean:
	rm -f minishell original minishell.o orden.o orden_original.o
