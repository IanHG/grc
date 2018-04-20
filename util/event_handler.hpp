#ifndef EVENT_HANDLER_H_INCLUDED
#define EVENT_HANDLER_H_INCLUDED

#include <string>
#include <map> // for multimap
#include <functional>
#include <vector>
#include <iostream>


#include "../gui/debug.hpp"

// EV := event type    F := function type
template<class EV, class F> 
class event_handler;

// EV := event type    T := function return type     Ts... := function argument types
template<class EV, class T, class... Ts> 
class event_handler<EV,T(Ts...)>
{
   public:
      //! Define some types.
      using event_t     = EV;
      using uid_basic_t = int;
      using uid_t       = std::pair<event_t, uid_basic_t>;
      using function_t  = std::function<T(Ts...)>;
      using tuple_t     = std::tuple<function_t, uid_basic_t>;
      using event_map_t = std::multimap<event_t, tuple_t>;

      using deregister_list_t = std::list<uid_t>;
      using register_list_t   = std::list<std::tuple<event_t, function_t, uid_basic_t> >;
   
   private: 
      using mutex_t = std::recursive_mutex;

      //! Map of registered events.
      event_map_t m_function_map;

      //! Mutex
      mutex_t m_mutex;
      
      //!
      register_list_t m_register_list;

      //!
      deregister_list_t m_deregister_list;
      
      
      //! Generate unique ID for registered events.
      uid_basic_t generate_uid()
      {
         static uid_basic_t uid = 0;
         return uid++;
      }
      
      // Register function
      void register_function_impl(const event_t& event, const function_t& f, const uid_basic_t& uid)
      {
         debug::message("REGISTERING EVENT");
         std::lock_guard<mutex_t> lock(m_mutex);

         //uid_basic_t uid = this->generate_uid();
         m_function_map.insert( {event, std::forward_as_tuple(f, uid) } );
         //return {event, uid};
      }
      
      //!
      void deregister_function_impl(const uid_t& uid)
      {  
         debug::message("DEREGISTER EVENT");
         std::lock_guard<mutex_t> lock(m_mutex);
         debug::message("DEREGISTER AFTER LOCK");

         auto range = m_function_map.equal_range(std::get<0>(uid));
         
         debug::message("AM I EVEN HERE??");
         for(auto iter = range.first; iter != range.second; )
         {
            debug::message("WTF");
            if(std::get<1>(iter->second) == std::get<1>(uid))
            {
               iter = m_function_map.erase(iter);
               break;
            }
            else
            {
               ++iter;
            }
            debug::message("LOL");
         }

         debug::message("DONE DEREGISTERING EVENT");
      }

   public:
      /**
       * Default constructor.
       **/
      event_handler() = default;

      /**
       * Delete copy constructor and copy assignment.
       **/
      event_handler(const event_handler&) = delete;
      event_handler& operator=(const event_handler&) = delete;
      
      /**
       * Register function with event handler
       **/
      uid_t register_function(const event_t& event, const function_t& f)
      {
         debug::message("REGISTERING EVENT");
         std::lock_guard<mutex_t> lock(m_mutex);

         uid_basic_t uid = this->generate_uid();
         //m_function_map.insert( {event, std::forward_as_tuple(f, uid) } );
         m_register_list.emplace_back(std::forward_as_tuple(event, f, uid) );
         return {event, uid};
      }

      /**
       * Deregister event with event handler using uid generated when function was registered.
       **/
      void deregister_function(const uid_t& uid)
      {
         std::lock_guard<mutex_t> lock(m_mutex);

         m_deregister_list.emplace_back(uid);
      }
      
      /**
       * Handle an event.
       **/
      void handle_event(const event_t& event, Ts&&... ts)
      {
         std::lock_guard<mutex_t> lock(m_mutex);

         debug::message("HANDING EVENT");

         auto range = m_function_map.equal_range(event);
         for(auto iter = range.first; iter != range.second; ++iter)
         {
            std::get<0>(iter->second)(std::forward<Ts>(ts)...);
         }

         for(const auto& uid : m_deregister_list)
         {
            deregister_function_impl(uid);
         }
         m_deregister_list.clear();

         for(const auto& t : m_register_list)
         {
            register_function_impl(std::get<0>(t), std::get<1>(t), std::get<2>(t));
         }
         m_register_list.clear();
      }
};

template<class EVH>
class event_registerable
{
   private:
      using event_handler_t = EVH;
      using uid_t = typename event_handler_t::uid_t;
      using event_t = typename event_handler_t::event_t;
      using function_t = typename event_handler_t::function_t;

      std::vector<uid_t> m_registered_events;
   protected:
      event_registerable() = default;

   public:
      void register_function(event_handler_t& evh, const event_t& event, const function_t& f)
      {
         debug::message("register in event_registerable");
         m_registered_events.emplace_back(evh.register_function(event, f));
      }

      void deregister_functions(event_handler_t& evh)
      {
         debug::message("deregister in event_registerable");
         for(const auto& ev : m_registered_events)
         {
            evh.deregister_function(ev);
         }
         m_registered_events.clear();
      }
};

#endif /* EVENT_HANDLER_H_INCLUDED */
