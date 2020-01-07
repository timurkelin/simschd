#include "schd_common.h"

int sc_main(
   int argc,
   char *argv[] ) {

   // Check command line arguments
   if( argc == 2 ) {
      SCHD_REPORT_INFO( "schd::cmdline" ) << "Preferences file: " << argv[1];

      schd::schd_pref.load(
            std::string( argv[1] ));
   }
   else {
      SCHD_REPORT_ERROR( "schd::cmdline" ) << "Incorrect command line arguments";
   }

   schd::schd_pref.parse();

   schd::schd_report.init(
         schd::schd_pref.report_p );

   schd::schd_time.init(
         schd::schd_pref.time_p );

   sc_core::sc_set_time_resolution(
         schd::schd_time.res_sec,
         sc_core::SC_SEC );

   schd::schd_trace.init(
         schd::schd_pref.trace_p );

   schd::schd_trace.save_map(
         schd::schd_pref.thrd_p );

   // Top-level connections
   schd::schd_core_c    core_i0(
         "core" );
   core_i0.init(
         schd::schd_pref.exec_p,
         schd::schd_pref.cres_p );

   schd::schd_planner_c plan_i0(
         "planner" );
   plan_i0.init(
         schd::schd_pref.thrd_p,
         schd::schd_pref.task_p,
         schd::schd_pref.exec_p );

   plan_i0.core_i.bind(
         core_i0.plan_ei );

   plan_i0.core_o.bind(
         core_i0.plan_eo );

   // Init data dump class
   schd::schd_dump.init(
         schd::schd_pref.dump_p );

   // Invoke the simulation
   if( schd::schd_time.end_sec != 0.0 ) {
      sc_core::sc_start(
            schd::schd_time.end_sec,
            sc_core::SC_SEC );
   }
   else {
      sc_core::sc_start();
   }

   SCHD_REPORT_INFO( "schd::main" ) << "Done.";

   // Ensure that all the dump files are closed before exiting
   schd::schd_dump.close_all();

   return 0;
}
