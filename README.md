#Ivory

Ivory is a static-typing programming language with grammar similar to Java<br><br>
This implmentation is majorly based on the source code of compiler & vm of a very similar language called Diksam offered in the book *自制编程语言* written by [Kazuya Maebasi](http://kmaebashi.com)

The extension of the original code mainly includes the try to make it more dynamic(mainly in grammar)

**NOTE**: this project is not developing anymore

##build

    cd src
    make

##test
    
    cd ../test
    ../src/bin/Ivory test_all/test.ivy

<br/>
If everything is OK, the output will be like this:

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
