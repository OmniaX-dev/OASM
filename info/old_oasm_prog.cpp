MemAddress var_1 = heap_addr++;
			//MemAddress var_2 = heap_addr++;
			//MemAddress var_3 = heap_addr++;

			//----------------------------------------------------------------------------------------
			TMemoryList str = BitEditor::stringToConstSream("Hello World!");
			TMemoryList str2 = BitEditor::stringToConstSream("_oasm!!!");;
			proc.m_code = {

//-----CODE START-----
p_inst(req), 		p_flg(req_all), 											63,
p_inst(reserve), 	p_addr(SingleOp_reg),										p_reg(R0),
p_inst(reserve), 	p_addr(SingleOp_reg),										p_reg(R1),
p_inst(reserve), 	p_addr(SingleOp_reg),										p_reg(R2),

//NEXT CODE ADDR:   12

p_inst(lda_str),	str[0],str[1],str[2],str[3],str[4],str[5],str[6],
p_inst(mem), 		p_addr(RegToAddr),											var_1,									p_reg(R31),
p_inst(realloc),	p_addr(ConstToPtr),											var_1,									14,
p_inst(cmd),		p_addr(ConstAddr),											p_cmd(PrintStringToConsole),			var_1,
p_inst(cmd),		p_addr(ConstConst),											p_cmd(PrintNewLineToConsole),			0,

p_inst(add_str),	p_addr(ConstToAddr),										p_flg(add_str_const_stream),			var_1,			str2[0],str2[1],str2[2],str2[3],str2[4],
p_inst(cmd),		p_addr(ConstAddr),											p_cmd(PrintStringToConsole),			var_1,
p_inst(cmd),		p_addr(ConstConst),											p_cmd(PrintNewLineToConsole),			0,
p_inst(free),		p_addr(SingleOp_addr),										var_1,

p_inst(end), 		p_addr(SingleOp_const), 									0xCACA
//-----CODE END-------

			};
			//----------------------------------------------------------------------------------------

			//vm.getCPU().addBreakPoint(36);


/*.data = load_string(var_1, "Hello World!")

realloc,		Ptr_Const,			var_1,						14
cmd,			Const_Addr,			PrintStringToConsole,		var_1
cmd,			Const_Const,		PrintNewLineToConsole,		0
add_str,		Addr_Const,			ADD_CONST_STREAM_TO_STR,	var_1,			"_oasm!!!"
cmd,			Const_Addr,			PrintStringToConsole,		var_1
cmd,			Const_Const,		PrintNewLineToConsole,		0
free,			Single_Addr,		var_1*/