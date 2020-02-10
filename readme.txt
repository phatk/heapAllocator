File: readme.txt
Author: Phathaphol Karnchanapimonul
----------------------

implicit
--------
I create a struct that keeps track of the size and if it is free or not. The sizeof my struct is 8 so it is quite
efficient. My allocator is also quite efficient. In addition to operating normally, I implement splitting so that 
the leftover free space can be used by another malloc call. 

The strength of implementing splitting is that my utilization rate increases for pattern-mixed and pattern-coalesce.script.
A weakness is that in certain cases without the option of splitting, my utilization rate for pattern*.script actually 
decreases.

In addition, another important weakness is that my instruction count almost doubles for patter-mixed. Hence, I am
unsure if more test cases need to be done to determine if it is necessary to implement splitting in implicit.

Time-consuming anecdote :(
My journal to finish implicit is such a struggle. To know how to typecast a void* as a struct as ((Header*)cur_header)
took me a while. And to know how to access the field of a struct was similar. I spend a day doing this :(

explicit
--------
I create another struct that keeps track of the previous free header and next free header. The sizeof my struct is 8 
so it is similarly efficient. 

I, however, did not have time to implement the splitting in my explicit because of the lack of time. 

The advantage is that it is much faster than explicit.
The disadvantage is that it could coalesce too much. 

Tell us about your quarter in CS107!
-----------------------------------

CS107 has been a meaningful course. but a lot of work.

I really was stuck so many times with the last assignment. That is, the last assignment was clearly the hardest to do
for me.

