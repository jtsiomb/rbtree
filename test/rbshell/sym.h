#ifndef SYM_H_
#define SYM_H_

int init_sym(void);
void destroy_sym(void);

int add_sym(const char *name, int id, int arity);

int sym_lookup(const char *str);
int sym_arity(const char *str);

#endif	/* SYM_H_ */
