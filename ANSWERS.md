1. The problems you found

Ans.: Because the Chat Messenger was already built it was only few changes and additions that had to be made in in terms of APIs. APIs where easy to understand as it was well documented. But, at some places I did go wrong. The problems faced were:

a) When I used ‘send’ and ‘receive’ functions with pulse audio APIs the output was appearing at the terminal but not as an audio. I could not figure out what was the error. If I had used ‘write’ and ‘read’ function instead of ‘send’ and ‘receive’ it worked fine.

b) There was an echo effect and noise when I tried loop-back and used internal mic.

c) There is a delay from when the audio is being sent from the client and being played by the server.


2. Your approach to debug it

Ans.:

a) To solve the error of why ‘send’ and ‘receive’ functions are not working I used a GDB debugger which gave exact function where the segmentation fault was occuring. This segmentation fault was because I was want declaring the array properly. There was array underflow and hence it could not play the audio and was giving segmentation fault. GDB was very useful. This also helped me learn how to use GDB and debug the error.

b) To avoid the echo effect and noise I used an external mic.

c) I’m still working on why delay is being produced.


3. How you had solved it

Ans.: The issue caused while using functions where solved using GDB debugger. Using debugger helped to find the error quickly and solve it.


4. What are the problems which are still not solved

Ans.: The delay that is caused between speaking and playing the audio is still not solved.


5. Improvements to make appliance better

Ans.: 

a) Solving problems with delay will give a real time effect for the appliance.

b) The audio signals while sending over the network should be compressed, so that it is easy to send when the audio size is less (not considering network of college).

