#include<stdio.h>
#include<stdlib.h>
#include<ctype.h>
#include<stdarg.h>
#include<stdbool.h>
#include<string.h>

typedef enum {
    TK_RESERVED,
    TK_NUM,
    TK_EOF,
    TK_KINDS_NUMBER,
} TokenKind;

typedef struct Token Token;

struct Token {
    TokenKind kind;
    Token *next;
    int val;
    char *str;
};

Token *token;

typedef enum {
    ND_ADD,
    ND_SUB,
    ND_MUL,
    ND_DIV,
    ND_NUM,
} NodeKind;

typedef struct Node Node;

struct Node {
    NodeKind Kind;
    Node *lhs;
    Node *rhs;
    int val;
}

char *user_input;

void error_at(char *loc, char *fmt, ...) {
    va_list ap;
    va_start(ap, fmt);

    int pos = loc - user_input;
    fprintf(stderr, "%s\n", user_input);
    fprintf(stderr, "%*s", pos, " ");
    fprintf(stderr, "^ ");

    vfprintf(stderr, fmt, ap);
    fprintf(stderr, "\n");
    exit(1);
}

bool consume(char op) {
    if (token->kind != TK_RESERVED || token->str[0] != op) {
	return false;
    }
    token = token->next;
    return true;
}

void expect(char op) {
    if (token->kind != TK_RESERVED || token->str[0] != op) {
	error_at(token->str, "It is not '%c'", op);
    }
    token = token->next;
}

int expect_number() {
    if (token->kind != TK_NUM){
	error_at(token->str, "It is not number");
    }
    int val = token->val;
    token = token->next;
    return val;
}

bool at_eof(){
    return token->kind == TK_EOF;
}

Token *new_token(TokenKind token_kind, Token *current, char *str) {
    Token *next = calloc(1, sizeof(Token));
    next->kind = token_kind;
    next->str = str;
    current->next = next;
    return next;
}

Token *tokenize(char *p){
    Token head;
    head.next = NULL;
    Token *current = &head;

    while(*p){
	if (isspace(*p)) {
	    p++;
	    continue;
	}

	if (*p == '+' || *p == '-') {
	    current = new_token(TK_RESERVED, current, p++);
	    continue;
	}

	if (isdigit(*p)) {
	    current = new_token(TK_NUM, current, p);
	    current->val = strtol(p, &p, 10);
	    continue;
	}

	error_at(p, "can not tokenize");
    }

    new_token(TK_EOF, current, p);
    return head.next;
}

Node *new_node(NodeKind kind, Node *lhs, Node *rhs) {
    Node *node = calloc(1, sizeof(Node));
    node->kind = kind;
    node->lhs = lhs;
    node->rhs = rhs;
    return node;
}

Node *new_node_num(int val) {
    Node *node = calloc(1, sizeof(Node));
    node->kind = ND_NUM;
    node->val = val;
    return node;
}

Node *expr() {
    Node *node = mul();

    for (;;) {
	if (consume('+'))
	    node = new_node(ND_ADD, node, mul());
	else if (consume('-'))
	    node = new_node(ND_SUB, node, mul());
	else
	    return node;
    }
}

Node *mul() {
    Node *node = primary();
    
    for (;;) {
	if (consume('*'))
	    node = new_node(ND_MUL, node, primary());
	else if (consume('/'))
	    node = new_node(ND_DIV, node, primary());
	else
	    return node;
    }
}

Node *primary() {
    if (consume('(')) {
	Node *node = expr();
	expect(')');
	return node;
    }

    return new_node_num(expect_number());
}

int main(int argc, char **argv) {
    if (argc != 2){
        fprintf(stderr, "The number of arguments is incorrect.\n");
        return 1;
    }

    user_input = argv[1];
    token = tokenize(user_input);

    printf(".intel_syntax noprefix\n");
    printf(".globl main\n");
    printf("main:\n");
    printf("    mov rax, %d\n", expect_number());

    while(!at_eof()){
	if (consume('+')){
	    printf("    add rax, %d\n", expect_number());
	    continue;
	}

	expect('-');
        printf("    sub rax, %d\n", expect_number());

    }

    printf("    ret\n");
    return 0;
}
