#include <check.h>
#include "../src/board.h"

int main(void)
{
    return 0;
}

START_TEST(test_init_board)
{
	Board *b = (Board*) malloc(sizeof (Board));

	init_board(b);

	int value = get_disc_contents(b,0,0);

	ck_assert_int_eq(2, value);

	free(b);
}
END_TEST

