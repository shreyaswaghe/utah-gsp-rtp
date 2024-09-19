import numpy as np
import math

from collections import namedtuple

from Estimator import Estimator

class Event():
    def __init__(self, eventid, time):
        self.eventid : str = eventid
        self.time : float = time

    def update(self, time : float):
        self.time -= time 
        assert self.time >= 0

class EventList():
    def __init__(self):
        self.events = []

    def add(self, event):
        self.events.append(event)

    def cancel(self, eventid):
        to_remove = [event for event in self.events if event.eventid == eventid]
        for event in to_remove:
            self.events.remove(event)

    def peek(self):
        val = min(self.events, lambda event: event.time)
        return val
    
    def get(self):
        val = min(self.events, key=lambda event: event.time)
        self.events.remove(val)
        for event in self.events: event.update(val.time)
        return val
    
    def clear(self):
        self.events = []
    

class RunAndTumble():
    def __init__(
            self, 
            dim=1, 
            a=5, 
            b=5,
            rate_switch = 2,
            rate_reset = 2,
            start=0, 
            reset=0,
            speed=1,
            polarity = None,
            maxtime = 10000,
            seed = 123456
        ):


        ss = np.random.SeedSequence(123456)
        seeds = ss.spawn(10)

        self.rngstreams = {
            'initial_polarization': np.random.default_rng(seeds[0]),
            'reset' : np.random.default_rng(seeds[1]),
            'switch': np.random.default_rng(seeds[2])
        }

        def reset(self:RunAndTumble):
            if polarity is None:
                rng = self.rngstreams['intial_polarization']
                self.polarity = -1 if rng.random() < 0.5 else 1
            else:
                self.polarity = polarity
            
            self.pos = start
            self.time = 0.0

        self.reset = lambda self: reset(self) 

        self.dim, self.a, self.b, self.start, self.reset, self.speed = dim, a, b, start, reset, speed
        self.rate_switch = rate_switch
        self.rate_reset = rate_reset

        if polarity is None:
            rng = self.rngstreams['initial_polarization']
            self.polarity = -1 if rng.random() < 0.5 else 1
        else:
            self.polarity = polarity
        
        self.pos = start
        self.time = 0.0

        self.events = EventList()

        self.timeestimator = Estimator()
        self.hitestimator = Estimator()

    def trial(self):
        done = False

        while not done:
            prevpos = self.pos
            event = self.events.get()

            self.pos = prevpos + event.time * self.speed * self.polarity
            self.time += event.time

            match self.event.eventid:
                case "reset":
                    self.pos = self.start 
                    self.polarity = -1 if self.rngstreams['initial_polarization'].random() < 0.5 else 1
                    self.events.cancel('switch')

                    self._add_reset()
                
                case "switch":
                    self.polarity *= -1
                    self._add_switch()

            if self.pos <= -self.a or self.pos >= self.b:
                done = True
                self.timeestimator.process_next_val(self.time)
                self.hitestimator.process_next_val(-1 if self.pos < 0 else 1)

    def run(self, num_trials, test = False):
        for _ in range(num_trials):
            self.events.clear()
            self.events.add(Event('switch', 0.0))
            self.trial()

    def _add_switch(self):
        u = self.rngstreams['switch'].random()
        time = math.log(1 - u) / (- self.rate_switch)
        self.events.add(Event("switch", time))

    def _add_reset(self):
        u = self.rngstreams['reset'].random()
        time = math.log(1 - u) / (- self.rate_reset)
        self.events.add(Event("reset", time))

        self._add_switch()
            
                



    