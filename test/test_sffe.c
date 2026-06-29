/* Unit tests for the sffe real (double) back-end.
 *
 * Uses Check (https://libcheck.github.io/check/), a widely used unit-testing
 * framework for C. Run with `make test` locally, or inside the Alpine image:
 *
 *     docker compose run --rm test
 */
#include <check.h>
#include <stdlib.h>

#include "sffe.h"

#define EPS 1e-9

/* Parse and evaluate a real expression, asserting that the parse succeeds. */
static double eval_expr(const char *expr)
{
    sffe *parser = sffe_alloc();
    ck_assert_ptr_nonnull(parser);

    int rc = sffe_parse(&parser, expr);
    ck_assert_msg(rc == 0, "parse failed (rc=%d) for \"%s\": %s",
                  rc, expr, parser->errormsg ? parser->errormsg : "(no message)");

    double result = sffe_eval(parser);
    sffe_free(&parser);
    return result;
}

/* Convenience: assert that `expr` evaluates to `expected`. */
#define ck_expr(expr, expected) \
    ck_assert_double_eq_tol(eval_expr(expr), (expected), EPS)

START_TEST(test_simple_precedence)
{
    /* Multiplication binds tighter than addition: 2 + (2*2) == 6. */
    ck_expr("2+2*2", 6.0);
}
END_TEST

/* ---- Nested brackets and `N*(...)` grouping ---------------------------- */

START_TEST(test_brackets_basic)
{
    ck_expr("3*(1+2)", 9.0);
    ck_expr("(1+2)*3", 9.0);
    ck_expr("3*(2+2*2)", 18.0);     /* precedence preserved inside group */
    ck_expr("(1+2)*(3+4)", 21.0);
    ck_expr("2+(3)", 5.0);
    ck_expr("(2)^(3)", 8.0);
    ck_expr("3*(1+2)^2", 27.0);     /* ^ applies to the group, then * */
    ck_expr("3*(1+2)*4", 36.0);
    ck_expr("100/(2*5)", 10.0);
}
END_TEST

START_TEST(test_brackets_nested)
{
    ck_expr("((1+2))", 3.0);
    ck_expr("(((5)))", 5.0);
    ck_expr("2*(3*(4+1))", 30.0);
    ck_expr("3*((1+2)+(3+4))", 30.0);
    ck_expr("((2+3)*(4))", 20.0);
    ck_expr("2*(3+4*(5+6))", 94.0);
    ck_expr("1+2*(3+4*(5))-6", 41.0);
    ck_expr("3*(((1+1)))", 6.0);
}
END_TEST

START_TEST(test_brackets_with_spaces)
{
    /* Phase 1 strips spaces, so layout must not matter. */
    ck_expr("3 * ( 1 + 2 )", 9.0);
    ck_expr("  ( 1 + 2 ) * 3  ", 9.0);
}
END_TEST

/* ---- Implicit (inferred) multiplication -------------------------------- *
 *
 * Contrary to the suspicion that `3(...)` isn't supported, the tokenizer
 * DOES inject a `*` for adjacency cases: n(, )n, )(, nf, )f.  These all pass.
 */

START_TEST(test_implicit_mul_number_bracket)
{
    ck_expr("3(1+2)", 9.0);     /* n ( */
    ck_expr("(1+2)3", 9.0);     /* ) n */
    ck_expr("2(3)", 6.0);
    ck_expr("10(2)", 20.0);
    ck_expr("1.5(2)", 3.0);
}
END_TEST

START_TEST(test_implicit_mul_bracket_bracket)
{
    ck_expr("(1+2)(3+4)", 21.0);   /* ) ( */
    ck_expr("(2+3)(4+5)", 45.0);
    ck_expr("(1)(2)(3)", 6.0);     /* chained */
    ck_expr("2(3)(4)", 24.0);
    ck_expr("100/(2)(5)", 250.0);  /* ( (100/2) * 5 ) per left-to-right */
}
END_TEST

START_TEST(test_implicit_mul_with_functions)
{
    ck_expr("2cos(0)", 2.0);        /* n f */
    ck_expr("cos(0)2", 2.0);        /* ) n */
    ck_expr("cos(0)cos(0)", 1.0);   /* ) f */
    ck_expr("sin(0)+2(3)", 6.0);
    ck_expr("4(1+1)^2", 16.0);
}
END_TEST

START_TEST(test_implicit_mul_constants)
{
    /* `2pi` -> 2 * PI; dividing back out isolates the implicit factor. */
    ck_expr("2pi/pi", 2.0);
    ck_expr("2*pi/pi", 2.0);
}
END_TEST

/* ---- Unary minus in front of a group ----------------------------------- */

START_TEST(test_unary_minus_group_ok)
{
    /* These cases happen to come out right because the surrounding operator
     * is + - or * (precedence <= the implicit multiply). */
    ck_expr("-(1+2)", -3.0);
    ck_expr("-(-3)", 3.0);
    ck_expr("3*-(1+2)", -9.0);
    ck_expr("-3*(1+2)", -9.0);
    ck_expr("(1+2)*-3", -9.0);
    ck_expr("2*-(1+1)", -4.0);
    ck_expr("2+-(1+1)", 0.0);
    /* Explicitly parenthesising the negation always works: */
    ck_expr("2^(-(1+1))", 0.25);
    ck_expr("(-(1+1))^2", 4.0);
}
END_TEST

/*
 * Regression: a unary minus applied to a bracketed group used to be emitted
 * as the literal number -1 followed by an *implicit* multiplication, i.e.
 * "-(a)" -> "-1 * (a)". That was harmless under +, -, * but wrong under ^ and
 * / (which bind tighter / are left-associative at equal precedence):
 *
 *     2^-(1+1)  was  (2^-1) * (1+1)  = 1     -- now 2^-2 = 0.25
 *     2/-(1+1)  was  (2/-1) * (1+1)  = -4    -- now 2/-2 = -1
 *
 * The tokenizer now binds an operator-preceded '-(...)' as a tight negation
 * function, so these evaluate to the mathematically correct values.
 */
START_TEST(test_unary_minus_group_tight_binding)
{
    ck_expr("2^-(1+1)", 0.25);     /* 2^-2 */
    ck_expr("2^-(2)",   0.25);     /* 2^-2 */
    ck_expr("4^-(2)",   0.0625);   /* 4^-2 */
    ck_expr("2/-(1+1)", -1.0);     /* 2 / -2 */
    ck_expr("8/-(2)",   -4.0);     /* 8 / -2 */
    ck_expr("3^-(1)",   1.0 / 3.0);
    /* The leading-minus precedence convention is preserved: -(a)^n == -(a^n) */
    ck_expr("-(1+1)^2", -4.0);
    ck_expr("(-(1+1))^2", 4.0);
}
END_TEST

static Suite *sffe_suite(void)
{
    Suite *s = suite_create("sffe");

    TCase *tc_core = tcase_create("core");
    tcase_add_test(tc_core, test_simple_precedence);
    suite_add_tcase(s, tc_core);

    TCase *tc_brackets = tcase_create("brackets");
    tcase_add_test(tc_brackets, test_brackets_basic);
    tcase_add_test(tc_brackets, test_brackets_nested);
    tcase_add_test(tc_brackets, test_brackets_with_spaces);
    suite_add_tcase(s, tc_brackets);

    TCase *tc_implicit = tcase_create("implicit_mul");
    tcase_add_test(tc_implicit, test_implicit_mul_number_bracket);
    tcase_add_test(tc_implicit, test_implicit_mul_bracket_bracket);
    tcase_add_test(tc_implicit, test_implicit_mul_with_functions);
    tcase_add_test(tc_implicit, test_implicit_mul_constants);
    suite_add_tcase(s, tc_implicit);

    TCase *tc_unary = tcase_create("unary_minus");
    tcase_add_test(tc_unary, test_unary_minus_group_ok);
    tcase_add_test(tc_unary, test_unary_minus_group_tight_binding);
    suite_add_tcase(s, tc_unary);

    return s;
}

int main(void)
{
    Suite *s = sffe_suite();
    SRunner *sr = srunner_create(s);

    srunner_run_all(sr, CK_NORMAL);
    int failed = srunner_ntests_failed(sr);
    srunner_free(sr);

    return (failed == 0) ? EXIT_SUCCESS : EXIT_FAILURE;
}
