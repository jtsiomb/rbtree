#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "rbtree.h"
#include "sym.h"

#define SEP	" \t\r\n,\"\'"

/* commands */
enum {
	CMD_CREATE,
	CMD_DESTROY,
	CMD_CLEAR,
	CMD_COPY,
	CMD_SIZE,
	CMD_INSERT,
	CMD_DELETE,
	CMD_FIND,
	CMD_PRINT,

	MAX_CMD
};

int runcmd(int cmd, int argc, char **argv);

#define NUM_TREES	8
struct rbtree *trees[NUM_TREES];


int main(void)
{
	char buf[512];

	/* init symbol table */
	if(init_sym() == -1) {
		return 1;
	}
	add_sym("create", CMD_CREATE, 0);
	add_sym("destroy", CMD_DESTROY, 1);
	add_sym("clear", CMD_CLEAR, 1);
	add_sym("copy", CMD_COPY, 2);
	add_sym("size", CMD_SIZE, 1);
	add_sym("insert", CMD_INSERT, 2);
	add_sym("delete", CMD_DELETE, 2);
	add_sym("find", CMD_FIND, 2);
	add_sym("print", CMD_PRINT, 1);

	for(;;) {
		int argc = 0;
		char *argv[8];
		int cmd_id, cmd_arity;

		printf("rbshell> ");
		fflush(stdout);

		if(!fgets(buf, sizeof buf, stdin)) {
			putchar('\n');
			break;
		}

		if(!(argv[argc] = strtok(buf, SEP)) || argv[argc][0] == '#') {
			return 0;	/* empty / comment */
		}
		argc++;

		if((cmd_id = sym_lookup(argv[0])) == -1) {
			continue;
		}
		cmd_arity = sym_arity(argv[0]);

		while((argv[argc] = strtok(0, SEP))) {
			argc++;
			if(argc >= sizeof argv / sizeof *argv) {
				break;
			}
		}

		if(argc - 1 != cmd_arity) {
			fprintf(stderr, "too many (%d) arguments for %s, expected %d\n", argc - 1, argv[0], cmd_arity);
			continue;
		}

		runcmd(cmd_id, argc, argv);
	}

	destroy_sym();
	return 0;
}

#define CHECKINT(i)		\
	if(!isdigit(argv[i][0])) { \
		fprintf(stderr, "expected number, got: %s\n", argv[i]); \
		return -1; \
	}

#define CHECKVALID(idx)	\
	if((idx) < 0 || (idx) >= NUM_TREES || !trees[idx]) { \
		fprintf(stderr, "invalid tree specified: %d\n", idx); \
		return -1; \
	}


int runcmd(int cmd, int argc, char **argv)
{
	int i, idx, idx2, n;
	struct rbnode *node;

	switch(cmd) {
	case CMD_CREATE:
		for(i=0; i<NUM_TREES; i++) {
			if(trees[i] == 0) {
				if(!(trees[i] = rb_create(RB_KEY_INT))) {
					fprintf(stderr, "failed to create a new tree\n");
					return -1;
				}
				printf("created new tree: %d\n", i);
				break;
			}
		}
		if(i == NUM_TREES) {
			fprintf(stderr, "can't create more trees\n");
			return -1;
		}
		break;

	case CMD_DESTROY:
		CHECKINT(1);
		idx = atoi(argv[1]);
		CHECKVALID(idx);

		rb_free(trees[idx]);
		trees[idx] = 0;
		printf("destroyed tree: %d\n", idx);
		break;

	case CMD_CLEAR:
		CHECKINT(1);
		idx = atoi(argv[1]);
		CHECKVALID(idx);

		rb_clear(trees[idx]);
		printf("cleared tree: %d\n", idx);
		break;

	case CMD_COPY:
		CHECKINT(1);
		CHECKINT(2);
		idx = atoi(argv[1]);
		idx2 = atoi(argv[2]);
		CHECKVALID(idx);
		CHECKVALID(idx2);

		rb_copy(trees[idx2], trees[idx]);
		printf("copied %d -> %d\n", idx, idx2);
		break;

	case CMD_SIZE:
		CHECKINT(1);
		idx = atoi(argv[1]);
		CHECKVALID(idx);

		printf("tree size: %d\n", rb_size(trees[idx]));
		break;

	case CMD_INSERT:
		CHECKINT(1);
		CHECKINT(2);
		idx = atoi(argv[1]);
		CHECKVALID(idx);

		n = atoi(argv[2]);
		rb_inserti(trees[idx], n, 0);
		printf("inserted: %d\n", n);
		break;

	case CMD_DELETE:
	case CMD_FIND:
		CHECKINT(1);
		CHECKINT(2);
		idx = atoi(argv[1]);
		CHECKVALID(idx);
		n = atoi(argv[2]);

		if(!rb_findi(trees[idx], n)) {
			fprintf(stderr, "%d not found\n", n);
		} else {
			if(cmd == CMD_DELETE) {
				rb_deletei(trees[idx], n);
				printf("deleted %d\n", n);
			} else {
				printf("found %d\n", n);
			}
		}
		break;

	case CMD_PRINT:
		CHECKINT(1);
		idx = atoi(argv[1]);
		CHECKVALID(idx);

		rb_begin(trees[idx]);
		while((node = rb_next(trees[idx]))) {
			printf("%d ", rb_node_keyi(node));
		}
		putchar('\n');
		break;

	default:
		/* can't happen */
		break;
	}
	return 0;
}
