
# -------------------------------------------------------------------- #
#  Makefile for FileTree Project                                       #
#  Author: Christopher Ronda & Benjamin Herber                         #
# -------------------------------------------------------------------- #

# Macros
CMPLR = gcc217 -g

# Useful Pseudo-Targets
all: ft

clobber: clean
	rm -f *~ \#*\# *.vscode *.dSYM

clean:
	rm -f ./ft ./*.o

# Executables
ft: ft_client.o ft.o dirNode.o fileNode.o dynarray.o
	$(CMPLR) -o ft ft_client.o ft.o dirNode.o fileNode.o dynarray.o

# Dependencies
ft_client.o: ft_client.c ft.h
	$(CMPLR) -c ft_client.c ft.h

ft.o: ft.c dirNode.h fileNode.h ft.h dynarray.h
	$(CMPLR) -c ft.c dirNode.h fileNode.h dynarray.h

dirNode.o: dirNode.c fileNode.h dirNode.h dynarray.h
	$(CMPLR) -c dirNode.c fileNode.h dirNode.h dynarray.h

fileNode.o: fileNode.c fileNode.h
	$(CMPLR) -c fileNode.c fileNode.h

dynarray.o: dynarray.c dynarray.h
	$(CMPLR) -c dynarray.c dynarray.h