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
} // schd_pref_c::load(

boost::optional<const boost_pt::ptree&> schd_pref_c::get_pref(
      const std::string& field_name,
      bool               check_error ) {

   const boost_pt::ptree& root_r = root;
   boost::optional<const boost_pt::ptree&> field_p = root_r.get_child_optional( field_name );

   if( check_error && !field_p.is_initialized()) {
      SCHD_REPORT_ERROR( "schd::pref" ) << "Missing <"
                                        << field_name
                                        << "> specification.";
   }

   return field_p;
} // schd_pref_c::get_pref(

void schd_pref_c::parse(
      void ) {
   bool check_error = true;

   thrd_p   = get_pref( "threads",   check_error );
   task_p   = get_pref( "tasks",     check_error );
   exec_p   = get_pref( "executors", check_error );
   cres_p   = get_pref( "common",    check_error );
   time_p   = get_pref( "time",      check_error );
   report_p = get_pref( "report",    check_error );
   trace_p  = get_pref( "trace",     check_error );
   dump_p   = get_pref( "dump",      check_error );
} // schd_pref_c::parse(

} // namespace schd
