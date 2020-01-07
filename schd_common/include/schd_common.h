/*
 * schd_common.h
 *
 *  Description:
 *    Global include file
 */

#ifndef SCHD_COMMON_INCLUDE_SCHD_COMMON_H_
#define SCHD_COMMON_INCLUDE_SCHD_COMMON_H_

// System components
#include "schd_pref.h"
#include "schd_core.h"
#include "schd_planner.h"

// Simulation infrastructure
#include "schd_assert.h"
#include "schd_report.h"
#include "schd_trace.h"
#include "schd_time.h"
#include "schd_dump.h"

namespace schd {
   extern schd::schd_pref_c   schd_pref;
   extern schd::schd_report_c schd_report;
   extern schd::schd_time_c   schd_time;
   extern schd::schd_dump_c   schd_dump;
}

#endif /* SCHD_COMMON_INCLUDE_SCHD_COMMON_H_ */
