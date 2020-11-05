
# -------------------------------------------------------------------- #
#  Makefile for FileTree Project                                       #
#  Author: Christian Ronda & Benjamin Herber                           #
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
ft: ft_client.o ft.o node.o dynarray.o
	$(CMPLR) -o ft ft_client.o ft.o node.o  dynarray.o

# Dependencies
ft_client.o: ft_client.c ft.h
	$(CMPLR) -c ft_client.c ft.h

ft.o: ft.c node.h ft.h dynarray.h
	$(CMPLR) -c ft.c node.h dynarray.h

node.o: node.c node.h dynarray.h
	$(CMPLR) -c node.c node.h dynarray.h

dynarray.o: dynarray.c dynarray.h
	$(CMPLR) -c dynarray.c dynarray.h