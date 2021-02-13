/*  Copyright (c) 2020, Karsten Ohme
 *  This file is part of GlobalPlatform.
 *
 *  GlobalPlatform is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU Lesser General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  GlobalPlatform is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public License
 *  along with GlobalPlatform.  If not, see <http://www.gnu.org/licenses/>.
 */
#include <stdlib.h>
#include <setjmp.h>
#include "util.h"
#include <stdarg.h>
#include <string.h>
#include <cmocka.h>

static void test_case1(void **state) {
	BYTE case1[] = { 0xA0, 0x01, 0x00, 0x00 };
	BYTE _case;
	DWORD lc, le;
	int result = parse_apdu_case(case1, sizeof(case1), &_case, &lc, &le);
	assert_int_equal(result, 0);
	assert_int_equal(_case, 1);
	assert_int_equal(lc, 0);
	assert_int_equal(le, 0);
}

static void test_case2(void **state) {
	BYTE case2[] = { 0xA0, 0x01, 0x00, 0x00, 0x01 };
	BYTE _case;
	DWORD lc, le;
	int result = parse_apdu_case(case2, sizeof(case2), &_case, &lc, &le);
	assert_int_equal(result, 0);
	assert_int_equal(_case, 2);
	assert_int_equal(lc, 0);
	assert_int_equal(le, 1);
}

static void test_case3(void **state) {
	BYTE case3[] = { 0xA0, 0x01, 0x00, 0x00, 0x02, 1, 2 };
	BYTE _case;
	DWORD lc, le;
	int result = parse_apdu_case(case3, sizeof(case3), &_case, &lc, &le);
	assert_int_equal(result, 0);
	assert_int_equal(_case, 3);
	assert_int_equal(lc, 2);
	assert_int_equal(le, 0);
}


static void test_case4(void **state) {
	BYTE case4[] = { 0xA0, 0x01, 0x00, 0x00, 0x02, 1, 2, 5 };
	BYTE _case;
	DWORD lc, le;
	int result = parse_apdu_case(case4, sizeof(case4), &_case, &lc, &le);
	assert_int_equal(result, 0);
	assert_int_equal(_case, 4);
	assert_int_equal(lc, 2);
	assert_int_equal(le, 5);
}


static void test_case2e(void **state) {
	BYTE case2[] = { 0xA0, 0x01, 0x00, 0x00, 0x00, 0x01, 0x01 };
	BYTE _case;
	DWORD lc, le;
	int result = parse_apdu_case(case2, sizeof(case2), &_case, &lc, &le);
	assert_int_equal(result, 0);
	assert_int_equal(_case, 0x2e);
	assert_int_equal(lc, 0);
	assert_int_equal(le, 257);
}

static void test_case3e(void **state) {
	BYTE case3[520] = { 0 };
	BYTE _case;
	DWORD lc, le;
	case3[5] = 2;
	case3[6] = 1;
	int result = parse_apdu_case(case3, sizeof(case3), &_case, &lc, &le);
	assert_int_equal(result, 0);
	assert_int_equal(_case, 0x3e);
	assert_int_equal(lc, 513);
	assert_int_equal(le, 0);
}


static void test_case4e(void **state) {
	BYTE case4[522] = { 0 };
	BYTE _case;
	DWORD lc, le;
	case4[5] = 2;
	case4[6] = 1;
	case4[sizeof(case4)-2] = 1;
	case4[sizeof(case4)-1] = 2;
	int result = parse_apdu_case(case4, sizeof(case4), &_case, &lc, &le);
	assert_int_equal(result, 0);
	assert_int_equal(_case, 0x4e);
	assert_int_equal(lc, 513);
	assert_int_equal(le, 258);
}


static int setup(void **state) {
	return 0;
}

int main(void) {
	const struct CMUnitTest tests[] = {
			cmocka_unit_test(test_case1),
			cmocka_unit_test(test_case2),
			cmocka_unit_test(test_case3),
			cmocka_unit_test(test_case4),
			cmocka_unit_test(test_case2e),
			cmocka_unit_test(test_case3e),
			cmocka_unit_test(test_case4e)
	};
	return cmocka_run_group_tests_name("util", tests, setup, NULL);
}
