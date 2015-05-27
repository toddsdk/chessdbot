CC = gcc
OS=$(shell uname -s)
CFLAGS= -Wall -D'BASE_DIR="${PWD}/"' -D$(OS) -O3
LDFLAGS= -lpthread -lexpat
BINDIR=bin
SW=chessdbot
SRCDIR=src
SRC=board.c cecp.c eco.c heuristic.c history.c levels.c main.c moves.c search.c transposition.c xml.c
SOURCES=$(addprefix $(SRCDIR)/, $(SRC))
OBJDIR=obj
OBJ=$(addprefix $(OBJDIR)/, $(SRC:.c=.o))
HEADERS=$(SOURCES:.c=.h)

.PHONY: all clean clean-all

all: $(BINDIR)/$(SW)

$(BINDIR)/$(SW): $(OBJ) 
	$(CC) -o $@ $(OBJ) $(LDFLAGS)

$(OBJDIR)/%.o: $(SRCDIR)/%.c $(HEADERS)
	@mkdir -p $(dir $@)
	$(CC) -c $(CFLAGS) $(LEVELS) -o $@ $<

clean: 
	-rm -rf $(OBJDIR)
clean-all: clean
	-rm -f $(BINDIR)/$(SW)
