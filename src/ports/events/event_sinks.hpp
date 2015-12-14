#ifndef SRC_PORTS_EVENTS_EVENT_SINKS_HPP_
#define SRC_PORTS_EVENTS_EVENT_SINKS_HPP_

#include <cassert>
#include <functional>

#include <core/traits.hpp>
#include <core/connection.hpp>
#include "ports/detail/port_tags.hpp"
#include "ports/detail/port_traits.hpp"

namespace fc
{

/**
 * \brief minimal input port for events
 *
 * fulfills passive_sink
 * \tparam event_t type of event expected, must be copy_constructable
 */
template<class event_t>
struct event_in_port
{
	typedef typename detail::handle_type<event_t>::type handler_t;
	explicit event_in_port(const handler_t& handler) :
			event_handler(handler)
	{
		assert(event_handler);
	}

	void operator()(auto&& in_event) // universal ref here?
	{
		assert(event_handler);
		event_handler(std::move(in_event));
	}

	event_in_port() = delete;

	typedef void result_t;

private:
	handler_t event_handler;

};

/// specialisation of event_in_port with void , necessary since operator() has no parameter.
template<>
struct event_in_port<void>
{
	typedef typename detail::handle_type<void>::type handler_t;
	explicit event_in_port(handler_t handler) :
			event_handler(handler)
	{
		assert(event_handler);
	}

	void operator()()
	{
		assert(event_handler);
		event_handler();
	}
	event_in_port() = delete;
private:
	handler_t event_handler;
};

/**
 * \brief Templated event sink port
 *
 * Calls a (generic) lambda when an event is received. This allows to defer
 * the actual type of the token until the port is called and also allows it
 * to be called for diffent types.
 *
 * See test_events.cpp for example
 *
 * The IN_PORT_TMPL macro can be used to define a getter for the port and
 * a declaration for the node member to be called in one go.
 *
 * \tparam lambda_t Lambda to call when event arrived.
 */
template<class lambda_t>
struct event_in_port_tmpl
{
public:
	explicit event_in_port_tmpl(lambda_t h)
		: lambda(h)
	{}

	void operator()(auto&& in_event) // universal ref here?
	{
		lambda(std::move(in_event));
	}

	event_in_port_tmpl() = delete;

	typedef void result_t;

	lambda_t lambda;
};

/*
 * Helper needed for type inference
 */
template<class lambda_t>
auto make_event_in_port_tmpl(lambda_t h) { return event_in_port_tmpl<lambda_t>{h}; }

#define IN_PORT_TMPL_HELPER(NAME, FUNCTION) \
	auto NAME()	\
	{ return make_event_in_port_tmpl( [this](auto event){ this->FUNCTION(event); } ); } \
	template<class event_t> \
	void FUNCTION(const event_t& event)

#define IN_PORT_TMPL(NAME) IN_PORT_TMPL_HELPER( NAME, NAME##MEM_FUN )

// traits
template<class T> struct is_port<event_in_port<T>> : public std::true_type {};
template<class T> struct is_passive_sink<event_in_port<T>> : std::true_type {};
//template<class T, class U> struct is_passive_sink<event_in_tmpl<T, U>> : std::true_type {};
template<class T> struct is_port<event_in_port_tmpl<T>> : public std::true_type {};
template<class T> struct is_passive_sink<event_in_port_tmpl<T>> : std::true_type {};

} // namespace fc

#endif /* SRC_PORTS_EVENTS_EVENT_SINKS_HPP_ */
