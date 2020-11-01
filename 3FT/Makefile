
# -------------------------------------------------------------------- #
#  Makefile for FileTree Project                                       #
#  Author: Christopher Ronda & Benjamin Herber                         #
# -------------------------------------------------------------------- #

# Macros
CMPLR = gcc217

# Useful Pseudo-Targets
all: ft

clobber: clean
	rm -f *~ \#*\# *.vscode *.dSYM

clean:
	rm -f ./ft ./*.o

# Executables
ft: ft_client.o ft.o dirNode.o fileNode.o
	$(CMPLR) -o ft ft_client.o ft.o dirNode.o fileNode.o

# Dependencies
ft_client.o: ft_client.c ft.h dirNode.h fileNode.h
	$(CMPLR) -c ft_client.c ft.h dirNode.h fileNode.h -o ft_client.o

ft.o: ft.c dirNode.h fileNode.h
	$(CMPLR) -c ft.c dirNode.h fileNode.h -o ft.o

dirNode.o: dirNode.c fileNode.h
	$(CMPLR) -c dirNode.c fileNode.h -o dirNode.o

fileNode.o: fileNode.c
	$(CMPLR) -c fileNode.c -o fileNode.o