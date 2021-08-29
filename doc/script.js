$(function(){

// ------------------------------------------Data------------------------------------------
	var IOps = {
		"Addr": "<p class=\"addr\">addr</p>",
		"IAddr": "<p class=\"i-addr\">[addr]</p>",
		"Reg": "<p class=\"reg\">reg</p>",
		"IReg": "<p class=\"i-reg\">[reg]</p>",
		"ConstVal": "<p class=\"value\">const</p>",
		"Ref": "<p class=\"ref\">&addr</p>",
		"ConstStream": "<p class=\"value\">c_stream</p>",
		"NoOperand": "---"
	};
	var InstructionTemplate = {
		"ID": "%__ID__%",
		"Instruction": "%_INST_NAME_%",
		"OpCode": "%_OP_CODE_%",
		"AddressingMode": "%_ADDRESSING_%",
		"Flags": "%_FLAGS_%",
		"Operator1": "%_OP1_%",
		"Operator2": "%_OP2_%",
		"Description": "%_DESCRIPTION_%",
		"Extra": "%_EXTRA_%"
	};
	var AddrModetemplate = {
		"ID": "%__ID__%",
		"Mode": "%_ADDR_MODE_%",
		"Operands": "%_OPERANDS_%",
		"ShortDescription": "%_SHORT_DESCRIPTION_%",
		"Description": "%_DESCRIPTION_%"
	};
	var FlagsTemplate = {
		"ID": "%__ID__%",
		"Flags": "%_FLAGS_%",
		"Hex": "%_HEX_%",
		"ShortDescription": "%_SHORT_DESCRIPTION_%",
		"Instruction": "%_INSTRUCTION_%",
		"Description": "%_DESCRIPTION_%"
	};
	var inst_template = "\
		<div class=\"hover-row-container\"><div class=\"table-row hover-row\"> \
			<div class=\"table-element inst-elem-id\">" + InstructionTemplate.ID + "</div> \
			<div class=\"table-element inst-elem-inst\">" + InstructionTemplate.Instruction + "</div> \
			<div class=\"table-element inst-elem-opcode\">(" + InstructionTemplate.OpCode + ")</div> \
			<div class=\"table-element inst-elem-addressing\">" + InstructionTemplate.AddressingMode + "</div> \
			<div class=\"table-element inst-elem-flags\">" + InstructionTemplate.Flags + "</div> \
			<div class=\"table-element inst-elem-op1\">" + InstructionTemplate.Operator1 + "</div> \
			<div class=\"table-element inst-elem-op2\">" + InstructionTemplate.Operator2 + "</div> \
			<div class=\"table-element inst-elem-extra\">" + InstructionTemplate.Extra + "</div> \
		</div> \
		<div class=\"table-row desc-row\"> \
			<div class=\"table-element\">" + InstructionTemplate.Description + "</div> \
		</div></div> \
	";
	var addr_mode_template = "\
		<div class=\"hover-row-container\"><div class=\"table-row hover-row\"> \
			<div class=\"table-element addr-elem-id\">" + AddrModetemplate.ID + "</div> \
			<div class=\"table-element addr-elem-mode\">(<p style=\"color: rgb(140, 140, 140);\">0x</p><p style=\"color: darkred; font-weight: bold;\">" + AddrModetemplate.Mode + "</p>)</div> \
			<div class=\"table-element addr-elem-operands\">" + AddrModetemplate.Operands + "</div> \
			<div class=\"table-element addr-elem-sdesc\">" + AddrModetemplate.ShortDescription + "</div> \
		</div> \
		<div class=\"table-row desc-row\"> \
			<div class=\"table-element\">" + AddrModetemplate.Description + "</div> \
		</div></div> \
	";
	var flags_template = "\
		<div class=\"hover-row-container\"><div class=\"table-row hover-row\"> \
			<div class=\"table-element flg-elem-id\">" + FlagsTemplate.ID + "</div> \
			<div class=\"table-element flg-elem-inst\">" + FlagsTemplate.Instruction + "</div> \
			<div class=\"table-element flg-elem-flag\">(<p style=\"color: rgb(140, 140, 140);\">0b</p>" + FlagsTemplate.Flags + ")</div> \
			<div class=\"table-element flg-elem-hex\">" + FlagsTemplate.Hex + "</div> \
			<div class=\"table-element flg-elem-sdesc\">" + FlagsTemplate.ShortDescription + "</div> \
		</div> \
		<div class=\"table-row desc-row\"> \
			<div class=\"table-element\">" + FlagsTemplate.Description + "</div> \
		</div></div> \
	";
	class InstOperand {
		 constructor(sop = IOps.NoOperand) {
			this.opString = sop;
			this.add = function (opType) {
				if (this.opString === IOps.NoOperand)
					this.opString = "";
				if (this.opString !== "")
					this.opString += "/";
				this.opString += opType;
				return this;
			};
			this.get = function () {
				return this.opString;
			};
		}
	}

	var instruction_table_size = 0;
	var addressing_mode_table_size = 0;
	var flags_table_size = 0;

// ----------------------------------------------------------------------------------------




// ------------------------------------------Functions------------------------------------------
	function createInstructionRow(ID, instruction, op_code, addressing, flags, op1, op2, description, extra)
	{
		if (instruction.trim().endsWith("_m"))
			instruction = "<p style=\"color: darkgray;\">" + instruction + "</p>"
		else if (instruction.trim().endsWith("_b"))
			instruction = "<p style=\"color: rgb(180, 140, 110);\">" + instruction + "</p>"
		var inst_row = inst_template.replaceAll(InstructionTemplate.ID, ID);
		inst_row = inst_row.replaceAll(InstructionTemplate.Instruction, instruction);
		inst_row = inst_row.replaceAll(InstructionTemplate.OpCode, "<p style=\"color: rgb(140, 140, 140);\">0x</p><p style=\"color: #b85300;\">" + op_code + "</p>");
		inst_row = inst_row.replaceAll(InstructionTemplate.AddressingMode, addressing);
		inst_row = inst_row.replaceAll(InstructionTemplate.Flags, flags);
		inst_row = inst_row.replaceAll(InstructionTemplate.Operator1, op1.get());
		inst_row = inst_row.replaceAll(InstructionTemplate.Operator2, op2.get());
		inst_row = inst_row.replaceAll(InstructionTemplate.Description, description);
		inst_row = inst_row.replaceAll(InstructionTemplate.Extra, extra);
		return inst_row;
	}

	function addInstruction(instruction, op_code, addressing, flags, op1, op2, description, extra = IOps.NoOperand)
	{
		$("div#instruction-table").append(createInstructionRow(++instruction_table_size, instruction, op_code, addressing, flags, op1, op2, description, extra));
	}

	function createFlagsRow(ID, flags, hex, inst, sdesc, description)
	{
		var _flags = "";
		flags = flags.replaceAll(" ", "");
		for (var i = 0; i < flags.length; i++)
		{
			if (flags.charAt(i) === '0')
				_flags += "<p style=\"background-color: rgb(250, 200, 200); color: rgb(120, 30, 30);\">0</p>";
			else if (flags.charAt(i) === '1')
				_flags += "<p style=\"background-color: rgb(200, 250, 200); color: rgb(30, 120, 30);\">1</p>";
			else if (flags.charAt(i) === '*')
				_flags += "<p style=\"background-color: rgb(200, 200, 250); color: rgb(30, 30, 120);\">*</p>";
			if ((i + 1) % 8 == 0) _flags += " ";
		}
		_flags = _flags.trim();	
		var flg_row = flags_template.replaceAll(FlagsTemplate.ID, ID);
		flg_row = flg_row.replaceAll(FlagsTemplate.Flags, _flags);
		flg_row = flg_row.replaceAll(FlagsTemplate.Hex, "<p style=\"color: rgb(140, 140, 140);\">0x</p><p style=\"color: #b85300;\">" + hex + "</p>");
		flg_row = flg_row.replaceAll(FlagsTemplate.Instruction, inst);
		flg_row = flg_row.replaceAll(FlagsTemplate.ShortDescription, sdesc);
		flg_row = flg_row.replaceAll(FlagsTemplate.Description, description);
		return flg_row;
	}

	function addFlags(flags, hex, inst, sdesc, description)
	{
		$("div#flags-table").append(createFlagsRow(++flags_table_size, flags, hex, inst, sdesc, description));
	}

	function createAddrModeRow(ID, mode, ops, shortDesc, description)
	{
		var addr_row = addr_mode_template.replaceAll(AddrModetemplate.ID, ID);
		addr_row = addr_row.replaceAll(AddrModetemplate.Mode, mode);
		addr_row = addr_row.replaceAll(AddrModetemplate.Operands, ops);
		addr_row = addr_row.replaceAll(AddrModetemplate.ShortDescription, shortDesc);
		addr_row = addr_row.replaceAll(AddrModetemplate.Description, description);
		return addr_row;
	}

	function addAddressingMode(mode, ops_list, shortDesc, description)
	{
		var ops = "(";
		for (var i = 0; i < ops_list.length; i++)
			ops += ops_list[i].get() + ", ";
		ops = ops.slice(0, -2) + ")";
		$("div#addressing-modes-table").append(createAddrModeRow(++addressing_mode_table_size, mode, ops, shortDesc, description));
	}

	function addSeparator(table) {
		$("div#" + table).append("<div class=\"separator-row\"></div>");
	}
// -----------------------------------------------------------------------------------------------




// ------------------------------------------Code------------------------------------------
	addInstruction("no_op", "F0F0", IOps.NoOperand, IOps.NoOperand, 
		new InstOperand(),
		new InstOperand(),
	"No operation");

	addInstruction("req", "F0F1", IOps.NoOperand, "0x****", 
		new InstOperand().add(IOps.ConstVal),
		new InstOperand(),
	"request instruction - the 2 least significant bits of <flags> are used to choose from: stack/heap/local");

	addInstruction("end", "F0F2", "0x****", IOps.NoOperand, 
		new InstOperand().add(IOps.ConstVal).add(IOps.Addr).add(IOps.Reg),
		new InstOperand(),
	"end instruction");

	addInstruction("reserve", "F0F3", "0x****", IOps.NoOperand,
		new InstOperand().add(IOps.Addr).add(IOps.Reg),
		new InstOperand(),
	"end instruction");

	addInstruction("free_s", "F0F4", "0x****", IOps.NoOperand, 
		new InstOperand().add(IOps.ConstVal).add(IOps.Addr).add(IOps.Reg),
		new InstOperand(),
	"free instruction");

	addInstruction("alloc", "F0F5", "0x****", IOps.NoOperand, 
		new InstOperand().add(IOps.ConstVal).add(IOps.Addr).add(IOps.Reg),
		new InstOperand(),
	"alloc instruction");

	addInstruction("free", "F0F6", "0x****", IOps.NoOperand, 
		new InstOperand().add(IOps.ConstVal).add(IOps.Addr).add(IOps.Reg),
		new InstOperand(),
	"free instruction");

	addInstruction("realloc", "F0F7", "0x****", IOps.NoOperand, 
		new InstOperand().add(IOps.Addr).add(IOps.Reg),
		new InstOperand().add(IOps.ConstVal).add(IOps.Addr).add(IOps.Reg),
	"realloc instruction");

	addInstruction("cmd", "F0F8", "0x****", IOps.NoOperand, 
		new InstOperand().add(IOps.ConstVal).add(IOps.Addr).add(IOps.Reg),
		new InstOperand().add(IOps.ConstVal).add(IOps.Addr).add(IOps.Reg),
	"cmd instruction");

	addInstruction("flg", "F0F9", "0x****", IOps.NoOperand, 
		new InstOperand().add(IOps.ConstVal).add(IOps.Addr).add(IOps.Reg),
		new InstOperand(),
	"flg instruction");

	addInstruction("flg_m", "F0FA", "0x****", IOps.NoOperand, 
		new InstOperand().add(IOps.ConstVal).add(IOps.Addr).add(IOps.Reg),
		new InstOperand(),
	"flg instruction");

	addSeparator("instruction-table");

	addInstruction("mem", "1000", "0x****", IOps.NoOperand, 
		new InstOperand().add(IOps.Addr).add(IOps.Reg),
		new InstOperand().add(IOps.ConstVal).add(IOps.Addr).add(IOps.Reg),
	"mem instruction");

	addInstruction("mem_m", "1001", "0x****", IOps.NoOperand, 
		new InstOperand().add(IOps.Addr).add(IOps.Reg),
		new InstOperand().add(IOps.ConstVal).add(IOps.Addr).add(IOps.Reg),
	"mem instruction");

	addInstruction("push", "1002", "0x****", IOps.NoOperand, 
		new InstOperand().add(IOps.ConstVal).add(IOps.Addr).add(IOps.Reg),
		new InstOperand(),
	"push instruction");

	addInstruction("push_m", "1003", "0x****", IOps.NoOperand, 
		new InstOperand().add(IOps.ConstVal).add(IOps.Addr).add(IOps.Reg),
		new InstOperand(),
	"push instruction");
	
	addInstruction("pop", "1004", "0x****", IOps.NoOperand, 
		new InstOperand().add(IOps.Addr).add(IOps.Reg),
		new InstOperand(),
	"pop instruction");
	
	addInstruction("pop_m", "1005", "0x****", IOps.NoOperand, 
		new InstOperand().add(IOps.Addr).add(IOps.Reg),
		new InstOperand(),
	"pop instruction");
	
	addInstruction("pop_r", "1006", "0x****", IOps.NoOperand, 
		new InstOperand().add(IOps.Addr).add(IOps.Reg),
		new InstOperand(),
	"pop instruction");
	
	addInstruction("pop_r_m", "1007", "0x****", IOps.NoOperand, 
		new InstOperand().add(IOps.Addr).add(IOps.Reg),
		new InstOperand(),
	"pop instruction");

	addInstruction("lda_str", "1008", IOps.NoOperand, IOps.NoOperand, 
		new InstOperand().add(IOps.ConstStream),
		new InstOperand("<p class=\"value\">***</p>"),
	"lda_str instruction", "<p class=\"value\">***</p>");

	addInstruction("str_cpy", "1009", IOps.NoOperand, IOps.NoOperand, 
		new InstOperand().add(IOps.Addr).add(IOps.Reg),
		new InstOperand().add(IOps.Addr).add(IOps.Reg),
	"str_cpy instruction");

	addInstruction("add_str", "100A", "0x****", "0x****", 
		new InstOperand().add(IOps.Addr).add(IOps.Reg),
		new InstOperand().add(IOps.ConstStream).add(IOps.Addr).add(IOps.Reg),
	"add_str instruction");

	addSeparator("instruction-table");

	addInstruction("inc", "1100", "0x****", IOps.NoOperand, 
		new InstOperand().add(IOps.Addr).add(IOps.Reg),
		new InstOperand(),
	"inc instruction");

	addInstruction("inc_m", "1101", "0x****", IOps.NoOperand, 
		new InstOperand().add(IOps.Addr).add(IOps.Reg),
		new InstOperand(),
	"inc instruction");

	addInstruction("dec", "1102", "0x****", IOps.NoOperand, 
		new InstOperand().add(IOps.Addr).add(IOps.Reg),
		new InstOperand(),
	"dec instruction");

	addInstruction("dec_m", "1103", "0x****", IOps.NoOperand, 
		new InstOperand().add(IOps.Addr).add(IOps.Reg),
		new InstOperand(),
	"dec instruction");

	addInstruction("add", "1104", "0x****", IOps.NoOperand, 
		new InstOperand().add(IOps.Addr).add(IOps.Reg),
		new InstOperand().add(IOps.ConstVal).add(IOps.Addr).add(IOps.Reg),
	"add instruction");

	addInstruction("add_m", "1105", "0x****", IOps.NoOperand, 
		new InstOperand().add(IOps.Addr).add(IOps.Reg),
		new InstOperand().add(IOps.ConstVal).add(IOps.Addr).add(IOps.Reg),
	"add instruction");

	addInstruction("sub", "1106", "0x****", IOps.NoOperand, 
		new InstOperand().add(IOps.Addr).add(IOps.Reg),
		new InstOperand().add(IOps.ConstVal).add(IOps.Addr).add(IOps.Reg),
	"sub instruction");

	addInstruction("sub_m", "1107", "0x****", IOps.NoOperand, 
		new InstOperand().add(IOps.Addr).add(IOps.Reg),
		new InstOperand().add(IOps.ConstVal).add(IOps.Addr).add(IOps.Reg),
	"sub instruction");

	addInstruction("mul", "1108", "0x****", IOps.NoOperand, 
		new InstOperand().add(IOps.Addr).add(IOps.Reg),
		new InstOperand().add(IOps.ConstVal).add(IOps.Addr).add(IOps.Reg),
	"mul instruction");

	addInstruction("mul_m", "1109", "0x****", IOps.NoOperand, 
		new InstOperand().add(IOps.Addr).add(IOps.Reg),
		new InstOperand().add(IOps.ConstVal).add(IOps.Addr).add(IOps.Reg),
	"mul instruction");

	addInstruction("div", "110A", "0x****", IOps.NoOperand, 
		new InstOperand().add(IOps.Addr).add(IOps.Reg),
		new InstOperand().add(IOps.ConstVal).add(IOps.Addr).add(IOps.Reg),
	"div instruction");

	addInstruction("div_m", "110B", "0x****", IOps.NoOperand, 
		new InstOperand().add(IOps.Addr).add(IOps.Reg),
		new InstOperand().add(IOps.ConstVal).add(IOps.Addr).add(IOps.Reg),
	"div instruction");

	addInstruction("cmp", "110C", "0x****", IOps.NoOperand, 
		new InstOperand().add(IOps.ConstVal).add(IOps.Addr).add(IOps.Reg),
		new InstOperand().add(IOps.ConstVal).add(IOps.Addr).add(IOps.Reg),
	"cmp instruction");

	addInstruction("cmp_m", "110D", "0x****", IOps.NoOperand, 
		new InstOperand().add(IOps.ConstVal).add(IOps.Addr).add(IOps.Reg),
		new InstOperand().add(IOps.ConstVal).add(IOps.Addr).add(IOps.Reg),
	"cmp instruction");

	addSeparator("instruction-table");

	addInstruction("jmp", "1200", "0x****", "0x****", 
		new InstOperand().add(IOps.ConstVal).add(IOps.Addr).add(IOps.Reg),
		new InstOperand(),
	"jmp instruction - least significant 3 bits of <flags> used to select the type of jump");

	addInstruction("call", "1201", "0x****", IOps.NoOperand, 
		new InstOperand().add(IOps.ConstVal).add(IOps.Addr).add(IOps.Reg),
		new InstOperand().add(IOps.ConstVal).add(IOps.Addr).add(IOps.Reg),
	"call instruction - first operand is code addr, second operand is param count <br /> Following, a list of all parameters with addressing mode for each. <br /> VirtualCPU Flags: variable_inst_size", 
		"<p style=\"color: orange; font-weight: bold;\">***</p>");

	addInstruction("ret", "1202", "0x****", IOps.NoOperand, 
		new InstOperand().add(IOps.ConstVal).add(IOps.Addr).add(IOps.Reg),
		new InstOperand(),
	"ret instruction");

	addInstruction("ret_m", "1203", "0x****", IOps.NoOperand, 
		new InstOperand().add(IOps.ConstVal).add(IOps.Addr).add(IOps.Reg),
		new InstOperand(),
	"ret instruction");

	addSeparator("instruction-table");

	addInstruction("and", "1400", "0x****", IOps.NoOperand, 
		new InstOperand().add(IOps.Addr).add(IOps.Reg),
		new InstOperand().add(IOps.ConstVal).add(IOps.Addr).add(IOps.Reg),
	"and instruction");

	addInstruction("and_m", "1401", "0x****", IOps.NoOperand, 
		new InstOperand().add(IOps.Addr).add(IOps.Reg),
		new InstOperand().add(IOps.ConstVal).add(IOps.Addr).add(IOps.Reg),
	"and instruction");

	addInstruction("or", "1402", "0x****", IOps.NoOperand, 
		new InstOperand().add(IOps.Addr).add(IOps.Reg),
		new InstOperand().add(IOps.ConstVal).add(IOps.Addr).add(IOps.Reg),
	"or instruction");

	addInstruction("or_m", "1403", "0x****", IOps.NoOperand, 
		new InstOperand().add(IOps.Addr).add(IOps.Reg),
		new InstOperand().add(IOps.ConstVal).add(IOps.Addr).add(IOps.Reg),
	"or instruction");

	addInstruction("not", "1404", "0x****", IOps.NoOperand, 
		new InstOperand().add(IOps.Addr).add(IOps.Reg),
		new InstOperand(),
	"not instruction");

	addInstruction("not_m", "1405", "0x****", IOps.NoOperand, 
		new InstOperand().add(IOps.Addr).add(IOps.Reg),
		new InstOperand(),
	"not instruction");

	addInstruction("bit", "1406", "0x****", IOps.NoOperand, 
		new InstOperand().add(IOps.Addr).add(IOps.Reg),
		new InstOperand().add(IOps.ConstVal).add(IOps.Addr).add(IOps.Reg),
	"bit instruction<br />-op2's MSB used to specify which bit to set (0-15)<br />-op2's LSB used to specify which bit to set (0=false, 1=true)");
	
	addInstruction("mask", "1407", "0x****", IOps.NoOperand, 
		new InstOperand().add(IOps.Addr).add(IOps.Reg),
		new InstOperand().add(IOps.ConstVal).add(IOps.Addr).add(IOps.Reg),
	"mask instruction<br />-op2 used as bit mask");



	addAddressingMode("0000",
		[new InstOperand(IOps.Addr), new InstOperand(IOps.ConstVal)],
	"const_val to addr", "Test Long Desc");
	addAddressingMode("0001",
		[new InstOperand(IOps.IAddr), new InstOperand(IOps.ConstVal)],
	"const_val to mem pointed by addr content", "Test Long Desc");
	addAddressingMode("0002",
		[new InstOperand(IOps.Reg), new InstOperand(IOps.ConstVal)],
	"const_val to reg", "Test Long Desc");
	addAddressingMode("0003",
		[new InstOperand(IOps.IReg), new InstOperand(IOps.ConstVal)],
	"const_val to mem pointed by reg content", "Test Long Desc");
	
	addSeparator("addressing-modes-table");

	addAddressingMode("0004",
		[new InstOperand(IOps.Addr), new InstOperand(IOps.Addr)],
	"addr2 content to addr1", "Test Long Desc");
	addAddressingMode("0005",
		[new InstOperand(IOps.IAddr), new InstOperand(IOps.Addr)],
	"addr2 content to mem pointed by addr1 content", "Test Long Desc");
	addAddressingMode("0006",
		[new InstOperand(IOps.Reg), new InstOperand(IOps.Addr)],
	"addr content to reg", "Test Long Desc");
	addAddressingMode("0007",
		[new InstOperand(IOps.IReg), new InstOperand(IOps.Addr)],
	"addr content to mem pointed by reg content", "Test Long Desc");
	
	addSeparator("addressing-modes-table");

	addAddressingMode("0008",
		[new InstOperand(IOps.Addr), new InstOperand(IOps.IAddr)],
	"mem pointed by addr2 content to addr1", "Test Long Desc");
	addAddressingMode("0009",
		[new InstOperand(IOps.IAddr), new InstOperand(IOps.IAddr)],
	"mem pointed by addr2 content to mem pointed by addr1 content", "Test Long Desc");
	addAddressingMode("000A",
		[new InstOperand(IOps.Reg), new InstOperand(IOps.IAddr)],
	"mem pointed by addr content to reg", "Test Long Desc");
	addAddressingMode("000B",
		[new InstOperand(IOps.IReg), new InstOperand(IOps.IAddr)],
	"mem pointed by addr content to mem pointed by reg content", "Test Long Desc");
	
	addSeparator("addressing-modes-table");

	addAddressingMode("000C",
		[new InstOperand(IOps.Addr), new InstOperand(IOps.Ref)],
	"addr2 value to addr1", "Test Long Desc");
	addAddressingMode("000D",
		[new InstOperand(IOps.IAddr), new InstOperand(IOps.Ref)],
	"addr2 value to mem pointed by addr1 content", "Test Long Desc");
	addAddressingMode("000E",
		[new InstOperand(IOps.Reg), new InstOperand(IOps.Ref)],
	"addr value to reg", "Test Long Desc");
	addAddressingMode("000F",
		[new InstOperand(IOps.IReg), new InstOperand(IOps.Ref)],
	"addr value to mem pointed by reg content", "Test Long Desc");
	
	addSeparator("addressing-modes-table");

	addAddressingMode("0010",
		[new InstOperand(IOps.Addr), new InstOperand(IOps.IReg)],
	"mem pointed by reg content to addr1", "Test Long Desc");
	addAddressingMode("0011",
		[new InstOperand(IOps.IAddr), new InstOperand(IOps.IReg)],
	"mem pointed by reg content to mem pointed by addr1 content", "Test Long Desc");
	addAddressingMode("0012",
		[new InstOperand(IOps.Reg), new InstOperand(IOps.IReg)],
	"mem pointed by reg content to reg", "Test Long Desc");
	addAddressingMode("0013",
		[new InstOperand(IOps.IReg), new InstOperand(IOps.IReg)],
	"mem pointed by reg content to mem pointed by reg content", "Test Long Desc");
	
	addSeparator("addressing-modes-table");

	addAddressingMode("0014",
		[new InstOperand(IOps.Addr), new InstOperand(IOps.Reg)],
	"reg content to addr", "Test Long Desc");
	addAddressingMode("0015",
		[new InstOperand(IOps.IAddr), new InstOperand(IOps.Reg)],
	"reg content to mem pointed by addr1 content", "Test Long Desc");
	addAddressingMode("0016",
		[new InstOperand(IOps.Reg), new InstOperand(IOps.Reg)],
	"reg2 content to reg1", "Test Long Desc");
	addAddressingMode("0017",
		[new InstOperand(IOps.IReg), new InstOperand(IOps.Reg)],
	"mem pointed by reg2 content to reg1", "Test Long Desc");
	
	addSeparator("addressing-modes-table");

	addAddressingMode("0018",
		[new InstOperand(IOps.ConstVal)],
	"Treat value as constant", "Test Long Desc");
	addAddressingMode("0019",
		[new InstOperand(IOps.Addr)],
	"Take addr content", "Test Long Desc");
	addAddressingMode("001A",
		[new InstOperand(IOps.Reg)],
	"Take reg content", "Test Long Desc");
	addAddressingMode("001B",
		[new InstOperand(IOps.IAddr)],
	"Take mem pointed by addr content", "Test Long Desc");
	addAddressingMode("001C",
		[new InstOperand(IOps.IReg)],
	"Take mem pointed by reg content", "Test Long Desc");
	addAddressingMode("001D",
		[new InstOperand(IOps.Ref)],
	"Take addr value", "Test Long Desc");

	addSeparator("addressing-modes-table");

	addAddressingMode("001E",
	[new InstOperand(IOps.ConstVal), new InstOperand(IOps.ConstVal)],
	"const_val op1 and const_val op2", "Test Long Desc");
	addAddressingMode("001F",
		[new InstOperand(IOps.ConstVal), new InstOperand(IOps.Addr)],
	"const_val op1 and addr op2", "Test Long Desc");
	addAddressingMode("0020",
		[new InstOperand(IOps.ConstVal), new InstOperand(IOps.IAddr)],
	"const_val op1 and ptr op2", "Test Long Desc");
	addAddressingMode("0021",
		[new InstOperand(IOps.ConstVal), new InstOperand(IOps.Ref)],
	"const_val op1 and ref op2", "Test Long Desc");
	addAddressingMode("0022",
		[new InstOperand(IOps.ConstVal), new InstOperand(IOps.IReg)],
	"const_val op1 and regPtr op2", "Test Long Desc");
	addAddressingMode("0023",
		[new InstOperand(IOps.ConstVal), new InstOperand(IOps.Reg)],
	"const_val op1 and reg op2", "Test Long Desc");
	
	addSeparator("addressing-modes-table");

	addAddressingMode("0***",
		[new InstOperand()],
	"Normal 2-byte mode", "For *_m intructions only <br />(LSB = Least significant Byte) <br />(MSB = Most significant Byte)");
	addAddressingMode("1***",
		[new InstOperand()],
	"op1's LSB, op2's LSB", "For *_m intructions only <br />(LSB = Least significant Byte) <br />(MSB = Most significant Byte)");
	addAddressingMode("2***",
		[new InstOperand()],
	"op1's LSB, op2's MSB", "For *_m intructions only <br />(LSB = Least significant Byte) <br />(MSB = Most significant Byte)");
	addAddressingMode("3***",
		[new InstOperand()],
	"op1's MSB, op2's LSB", "For *_m intructions only <br />(LSB = Least significant Byte) <br />(MSB = Most significant Byte)");
	addAddressingMode("4***",
		[new InstOperand()],
	"op1's MSB, op2's MSB", "For *_m intructions only <br />(LSB = Least significant Byte) <br />(MSB = Most significant Byte)");
	addAddressingMode("5***",
		[new InstOperand()],
	"Single op LSB", "For *_m intructions only <br />(LSB = Least significant Byte) <br />(MSB = Most significant Byte)");
	addAddressingMode("6***",
		[new InstOperand()],
	"Single op MSB", "For *_m intructions only <br />(LSB = Least significant Byte) <br />(MSB = Most significant Byte)");
	addAddressingMode("7***",
		[new InstOperand()],
	"op1 2-Byte, op2's LSB", "For *_m intructions only <br />(LSB = Least significant Byte) <br />(MSB = Most significant Byte)");
	addAddressingMode("8***",
		[new InstOperand()],
	"op1 2-Byte, op2's MSB", "For *_m intructions only <br />(LSB = Least significant Byte) <br />(MSB = Most significant Byte)");
	addAddressingMode("9***",
		[new InstOperand()],
	"op1's LSB, op2 2-Byte", "For *_m intructions only <br />(LSB = Least significant Byte) <br />(MSB = Most significant Byte)");
	addAddressingMode("A***",
		[new InstOperand()],
	"op1's MSB, op2 2-Byte", "For *_m intructions only <br />(LSB = Least significant Byte) <br />(MSB = Most significant Byte)");




	addFlags("00000000 00000011", "0007", "req", "Request All", "Test long desc");
	addFlags("00000000 00000001", "0001", "req", "Request stack", "Test long desc");
	addFlags("00000000 00000010", "0002", "req", "Request Heap", "Test long desc");
	addSeparator("flags-table");
	addFlags("00000000 00111111", "0000", "jmp", "Unconditional Jump", "Test long desc");
	addFlags("00000000 00000001", "0001", "jmp", "Jump if greater", "Test long desc");
	addFlags("00000000 00000010", "0002", "jmp", "Jump if less", "Test long desc");
	addFlags("00000000 00000100", "0004", "jmp", "Jump if equals", "Test long desc");
	addFlags("00000000 00001000", "0008", "jmp", "Jump if not equals", "Test long desc");
	addFlags("00000000 00010000", "000F", "jmp", "Jump if greater or equals", "Test long desc");
	addFlags("00000000 00100000", "0020", "jmp", "Jump if less or equals", "Test long desc");
	addSeparator("flags-table");
	addFlags("00000000 00000001", "0001", "add_str", "const_stream Op2", "Test long desc");
	addFlags("00000000 00000010", "0002", "add_str", "String pointer Op2", "Test long desc");
	addFlags("00000000 00000100", "0004", "add_str", "integer op2", "Test long desc");
	addFlags("00000000 00001000", "0008", "add_str", "single character (integer) op2", "Test long desc");


	$("div.table-container").hide();
	$("div#instruction-table").show();
// ----------------------------------------------------------------------------------------




// ------------------------------------------Events------------------------------------------
	var sel_row = null, down_row = null;
	const table_col = "rgb(253, 245, 230)";
	const row_col_h = "rgb(200, 245, 230)";
	$("div.hover-row-container").attr("marked", "false");

	$("div.hover-row-container").mouseover(function(evt){
			$(this).children("div.hover-row").css("background-color", row_col_h);
		sel_row = this;
	});
	$("div.hover-row-container").mouseout(function(evt){
		if (down_row !== this)
			$(this).children("div.hover-row").css("background-color", "transparent");
		sel_row = null;
	});
	$("div.hover-row-container").mousedown(function(evt){
		if (evt.which === 2)
		{
			if ($(this).attr("marked") === "true")
			{
				$(this).attr("marked", "false");
				$(this).css("background-color", "transparent");
			}
			else
			{
				$(this).attr("marked", "true");
				$(this).css("background-color", "#DD5555");
			}
			return;
		}
		var table = $(this).parent();
		if (down_row === sel_row)
		{
			$(this).children("div.desc-row").css("visibility", "hidden");
			$(this).children("div.desc-row").css("height", "0px");
			$(this).children("div.hover-row").css("box-shadow", "none");
			table.children("div.hover-row-container").css("filter", "blur(0px)");
			table.children("div.hover-row-container").css("opacity", "100%");
			table.children("div.hover-row-container").css("transform", "scale(1)");
			table.children("div.hover-row-container").css("border", "none");
			table.css("background-color", table_col);
			down_row = null;

		}
		else
		{
			if (down_row !== null)
			{
				$(down_row).children("div.desc-row").css("visibility", "hidden");
				$(down_row).children("div.desc-row").css("height", "0px");
				$(down_row).children("div.desc-row").css("box-shadow", "none");
				$(down_row).children("div.hover-row").css("background-color", "transparent");
				$(down_row).children("div.hover-row").css("box-shadow", "none");
				$(down_row).css("filter", "blur(2px)");
				$(down_row).css("opacity", "60%");
				$(down_row).css("transform", "scale(1)");
				$(down_row).css("border", "none");
			}
			
			var other_tables_row = $("div.table-container").not("#" + table.attr("id")).children("div.hover-row-container");
	
			other_tables_row.css("filter", "blur(0px)");
			other_tables_row.css("opacity", "100%");
			other_tables_row.css("transform", "scale(1)");
			other_tables_row.css("border", "none");
			other_tables_row.parent().css("background-color", table_col);

			table.children("div.hover-row-container").css("filter", "blur(2px)");
			table.children("div.hover-row-container").css("opacity", "60%");
			table.css("background-color", "rgb(160, 160, 160)");
			$(this).css("filter", "blur(0px)");
			$(this).css("opacity", "100%");
			$(this).css("transform", "scale(1.04)");
			$(this).css("border", "1px solid black");
			$(this).children("div.desc-row").css("visibility", "visible");
			$(this).children("div.desc-row").css("height", "auto");
			$(this).children("div.desc-row").css("box-shadow", "0px 2px 12px rgb(100, 100, 100)");
			$(this).children("div.desc-row").css("clip-path", "inset(0px 0px -12px 0px)");
			$(this).children("div.hover-row").css("box-shadow", "0px -4px 16px rgb(20, 20, 20)");
			$(this).children("div.hover-row").css("clip-path", "inset(-20px 0px 0px 0px)");
			down_row = this;
		}
	});

	$("div#inst-set-btn").mousedown(function(evt) {
		$("div.table-container").hide();
		$("div#instruction-table").show();
	});

	$("div#addr-mode-btn").mousedown(function(evt) {
		$("div.table-container").hide();
		$("div#addressing-modes-table").show();
	});

	$("div#flg-btn").mousedown(function(evt) {
		$("div.table-container").hide();
		$("div#flags-table").show();
	});

	$("div#reg-layout-btn").mousedown(function(evt) {
		$("div.table-container").hide();
		$("div#register-layout-table").show();
	});
// ------------------------------------------------------------------------------------------
});