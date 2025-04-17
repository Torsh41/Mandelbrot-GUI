# OK - according to GNU standard:
# https://www.gnu.org/software/make/manual/html_node/Implicit-Variables.html

# Reason for compiling .o files
# https://stackoverflow.com/questions/18777326/compiling-multiple-c-files-with-gcc
#  STEPS
#  =====
#   -- C vs CPP:
#           change all appearences of .cpp to .c or vice versa
#	- compile each .c file to .o file (-c: stop after compilation phase, without linking)
#			gcc -c module_n.c -o module_n.o
#	- link .o files
#			gcc -o myprog module_1.o module_2.o module_3.o

EXT = .c
BIN = exec
BINDIR = ./

# Compiler
CC = gcc
# Flags
# CFLAGS = -Wall -g # -g enables gdb flags
CFLAGS += -I./
CFLAGS += -std=c11 -Wall -Wextra -pedantic -Wno-unused-function -O2 \
	-fno-strict-aliasing -march=native -DONLINE_JUDGE -static -Wl,-s,--stack=67108864 \
	-Wold-style-cast -g
# CFLAGS += -x c++ -std=c++23 -O2 -fno-strict-aliasing -march=native \
#     -DONLINE_JUDGE -static -Wl,-s,--stack=67108864


LDLIBS = -lglfw -lvulkan -lm


SOURCEDIR = ./
BUILDDIR = build
# SRC = $(SOURCEDIR)/slitherine.c \
# 	 FileSystemTreeManip.cpp \
#      lab1.h   # No headers!!!
# SRC = $(wildcard $(SOURCEDIR)/*.c)
SRC = $(SOURCEDIR)/main.c

# OBJ = $(SRC:.c=.o)
OBJ = $(patsubst $(SOURCEDIR)/%$(EXT),$(BUILDDIR)/%.o,$(SRC))

$(info "SRC:"$(SRC))
$(info "OBJ:"$(OBJ))


.PHONY: all
all: $(OBJ) $(BIN)



# compile object files
$(BUILDDIR)/%.o: $(SOURCEDIR)/%$(EXT)
	$(info "Compiling "$<" into "$@)
	@mkdir -p $(BUILDDIR) # create separate dir for precompiled objects
	$(CC) $(LDFLAGS) $(CFLAGS) -c -o $@ $<

# link object files
$(BIN): $(OBJ)
	@mkdir -p $(BINDIR)
	rm -f $(BINDIR)/$(BIN) $(OBJS)
	$(CC) $(OBJ) -o $(BINDIR)/$(BIN) $(LDLIBS)



# TODO: compile shared libraries
# TODO: AND compile static libraries
# TODO: create a TEST command
#.PHONY: build
#	
.PHONY: clean
clean:
	rm -rf $(BUILDDIR)/ *.o
.PHONY: vimclean
vimclean:
	rm -rf ./*/*.sw*
.PHONY: libclean
libclean:
	rm -rf *.a *.so
