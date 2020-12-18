using System;
using System.Collections.Generic;
using System.Linq;
using System.Runtime.InteropServices;
using System.Text;

namespace rpcCSharp
{
	[UnmanagedFunctionPointer(CallingConvention.Cdecl)]
	public delegate string RequestCallback(string content);

	public class TrpcSDK
	{
#if __linux__
[DllImport("trpcdylib.so", EntryPoint = "StartServerListen", CallingConvention = CallingConvention.Cdecl)]
#else
		[DllImport("trpcdylib.dll", EntryPoint = "StartServerListen", CallingConvention = CallingConvention.Cdecl)]
#endif
		static extern public IntPtr StartServerListen(TrpcServerInit trpcServerInit);
#if __linux__
		[DllImport("trpcdylib.so", EntryPoint = "ClientSendAndReceive", CallingConvention = CallingConvention.Cdecl)]
#else
		[DllImport("trpcdylib.dll", EntryPoint = "ClientSendAndReceive", CallingConvention = CallingConvention.Cdecl)]
#endif
		static extern public string ClientSendAndReceive(IntPtr pRpc, TrpcEpSet epSet,string msg);

		[DllImport("trpcdylib.dll", EntryPoint = "_RpcOpen", CallingConvention = CallingConvention.Cdecl)]
		static extern public IntPtr _RpcOpen(_SRpcInit rpcInit);

		[DllImport("trpcdylib.dll", EntryPoint = "_RpcClose", CallingConvention = CallingConvention.Cdecl)]
		static extern public void _RpcClose(IntPtr param);		

		[DllImport("trpcdylib.dll", EntryPoint = "InitLog", CallingConvention = CallingConvention.Cdecl)]
		static extern public int InitLog(string logName, int numOfLogLines, int maxFiles);

		[DllImport("trpcdylib.dll", EntryPoint = "SetDebug", CallingConvention = CallingConvention.Cdecl)]
		static extern public void SetDebug(int rpcDebugFlag);

		[DllImport("trpcdylib.dll", EntryPoint = "CloseLog", CallingConvention = CallingConvention.Cdecl)]
		static extern public void CloseLog();

		[DllImport("trpcdylib.dll", EntryPoint = "ResetLog", CallingConvention = CallingConvention.Cdecl)]
		static extern public void ResetLog();

		[DllImport("trpcdylib.dll", EntryPoint = "SetCompressMsgSize", CallingConvention = CallingConvention.Cdecl)]
		static extern public void SetCompressMsgSize(int CompressMsgSize);
	}

	[StructLayout(LayoutKind.Sequential)]
	public struct TrpcServerInit
	{		
		[MarshalAs(UnmanagedType.U1)]
		public bool commit;
		[MarshalAs(UnmanagedType.FunctionPtr)]
		public RequestCallback requestcbk;
		[MarshalAs(UnmanagedType.ByValTStr, SizeConst = 20)]
		public string dataFile;
		[MarshalAs(UnmanagedType.Struct)]
		public _SRpcInit rpcInit;
	}

	[StructLayout(LayoutKind.Sequential)]
	public struct TrpcEpSet
	{
		[MarshalAs(UnmanagedType.U1)]
		public sbyte inUse;		
		[MarshalAs(UnmanagedType.U1)]
		public sbyte numOfEps;
		[MarshalAs(UnmanagedType.ByValArray, SizeConst = 5, ArraySubType = UnmanagedType.U2)]
		public ushort[] port;
		[MarshalAs(UnmanagedType.ByValArray, SizeConst = 5, ArraySubType = UnmanagedType.BStr)]
		public IntPtr[] fqdn;		
	}

	[StructLayout(LayoutKind.Sequential)]
	public struct _SRpcInit
	{
		[MarshalAs(UnmanagedType.U2)]
		public ushort localPort;
		[MarshalAs(UnmanagedType.ByValTStr, SizeConst = 8)]
		public string label;
		[MarshalAs(UnmanagedType.U4)]
		public int numOfThreads;
		[MarshalAs(UnmanagedType.U4)]
		public int sessions;
		[MarshalAs(UnmanagedType.U1)]
		public byte connType;
		[MarshalAs(UnmanagedType.U4)]
		public int idleTime;
		[MarshalAs(UnmanagedType.ByValTStr, SizeConst = 24)]
		public string user;
		[MarshalAs(UnmanagedType.U1)]
		public char spi;
		[MarshalAs(UnmanagedType.U1)]
		public char encrypt;
		[MarshalAs(UnmanagedType.ByValTStr, SizeConst = 16)]
		public string secret;
		[MarshalAs(UnmanagedType.ByValTStr, SizeConst = 16)]
		public string ckey;
	}
}
