README.txt


Assumptions we made

1) gold is infinitely dividable = we do not round up or down on the gold, we simply take the 1 minute = 1 gold so 1.23456 seconds = 1.23456 gold

2) we use the full time of 24 minutes = 1 day to calculate our time each team was busy and free
We add up all the values of time each character was inside the store, then divide it by k teams to get the average time each team was busy
We do not look at each team individually because we have no distinction between team 1 and team 2 due to how we used semaphores. 
I furthermore used 24m minus the time in the store to get the time the team was free.
I felt this was fine, while still answering the logicistical question

3) it was fine to print out separate visits as separate entries
Example, user 1 decides to come back, so within our table we print out our user 1's first visit and second visit separately


Once you have completed your solution, explain how your solution maximizes profits while not deprivingone side or the other of the costume department. Your description should include the output of an examplerun and an analysis of your synchronization scheme in the context of that run.  Save this in a text file,problem1explanation.txt, accompanying your code.
