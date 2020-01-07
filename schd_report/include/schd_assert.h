/*
 * schd_assert.h
 *
 *  Description:
 *    assert macro specific to simulation SystemC phase
 */

#ifndef SCHD_REPORT_INCLUDE_SCHD_ASSERT_H_
#define SCHD_REPORT_INCLUDE_SCHD_ASSERT_H_

#include <cassert>
#include <systemc>

#define schd_assert( expr ) sc_assert( expr );

#endif /* SCHD_REPORT_INCLUDE_SCHD_ASSERT_H_ */
