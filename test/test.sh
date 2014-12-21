../Ivory test.ivy > test_.result 2>&1
../Ivory array.ivy > array_.result 2>&1
../Ivory class01.ivy > class01_.result 2>&1
../Ivory class02.ivy > class02_.result 2>&1
../Ivory class03.ivy > class03_.result 2>&1
../Ivory method.ivy > method_.result 2>&1
../Ivory cast.ivy > cast_.result 2>&1
../Ivory classmain.ivy > classmain_.result 2>&1
../Ivory downcast.ivy > downcast_.result 2>&1
../Ivory instanceof.ivy > instanceof_.result 2>&1
../Ivory super.ivy > super_.result 2>&1
../Ivory exception.ivy > exception_.result 2>&1
../Ivory shapemain.ivy > shapemain_.result 2>&1
../Ivory throws.ivy > throws_.result 2>&1
../Ivory nullpointer.ivy > nullpointer_.result 2>&1
../Ivory array_ex.ivy > array_ex_.result 2>&1
../Ivory else_ex.ivy > else_ex_.result 2>&1
../Ivory native.ivy
../Ivory switch.ivy > switch_.result 2>&1
../Ivory final.ivy > final_.result 2>&1
../Ivory do_while.ivy > do_while_.result 2>&1
../Ivory enum.ivy > enum_.result 2>&1
../Ivory delegate.ivy > delegate_.result 2>&1
../Ivory rename.ivy > rename_.result 2>&1

echo "test"
diff test.result test_.result
echo "array"
diff array.result array_.result
echo "class01"
diff class01.result class01_.result
echo "class02"
diff class02.result class02_.result
echo "class03"
diff class03.result class03_.result
echo "method"
diff method.result method_.result
echo "cast"
diff cast.result cast_.result
echo "classmain"
diff classmain.result classmain_.result
echo "downcast"
diff downcast.result downcast_.result
echo "instanceof"
diff instanceof.result instanceof_.result
echo "super"
diff super.result super_.result
echo "exception"
diff exception.result exception_.result
echo "shapemain"
diff shapemain.result shapemain_.result
echo "throws"
diff throws.result throws_.result
echo "nullpointer"
diff nullpointer.result nullpointer_.result
echo "array_ex"
diff array_ex.result array_ex_.result
echo "else_ex"
diff else_ex.result else_ex_.result
echo "test"
diff test.ivy test.copy
echo "switch"
diff switch.result switch_.result
echo "final"
diff final.result final_.result
echo "do_while"
diff do_while.result do_while_.result
echo "enum"
diff enum.result enum_.result
echo "delegate"
diff delegate.result delegate_.result
echo "rename"
diff rename.result rename_.result
