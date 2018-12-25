using System;
using System.Collections.Generic;
using System.Runtime.InteropServices;

public abstract class PInvoke
{
    public const string LIB_PATH = "../build/scout-api/libscout-api.so";
}

public partial class ControlFlowGraph : PInvoke
{
    [DllImport(LIB_PATH, EntryPoint = "cfg_construct")]
    private static extern IntPtr CfgConstruct(string fileName);

    [DllImport(LIB_PATH, EntryPoint = "cfg_destruct")]
    private static extern void CfgDestruct(IntPtr cfg);

    [DllImport(LIB_PATH, EntryPoint = "cfg_get_root")]
    private static extern IntPtr CfgGetRoot(IntPtr cfg);
}

public partial class Block : PInvoke
{
    [DllImport(LIB_PATH, EntryPoint = "cfg_block_count_successors")]
    private static extern int CfgBlockCountSuccessors(IntPtr cfgBlock);
    [DllImport(LIB_PATH, EntryPoint = "cfg_block_get_successor")]
    private static extern IntPtr CfgBlockGetSuccessor(IntPtr cfgBlock, int index);

    [DllImport(LIB_PATH, EntryPoint = "cfg_block_count_instructions")]
    private static extern int CfgBlockCountInstructions(IntPtr cfgBlock);
    [DllImport(LIB_PATH, EntryPoint = "cfg_block_get_instruction")]
    private static extern void CfgBlockGetInstruction(IntPtr cfgBlock, int index, out Instruction instruction);
}

public partial struct Instruction
{
#pragma warning disable 169, 649

    private uint _id;

    private ulong _address;

    private ushort _size;
    [MarshalAs(UnmanagedType.ByValArray, SizeConst = 16)]
    private byte[] _bytes;

    [MarshalAs(UnmanagedType.ByValTStr, SizeConst = 32)]
    private string _mnemonic;

    [MarshalAs(UnmanagedType.ByValTStr, SizeConst = 160)]
    private string _opStr;

    private IntPtr _detail;

#pragma warning restore 169, 649
}
