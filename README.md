#[Ivory](http://ivory-next.github.io/Ivory-Interpreter/ "Ivory Homepage")

#####Ivory is an bytecode-based programming language#####
<br/><br/>
##Build & Test

###Linux?<br/>
First, download the source code;<br/>
Then, compile it:<br/>

    #cd into the source dir
    cd src
    
    #make it!
    make

**NOTE:**make clean to clean up the source code<br/><br/>
Then do the test:
    
    #cd into the test dir
    cd ../test
    
    #run
    ../src/bin/Ivory test_all/test.ivy

<br/>
If everything is OK, it will output like this:

	-->test start<--

	BLOCK1	110	hello, world!
	BLOCK2	990201
	BLOCK3	3.141500000000	2	0.000000	2	3.000000
	BLOCK4	20.000000000000	20.000000000000	20.000000000000	20.000000000000	20.000000000000
	BLOCK5	Yes!	1	2	2	3	3	3	4	4	4	4	it's false!
	BLOCK6	done!	7A6B8D013A88B8FACCB81BA98147706D
	BLOCK7	20	1223
	BLOCK8	28	28	28
	BLOCK9	INT_TYPE	DOUBLE_TYPE	LONG_DOUBLE_TYPE	OBJECT_TYPE
		STRING_TYPE	CLASS_TYPE	DELEGATE_TYPE		ENUM_TYPE
		NULL_TYPE	NATIVE_POINTER_TYPE
		20 is int --> true
		20.000000 is int --> false
		20.000000000000 is long double --> true
		"I'm a string" is string --> true
			typeof("I'm a string") is Type --> true
		pucha!
	BLOCK10	1.234420	11.000000	3.141593	333.000000	10.000000	100.000000	1000.000000

		Hello, this is Mr.Lin.
		I am very angry.
		I failed my test.
		Because of my careless.

	BLOCK11	1024	3072	7168
	
	-->test end<--
	press enter to exit...

Then, press enter to exit...

**NOTE:**Ivory for Windows is comming soon
<br/><br/>

##Hello, world!
<br/>
Create an file named hello.ivy(any name you want)
Add these code:

    /*using Ivory.lang;*/
    println("hello, world!");

Now, run it with the executable file which is compiled just now:

    ../src/bin/Ivory hello.ivy

hello, world!<br/>
**NOTE:** More details will be  in *IVORY_BOOK*

<br/><br/>
##Thanks to Diksam and its writer
>Ivory is based on the interpreter of another language called [Diksam](http://kmaebashi.com/programmer/devlang/diksam.html "Diksam")<br/>
>written by Maebasi Kazuya (前橋 和弥) ( http://kmaebashi.com )
