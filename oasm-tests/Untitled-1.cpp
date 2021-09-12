

function main(arg1 : int, arg2 : string) : int
{
	if (arg1 > strlen(arg2))
	{
		print("Big");
		newLine();
		return 0xCACA;
	}
	else
	{
		print("Small");
		newLine();
		return 0xBABA;
	}
}


:__main__:
	.def arg1 = R29
	.def arg2 = R28
	.def _len = R27
	pop, Single_Reg, arg2
	pop, Single_Reg, arg1

	cmd, Const_Addr, StringLength, arg2
	mem, Reg_Reg, _len, R31

	cmp, Reg_Reg, arg1, _len
	jmp, Single_Const, JMP_LESS_OR_EQUALS, __else_0001__:


	:__else_0001__:


	:__end_if_0001__:
	.undef arg1
	.undef arg2
	.undef _len
	ret, Single_Const, 0xCACA
//end __main__