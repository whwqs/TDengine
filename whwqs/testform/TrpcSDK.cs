using System;
using System.Collections.Generic;
using System.Linq;
using System.Runtime.InteropServices;
using System.Text;

namespace testform
{
	[UnmanagedFunctionPointer(CallingConvention.Cdecl)]
	public delegate string RequestCallback(string content);

	public class TrpcSDK
	{
		[DllImport("trpcdll.dll", EntryPoint = "StartServerListen", CallingConvention = CallingConvention.Cdecl)]
		static extern public IntPtr StartServerListen(TrpcServerInit trpcServerInit);

		[DllImport("trpcdll.dll", EntryPoint = "ClientSendAndReceive", CallingConvention = CallingConvention.Cdecl)]
		static extern public string ClientSendAndReceive(IntPtr pRpc, TrpcEpSet epSet,string msg);

		[DllImport("trpcdll.dll", EntryPoint = "_RpcOpen", CallingConvention = CallingConvention.Cdecl)]
		static extern public IntPtr _RpcOpen(_SRpcInit rpcInit);

		[DllImport("trpcdll.dll", EntryPoint = "_RpcClose", CallingConvention = CallingConvention.Cdecl)]
		static extern public void _RpcClose(IntPtr param, bool bServer);		

		[DllImport("trpcdll.dll", EntryPoint = "InitLog", CallingConvention = CallingConvention.Cdecl)]
		static extern public int InitLog(string logName, int numOfLogLines, int maxFiles);

		[DllImport("trpcdll.dll", EntryPoint = "SetDebug", CallingConvention = CallingConvention.Cdecl)]
		static extern public void SetDebug(int rpcDebugFlag);

		[DllImport("trpcdll.dll", EntryPoint = "CloseLog", CallingConvention = CallingConvention.Cdecl)]
		static extern public void CloseLog();

		[DllImport("trpcdll.dll", EntryPoint = "ResetLog", CallingConvention = CallingConvention.Cdecl)]
		static extern public void ResetLog();

		[DllImport("trpcdll.dll", EntryPoint = "SetCompressMsgSize", CallingConvention = CallingConvention.Cdecl)]
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
