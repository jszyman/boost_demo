// boost_statechart.cpp : Defines the entry point for the console application.
//
#include <iostream>
#include <ctime>
#include <boost/statechart/transition.hpp>
#include <boost/statechart/event.hpp>
#include <boost/statechart/state_machine.hpp>
#include <boost/statechart/simple_state.hpp>


namespace sc = boost::statechart;

// We are declaring all types as structs only to avoid having to
// type public. If you don't mind doing so, you can just as well
// use class.

// We need to forward-declare the initial state because it can
// only be defined at a point where the state machine is
// defined.
struct Greeting;

// Boost.Statechart makes heavy use of the curiously recurring
// template pattern. The deriving class must always be passed as
// the first parameter to all base class templates.
//
// The state machine must be informed which state it has to
// enter when the machine is initiated. That's why Greeting is
// passed as the second template parameter.
struct Machine : sc::state_machine< Machine, Greeting > {};

// For each state we need to define which state machine it
// belongs to and where it is located in the statechart. Both is
// specified with Context argument that is passed to
// simple_state<>. For a flat state machine as we have it here,
// the context is always the state machine. Consequently,
// Machine must be passed as the second template parameter to
// Greeting's base (the Context parameter is explained in more
// detail in the next example).
struct Greeting : sc::simple_state< Greeting, Machine >
{
    // Whenever the state machine enters a state, it creates an
    // object of the corresponding state class. The object is then
    // kept alive as long as the machine remains in the state.
    // Finally, the object is destroyed when the state machine
    // exits the state. Therefore, a state entry action can be
    // defined by adding a constructor and a state exit action can
    // be defined by adding a destructor.
    Greeting() { std::cout << "Hello World!\n"; } // entry
    ~Greeting() { std::cout << "Bye Bye World!\n"; } // exit
};


struct IElapsedTime
{
    virtual double ElapsedTime() const = 0;
};

struct EvStartStop : sc::event< EvStartStop > {};
struct EvReset : sc::event< EvReset > {};

struct Active;
struct StopWatch : sc::state_machine< StopWatch, Active > 
{
    double ElapsedTime() const
    {
        return state_cast< const IElapsedTime & >().ElapsedTime();
    }
};

struct Stopped;

// The simple_state class template accepts up to four parameters:
// - The third parameter specifies the inner initial state, if
//   there is one. Here, only Active has inner states, which is
//   why it needs to pass its inner initial state Stopped to its
//   base
// - The fourth parameter specifies whether and what kind of
//   history is kept

// Active is the outermost state and therefore needs to pass the
// state machine class it belongs to
struct Active : sc::simple_state< Active, StopWatch, Stopped > 
{
public:
    typedef sc::transition< EvReset, Active > reactions;
    Active() : elapsedTime_(0.0) {}
    double ElapsedTime() const { return elapsedTime_; }
    double & ElapsedTime() { return elapsedTime_; }
private:
    double elapsedTime_;
};

// Stopped and Running both specify Active as their Context,
// which makes them nested inside Active
struct Running : IElapsedTime, sc::simple_state< Running, Active >
{
public:
    typedef sc::transition< EvStartStop, Stopped > reactions;
    Running() : startTime_(std::time(0)) {}
    ~Running()
    {
        context< Active >().ElapsedTime() = ElapsedTime();
    }

    virtual double ElapsedTime() const
    {
        return context< Active >().ElapsedTime() + std::difftime(std::time(0), startTime_);
    }
private:
    std::time_t startTime_;
};
struct Stopped : IElapsedTime, sc::simple_state< Stopped, Active >
{
    typedef sc::transition< EvStartStop, Running > reactions;
    
    virtual double ElapsedTime() const
    {
        return context< Active >().ElapsedTime();
    }
};

// Because the context of a state must be a complete type (i.e.
// not forward declared), a machine must be defined from
// "outside to inside". That is, we always start with the state
// machine, followed by outermost states, followed by the direct
// inner states of outermost states and so on. We can do so in a
// breadth-first or depth-first way or employ a mixture of the
// two.

int main()
{
    Machine myMachine;
    // The machine is not yet running after construction. We start
    // it by calling initiate(). This triggers the construction of
    // the initial state Greeting
    myMachine.initiate();
    // When we leave main(), myMachine is destructed what leads to
    // the destruction of all currently active states.

    StopWatch myWatch;
    myWatch.initiate();
    std::cout << myWatch.ElapsedTime() << "\n";
    myWatch.process_event(EvStartStop());
    std::cout << myWatch.ElapsedTime() << "\n";
    myWatch.process_event(EvStartStop());
    std::cout << myWatch.ElapsedTime() << "\n";
    myWatch.process_event(EvStartStop());
    std::cout << myWatch.ElapsedTime() << "\n";
    myWatch.process_event(EvReset());
    std::cout << myWatch.ElapsedTime() << "\n";

    return 0;
}