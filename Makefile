FILES = $(basename $(wildcard *.cpp))
OBJS = $(addsuffix .o, $(FILES))
BINARY = demo

GREEN = "\\033[32m"
YELLOW = "\\033[33m"
RESET = "\\033[0m"

CC=clang
CPPFLAGS=-Wall -Wextra -ggdb -std=c++14 -stdlib=libc++
CFLAGS=-Wall -Wextra -ggdb

ifeq ($(shell uname), Darwin)
  LFLAGS=-lstdc++ -stdlib=libc++
else
  LFLAGS=-lstdc++ -lm
endif

%.a: %.cpp %.hpp
	@echo "$(YELLOW)Compiling $< => $@$(RESET)"
	@echo $(CC) -c $(CFLAGS) $< -o $@
	@$(CC) -c $(CPPFLAGS) $< -o $@

all: $(BINARY)

run: $(BINARY)
	./$(BINARY)

test: $(BINARY)
	./$(BINARY) test

info:
	@echo "CC: $(CC)"
	@echo "LFLAGS: $(LFLAGS)"
	@echo "CFLAGS: $(CFLAGS)"
	@echo

$(BINARY): $(OBJS)
	@echo
	@echo "$(GREEN)Linking $(OBJS) => $(BINARY)$(RESET)"
	$(CC) $(OBJS) $(LFLAGS) -o $(BINARY)

clean:
	rm -f *.o $(BINARY)

.PHONY: info all $(BINARY) clean
