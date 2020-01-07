/*
 * schd_core.cpp
 *
 *  Description:
 *    Execution core
 */

#include <algorithm>
#include <iterator>
#include <utility>
#include <boost/foreach.hpp>
#include "schd_core.h"
#include "schd_assert.h"
#include "schd_report.h"
#include "schd_trace.h"

namespace schd {

SC_HAS_PROCESS( schd::schd_core_c );
schd_core_c::schd_core_c(
      sc_core::sc_module_name nm )
   : sc_core::sc_module( nm )
   , plan_ei( "plan_ei" )
   , plan_eo( "plan_eo" )
   , chn_core_plan( "chn_core_plan", chn_core_plan_size )
   , chn_plan_core( "chn_plan_core", chn_plan_core_size )
   , mux_plan_exec( "mux_plan_exec" )
   , mux_exec_plan( "mux_exec_plan" )
   , mux_cres_exec( "mux_cres_exec" )
   , mux_exec_cres( "mux_exec_cres" ) {

} // schd_core_c::schd_core_c(

void schd_core_c::init(
      boost::optional<const boost_pt::ptree&> _exec_p,
      boost::optional<const boost_pt::ptree&> _cres_p ) {

   boost_pt::ptree endpoint_pt;
   boost_pt::ptree exec_list_pt;
   boost_pt::ptree cres_list_pt;
   boost_pt::ptree plan_list_pt;

   endpoint_pt.clear();
   endpoint_pt.put( "name", "planner" );
   plan_list_pt.push_back( std::make_pair( "", endpoint_pt ));

   // Create exec instances and the corresponding channels
   BOOST_FOREACH( const boost_pt::ptree::value_type& exec_el, _exec_p.get()) {
      if( !exec_el.first.empty()) {
         SCHD_REPORT_ERROR( "schd::core" ) << name() <<  " Incorrect structure";
      }

      boost::optional<std::string> name_p = exec_el.second.get_optional<std::string>("name");

      if( !name_p.is_initialized() ) {
         SCHD_REPORT_ERROR( "schd::core" ) << name() <<  " Incorrect exec name";
      }
      else if( exec_list.find( name_p.get()) != exec_list.end()) {
         SCHD_REPORT_ERROR( "schd::core" ) << name() <<  " Duplicate exec name: " << name_p.get();
      }

      schd_exec_info_t exec_info;

      exec_info.idx    = std::distance( exec_list.begin(), exec_list.end() );
      exec_info.pref_p = boost::optional<const boost_pt::ptree&>( exec_el.second );

      schd_exec_c *exec_ptr = new schd_exec_c( name_p.get().c_str() );
      exec_info.mod_p  = boost::optional<schd_exec_c&>( *exec_ptr );

      exec_info.mod_p.get().init( exec_info.pref_p, _cres_p ); // Initialize exec

      sc_core::sc_fifo<schd_sig_ptree_c> *fifo_ptr;
      fifo_ptr = new sc_core::sc_fifo<schd_sig_ptree_c>(
            std::string( name_p.get() + "_plan_exec" ).c_str(),
            exec_info.chn_plan_exec_size );
      exec_info.chn_plan_exec_p = boost::optional<sc_core::sc_fifo<schd_sig_ptree_c>&>( *fifo_ptr );

      fifo_ptr = new sc_core::sc_fifo<schd_sig_ptree_c>(
            std::string( name_p.get() + "_exec_plan" ).c_str(),
            exec_info.chn_exec_plan_size );
      exec_info.chn_exec_plan_p = boost::optional<sc_core::sc_fifo<schd_sig_ptree_c>&>( *fifo_ptr );

      fifo_ptr = new sc_core::sc_fifo<schd_sig_ptree_c>(
            std::string( name_p.get() + "_cres_exec" ).c_str(),
            exec_info.chn_cres_exec_size );
      exec_info.chn_cres_exec_p = boost::optional<sc_core::sc_fifo<schd_sig_ptree_c>&>( *fifo_ptr );

      fifo_ptr = new sc_core::sc_fifo<schd_sig_ptree_c>(
            std::string( name_p.get() + "_exec_cres" ).c_str(),
            exec_info.chn_exec_cres_size );
      exec_info.chn_exec_cres_p = boost::optional<sc_core::sc_fifo<schd_sig_ptree_c>&>( *fifo_ptr );

      // Update list
      exec_list.emplace( std::make_pair( name_p.get(), exec_info ));

      endpoint_pt.clear();
      endpoint_pt.put( "name", name_p.get() );
      exec_list_pt.push_back( std::make_pair( "", endpoint_pt ));
   } // BOOST_FOREACH( const boost_pt::ptree::value_type& exec_el, _exec_p.get())

   // Create cres instances and the corresponding channels
   BOOST_FOREACH( const boost_pt::ptree::value_type& cres_el, _cres_p.get()) {
      if( !cres_el.first.empty()) {
         SCHD_REPORT_ERROR( "schd::core" ) << name() <<  " Incorrect structure";
      }

      boost::optional<std::string> name_p = cres_el.second.get_optional<std::string>("name");

      if( !name_p.is_initialized() ) {
         SCHD_REPORT_ERROR( "schd::core" ) << name() <<  " Incorrect cres name";
      }
      else if( cres_list.find( name_p.get()) != cres_list.end()) {
         SCHD_REPORT_ERROR( "schd::core" ) << name() <<  " Duplicate cres name: " << name_p.get();
      }

      schd_cres_info_t cres_info;

      cres_info.idx    = std::distance( cres_list.begin(), cres_list.end() );
      cres_info.pref_p = boost::optional<const boost_pt::ptree&>( cres_el.second );

      schd_cres_c *cres_ptr = new schd_cres_c( name_p.get().c_str() );
      cres_info.mod_p  = boost::optional<schd_cres_c&>( *cres_ptr );

      cres_info.mod_p.get().init( cres_info.pref_p, _exec_p ); // Initialize cres

      sc_core::sc_fifo<schd_sig_ptree_c> *fifo_ptr;
      fifo_ptr = new sc_core::sc_fifo<schd_sig_ptree_c>(
            std::string( name_p.get() + "_cres_exec" ).c_str(),
            cres_info.chn_cres_exec_size );
      cres_info.chn_cres_exec_p = boost::optional<sc_core::sc_fifo<schd_sig_ptree_c>&>( *fifo_ptr );

      fifo_ptr = new sc_core::sc_fifo<schd_sig_ptree_c>(
            std::string( name_p.get() + "_exec_cres" ).c_str(),
            cres_info.chn_exec_cres_size );
      cres_info.chn_exec_cres_p = boost::optional<sc_core::sc_fifo<schd_sig_ptree_c>&>( *fifo_ptr );

      // Update list
      cres_list.emplace( std::make_pair( name_p.get(), cres_info ));

      endpoint_pt.clear();
      endpoint_pt.put( "name", name_p.get() );
      cres_list_pt.push_back( std::make_pair( "", endpoint_pt ));
   } // BOOST_FOREACH( const boost_pt::ptree::value_type& cres_el, _cres_p.get())

   // Initilaize MUX instances
   boost_pt::ptree mux_pt;

   // plan -> exec
   mux_pt.clear();
   mux_pt.add_child( "src_list", plan_list_pt );
   mux_pt.add_child( "dst_list", exec_list_pt );
   mux_plan_exec.init( boost::optional<const boost_pt::ptree&>( mux_pt ));

   // exec -> plan
   mux_pt.clear();
   mux_pt.add_child( "src_list", exec_list_pt );
   mux_pt.add_child( "dst_list", plan_list_pt );
   mux_exec_plan.init( boost::optional<const boost_pt::ptree&>( mux_pt ));

   // cres -> exec
   mux_pt.clear();
   mux_pt.add_child( "src_list", cres_list_pt );
   mux_pt.add_child( "dst_list", exec_list_pt );
   mux_cres_exec.init( boost::optional<const boost_pt::ptree&>( mux_pt ));

   // exec -> cres
   mux_pt.clear();
   mux_pt.add_child( "src_list", exec_list_pt );
   mux_pt.add_child( "dst_list", cres_list_pt );
   mux_exec_cres.init( boost::optional<const boost_pt::ptree&>( mux_pt ));

   // Interconnect
   BOOST_FOREACH( const exec_list_t::value_type& exec_info, exec_list ) {
      // exec.plan_o -> chn_exec_plan -> mux_exec_plan.vi[]
      exec_info.second.mod_p.get().plan_o.bind( exec_info.second.chn_exec_plan_p.get() );
      mux_exec_plan.vi.at( exec_info.second.idx ).bind( exec_info.second.chn_exec_plan_p.get() );

      // mux_plan_exec.vo[] -> chn_plan_exec -> exec.plan_i
      exec_info.second.mod_p.get().plan_i.bind( exec_info.second.chn_plan_exec_p.get() );
      mux_plan_exec.vo.at( exec_info.second.idx ).bind( exec_info.second.chn_plan_exec_p.get() );

      // exec.cres_o -> chn_exec_cres -> mux_exec_cres.vi[]
      exec_info.second.mod_p.get().cres_o.bind( exec_info.second.chn_exec_cres_p.get() );
      mux_exec_cres.vi.at( exec_info.second.idx ).bind( exec_info.second.chn_exec_cres_p.get() );

      // mux_cres_exec.vo[] -> chn_cres_exec -> exec.cres_i
      exec_info.second.mod_p.get().cres_i.bind( exec_info.second.chn_cres_exec_p.get() );
      mux_cres_exec.vo.at( exec_info.second.idx ).bind( exec_info.second.chn_cres_exec_p.get() );

      //SCHD_REPORT_INFO( "schd::core" ) << name() <<  " Processed: " << exec_info.first;
   } // BOOST_FOREACH( const exec_list_t::value_type exec_el, exec_list )

   // core.plan_eo -> chn_plan_core -> mux_plan_exec.vi[0]
   plan_eo.bind( chn_plan_core );
   mux_plan_exec.vi.at( 0 ).bind( chn_plan_core );

   // mux_exec_plan.vo[0] -> chn_core_plan -> core.plan_ei
   plan_ei.bind( chn_core_plan );
   mux_exec_plan.vo.at( 0 ).bind( chn_core_plan );

   BOOST_FOREACH( const cres_list_t::value_type& cres_info, cres_list ) {
      // cres.exec_o -> chn_cres_exec -> mux_cres_exec.vi[]
      cres_info.second.mod_p.get().exec_o.bind( cres_info.second.chn_cres_exec_p.get() );
      mux_cres_exec.vi.at( cres_info.second.idx ).bind( cres_info.second.chn_cres_exec_p.get() );

      // mux_exec_cres.vo[] -> chn_exec_cres -> cres.exec_i
      cres_info.second.mod_p.get().exec_i.bind( cres_info.second.chn_exec_cres_p.get() );
      mux_exec_cres.vo.at( cres_info.second.idx ).bind( cres_info.second.chn_exec_cres_p.get() );

      //SCHD_REPORT_INFO( "schd::core" ) << name() <<  " Processed: " << cres_info.first;
   } // BOOST_FOREACH( const cres_list_t::value_type& cres_info, cres_list )

   // exec trace
   BOOST_FOREACH( const exec_list_t::value_type& exec_info, exec_list ) {
      exec_info.second.mod_p.get().add_trace( schd_trace.tf, name());
   }

   // cres trace
   BOOST_FOREACH( const cres_list_t::value_type& cres_info, cres_list ) {
      cres_info.second.mod_p.get().add_trace( schd_trace.tf, name());
   }
} // schd_core_c::init(

} // namespace schd
