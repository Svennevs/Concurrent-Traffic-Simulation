#include <iostream>
#include <random>
#include "TrafficLight.h"

/* Implementation of class "MessageQueue" */

// FP.5a : The method receive should use std::unique_lock<std::mutex> and _condition.wait() 
// to wait for and receive new messages and pull them from the queue using move semantics. 
// The received object should then be returned by the receive function. 
// TrafficLightPhase MessageQueue::receive()
void MessageQueue::receive()
{
	// wait for _state to be modified to green state
    std::unique_lock<std::mutex> uLock(_mutex);
    _cond.wait(uLock, [this] { return _state == TrafficLightPhase::green; }); 
  	return;
}

// FP.4a : The method send should use the mechanisms std::lock_guard<std::mutex> 
// as well as _condition.notify_one() to add a new message to the queue and afterwards send a notification.
void MessageQueue::send(TrafficLightPhase &&msg)
{
	// perform operation under the lock
    std::lock_guard<std::mutex> uLock(_mutex);

    // change traffic light state
  	_state = std::move(msg);
    _cond.notify_one(); // notify client after changing traffic light state
}


/* Implementation of class "TrafficLight" */
TrafficLight::TrafficLight()
{
  std::lock_guard<std::mutex> lock(_mutex);
  _currentPhase = TrafficLightPhase::red;
}

void TrafficLight::waitForGreen()
{
    // FP.5b : add the implementation of the method waitForGreen, in which an infinite while-loop 
    // runs and repeatedly calls the receive function on the message queue. 
    // Once it receives TrafficLightPhase::green, the method returns.  

  	//Simplified, no while-loop needed because receive function waites for a green state
    _trafficLightPhaseQueue.receive();  //wait for traffic light to change to green
     return;
}

TrafficLightPhase TrafficLight::getCurrentPhase()
{
  	std::lock_guard<std::mutex> lock(_mutex);
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
    
  	// initalize variables
  	std::random_device generator;  //random number generator
  	std::uniform_int_distribution<long> distribution(4000,6000); //between 4000 and 6000 ms
      
  	long cycleDuration = distribution(generator);
    std::chrono::time_point<std::chrono::system_clock> lastUpdate;
	
    // init stop watch
    lastUpdate = std::chrono::system_clock::now();
  
  	while (true)
    {
        // sleep at every iteration to reduce CPU usage
        std::this_thread::sleep_for(std::chrono::milliseconds(1));

        // compute time difference to stop watch
        long timeSinceLastUpdate = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now() - lastUpdate).count();
        if (timeSinceLastUpdate >= cycleDuration)
        {
        	cycleDuration = distribution(generator); //change cycle duration each time
          
          	//toggle light
          	if(getCurrentPhase()==TrafficLightPhase::red)
            {  
            	std::unique_lock<std::mutex> lck(_mutex);
              	_currentPhase=TrafficLightPhase::green;
              	lck.unlock();
            }
            else
            {  
              	std::unique_lock<std::mutex> lck(_mutex); 
              	_currentPhase=TrafficLightPhase::red;
              	lck.unlock();
            }
                    
            //send update to message queue
          	_trafficLightPhaseQueue.send(std::move(_currentPhase));
    
          	 // reset stop watch for next cycle
            lastUpdate = std::chrono::system_clock::now();
        }
    }
}

