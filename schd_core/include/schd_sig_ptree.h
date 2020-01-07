/*
 * schd_sig_ptree.h
 *
 *  Description:
 *    Declaration of the signal which carries a generic property tree
 *    This signal is used as a universal transport between blocks
 */

#ifndef SCHD_CORE_INCLUDE_SCHD_SIG_PTREE_H_
#define SCHD_CORE_INCLUDE_SCHD_SIG_PTREE_H_

#include <string>
#include <iostream>
#include <boost/property_tree/ptree.hpp>
#include <systemc>

// Short alias for the namespace
namespace boost_pt = boost::property_tree;

namespace schd {
   class schd_sig_ptree_c;  // Forward declaration
} // namespace schd

namespace sc_core {

   void sc_trace(
         sc_core::sc_trace_file* tf,
         const schd::schd_sig_ptree_c& sig,
         const std::string& name );

} // namespace sc_core

namespace schd {
   std::ostream& operator << (
         std::ostream& os,
         const schd_sig_ptree_c& sig );

   class schd_sig_ptree_c {
   private:
      boost_pt::ptree data;
      std::size_t     data_hash = 0;

   public:
      // Required for the assignment operations
      schd_sig_ptree_c& set(
            const boost_pt::ptree& rhs ); // Set from property tree rhs

      schd_sig_ptree_c& set(
            const std::string& rhs );     // Set from json string rhs

      const boost_pt::ptree& get(
            void ) const;

      // Required by sc_signal<> and sc_fifo<>
      schd_sig_ptree_c& operator = (
            const schd_sig_ptree_c& rhs );

      // Required by sc_signal<>
      bool operator == (
            const schd_sig_ptree_c& rhs) const;

      friend void sc_core::sc_trace(
            sc_core::sc_trace_file* tf,
            const schd_sig_ptree_c& sig,
            const std::string& name );

      friend std::ostream& operator << (
            std::ostream& os,
            const schd_sig_ptree_c& sig );
   }; // class schd_sig_ptree_c
} // namespace schd

#endif /* SCHD_CORE_INCLUDE_SCHD_SIG_PTREE_H_ */
