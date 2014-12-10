#Ivory

#####Ivory is an programming language which can compile JIT to Ivory bytecode and run.#####
<br/><br/>
##Build & Test

###Linux?<br/>
First, download the source code;<br/>
Then, compile it:<br/>

    #cd into the source dir
    cd src/Main
    
    #make it!
    make

<br/>
Then do the test:
    
    #cd into the test dir
    cd ../../test
    
    #run
    ../src/Main/Ivory test_all/test.ivy

<br/>
If everything is OK, it will output like this:

    -->test start<--

    BLOCK1    110	hello, world!
    BLOCK2	990201
    BLOCK3	3.141500000000	2	0.000000	2	3.000000
    BLOCK4	20.000000000000	20.000000000000	20.000000000000	20.000000000000	20.000000000000
    BLOCK5	Yes!	1	2	2	3	3	3	4	4	4	4	it's false!
    BLOCK6	3	hello, world!	3.141592654000	2	me!	129.000000
    BLOCK7	20
    
    -->test end<--
    press enter to exit...

Then, press enter to exit...

**NOTE:**Ivory in Windows is comming soon
<br/><br/>

##Hello, world!
<br/>
Create an file named hellooo.ivy(any name you want)
Add these code:

    using Ivory.lang;
    println("hello, world!");

Now, run it with the executable file which is compiled just now:

    ../src/Main/Ivory hellooo.ivy

hello, world!<br/>
**NOTE:** More details will be  in *IVORY_BOOK**=

<br/><br/>
##Special Thanks
>Based on another self-made programming language [Diksam](https://github.com/zeyuanxy/Diksam "Diksam")<br/>
>written by Zeyuanxy( https://github.com/zeyuanxy )
