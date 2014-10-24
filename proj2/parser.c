#include <stdio.h>
#include "parser.h"
#include "array.h"

static array *symbols;
static array *tokens;

// to be used to determine the token type
// if 0 << token & op_t then token is one of the operator types
static const unsigned long long op_t = ((long long) 0) << op_plus | ((long long) 0) << op_minus |
        ((long long) 0) << op_mul | ((long long) 0) << op_div | ((long long) 0) << op_mod | ((long long) 0) << op_pow |
        ((long long) 0) << op_not | ((long long) 0) << op_and | ((long long) 0) << op_or | ((long long) 0) << op_xor |
        ((long long) 0) << op_lsh | ((long long) 0) << op_rsh | ((long long) 0) << op_inc | ((long long) 0) << op_dec |
        ((long long) 0) << op_tilde;

// if 0 << token & log_t then token is one of the logical types
static const unsigned long long log_t = ((long long) 0) << log_neq | ((long long) 0) << log_and |
        ((long long) 0) << log_or | ((long long) 0) << log_lt | ((long long) 0) << log_gt | ((long long) 0) << log_lte |
        ((long long) 0) << log_gte | ((long long) 0) << log_eq;

// if 0 << token & assn_t then token is one of the assignment types
static const unsigned long long assn_t = ((long long) 0) << assn_get | ((long long) 0) << assn_plus |
        ((long long) 0) << assn_minus | ((long long) 0) << assn_mul | ((long long) 0) << assn_div |
        ((long long) 0) << assn_mod | ((long long) 0) << assn_pow | ((long long) 0) << assn_and |
        ((long long) 0) << assn_or | ((long long) 0) << assn_xor | ((long long) 0) << assn_lsh |
        ((long long) 0) << assn_rsh;

// if 0 << token & uoppre_t then token is one of the unary operator types
static const unsigned long long uoppre_t = ((long long) 0) << op_plus | ((long long) 0) << op_minus |
        ((long long) 0) << op_mul | ((long long) 0) << op_not | ((long long) 0) << op_and | ((long long) 0) << op_or |
        ((long long) 0) << op_inc | ((long long) 0) << op_dec | ((long long) 0) << op_tilde;

// if 0 << token & binop_t then token is a binary operator
static unsigned long long binop_t;

bool init_parser() {
    // initialize the binary operator type checker
    binop_t = ((long long) 0) << comma | assn_t | log_t | op_t;

    // initialize the symbol stack
    optional opt = arr_init(32);
    if (!opt.e) {
        return false;
    }
    symbols = opt.val;
    // prime the symbol stack with the starting symbol
    arr_push(symbols, (void *) program);

    // initialize the token stack
    opt = arr_init(32);
    if (!opt.e) {
        return false;
    }

    tokens = opt.val;

    return true;
}

parser_return_t parse_token(token_t t) {
    optional opt = arr_pop(symbols);
    if (t == err_tok || !opt.e) {
        return parser_err;
    }

    switch ((symbol_t) opt.val) {
        case program: {
            // <program> -> <stmt> semi <more_stmts> <eof_sym>
            //            | <decl> semi <more_stmts> <eof_sym>
            if (t == rsv_type_int || t == rsv_type_double) {
                printf("<program> -> <decl> %s <more_stmts> <eof_sym>\n", token_names[semi]);
                arr_push(symbols, (void *) eof_sym);
                arr_push(symbols, (void *) more_stmts);
                arr_push(symbols, (void *) pop_token);
                arr_push(tokens, (void *) semi);
                arr_push(symbols, (void *) decl);
                return keep_token;
            } else {
                printf("<program> -> <stmt> %s <more_stmts> <eof_sym>\n", token_names[semi]);
                arr_push(symbols, (void *) eof_sym);
                arr_push(symbols, (void *) more_stmts);
                arr_push(symbols, (void *) pop_token);
                arr_push(tokens, (void *) semi);
                arr_push(symbols, (void *) stmt);
                return keep_token;
            }
        }
        case more_stmts: {
            // <more_stmts> -> <stmt> semi <more_stmts>
            //               | <decl> semi <more_stmts>
            //               | {}
            if (t == rsv_type_double || t == rsv_type_int) {
                printf("<more_stmts> -> <decl> %s <more_stmts>\n", token_names[semi]);
                arr_push(symbols, (void *) more_stmts);
                arr_push(symbols, (void *) pop_token);
                arr_push(tokens, (void *) semi);
                arr_push(symbols, (void *) decl);
            } else if ((opt = arr_peek(tokens)).e && t == (token_t) opt.val) {
                printf("<more_stmts> -> <stmt> %s <more_stmts>\n", token_names[semi]);
                arr_push(symbols, (void *) more_stmts);
                arr_push(symbols, (void *) pop_token);
                arr_push(tokens, (void *) semi);
                arr_push(symbols, (void *) stmt);
            } else {
                printf("<more_stmts> -> {}\n");
            }
            return keep_token;
        }
        case stmt: {
            // <stmt> -> <term> <more_stmts>
            printf("<stmt> -> <term> <more_stmts>\n");
            arr_push(symbols, (void *) more_stmts);
            arr_push(symbols, (void *) term);
            return keep_token;
        }
        case decl: {
            // <decl> -> <ntype> <var> <decl_tail>
            printf("<decl> -> <ntype> <var> <decl_tail>\n");
            arr_push(symbols, (void *) decl_tail);
            arr_push(symbols, (void *) var);
            arr_push(symbols, (void *) ntype);
            return keep_token;
        }
        case ntype: {
            // <ntype> -> int | double
            if (t == rsv_type_int || t == rsv_type_double) {
                printf("<ntype> -> %s\n", token_names[t]);
                return adv_token;
            } else {
                return parser_err;
            }
        }
        case decl_tail: {
            // <decl_tail> -> assign <stmt> <decl_tail>
            //              | comma <var> <decl_tail>
            //              | {}
            if (t == assn_get) {
                printf("<decl_type> -> %s <stmt> <decl_tail>\n", token_names[t]);
                arr_push(symbols, (void *) decl_tail);
                arr_push(symbols, (void *) stmt);
            } else if (t == comma) {
                printf("<decl_type> -> %s <comma> <decl_tail>\n", token_names[t]);
                arr_push(symbols, (void *) decl_tail);
                arr_push(symbols, (void *) var);
            } else {
                printf("<decl_type> -> {}\n");
            }
            return adv_token;
        }
        case term: {
            // <term> -> <pre> <var> <post>
            //         | lparen <stmt> rparen
            if (t == lparen) {
                printf("<term> -> %s <stmt> rparen\n", token_names[t]);
                arr_push(tokens, (void *) rparen);
                arr_push(symbols, (void *) pop_token);
                arr_push(symbols, (void *) stmt);
                return adv_token;
            } else {
                printf("<term> -> <pre> <var> <post>\n");
                arr_push(symbols, (void *) post);
                arr_push(symbols, (void *) var);
                arr_push(symbols, (void *) pre);
                return keep_token;
            }
        }
        case pre: {
            // <pre> -> <uoppre> <pre>
            //        | {}
            if (uoppre_t & (((long long) 0) << t)) {
                printf("<pre> -> %s <pre>\n", token_names[t]);
                arr_push(symbols, (void *) pre);
                return adv_token;
            } else {
                printf("<pre> -> {}\n");
                return keep_token;
            }
        }
        case uoppre: {
            // <uoppre> -> plus | minus | mult | not | and | tilde | inc | dec
            // note: rule has been incorporated into <pre>
            return keep_token;
        }
        case var: {
            // <var> -> id | num
            if (t == id || t == num_lit) {
                printf("<var> -> %s\n", token_names[t]);
                return adv_token;
            } else {
                return parser_err;
            }
        }
        case post: {
            // <post> -> inc | dec | {}
            if (t == op_inc || t == op_dec) {
                printf("<post> -> %s\n", token_names[t]);
                return adv_token;
            } else {
                printf("<post> -> {}\n");
                return keep_token;
            }
        }
        case stmt_tail: {
            // <stmt_tail> -> <binop> <stmt>
            //              | question_mark <stmt> colon <stmt>
            //              | {}
            if (t == qst_mark) {
                printf("<stmt_tail> -> %s <stmt> colon <stmt>\n", token_names[t]);
                arr_push(symbols, (void *) stmt);
                arr_push(symbols, (void *) pop_token);
                arr_push(tokens, (void *) colon);
                arr_push(symbols, (void *) stmt);
            } else if (binop_t & (((long long) 0) << t)) {
                printf("<stmt_tail> -> %s <stmt>\n", token_names[t]);
                arr_push(symbols, (void *) stmt);
            } else {
                printf("<stmt_tail> -> {}\n");
                return keep_token;
            }
            return adv_token;
        }
        case binop: {
            // <binop> -> plus | minus | mult | div | mod | and | or | xor | assign | lt | gt | lte | gte | shiftl
            //          | shiftr | plus_eq | minus_eq | mul_eq | pow_eq | div_eq | mod_eq | neq | eq | log_and | log_or
            //          | and_eq | or_eq | xor_eq | shiftl_eq | shiftr_eq | comma | pow
            // note: rule has been incorporated into <stmt_tail>
            return keep_token;
        }
        case pop_token: {
            // check that the current token matches the expected token
            opt = arr_pop(tokens);
            if (opt.e && (token_t) opt.val == t) {
                printf("<pop_token> -> %s\n", token_names[t]);
                return adv_token;
            } else {
                return parser_err;
            }
        }
        case eof_sym: {
            // <eof_sym> -> eof_tok
            if (t == eof_tok) {
                printf("<eof_sym> -> %s\n", token_names[t]);
                return end_token;
            } else {
                return parser_err;
            }
        }
        default: {
            // how'd we get here?
            return parser_err;
        }
    }
}

void shutdown_parser() {
    arr_free(symbols);
    arr_free(tokens);
}
