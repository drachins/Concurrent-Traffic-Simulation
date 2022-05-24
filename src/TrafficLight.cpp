#include <iostream>
#include <random>
#include <chrono>

#include "TrafficLight.h"

/* Implementation of class "MessageQueue" */


template <typename T>
T MessageQueue<T>::receive()
{
    // FP.5a : The method receive should use std::unique_lock<std::mutex> and _condition.wait() 
    // to wait for and receive new messages and pull them from the queue using move semantics. 
    // The received object should then be returned by the receive function. 

    // initialize unique lock
    std::unique_lock<std::mutex> rLock(_mutex);
    // loop while checking if new data is available
    _condition.wait(rLock, [this] {return !_queue.empty();});

    // load message into container
    T message = _queue.back();
    // remove message from queue
    _queue.pop_back();

    return message; 


}

template <typename T>
void MessageQueue<T>::send(T &&msg)
{
    // FP.4a : The method send should use the mechanisms std::lock_guard<std::mutex> 
    // as well as _condition.notify_one() to add a new message to the queue and afterwards send a notification.

    // initialize lock_guard
    std::lock_guard<std::mutex> sLock(_mutex);
    // send msg
    _queue.push_back(std::move(msg));
    // unblock thread
    _condition.notify_one();

}


/* Implementation of class "TrafficLight" */


TrafficLight::TrafficLight()
{
    _currentPhase = TrafficLightPhase::red;
}

TrafficLightPhase TrafficLight::waitForGreen()
{
    // FP.5b : add the implementation of the method waitForGreen, in which an infinite while-loop 
    // runs and repeatedly calls the receive function on the message queue. 
    // Once it receives TrafficLightPhase::green, the method returns.

    while(true)
    {
        // check if new data has been recieved
        TrafficLightPhase tlPhase = _trafficMsgs.receive();
        // returns if light is green 
        if(tlPhase == TrafficLightPhase::green)
            return tlPhase; 

    }


}


TrafficLightPhase TrafficLight::getCurrentPhase()
{
    return _currentPhase;
}


void TrafficLight::simulate()
{
    // FP.2b : Finally, the private method „cycleThroughPhases“ should be started in a thread when the public method „simulate“ is called. To do this, use the thread queue in the base class. 
    threads.emplace_back(std::thread(&TrafficLight::cycleThroughPhases, this));
}

// virtual function which is executed in a thread
void TrafficLight::cycleThroughPhases()
{
    // FP.2a : Implement the function with an infinite loop that measures the time between two loop cycles 
    // and toggles the current phase of the traffic light between red and green and sends an update method 
    // to the message queue using move semantics. The cycle duration should be a random value between 4 and 6 seconds. 
    // Also, the while-loop should use std::this_thread::sleep_for to wait 1ms between two cycles. 

    // Get random time duration between 4-6 seconds.
    std::default_random_engine phaseEngine;
    std::uniform_real_distribution<double> phaseDistro(4000,6000);
    double phaseDuration = phaseDistro(phaseEngine);

    // Get current time 
    std::chrono::time_point<std::chrono::system_clock> lastCycle = std::chrono::system_clock::now();
    while(true)
    {
        // Calculate time duration for current cycle
        long timeSinceLastCycle = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now() - lastCycle).count();
        if(timeSinceLastCycle >= phaseDuration)
        {
            // Toggle value of _currentPhase
            this->_currentPhase = (this->_currentPhase == TrafficLightPhase::red) ? TrafficLightPhase::green : TrafficLightPhase::red;

            //Send update to message queue. 
            _trafficMsgs.send(std::move(_currentPhase));

            // idle thread for 1 millisecond between cycles 
            std::this_thread::sleep_for(std::chrono::milliseconds(1));
            // reset cycle
            lastCycle = std::chrono::system_clock::now();
        }

        
    }



}

