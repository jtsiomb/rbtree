#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "sym.h"
#include "rbtree.h"

struct symbol {
	char *name;
	int id, arity;
};

static void del_func(struct rbnode *node, void *cls);

struct rbtree *symbols;

int init_sym(void)
{
	if(!(symbols = rb_create(RB_KEY_STRING))) {
		fprintf(stderr, "failed to create symbol table\n");
		return -1;
	}
	rb_set_delete_func(symbols, del_func, 0);
	return 0;
}

void destroy_sym(void)
{
	rb_free(symbols);
}

int add_sym(const char *name, int id, int arity)
{
	struct symbol *sym;

	if(!(sym = malloc(sizeof *sym)) || !(sym->name = malloc(strlen(name) + 1))) {
		perror("failed to allocate symbol");
		free(sym);
		return -1;
	}
	strcpy(sym->name, name);
	sym->id = id;
	sym->arity = arity;

	return rb_insert(symbols, sym->name, sym);
}

int sym_lookup(const char *str)
{
	struct rbnode *node;

	if(!(node = rb_find(symbols, (char*)str))) {
		fprintf(stderr, "undefined symbol: %s\n", str);
		return -1;
	}
	return ((struct symbol*)node->data)->id;
}

int sym_arity(const char *str)
{
	struct rbnode *node;

	if(!(node = rb_find(symbols, (char*)str))) {
		fprintf(stderr, "undefined symbol: %s\n", str);
		return -1;
	}
	return ((struct symbol*)node->data)->arity;
}

static void del_func(struct rbnode *node, void *cls)
{
	free(((struct symbol*)node->data)->name);
	free(node->data);
}
