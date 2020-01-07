/*
 * schd_pref.h
 *
 *  Description:
 *    Simulation preferences builder, reader and parser
 */


#ifndef SCHD_PREF_INCLUDE_SCHD_PREF_H_
#define SCHD_PREF_INCLUDE_SCHD_PREF_H_

#include <boost/property_tree/ptree.hpp>

#include <boost/optional/optional.hpp>

#include <systemc>

// Short alias for the namespace
namespace boost_pt = boost::property_tree;

namespace schd {
   class schd_pref_c {
   public:
      void load(
            const std::string& fname );

      void init(
            void );

      void parse(
            void );

      boost::optional<const boost_pt::ptree&> thrd_p;  // Threads
      boost::optional<const boost_pt::ptree&> task_p;  // Tasks / procedures
      boost::optional<const boost_pt::ptree&> exec_p;  // Executors
      boost::optional<const boost_pt::ptree&> cres_p;  // Common resources

      boost::optional<const boost_pt::ptree&> time_p;
      boost::optional<const boost_pt::ptree&> report_p;
      boost::optional<const boost_pt::ptree&> trace_p;
      boost::optional<const boost_pt::ptree&> dump_p;

   protected:
      boost_pt::ptree root;

   }; // class schd_pref_c

} // namespace schd

#endif /* SCHD_PREF_INCLUDE_SCHD_PREF_H_ */
