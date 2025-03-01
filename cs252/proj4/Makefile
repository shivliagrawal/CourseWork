# Use GNU compiler
CC = gcc -g -Wextra -pedantic -Wall -Werror -pthread

PARTS = part1 part2

all: $(PARTS) git

$(PARTS): %: %.c %.h
	$(CC) $(WARNINGS) $< -o $@

.PHONY: git
# DO NOT MODIFY
git:
	git add *.c *.h Makefile >> .local.git.out || echo
	git commit -a -m "Commit lab 4" >> .local.git.out || echo
	git push origin HEAD:master

.PHONY: git
clean:
	rm -f $(PARTS)

.PHONEY: submit
submit:
	@date >> .date
	@git add .date *.c *.h Makefile >> .local.git.out || echo
	@git commit -a -m "Submit lab 4" >> .local.git.out || echo
	@git push origin HEAD:master
	@echo "Final submission complete"
