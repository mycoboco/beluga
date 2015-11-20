/*
 *  stack (cdsl)
 */

#ifndef STACK_H
#define STACK_H


/* stack */
typedef struct stack_t stack_t;


stack_t *stack_new(void);
void stack_free(stack_t **);
void stack_push(stack_t *, void *);
void *stack_pop(stack_t *);
void *stack_peek(const stack_t *);
int stack_empty(const stack_t *);


#endif    /* STACK_H */

/* end of stack.h */
