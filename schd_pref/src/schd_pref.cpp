/*
 * schd_pref.cpp
 *
 *  Description:
 *    Reader and initial parser of the simulation preferences
 */

#include "schd_pref.h"

#include <string>
#include <boost/property_tree/json_parser.hpp>
#include <boost/property_tree/detail/file_parser_error.hpp>
#include "schd_report.h"

namespace boost_jp = boost::property_tree::json_parser;

namespace schd {
void schd_pref_c::load(
      const std::string& fname  ) {

   try {
      boost_pt::read_json(
            fname,
            root );
   }
   catch( const boost_jp::json_parser_error& err ) {
      SCHD_REPORT_ERROR( "schd::pref" ) << err.what();
   }
   catch( ... ) {
      SCHD_REPORT_ERROR( "schd::pref" ) << "Unexpected";
   }
}

void schd_pref_c::parse(
      void ) {

   thrd_p = root.get_child_optional( "threads" );
   if( !thrd_p.is_initialized() ) {
      SCHD_REPORT_ERROR( "schd::pref" ) << "Missing <threads> specification.";
   }

   task_p = root.get_child_optional( "tasks" );
   if( !task_p.is_initialized() ) {
      SCHD_REPORT_ERROR( "schd::pref" ) << "Missing <tasks> specification.";
   }

   exec_p = root.get_child_optional( "executors" );
   if( !exec_p.is_initialized() ) {
      SCHD_REPORT_ERROR( "schd::pref" ) << "Missing <executors> specification.";
   }

   cres_p = root.get_child_optional( "common" );
   if( !cres_p.is_initialized() ) {
      SCHD_REPORT_ERROR( "schd::pref" ) << "Missing <common> specification.";
   }

   time_p   = root.get_child_optional( "time" );
   if( !time_p.is_initialized() ) {
      SCHD_REPORT_ERROR( "schd::pref" ) << "Missing <time> specification.";
   }

   report_p = root.get_child_optional( "report" );
   if( !report_p.is_initialized() ) {
      SCHD_REPORT_INFO( "schd::pref" ) << "Missing <report> specification.";
   }

   trace_p  = root.get_child_optional( "trace" );
   if( !trace_p.is_initialized() ) {
      SCHD_REPORT_INFO( "schd::pref" ) << "Missing <trace> specification.";
   }

   dump_p  = root.get_child_optional( "dump" );
   if( !dump_p.is_initialized() ) {
      SCHD_REPORT_INFO( "schd::pref" ) << "Missing <dump> specification.";
   }
}

} // namespace schd
