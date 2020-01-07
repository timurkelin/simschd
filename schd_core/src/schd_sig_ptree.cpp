/*
 * schd_sig_ptree.cpp
 *
 *  Description:
 *    Class methods for the signal which carries a generic property tree
 *    This signal is used as a universal transport between blocks
 */

#include <boost/property_tree/json_parser.hpp>
#include <boost/property_tree/detail/file_parser_error.hpp>
#include <boost/container/map.hpp>
#include <boost/container/throw_exception.hpp>
#include "schd_sig_ptree.h"
#include "schd_conv_ptree.h"
#include "schd_hash_ptree.h"
#include "schd_report.h"

namespace boost_jp = boost::property_tree::json_parser;

namespace schd {

// Signal assignment (property tree)
schd_sig_ptree_c& schd_sig_ptree_c::set(
      const boost_pt::ptree& rhs ) {
   data = rhs;

   data_hash = 0;
   boost::hash_combine(
         data_hash,
         data );

   return *this;
}

// Signal assignment (json string)
schd_sig_ptree_c& schd_sig_ptree_c::set(
      const std::string& rhs ) {
   str2pt( rhs, data );

   data_hash = 0;
   boost::hash_combine(
         data_hash,
         data );

   return *this;
}

const boost_pt::ptree& schd_sig_ptree_c::get(
      void ) const {
   return data;
}

// Required by sc_signal<> and sc_fifo<>
schd_sig_ptree_c& schd_sig_ptree_c::operator = (
      const schd_sig_ptree_c& rhs ) {
   data = rhs.data;

   data_hash = 0;
   boost::hash_combine(
         data_hash,
         rhs.data );

   return *this;
}

// Required by sc_signal<>
bool schd_sig_ptree_c::operator == (
      const schd_sig_ptree_c& rhs) const {

   return ( data == rhs.data );
}

std::ostream& operator << (
      std::ostream& os,
      const schd::schd_sig_ptree_c& sig ) {

   try {
      boost_jp::write_json(
            os,
            sig.data );
   }
   catch( const boost_jp::json_parser_error& err ) {
      SCHD_REPORT_ERROR( "schd::ptree_sig" ) << err.what();
   }
   catch( ... ) {
      SCHD_REPORT_ERROR( "schd::ptree_sig" ) << "Unexpected";
   }

   return os;
}
} // namespace schd

namespace sc_core {
void sc_trace(
      sc_core::sc_trace_file* tf,
      const schd::schd_sig_ptree_c& sig,
      const std::string& name ) {

   // for property tree we trace only hash value to observe the changes
   sc_core::sc_trace(
         tf,
         sig.data_hash,
         name + ".hash" );
}
} // namespace sc_core
