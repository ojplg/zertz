#include <check.h>
#include <stdlib.h>
#include "../src/board.h"

START_TEST(test_init_board)
{
	Board *b = (Board*) malloc(sizeof (Board));

	init_board(b);

	int value = get_disc_contents(b,0,0);

	ck_assert_int_eq(2, value);

	free(b);
}
END_TEST

Suite * board_suite(void)
{
    Suite *s;
    TCase *tc_core;

    s = suite_create("Board");

    /* Core test case */
    tc_core = tcase_create("Core");

    tcase_add_test(tc_core, test_init_board);
    suite_add_tcase(s, tc_core);

    return s;
}

int main(void)
{
	int number_failed;
    Suite *s;
    SRunner *sr;

    s = board_suite();
    sr = srunner_create(s);

    srunner_run_all(sr, CK_NORMAL);
    number_failed = srunner_ntests_failed(sr);
    srunner_free(sr);
    return (number_failed == 0) ? EXIT_SUCCESS : EXIT_FAILURE;
}
