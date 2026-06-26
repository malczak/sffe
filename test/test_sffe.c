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

/* Parse and evaluate a real expression, asserting that the parse succeeds. */
static double eval_expr(const char *expr)
{
    sffe *parser = sffe_alloc();
    ck_assert_ptr_nonnull(parser);

    int rc = sffe_parse(&parser, expr);
    ck_assert_int_eq(rc, 0);

    double result = sffe_eval(parser);
    sffe_free(&parser);
    return result;
}

START_TEST(test_simple_precedence)
{
    /* Multiplication binds tighter than addition: 2 + (2*2) == 6. */
    ck_assert_double_eq_tol(eval_expr("2+2*2"), 6.0, 1e-9);
}
END_TEST

static Suite *sffe_suite(void)
{
    Suite *s = suite_create("sffe");

    TCase *tc_core = tcase_create("core");
    tcase_add_test(tc_core, test_simple_precedence);
    suite_add_tcase(s, tc_core);

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
